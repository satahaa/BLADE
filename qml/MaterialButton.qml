import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    property string iconSource: ""
    property bool isPrimary: true

    font.family: "Segoe UI"
    font.pixelSize: 16
    font.weight: Font.DemiBold
    font.letterSpacing: -0.15

    contentItem: Row {
        spacing: 12
        anchors.centerIn: parent

        Image {
            visible: iconSource !== ""
            anchors.verticalCenter: parent.verticalCenter
            source: iconSource
            sourceSize: Qt.size(20, 20)
            smooth: true
            width: 20
            height: 20
        }

        Text {
            text: control.text
            font: control.font
            color: "#ffffff"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        color: {
            if (!control.enabled) return "#6b7280"
            if (control.pressed) return isPrimary ? "#1f2937" : "#111111"
            if (control.hovered) return isPrimary ? "#374151" : "#1a1a1a"
            return isPrimary ? "#4b5563" : "#0a0a0a"
        }
        border.color: isPrimary ? "transparent" : "#374151"
        border.width: isPrimary ? 0 : 1
        radius: 8

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onPressed: mouse.accepted = false
    }
}

