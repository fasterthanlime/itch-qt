import QtQuick 2.10
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Window 2.10
import Qt.itch.butlerd 1.0

ApplicationWindow {
    visible: true
    width: 1280
    height: 720
    color: "#2e2b2c"
    title: qsTr("itch")

    Connections {
        target: ButlerdConnection
        onGotProfiles: {
            console.log("Got profiles: ", profileNames)
            usernameField.text = "Remembered profiles: " + profileNames
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("Account")
            MenuItem {
                text: qsTr("&Logout")
                onTriggered: console.log("Wtf")
            }
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit()
            }
        }
    }

    ColumnLayout {
        id: columnLayout
        x: 400
        y: 145
        width: 480
        height: 430
        spacing: 5

        Image {
            id: image
            width: 100
            height: 100
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            transformOrigin: Item.Center
            source: "../Dev/itch/src/static/images/logos/app-white.svg"
        }

        Text {
            id: usernameLabel
            color: "#fefdfd"
            text: qsTr("Username or e-mail")
            font.pixelSize: 16
        }

        TextField {
            id: usernameField
            height: 30
            font.pointSize: 16
            Layout.fillWidth: true
            placeholderText: qsTr("")
        }

        Text {
            id: passwordLabel
            color: "#ffffff"
            text: qsTr("Password")
            font.pixelSize: 16
        }

        TextField {
            id: passwordField
            height: 30
            font.pointSize: 16
            echoMode: 1
            inputMask: ""
            Layout.fillWidth: true
            placeholderText: qsTr("")
        }

        Button {
            id: loginButton
            x: 0
            text: qsTr("Log in to itch.io")
            Layout.preferredWidth: -1
            isDefault: false
            iconSource: ""
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            onClicked: ButlerdConnection.connect(usernameField.text, passwordField.text)
        }

    }

}


