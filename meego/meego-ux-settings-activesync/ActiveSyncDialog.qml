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

    property alias emailAddress: fields.emailAddress
    property alias username: fields.username
    property alias password: fields.password
    property alias serverURL: fields.serverURL

    property variant model

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

    buttonMinHeight: 70
    buttonMinWidth: 140

    ConfigFields {
        id: fields

        serverURL: "https://m.domain.com/Microsoft-Server-ActiveSync"
    }

    content: fields

    onAccepted: {
        if (emailAddress != "" && username != "") {
            if (fields.acceptableInput) {

                // Set the ActiveSync GConf keys.
                model.appendConfig(emailAddress, username, password, serverURL);

                // Close the Dialog.
                //container.destroy();
                dialog.hide();
            } else {
                // Display the sample e-mail again.
                emailAddress = ""
            }
        }
    }

}

