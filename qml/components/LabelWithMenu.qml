import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: root
    property alias text: label.text
    property alias description: description.text
    contentHeight: column.height + Theme.paddingMedium*2

    Column {
        id: column
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        Label {
            id: label
            anchors.left: parent.left
            anchors.leftMargin: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin*2
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: description
            anchors.left: parent.left
            anchors.leftMargin: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin*2
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }
}
