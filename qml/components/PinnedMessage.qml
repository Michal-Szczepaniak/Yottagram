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

Row {
    id: pinnedMessage
    signal closed()

    function scrollToReply() {
        var messageIndex = chat.getMessageIndex(chat.pinnedMessageId)
        if (messageIndex === -1) {
            messages.historyBrowsing = chat.pinnedMessageId
            chat.scrollToMessage(chat.pinnedMessageId)
        } else {
            messages.positionViewAtIndex(chatProxyModel.mapFromSource(messageIndex), ListView.SnapPosition)
        }
    }

    Item {
        height: 1
        width: Theme.horizontalPageMargin
    }

    Rectangle {
        width: 3
        height: parent.height
        color: Theme.highlightColor
    }

    Item {
        height: 1
        width: Theme.paddingMedium
    }

    Column {
        width: pinnedMessage.width - closeButton.width - Theme.paddingLarge*2

        Label {
            id: pinnedName
            text: chat.pinnedMessageId !== 0 ? users.getUserAsVariant(chat.pinnedMessage.senderUserId).name : ""
            truncationMode: TruncationMode.Fade
            width: parent.width

            MouseArea {
                anchors.fill: parent
                onClicked: pinnedMessage.scrollToReply()
            }
        }
        Label {
            id: pinnedText
            text: chat.pinnedMessageId === 0 ? "" :
                      (chat.pinnedMessage.type === "text" ? chat.pinnedMessage.text.trim().replace(/\r?\n|\r/g, " ")
                                                                           : chat.pinnedMessage.type)
            width: parent.width
            truncationMode: TruncationMode.Fade

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
