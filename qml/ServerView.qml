import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    property alias qrImage: qrCodeImage.source
    property alias serverUrl: urlText.text

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
                width: Math.min(parent.width - 100, 600)
                spacing: 30

                // Status Indicator
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: statusRow.width + 40
                    Layout.preferredHeight: 56
                    color: "#1a1a1a"
                    border.color: "#10b981"
                    border.width: 2
                    radius: 12

                    RowLayout {
                        id: statusRow
                        anchors.centerIn: parent
                        spacing: 12

                        Image {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            source: "qrc:/icons/check_circle.svg"
                            sourceSize: Qt.size(24, 24)
                            smooth: true
                        }

                        Text {
                            text: "Server Running"
                            font.family: "Segoe UI"
                            font.pixelSize: 20
                            font.weight: Font.DemiBold
                            font.letterSpacing: -0.3
                            color: "#10b981"
                        }
                    }
                }

                Item { Layout.preferredHeight: 10 }

                // QR Code Container
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 400
                    Layout.preferredHeight: 400
                    color: "#ffffff"
                    radius: 16
                    border.color: "#374151"
                    border.width: 1

                    Image {
                        id: qrCodeImage
                        anchors.centerIn: parent
                        width: parent.width - 60
                        height: parent.height - 60
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                        antialiasing: false
                    }
                }

                Item { Layout.preferredHeight: 10 }

                // URL Display
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: urlLayout.height + 40
                    color: "#1a1a1a"
                    border.color: "#374151"
                    border.width: 1
                    radius: 12

                    ColumnLayout {
                        id: urlLayout
                        anchors.centerIn: parent
                        width: parent.width - 60
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Image {
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                source: "qrc:/icons/public.svg"
                                sourceSize: Qt.size(24, 24)
                                smooth: true
                            }

                            Text {
                                id: urlText
                                Layout.fillWidth: true
                                text: ""
                                font.family: "Segoe UI"
                                font.pixelSize: 20
                                font.weight: Font.Medium
                                font.letterSpacing: -0.2
                                color: "#d4d4d8"
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#374151"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 20

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Image {
                                    Layout.preferredWidth: 20
                                    Layout.preferredHeight: 20
                                    source: "qrc:/icons/public.svg"
                                    sourceSize: Qt.size(20, 20)
                                    smooth: true
                                }

                                Text {
                                    text: "Web Interface: Port 80"
                                    font.family: "Segoe UI"
                                    font.pixelSize: 14
                                    font.weight: Font.Normal
                                    color: "#9ca3af"
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Image {
                                    Layout.preferredWidth: 20
                                    Layout.preferredHeight: 20
                                    source: "qrc:/icons/folder.svg"
                                    sourceSize: Qt.size(20, 20)
                                    smooth: true
                                }

                                Text {
                                    text: "File Transfer: Port 8080"
                                    font.family: "Segoe UI"
                                    font.pixelSize: 14
                                    font.weight: Font.Normal
                                    color: "#9ca3af"
                                }
                            }
                        }
                    }
                }

                // Info Card
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: infoLayout.height + 40
                    color: "#1a1a1a"
                    border.color: "#374151"
                    border.width: 1
                    radius: 12

                    RowLayout {
                        id: infoLayout
                        anchors.centerIn: parent
                        width: parent.width - 60
                        spacing: 16

                        Image {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            Layout.alignment: Qt.AlignTop
                            source: "qrc:/icons/phone_android.svg"
                            sourceSize: Qt.size(32, 32)
                            smooth: true
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "Scan the QR code or visit the URL from any device on your network"
                            font.family: "Segoe UI"
                            font.pixelSize: 16
                            font.weight: Font.Normal
                            font.letterSpacing: -0.15
                            color: "#9ca3af"
                            wrapMode: Text.WordWrap
                        }
                    }
                }

                Item { Layout.preferredHeight: 20 }
            }
        }
    }
}

