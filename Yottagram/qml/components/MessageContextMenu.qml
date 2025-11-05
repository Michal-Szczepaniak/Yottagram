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
import org.nemomobile.notifications 1.0

AppContextMenu {
    id: contextMenu

    Row {
        property int __row
        spacing: 0
        width: parent.width
        height: screen.sizeCategory <= Screen.Medium && parent && parent.hasOwnProperty('__silica_pulleymenu_content')
                ? Theme.itemSizeExtraSmall : Theme.itemSizeSmall
        property int childrenCount: {
            var count = 0;
            for (var i = 0; i < children.length; i++) {
                if (children[i].visible) {
                    count++;
                }
            }
            return count
        }

        AppIconMenuItem {
            icon.source: "qrc:/icons/icon-s-pinned.png"
            description: qsTr("Pin")

            width: parent.width / parent.childrenCount
            visible: {
                var type = chat.getChatType()
                if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canPinMessages
                if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canPinMessages
                return chat.canPinMessages
            }

            onClicked: pinnedMessages.contains(messageId) ?
                           chat.unpinMessage(messageId) :
                           pageStack.push(pinMessageDialog, {messageId: messageId, priv: (type === "private" || type === "secret")})
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-message-reply"
            description: qsTr("Reply")

            width: parent.width / parent.childrenCount
            visible: chatList.selection.length == 0

            onClicked: {
                chatPage.newEditMessageId = 0
                chatPage.newReplyMessageId = messageId
                textInput.focus = true
            }
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-message-forward"
            description: qsTr("Forward")

            width: parent.width / parent.childrenCount

            onClicked: {
                selection.push(messageId)
                chatList.selection = []
                chatPage.selection.forEach(function (element) {
                    var tmp = chatList.selection
                    tmp.push(element)
                    chatList.selection = tmp
                })
                chatList.forwardedFrom = chat.id
                pageStack.navigateBack()
            }
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-delete"
            description: qsTr("Delete")

            width: parent.width / parent.childrenCount
            visible: canBeDeleted

            onClicked: remove()
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-outline-like"
            description: qsTr("React")

            width: parent.width / parent.childrenCount

            onClicked: {
                chat.getAvailableReactions(messageId)

                attachmentLoader.reactionMessage = messageId
                attachmentLoader.sourceComponent = emojiPicker
                attachmentLoader.active = true
                var timer = Qt.createQmlObject("import QtQuick 2.0; Timer {}", contextMenuLoader);
                timer.interval = 1
                timer.repeat = false
                timer.triggered.connect(function () {uploadFlickable.scrollToBottom()})
                timer.start()
            }
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-clipboard"
            description: qsTr("Copy")

            width: parent.width / parent.childrenCount

            onClicked: Clipboard.text = messageTextUnformatted.trim()
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-edit"
            description: qsTr("Edit")

            width: parent.width / parent.childrenCount

            visible: canBeEdited && chatList.selection.length == 0
            onClicked: {
                textInput.text = messageEditText
                textInput.focus = true
                textInput.cursorPosition = textInput.length
                chatPage.newEditMessageId = messageId
                chatPage.newReplyMessageId = 0
            }
        }

        AppIconMenuItem {
            icon.source: "image://theme/icon-m-cloud-download"
            description: qsTr("Save")

            width: parent.width / parent.childrenCount
            visible: messageType === "photo" || messageType === "video" || messageType === "animation" || messageType === "audio" || messageType === "voiceNote" || messageType === "videoNote" || messageType === "document"

            onClicked: {
                var savedPath = "";

                switch(messageType) {
                case "photo":
                    if (file.biggestPhoto.localPath) savedPath = dataManager.savePhoto(file.biggestPhoto.localPath)
                    break;
                case "video":
                    if (file.video.localPath) savedPath = dataManager.saveVideo(file.video.localPath)
                    break;
                case "animation":
                    if (file.animation.localPath) savedPath = dataManager.saveVideo(file.animation.localPath)
                    break;
                case "audio":
                    if (file.audio.localPath) savedPath = dataManager.saveAudio(file.audio.localPath)
                    break;
                case "voiceNote":
                    if (file.voicenote.localPath) savedPath = dataManager.saveAudio(file.voicenote.localPath)
                    break;
                case "videoNote":
                    if (file.videonote.localPath) savedPath = dataManager.saveVideo(file.videonote.localPath)
                    break;
                case "document":
                    if (file.document.localPath) savedPath = dataManager.saveDocument(file.document.localPath)
                    break;
                }

                if (savedPath !== "") {
                    saveNotification.summary = qsTr("Saved to: %1").arg(savedPath)
                    saveNotification.previewSummary = saveNotification.summary
                } else {
                    saveNotification.summary = qsTr("You must download it first")
                    saveNotification.previewSummary = saveNotification.summary
                }

                saveNotification.publish()
            }

            Notification {
                id: saveNotification
            }
        }
    }
}
