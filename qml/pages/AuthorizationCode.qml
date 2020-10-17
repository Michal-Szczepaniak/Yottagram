import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    Connections {
        target: authorization
        onWaitingForPassword: {
            pageStack.replace(Qt.resolvedUrl("AuthorizationPassword.qml"));
        }
        onIsAuthorizedChanged: {
            if (page.status === PageStatus.Active)
                pageStack.replace(Qt.resolvedUrl("ChatList.qml"))
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("UI Template")
            }
            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("Code")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }
            TextField {
                id: code
                width: column.width
                Keys.onReturnPressed: {
                    authorization.sendCode(code.text)
                }
            }
        }
    }
}
