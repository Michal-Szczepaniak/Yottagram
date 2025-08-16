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
import com.verdanditeam.pinnedmessages 1.0

Row {
    id: pinnedMessage
    signal closed()

    function scrollToReply() {
        var messageIndex = chat.getMessageIndex(pinnedMessages.id)
        if (messageIndex === -1) {
            messages.historyBrowsing = pinnedMessages.id
            chat.scrollToMessage(pinnedMessages.id)
        } else {
            messages.positionViewAtIndex(chatProxyModel.mapFromSource(messageIndex), ListView.SnapPosition)
        }
        pinnedMessages.cycleMessage();
    }

    Item {
        height: 1
        width: Theme.horizontalPageMargin
    }

    Rectangle {
        width: 3
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height - Theme.paddingLarge/2
        color: Theme.highlightColor
    }

    Item {
        height: 1
        width: Theme.paddingMedium
    }

    Column {
        id: col
        anchors.verticalCenter: parent.verticalCenter
        width: pinnedMessage.width - closeButton.width - Theme.paddingLarge*2

        Label {
            id: pinnedName
            text: pinnedMessages.id !== 0 ? (pinnedMessages.senderUserId !== 0 ? users.getUserAsVariant(pinnedMessages.senderUserId).name : chat.title) : ""
            truncationMode: TruncationMode.Fade
            width: parent.width
            font.pixelSize: Theme.fontSizeSmall

            MouseArea {
                anchors.fill: parent
                onClicked: pinnedMessage.scrollToReply()
            }
        }
        Label {
            id: pinnedText
            text: pinnedMessages.id === 0 ? "" : pinnedMessages.typeText
            width: parent.width
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeSmall
            linkColor: color
            maximumLineCount: 1

            MouseArea {
                anchors.fill: parent
                onClicked: pinnedMessage.scrollToReply()
            }
        }
    }

    IconButton {
        id: closeButton
        icon.asynchronous: true
        icon.source: "image://theme/icon-m-close"
        anchors.verticalCenter: parent.verticalCenter
        onClicked: pinnedMessage.closed()
    }
}
