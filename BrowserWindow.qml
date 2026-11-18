import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import BrowserBackend 1.0

Window {
    id: browserWin
    width: 540
    height: 400
    visible: true

    // --- PROPERTIES ---
    property int myWindowId: 1
    property var masterModel: null
    property var appController: null

    // Tracks the active tab using its MASTER index in TabModel
    property int activeMasterIndex: -1

    // Tracks which master tab is currently being dragged (for dimming)
    property int draggedMasterIndex: -1

    readonly property int otherWindowId: (myWindowId === 1) ? 2 : 1
    readonly property int tabWidth: 130
    readonly property int tabSpacing: 4
    readonly property int leftMargin: 6

    // Role IDs matching TabModel::TabRoles enum (Qt.UserRole = 256)
    readonly property int roleTitle: 257
    readonly property int roleColor: 258
    readonly property int roleContent: 259
    readonly property int roleOwnerWindow: 260
    readonly property int roleOrder: 261

    title: "Browser - Window " + myWindowId

    // --- C++ FILTER & SORT PROXY ---
    TabFilterProxy {
        id: localProxy
        sourceModel: browserWin.masterModel
        windowId: browserWin.myWindowId
    }

    function selectFirstAvailableTab() {
        if (localProxy.rowCount() > 0) {
            activeMasterIndex = localProxy.mapToMasterIndex(0)
        } else {
            activeMasterIndex = -1
        }
    }

    // Helper: Given an X coordinate in tabBar, which proxy slot (0, 1, 2...) is under it?
    function proxySlotAtX(mouseX) {
        if (localProxy.rowCount() === 0) return -1
        var slotWidth = tabWidth + tabSpacing
        var slot = Math.floor((mouseX - leftMargin) / slotWidth)
        if (slot < 0 || slot >= localProxy.rowCount()) return -1
        return slot
    }

    // Listen for changes in the master C++ model
    Connections {
        target: browserWin.masterModel

        function onDataChanged(topLeft, bottomRight, roles) {
            var changedMasterIdx = topLeft.row

            // ONLY change active tab selection when a tab moves between windows (roleOwnerWindow = 260)
            // Ignore OrderRole (261) changes so reordering doesn't hijack your active tab!
            if (roles.indexOf(browserWin.roleOwnerWindow) !== -1) {
                var proxyRow = localProxy.mapFromMasterIndex(changedMasterIdx)

                if (proxyRow >= 0) {
                    // Tab arrived in THIS window -> select it
                    browserWin.activeMasterIndex = changedMasterIdx
                } else if (browserWin.activeMasterIndex === changedMasterIdx) {
                    // Active tab left THIS window -> select another local tab
                    browserWin.selectFirstAvailableTab()
                }
            }
        }
    }

    Component.onCompleted: selectFirstAvailableTab()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ==========================================
        // 1. TOP TAB BAR
        // ==========================================
        Rectangle {
            id: tabBar
            Layout.fillWidth: true
            height: 42
            color: "#dfe1e5"

            Row {
                anchors.fill: parent
                anchors.leftMargin: browserWin.leftMargin
                anchors.topMargin: 6
                spacing: browserWin.tabSpacing

                Repeater {
                    id: tabRepeater
                    model: localProxy

                    delegate: Rectangle {
                        id: tabDelegate
                        width: browserWin.tabWidth
                        height: 36
                        radius: 6

                        readonly property int myMasterIndex: localProxy.mapToMasterIndex(index)
                        readonly property bool isActive: (myMasterIndex === browserWin.activeMasterIndex)
                        readonly property bool isBeingDragged: (myMasterIndex === browserWin.draggedMasterIndex)

                        color: isActive ? "#ffffff" : "#c8ccd1"
                        // Hide the real tab entirely while dragging, revealing only the ghost
                        opacity: tabMouseArea.isDragging ? 0.0 : 1.0

                        Text {
                            anchors.centerIn: parent
                            text: model.tabTitle
                            font.bold: isActive
                            elide: Text.ElideRight
                            width: parent.width - 16
                            horizontalAlignment: Text.AlignHCenter
                        }

                        MouseArea {
                            id: tabMouseArea
                            anchors.fill: parent

                            property bool isDragging: false
                            property point pressPos: Qt.point(0, 0)

                            onPressed: {
                                browserWin.activeMasterIndex = tabDelegate.myMasterIndex
                                pressPos = Qt.point(mouse.x, mouse.y)
                                isDragging = false
                            }

                            onPositionChanged: {
                                if (!pressed || !browserWin.masterModel) return

                                // Initiate drag
                                if (!isDragging) {
                                    if (Math.abs(mouse.x - pressPos.x) > 6 || Math.abs(mouse.y - pressPos.y) > 6) {
                                        isDragging = true
                                        browserWin.appController.ghostTab.tabTitle = model.tabTitle
                                        browserWin.appController.ghostTab.visible = true
                                    }
                                }

                                // Update ghost position globally
                                if (isDragging && browserWin.appController) {
                                    var globalPos = tabMouseArea.mapToGlobal(mouse.x, mouse.y)
                                    browserWin.appController.ghostTab.x = globalPos.x - 65
                                    browserWin.appController.ghostTab.y = globalPos.y - 18
                                }
                            }

                            onReleased: {
                                if (isDragging && browserWin.appController) {
                                    var globalPos = tabMouseArea.mapToGlobal(mouse.x, mouse.y)
                                    // Submit the final drop coordinates to C++ once
                                    browserWin.appController.handleTabDrop(tabDelegate.myMasterIndex, globalPos.x, globalPos.y)
                                }
                                isDragging = false
                            }
                        }
                    }
                }
            }
        }

        // ==========================================
        // 2. MAIN PAGE CONTENT AREA
        // ==========================================
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            property int activeProxyRow: (browserWin.activeMasterIndex >= 0)
                                         ? localProxy.mapFromMasterIndex(browserWin.activeMasterIndex)
                                         : -1

            function getActiveRole(roleId) {
                if (activeProxyRow < 0) return ""
                return localProxy.data(localProxy.index(activeProxyRow, 0), roleId)
            }

            color: activeProxyRow >= 0 ? getActiveRole(browserWin.roleColor) : "#f0f0f0"

            Column {
                anchors.centerIn: parent
                spacing: 14

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: parent.parent.activeProxyRow >= 0
                          ? parent.parent.getActiveRole(browserWin.roleTitle)
                          : "No Tabs Open"
                    font.pixelSize: 28
                    font.bold: true
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: parent.parent.activeProxyRow >= 0
                          ? parent.parent.getActiveRole(browserWin.roleContent)
                          : "Drag a tab here to view it"
                    font.pixelSize: 16
                    color: "#555555"
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: parent.parent.activeProxyRow >= 0
                    text: "Move Tab to Window " + browserWin.otherWindowId
                    onClicked: {
                        // -1 tells C++ to append it to the end of the target window
                        browserWin.masterModel.moveTab(
                                    browserWin.activeMasterIndex, browserWin.otherWindowId, -1)
                    }
                }
            }
        }
    }
}