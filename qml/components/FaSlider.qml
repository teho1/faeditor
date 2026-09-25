import QtQuick
import QtQuick.Controls
import "FlickableGuard.js" as FlickableGuard

Slider {
    id: root
    // Grab immediately so a parent Flickable does not take the first pixels of a drag.
    touchDragThreshold: 0

    property var _lockedFlickables: []

    onPressedChanged: {
        if (pressed)
            root._lockedFlickables = FlickableGuard.lockFrom(root)
        else {
            FlickableGuard.unlock(root._lockedFlickables)
            root._lockedFlickables = []
        }
    }
}
