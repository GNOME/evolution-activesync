import QtQuick 1.0
import MeeGo.Components 0.1


Item {
    anchors.fill: parent
//    spacing: 10

    property alias emailAddress: emailField.text
    property alias username: usernameField.text
    property alias password: passwordField.text
    property alias serverURL: serverUrlEntry.text

    property bool acceptableInput: emailField.textInput.acceptableInput

    Theme {
        id: theme
    }

    TextEntry {
        id: emailField
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 10

        width: parent.width - 20

        //: E-mail address example text.  Note: do not translate "example.com"!
        property string example: qsTr("(ex: foo@example.com)")

        //: Sync account username (e.g. foo.bar@yahoo.com) login field label, where arg 1 is an example, which may or not be visible.
        defaultText: qsTr("E-mail address %1").arg(example)
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

        onFocusChanged: {
            // This triggers an attempt to pull existing ActiveSync configuration values
            // corresponding to this e-mail address from GConf.
            //
            // @todo Make this an explicit function call.  No behind the scenes magic!
//            if (emailField.text != "" && emailField.textInput.acceptableInput)
//                config.emailAddress = usernameField.text;
        }
    }

    TextEntry {
        id: usernameField
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: emailField.bottom
        anchors.topMargin: 10

        width: emailField.width

        //: ActiveSync account username.
        defaultText: qsTr("Username")
        inputMethodHints: Qt.ImhNoAutoUppercase

        Keys.onTabPressed: {
            passwordField.textInput.focus = true;
        }
    }

    TextEntry {
        // We shouldn't ask for the password in advance; SSO might work, and the dæmon will
        // request it if it needs it.   -DW

        id: passwordField
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: usernameField.bottom
        anchors.topMargin: 10
        width: emailField.width

        //: ActiveSync account password login field label
        defaultText: qsTr("Password")
        inputMethodHints: Qt.ImhNoAutoUppercase

        textInput.echoMode: TextInput.Password

        Keys.onTabPressed: {
            serverUrlField.textInput.focus = true;
        }
    }

    Item {
        // We should do autodiscover here to find the URL.  -DW

        id: serverUrlField
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: passwordField.bottom
        anchors.topMargin: 10
        height: childrenRect.height
        width: emailField.width
//        focus: serverUrlEntry.focus  // Allow for "tabbing".

        property string text/*: serverUrlEntry.text*/

        TextEntry {
            id: serverUrlEntry
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width

            text: serverUrlField.text

            //: Sync account password login field label
            defaultText: qsTr("Server URL ")
            inputMethodHints: Qt.ImhNoAutoUppercase

            Keys.onTabPressed: {
                emailField.textInput.focus = true;
            }
        }
        Text {
            property string sampleURL: "https://foo.example.com/Microsoft-Server-ActiveSync"
            anchors.top:  serverUrlEntry.bottom
            anchors.left:  serverUrlEntry.left
            text: qsTr("Enter the ActiveSync server URL\ne.g. %1").arg(sampleURL)
            font.pixelSize: theme.fontPixelSizeSmall
        }
    }
}
