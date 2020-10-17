import QtQuick 2.0
import Sailfish.Silica 1.0

ContextMenu {
    id: contextMenu
    enabled: contextMenuLoader.status === Loader.Ready
    visible: contextMenuLoader.status === Loader.Ready

    MenuItem {
        text: qsTr("Reply")
        onClicked: chatPage.replyMessageId = messageId
    }
    MenuItem {
        text: qsTr("Edit")
        visible: messageType === "text" && !received
        onClicked: {
            textInput.text = messageText
            chatPage.editMessageId = messageId
        }
    }
    MenuItem {
        text: qsTr("Copy")
        onClicked: Clipboard.text = messageText.trim()
    }
    MenuItem {
        text: qsTr("Delete")
        visible: !received
        onClicked: remorse.execute(item, "Deleted", function() { chat.deleteMessage(messageId) } )
    }
}
