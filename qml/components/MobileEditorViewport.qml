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
    ScrollBar.horizontal.policy: contentWidth > availableWidth ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
    Loader {
        width: root.contentWidth
        height: root.contentHeight
        sourceComponent: root.editor
    }
}
