import QtQuick 2.15
import QtQuick.Window 2.15

// A tiny borderless floating window that follows the mouse during a drag
Window {
    id: ghostWin
    width: 130
    height: 32
    visible: false
    color: "transparent"

    // Frameless + AlwaysOnTop + ignores mouse clicks so it doesn't block drop detection
    flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput

    property string tabTitle: ""

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: "#ffffff"
        border.color: "#1a73e8"
        border.width: 2
        opacity: 0.9

        Text {
            anchors.centerIn: parent
            text: ghostWin.tabTitle
            font.bold: true
            elide: Text.ElideRight
            width: parent.width - 16
            horizontalAlignment: Text.AlignHCenter
        }
    }
}