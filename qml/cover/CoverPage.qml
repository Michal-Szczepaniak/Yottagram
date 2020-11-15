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
import org.nemomobile.configuration 1.0

CoverBackground {
    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property bool combineWithMuted: false
    }

    Column {
        width: parent.width - Theme.paddingSmall*2
        anchors.centerIn: parent

        Label {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Yottagram")
            font.bold: true
            font.pixelSize: Theme.fontSizeLarge
        }

        Item {
            height: Theme.paddingLarge
            width: 1
        }

        Label {
            id: unreadChats
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Unread chats: %L1").arg(chatList.chatUnreadUnmutedCount + (settings.combineWithMuted ? chatList.chatUnreadCount : 0))
            width: parent.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            id: unreadMessages
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("New messages: %L1").arg(chatList.messageUnreadUnmutedCount + (settings.combineWithMuted ? chatList.messageUnreadCount : 0))
            width: parent.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
