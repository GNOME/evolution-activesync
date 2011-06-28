/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

import QtQuick 1.0
import MeeGo.Components 0.1
import MeeGo.ActiveSync 0.1

ModalDialog {
    id: dialog

    property string emailAddress
    property string username: config.username
    property string password: config.password
    property string serverURL: config.serverURL

    showAcceptButton: true
    showCancelButton: true

    //: "Sign in" button text displayed in ActiveSync account login dialog.
    acceptButtonText: qsTr("Sign in")
    //: "Cancel" button text displayed in ActiveSync account login dialog.
    cancelButtonText: qsTr("Cancel")
    //: The argument is the name of the remote sync service (e.g. Google, Yahoo!, etc).
    title: qsTr("Configure your ActiveSync account")

    sizeHintWidth: 600
    sizeHintHeight: 475

    ActiveSyncConfig {
        id: config
    }

    content: Column {

        anchors.centerIn: parent
        width: 500
        spacing: 10

        TextEntry {
            id: emailField
            anchors.horizontalCenter: parent.horizontalCenter

            width: parent.width - 10

            //: E-mail address example text.  Note: do not translate "example.com"!
            property string example: qsTr("(ex: foo@example.com)")

            //: Sync account username (e.g. foo.bar@yahoo.com) login field label, where arg 1 is an example, which may or not be visible.
            defaultText: qsTr("E-mail address %1").arg(example)
            text: config.emailAddress
            inputMethodHints: Qt.ImhEmailCharactersOnly | Qt.ImhNoAutoUppercase

            // Set up an e-mail address validator
            //
            // THIS REGULAR EXPRESSION DOES NOT COVER SOME RARE E-MAIL ADDRESS
            // CORNER CASES.
            //
            textInput.validator: RegExpValidator {
//                        regExp: /^([a-zA-Z0-9_\.\-\+])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/
                regExp: /^[a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+.[a-zA-Z]{2,4}$/
            }

            Keys.onTabPressed: {
                usernameField.textInput.focus = true;
            }
            Keys.onReturnPressed: {
                accepted();  // Simulate pressing the sign-in button.
            }

            onFocusChanged: {
                // This triggers an attempt to pull existing ActiveSync configuration values
                // corresponding to this e-mail address from GConf.
                //
                // @todo Make this an explicit function call.  No behind the scenes magic!
                if (emailField.text != "" && emailField.textInput.acceptableInput)
                    config.emailAddress = usernameField.text;
            }
        }

        TextEntry {
            id: usernameField
            anchors.horizontalCenter: parent.horizontalCenter

            width: emailField.width

            //: ActiveSync account username.
            defaultText: qsTr("Username")
            text: config.username
            inputMethodHints: Qt.ImhNoAutoUppercase

            Keys.onTabPressed: {
                passwordField.textInput.focus = true;
            }
            Keys.onReturnPressed: {
                accepted();  // Simulate pressing the sign-in button.
            }
        }

        TextEntry {
            // We shouldn't ask for the password in advance; SSO might work, and the dæmon will
            // request it if it needs it.   -DW

            id: passwordField
            anchors.horizontalCenter: parent.horizontalCenter
            width: emailField.width

            //: ActiveSync account password login field label
            defaultText: qsTr("Password")
            text: config.password
            inputMethodHints: Qt.ImhNoAutoUppercase

            textInput.echoMode: TextInput.Password

            Keys.onTabPressed: {
                serverUrlField.textInput.focus = true;
            }
            Keys.onReturnPressed: {
                accepted();  // Simulate pressing the sign-in button.
            }
        }

        Item {
            // We should do autodiscover here to find the URL.  -DW

            id: serverUrlField
            anchors.horizontalCenter: parent.horizontalCenter
            height: childrenRect.height
            width: emailField.width
            focus: serverUrlEntry.focus  // Allow for "tabbing".

            property alias text: serverUrlEntry.text

            TextEntry {
                id: serverUrlEntry
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width

                //: Sync account password login field label
                defaultText: qsTr("Server URL ")
                text: serverURL
                inputMethodHints: Qt.ImhNoAutoUppercase

                Keys.onTabPressed: {
                    emailField.textInput.focus = true;
                }
                Keys.onReturnPressed: {
                    accepted();  // Simulate pressing the sign-in button.
                }
            }
            Text {
                anchors.top:  serverUrlEntry.bottom
                anchors.left:  serverUrlEntry.left
                text: qsTr("Enter the ActiveSync server URL\ne.g. http://foo.example.com/ActiveSyncServer")
                font.pixelSize: theme.fontPixelSizeSmall
            }
        }
    }

    onAccepted: {
        if (emailField.text != "" && usernameField.text != "") {
            if (emailField.textInput.acceptableInput) {

                // Set the ActiveSync GConf keys.
                config.emailAddress = emailField.text
                config.writeConfig(usernameField.text, passwordField.text, serverUrlField.text)

                // Close the Dialog.
                //container.destroy();
                dialog.hide();
            } else {
                // Display the sample e-mail again.
                emailField.text = ""
            }
        }
    }

    onRejected: {
        popPage();
    }

    Component.onCompleted: {
        dialog.show()
    }
}

