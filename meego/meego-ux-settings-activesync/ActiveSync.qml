
import QtQuick 1.0
import MeeGo.Components 0.1
import MeeGo.ActiveSync 0.1


AppPage {
    id: asContainer

    //: The title of the ActiveSync UI displayed to the user.
    pageTitle: qsTr("ActiveSync Settings")

    Theme {
        id: theme
    }

    ActiveSyncConfigModel {
        id: configModel
    }

    Column {
        id: configsColumn
        anchors.margins: 10
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Repeater {
            id: configs

            model: configModel
            delegate: ExpandingBox {
                height: 80
                width: configsColumn.width
                titleText: email
                detailsComponent: Item {
                    height: childrenRect.height
                    width: parent.width
                    anchors.margins: 10

                    Text {
                        id: usernameLabel

                        font.pixelSize: theme.fontPixelSizeNormal

                        text: qsTr("Username: %1").arg(model.username)
                    }

                    Text {
                        id: serverUrlLabel

                        anchors.top: usernameLabel.bottom
                        anchors.topMargin: 10

                        font.pixelSize: theme.fontPixelSizeNormal

                        text: qsTr("Server URL: %1").arg(model.serverUrl)
                    }

                    Button {
                        height: 60
                        width: 200
                        text: qsTr("Remove Account")
                        anchors.top: serverUrlLabel.bottom
                        anchors.topMargin: 10

                        bgSourceUp: "image://themedimage/widgets/common/button/button-negative"
                        bgSourceDn: "image://themedimage/widgets/common/button/button-negative-pressed"

                        onClicked: {
                            confirmRemoval.accountIndex = index
                            confirmRemoval.show()
                        }
                    }
                }
            }
        }


        ModalMessageBox {
            id: confirmRemoval

            property int accountIndex: -1

            title: qsTr("ActiveSync account removal")
            text:  qsTr("Are you sure want to remove this ActiveSync account?")

            acceptButtonText: qsTr("Remove")
            acceptButtonImage: "image://themedimage/widgets/common/button/button-negative"
            acceptButtonImagePressed: "image://themedimage/widgets/common/button/button-negative-pressed"

            buttonMinHeight: 60
            buttonMinWidth: 120

            onAccepted: {
                // Remove the ActiveSync account.
                configModel.removeConfig(accountIndex)
            }
        }

        Button {
            id: addConfig
            text: qsTr("Add ActiveSync Configuration")

            minHeight: 60
            minWidth: 120

            onClicked: {
                asDialog.show()
            }
        }

    }

    ActiveSyncDialog {
        id: asDialog
        model: configs.model
    }

}
