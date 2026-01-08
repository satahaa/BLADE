import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Window {
    id: mainWindow
    width: 800
    height: 900
    visible: true
    title: "BLADE - Bi-Directional LAN Asset Distribution Engine"
    color: "#0a0a0a"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: loginViewComponent

        Component {
            id: loginViewComponent
            LoginView {
                objectName: "loginView"
            }
        }

        Component {
            id: serverViewComponent
            ServerView {
                objectName: "serverView"
            }
        }
    }

    // Function to switch to server view
    function showServerView(url) {
        console.log("Switching to server view with URL:", url)
        stackView.push(serverViewComponent)

        // Set properties on the new view after a brief delay to ensure it's loaded
        Qt.callLater(function() {
            var currentItem = stackView.currentItem
            if (currentItem) {
                currentItem.serverUrl = url
            }
        })
    }

    // Function to update QR code image
    function setQRImage(imagePath) {
        console.log("Setting QR image:", imagePath)
        Qt.callLater(function() {
            var currentItem = stackView.currentItem
            if (currentItem && currentItem.objectName === "serverView") {
                currentItem.qrImage = "file:///" + imagePath
            }
        })
    }
}

