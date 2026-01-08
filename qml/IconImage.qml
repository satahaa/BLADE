import QtQuick 2.15

Image {
    id: root

    smooth: true
    antialiasing: true

    // For SVG icons, use sourceSize to control the rendering size
    property size sourceSize: Qt.size(width, height)
}

