import QtQuick
import QtQuick.Controls

// Preserve the dense editors on narrow screens without scaling their touch targets.
ScrollView {
    id: root
    property real minimumEditorWidth: 760
    property real minimumEditorHeight: 560
    property Component editor
    clip: true
    contentWidth: Math.max(availableWidth, minimumEditorWidth)
    contentHeight: Math.max(availableHeight, minimumEditorHeight)
    ScrollBar.horizontal.policy: contentWidth > availableWidth + 1 ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: contentHeight > availableHeight + 1 ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff

    function applyScrollPolicy() {
        const fl = contentItem
        if (!fl || typeof fl.cancelFlick !== "function")
            return
        fl.boundsBehavior = Flickable.StopAtBounds
        if (fl.boundsMovement !== undefined)
            fl.boundsMovement = Flickable.StopAtBounds
        fl.interactive = contentWidth > availableWidth + 1
                || contentHeight > availableHeight + 1
    }

    Component.onCompleted: applyScrollPolicy()
    onContentWidthChanged: applyScrollPolicy()
    onContentHeightChanged: applyScrollPolicy()
    onAvailableWidthChanged: applyScrollPolicy()
    onAvailableHeightChanged: applyScrollPolicy()

    Loader {
        width: root.contentWidth
        height: root.contentHeight
        sourceComponent: root.editor
    }
}
