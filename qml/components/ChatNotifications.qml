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

            Label {
                text: (chat.defaultShowPreview ? qsTr("Default") + "\n" : "") + qsTr("These settings will override global values in settings.")
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Mentions and replies")
                checked: !chat.disableMentionNotifications
                onCheckedChanged: chat.disableMentionNotifications = !checked
            }

            Label {
                text: (chat.defaultDisableMentionNotifications ? qsTr("Default") + "\n" : "") + qsTr("These settings will override global values in settings.")
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Pinned messages")
                checked: !chat.disablePinnedMessageNotifications
                onCheckedChanged: chat.disablePinnedMessageNotifications = !checked
            }

            Label {
                text: (chat.defaultDisablePinnedMessageNotifications ? qsTr("Default") + "\n" : "") + qsTr("These settings will override global values in settings.")
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
            }
        }
    }
}
