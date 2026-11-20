import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import BrowserBackend 1.0

QtObject {
    id : appRoot

    // QtObject doesnt have visual children.
    // So they are declared as properties

    // THE C++ MASTER MODEL (Single Source of Truth)
    property TabModel globalTabModel: TabModel {}

    // FLOATING GHOST TAB (Used when dragging across windows)
    property GhostTab ghostTab: GhostTab {}

    // Check if a global (x,y) point falls inside a window
    function isPointInsideWindow(win, globalX, globalY) {
            if (!win || !win.visible) return false
            return (globalX >= win.x && globalX <= (win.x + win.width) &&
                    globalY >= win.y && globalY <= (win.y + win.height))
        }

    // Replace finishCrossWindowDrag with this:
    function handleTabDrop(masterIndex, globalX, globalY) {
        ghostTab.visible = false

        var targetWin = null
        if (isPointInsideWindow(windowOne, globalX, globalY)) targetWin = windowOne
        else if (isPointInsideWindow(windowTwo, globalX, globalY)) targetWin = windowTwo

        if (targetWin) {
            // Map global pos to the target window's tab bar
            var localX = globalX - targetWin.x - 6
            var slotWidth = 130 + 4 // tabWidth + spacing

            // Adding half a slot width allows dropping into the closest gap naturally
            var dropIndex = Math.floor(Math.max(0, localX + (slotWidth / 2)) / slotWidth)

            globalTabModel.moveTab(masterIndex, targetWin.myWindowId, dropIndex)
        }
    }

    // Called by BrowserWindow when the user releases a dragged tab
    // function finishCrossWindowDrag(masterIndex, globalX, globalY) {
    //     ghostTab.visible = false

    //     if (isPointInsideWindow(windowOne, globalX, globalY)) {
    //         globalTabModel.moveTabToWindow(masterIndex, 1)
    //     } else if (isPointInsideWindow(windowTwo, globalX, globalY)) {
    //         globalTabModel.moveTabToWindow(masterIndex, 2)
    //     }
    // }
    
    function finishTabDrag(masterIndex, globalX, globalY){
        ghostTab.visible  = false

        if (isPointInsideWindow(windowOne, globalX, globalY)) {
            globalTabModel.setProperty(masterIndex, "ownerWindow", 1)
        } else if (isPointInsideWindow(windowTwo, globalX, globalY)) {
            globalTabModel.setProperty(masterIndex, "ownerWindow", 2)
        }
    }



    // 2. Window 1
    property BrowserWindow windowOne: BrowserWindow{
        myWindowId : 1
        masterModel: appRoot.globalTabModel
        appController: appRoot
        x: 100
        y: 150
    }

    // 3. WINDOW 2
    property BrowserWindow windowTwo: BrowserWindow {
        myWindowId: 2
        masterModel: appRoot.globalTabModel
        appController: appRoot
        x: 650
        y: 150
    }
}


