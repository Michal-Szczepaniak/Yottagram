/*

This file is part of Yottagram.
Copyright 2020, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Yottagram is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Yottagram is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

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
