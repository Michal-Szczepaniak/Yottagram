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
    height: Theme.itemSizeMedium
    visible: chat.pinnedMessageId !== 0

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

        Label {
            id: pinnedName
            text: chat.pinnedMessageId !== 0 ? users.getUserAsVariant(pinnedMessage.getPinnedData("authorId")).name : ""
            truncationMode: TruncationMode.Fade

            MouseArea {
                anchors.fill: parent
                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(chat.pinnedMessageId)), ListView.SnapPosition)
            }
        }
        Label {
            id: pinnedText
            width: pinnedMessage - Theme.paddingLarge
            text: chat.pinnedMessageId === 0 ? "" :
                      (pinnedMessage.getPinnedData("messageType") === "text" ? pinnedMessage.getPinnedData("messageText").trim().replace(/\r?\n|\r/g, " ")
                                                                           : pinnedMessage.getPinnedData("messageType"))
            truncationMode: TruncationMode.Fade

            MouseArea {
                anchors.fill: parent
                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(chat.pinnedMessageId)), ListView.SnapPosition)
            }
        }
    }

    function getPinnedData(roleName) {
        var result = chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(chat.pinnedMessageId)), 0), chatProxyModel.roleForName(roleName))
        if (result === void(0)) return "";
        return result
    }
}
