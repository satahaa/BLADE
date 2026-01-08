import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0a0a0a" }
            GradientStop { position: 1.0; color: "#1a1a1a" }
        }

        Flickable {
            anchors.fill: parent
            contentHeight: contentColumn.height + 100
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: contentColumn
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 60
                width: Math.min(parent.width - 100, 500)
                spacing: 30

                // Logo
                Item {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 120

                    Image {
                        anchors.fill: parent
                        source: "../blade.ico"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        antialiasing: true
                        mipmap: true
                    }
                }

                // Title Section
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "BLADE"
                        font.family: "Segoe UI"
                        font.pixelSize: 56
                        font.weight: Font.DemiBold
                        font.letterSpacing: -1.5
                        color: "#d4d4d8"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Local Network File Transfer"
                        font.family: "Segoe UI"
                        font.pixelSize: 16
                        font.weight: Font.Normal
                        font.letterSpacing: -0.2
                        color: "#9ca3af"
                    }
                }

                Item { Layout.preferredHeight: 20 }

                // Form Container
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: formLayout.height + 80
                    color: "#1a1a1a"
                    border.color: "#374151"
                    border.width: 1
                    radius: 12

                    ColumnLayout {
                        id: formLayout
                        anchors.centerIn: parent
                        width: parent.width - 80
                        spacing: 20

                        // Username Field
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: "Username (Optional)"
                                font.family: "Segoe UI"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: "#9ca3af"
                                font.letterSpacing: -0.15
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 48
                                color: "#0a0a0a"
                                border.color: usernameField.activeFocus ? "#4b5563" : "#374151"
                                border.width: 1
                                radius: 6

                                Behavior on border.color {
                                    ColorAnimation { duration: 200 }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Image {
                                        Layout.preferredWidth: 20
                                        Layout.preferredHeight: 20
                                        source: "qrc:/icons/person.svg"
                                        sourceSize: Qt.size(20, 20)
                                        smooth: true
                                    }

                                    TextInput {
                                        id: usernameField
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        verticalAlignment: TextInput.AlignVCenter
                                        font.family: "Segoe UI"
                                        font.pixelSize: 16
                                        color: "#e5e7eb"
                                        selectByMouse: true
                                        selectionColor: "#4b5563"
                                        clip: true

                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            text: "Enter username"
                                            font: parent.font
                                            color: "#6b7280"
                                            visible: !parent.text && !parent.activeFocus
                                        }
                                    }
                                }
                            }
                        }

                        // Password Field
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: "Password (Optional)"
                                font.family: "Segoe UI"
                                font.pixelSize: 14
                                font.weight: Font.Medium
                                color: "#9ca3af"
                                font.letterSpacing: -0.15
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 48
                                color: "#0a0a0a"
                                border.color: passwordField.activeFocus ? "#4b5563" : "#374151"
                                border.width: 1
                                radius: 6

                                Behavior on border.color {
                                    ColorAnimation { duration: 200 }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 10

                                    Image {
                                        Layout.preferredWidth: 20
                                        Layout.preferredHeight: 20
                                        source: "qrc:/icons/lock.svg"
                                        sourceSize: Qt.size(20, 20)
                                        smooth: true
                                    }

                                    TextInput {
                                        id: passwordField
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        verticalAlignment: TextInput.AlignVCenter
                                        font.family: "Segoe UI"
                                        font.pixelSize: 16
                                        color: "#e5e7eb"
                                        selectByMouse: true
                                        selectionColor: "#4b5563"
                                        echoMode: showPassword ? TextInput.Normal : TextInput.Password
                                        clip: true

                                        property bool showPassword: false

                                        Text {
                                            anchors.fill: parent
                                            verticalAlignment: Text.AlignVCenter
                                            text: "Enter password"
                                            font: parent.font
                                            color: "#6b7280"
                                            visible: !parent.text && !parent.activeFocus
                                        }
                                    }

                                    MouseArea {
                                        Layout.preferredWidth: 24
                                        Layout.preferredHeight: 24
                                        cursorShape: Qt.PointingHandCursor

                                        onClicked: passwordField.showPassword = !passwordField.showPassword

                                        IconImage {
                                            anchors.fill: parent
                                            source: passwordField.showPassword ? "qrc:/icons/visibility_off.svg" : "qrc:/icons/visibility.svg"
                                            sourceSize: Qt.size(24, 24)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item { Layout.preferredHeight: 10 }

                // Buttons
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width
                    spacing: 15

                    // Start with Authentication Button
                    MaterialButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        text: "Start with Authentication"
                        iconSource: "qrc:/icons/lock.svg"
                        isPrimary: true

                        onClicked: {
                            var username = usernameField.text.trim()
                            var password = passwordField.text.trim()
                            applicationBridge.startWithAuth(username, password)
                        }
                    }

                    // Start without Authentication Button
                    MaterialButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        text: "Start without Authentication"
                        iconSource: "qrc:/icons/rocket_launch.svg"
                        isPrimary: false

                        onClicked: {
                            applicationBridge.startNoAuth()
                        }
                    }
                }

                Item { Layout.preferredHeight: 20 }
            }
        }
    }
}

