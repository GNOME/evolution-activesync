
import QtQuick 1.0
import MeeGo.Components 0.1

AppPage {
    id: asContainer

    //: The title of the ActiveSync UI displayed to the user.
    pageTitle: qsTr("ActiveSync Settings")

    ActiveSyncDialog {
        id: asDialog
    }

    // Temporary hack until ActiveSync settings is given a proper design and home in MeeGo Settings.
    Button {
        text: qsTr("Configure")

        onClicked: asDialog.show()
    }
}
