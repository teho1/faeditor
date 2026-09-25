import QtQuick
import QtQuick.Controls

// Scroll only when content is larger than the viewport — no rubber-band when it fits.
Flickable {
    id: root
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    boundsMovement: Flickable.StopAtBounds
    // Incremented while a fader/slider is down so we do not break `interactive`.
    property int pointerLocks: 0
    interactive: pointerLocks === 0 && (
        flickableDirection === Flickable.HorizontalFlick
            ? contentWidth > width + 1
            : flickableDirection === Flickable.VerticalFlick
                ? contentHeight > height + 1
                : contentWidth > width + 1 || contentHeight > height + 1)
}
