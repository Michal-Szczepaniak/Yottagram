import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    property var chat


    Column {
        anchors.fill: parent

        PageHeader {
            title: qsTr("Notification settings")
        }

        Column {
            x: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin*2
            height: parent.height

            TextSwitch {
                width: parent.width
                text: qsTr("Show preview")
                checked: chat.showPreview
                onCheckedChanged: chat.showPreview = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Mentions and replies")
                checked: !chat.disableMentionNotifications
                onCheckedChanged: chat.disableMentionNotifications = !checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Pinned messages")
                checked: !chat.disablePinnedMessageNotifications
                onCheckedChanged: chat.disablePinnedMessageNotifications = !checked
            }
        }
    }
}
