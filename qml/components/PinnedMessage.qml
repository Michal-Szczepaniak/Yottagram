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
