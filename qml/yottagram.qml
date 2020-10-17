import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "dialogs"

ApplicationWindow
{
    id: app

    initialPage: Component { ChatList { } }

    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
