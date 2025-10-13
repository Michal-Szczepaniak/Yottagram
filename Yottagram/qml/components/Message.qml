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

import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Silica.private 1.0
import "messageContent"
import "functions/foramtDuration.js" as FormatDuration
import "functions/twemoji.js" as Twemoji

Column {
    id: column
    spacing: 0
    height: (name.visible ? name.height : 0) + replyLoader.height + contentLoader.height + serviceMessage.height + textField.height + pollLoader.height + webPageLoader.height + stamp.height
    width: parent.width

    Label {
        id: name
        text: !isForwarded ? (sender === "chat" ? chatList.getChatAsVariant(senderId).title : users.getUserAsVariant(senderId).name) : (qsTr("Forwarded from %1").arg(getName()) + " " + forwardTimestamp).trim()
        font.pixelSize: Theme.fontSizeMedium
        font.bold: true
        horizontalAlignment: received ? Text.AlignLeft : Text.AlignRight
        color: Theme.highlightColor
        visible: (
                     (displayAvatar && (listItem.prevMessage !== undefined ? (listItem.prevMessage.sender !== sender || listItem.prevMessage.senderId !== senderId) : true)) ||
                     isForwarded ||
                     chat.getChatType() === "channel"
                 )
        width: Math.min(implicitWidth, column.width)
        maximumLineCount: 1

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (sender === "user")
                    pageStack.push(Qt.resolvedUrl("../components/chatInfo/UserInfo.qml"), { userId: senderId, chat: chatList.getChatAsVariantForUser(senderId) })
            }
        }

        function getName() {
            if (forwardUserId != 0) return users.getUserAsVariant(forwardUserId).name
            if (forwardChatId != 0) return chatList.getChatAsVariant(forwardChatId).title
            if (forwardUsername !== "") return forwardUsername;
            if (forwardChannelId != 0) return chatList.getChatAsVariant(forwardChannelId).title
        }
    }

    Loader {
        id: replyLoader
        asynchronous: true
        height: (replyMessageId != 0) ? Theme.itemSizeSmall : 0
        anchors.right: if (!received) parent.right
        anchors.left: if (received) parent.left
        sourceComponent: if (replyMessageId != 0) replyComponent
        active: replyMessageId != 0 && typeof chat.getMessage(replyMessageId) !== "undefined"
        Component.onCompleted: {
            if (replyMessageId != 0 && typeof chat.getMessage(replyMessageId) === "undefined") chat.fetchMessage(replyMessageId)
        }

        Connections {
            target: chat
            onGotMessage: {
                if (messageId == replyMessageId) {
                    replyLoader.active = true
                }
            }
        }
    }

    Component {
        id: replyComponent

        Row {
            id: replyMessage
            width: Math.max(textField.width, Theme.itemSizeHuge, contentLoader.width)
            visible: replyMessageId != 0

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
                id: replyColumn
                function scrollToReply() {
                    messages.originMessage = messageId
                    var messageIndex = chat.getMessageIndex(replyMessageId)
                    if (messageIndex === -1) {
                        messages.historyBrowsing = replyMessageId
                        chat.scrollToMessage(replyMessageId)
                    } else {
                        messages.positionViewAtIndex(chatProxyModel.mapFromSource(messageIndex), ListView.SnapPosition)
                    }
                }

                Label {
                    id: replyName
                    readonly property var replyMessageObj: chat.getMessage(replyMessageId)
                    text: replyMessageObj.sender == "chat" ? chatList.getChatAsVariant(replyMessageObj.senderChatId).title : users.getUserAsVariant(replyMessageObj.senderUserId).name
                    font.bold: true
                    font.pixelSize: Theme.fontSizeSmall
                    truncationMode: TruncationMode.Fade

                    MouseArea {
                        anchors.fill: parent
                        onClicked: replyColumn.scrollToReply()
                    }
                }

                Label {
                    id: replyText
                    width: replyMessage.width - Theme.paddingLarge
                    font.pixelSize: Theme.fontSizeSmall
                    text: chat.getMessage(replyMessageId).typeText
                    truncationMode: TruncationMode.Fade
                    color: Theme.primaryColor
                    linkColor: Theme.highlightColor
                    maximumLineCount: 1

                    MouseArea {
                        anchors.fill: parent
                        onClicked: replyColumn.scrollToReply()
                    }
                }
            }
        }
    }

    Loader {
        id: contentLoader
        width: {
            console.log(messageType)
            switch(messageType) {
            case "photo":
                if (file.biggestPhotoSize.width == 0) return Theme.itemSizeHuge*2
                return Math.min(Theme.itemSizeHuge*2, file.biggestPhotoSize.width)
            case "animatedEmoji":
                return settings.animatedEmoji ? Theme.itemSizeHuge*1.5 : 0
            case "sticker":
                return Theme.itemSizeHuge*1.5
            case "video":
                if (file.size.width == 0) return Theme.itemSizeHuge*2
                return Math.min(Theme.itemSizeHuge*2, file.size.width)
            case "animation":
                return chatPage.width/2.5
            case "audio":
                return Theme.itemSizeSmall + Theme.itemSizeHuge*2 + Theme.paddingMedium
            case "voiceNote":
                return Theme.itemSizeSmall + Theme.itemSizeHuge*2
            case "videoNote":
                return chatPage.width/2.5
            case "document":
                return Theme.itemSizeMedium + Theme.itemSizeHuge*2
            case "location":
            case "venue":
                return parseInt(chatPage.width/2.2)
            case "contact":
                return chatPage.width/2.5
            default:
                return 0
            }
        }
        height: {
            switch(messageType) {
            case "photo":
                if (file.biggestPhotoSize.height == 0) return Theme.itemSizeHuge*2
                return width * file.biggestPhotoSize.height/file.biggestPhotoSize.width;
            case "animatedEmoji":
                return settings.animatedEmoji ? Theme.itemSizeHuge*1.5 : 0
            case "sticker":
                return Theme.itemSizeHuge*1.5
            case "video":
                if (file.size.height == 0) return Theme.itemSizeHuge*2
                return width * file.size.height/file.size.width;
            case "animation":
                return (file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeLarge
            case "audio":
                return Theme.itemSizeSmall
            case "voiceNote":
                return Theme.itemSizeSmall
            case "videoNote":
                return chatPage.width/2.5
            case "document":
                return Theme.itemSizeMedium
            case "location":
            case "venue":
                return chatPage.width/3
            case "contact":
                return Theme.itemSizeSmall
            default:
                return 0
            }
        }

        anchors.right: if (!received) parent.right
        asynchronous: true
        sourceComponent: {
            switch(messageType) {
            case "photo":
                autoDownloadTimer.start()
                return imageComponent;
            case "sticker":
                autoDownloadTimer.start()
                if (file.isAnimated) {
                    return animatedStickerComponent;
                } else {
                    return stickerComponent;
                }
            case "video":
                autoDownloadTimer.start()
                return videoComponent;
            case "animation":
                autoDownloadTimer.start()
                return animationComponent;
            case "audio":
                autoDownloadTimer.start()
                return audioComponent;
            case "voiceNote":
                autoDownloadTimer.start()
                return voicenoteComponent;
            case "videoNote":
                autoDownloadTimer.start()
                return videonoteComponent;
            case "document":
                autoDownloadTimer.start()
                return documentComponent;
            case "location":
                return locationComponent;
            case "venue":
                return venueComponent;
            case "contact":
                return contactComponent;
            case "animatedEmoji":
                if (settings.animatedEmoji) {
                    autoDownloadTimer.start()
                    return animatedEmojiComponent;
                }
            }
        }

        Timer {
            id: autoDownloadTimer
            interval: 1000
            repeat: false
            onTriggered: handleAutodownload()

            function handleAutodownload() {
                if (!chatList.autoDownloadEnabled()) return;
                if (messageType == "photo") chatList.autoDownload(file.biggestPhoto.id, messageType)
                else chatList.autoDownload(file.id, messageType)
            }
        }
    }

    Component {
        id: imageComponent

        ImageContent { }
    }

    Component {
        id: stickerComponent

        StickerContent { }
    }

    Component {
        id: animatedStickerComponent

        AnimatedStickerContent { }
    }

    Component {
        id: videoComponent

        VideoContent { }
    }

    Component {
        id: audioComponent

        AudioContent { }
    }

    Component {
        id: documentComponent

        DocumentContent { }
    }

    Component {
        id: animationComponent

        AnimationContent { }
    }

    Component {
        id: voicenoteComponent

        VoiceNoteContent { }
    }

    Component {
        id: videonoteComponent

        VideoNoteContent { }
    }

    Component {
        id: locationComponent

        LocationContent { }
    }

    Component {
        id: venueComponent

        VenueContent { }
    }

    Component {
        id: contactComponent

        ContactContent { }
    }

    Component {
        id: animatedEmojiComponent

        AnimatedEmojiContent { }
    }

    Label {
        id: serviceMessage
        width: chatPage.width
        height: isService ? Theme.itemSizeMedium : 0
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: messageText.trim()
        color: Theme.highlightColor
        visible: isService
    }

    Label {
        id: textField
        textFormat: Text.StyledText
        text: Twemoji.emojify(messageText.replace(/\n/g, '<br>'), Theme.fontSizeSmall)
        font.pixelSize: settings.fontSize
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        width: parent.width
        anchors.right: if (!received) parent.right
        anchors.left: if (received) parent.left
        color: Theme.primaryColor
        linkColor: Theme.highlightColor
        onLinkActivated: {
            if (link.substring(0, 9) === "userid://") {
                while (pageStack.depth > 1)
                    pageStack.pop(undefined, PageStackAction.Immediate)

                pageStack.push(Qt.resolvedUrl("../components/chatInfo/UserInfo.qml"), { userId: link.substring(9), chat: chatList.getChatAsVariantForUser(link.substring(9)) })
            } else if (link.substring(0, 10) === "command://") {
                chat.sendMessage(link.substring(10), 0)
            } else {
                Qt.openUrlExternally(link)
            }
        }
        visible: messageText !== "" && !serviceMessage.visible && (messageType !== "animatedEmoji" || !settings.animatedEmoji)
        horizontalAlignment: received ? Text.AlignLeft : Text.AlignRight
    }

    Loader {
        id: pollLoader
        asynchronous: false
        sourceComponent: {
            if (messageType === "poll") {
                return pollComponent;
            }
        }
    }

    Component {
        id: pollComponent

        Poll {
            width: chatPage.width/chatPage.messageWidth
        }
    }

    Loader {
        id: webPageLoader
        asynchronous: false
        sourceComponent: if (hasWebPage) webPageComponent
    }

    Component {
        id: webPageComponent

        WebPage {}
    }

    Row {
        id: stamp
        spacing: Theme.paddingSmall
        visible: !serviceMessage.visible

        Row {
            id: reactionsRow

            Repeater {
                model: reactions

                Rectangle {
                    height: Theme.iconSizeSmall
                    width: reactionRow.width + Theme.paddingLarge
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.rgba(selectedReaction === index ? Theme.highlightColor : Theme.secondaryHighlightColor, selectedReaction === index ? 1 : Theme.highlightBackgroundOpacity)
                    radius: height

                    Row {
                        id: reactionRow
                        anchors.centerIn: parent
                        spacing: Theme.paddingSmall

                        Image {
                            id: reactionImage
                            width: reaction.height
                            height: width
                            visible: Twemoji.test(modelData)
                            source: 'qrc:///emoji/' + Twemoji.emojifyRaw(modelData) + '.svg'
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        AnimatedImage {
                            width: reaction.height
                            height: width
                            visible: !reactionImage.visible
                            source: if (!reactionImage.visible) modelData
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Label {
                            id: reaction
                            text: reactionsCount[index]
                            font.pixelSize: Theme.fontSizeTiny
                            verticalAlignment: Text.AlignVCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: console.log("react")
                    }
                }
            }
        }

        Item {
            height: 1
            width: column.width - time.width - readIcon.width - Theme.paddingSmall*2 - reactionsRow.width
            visible: !received
        }

        Label {
            id: time
            text: (edited ? qsTr("Edited") + " ": "") + timestamp + (messageType === "call" ? ", " + FormatDuration.format(call.duration) : "")
            anchors.verticalCenter: parent.verticalCenter
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeExtraSmall
        }

        Icon {
            id: readIcon
            width: Theme.iconSizeExtraSmall
            height: Theme.iconSizeExtraSmall
            anchors.verticalCenter: parent.verticalCenter
            visible: !received
            source: isRead ? "qrc:///icons/icon-s-read.svg" : "qrc:///icons/icon-s-sent.svg"
        }
    }
}
