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
import QtMultimedia 5.0
import QtGraphicalEffects 1.0
import org.nemomobile.notifications 1.0
import org.nemomobile.configuration 1.0
import Sailfish.Pickers 1.0
import SortFilterProxyModel 0.2
import com.verdanditeam.user 1.0
import com.verdanditeam.audiorecorder 1.0
//import "qrc:///vendor/vendor/lottie/src/qml/"
import "../components/functions/muteFormat.js" as TimeFormat
import "../components"
import "../components/messageContent"
import "../dialogs"

Page {
    id: chatPage
    property string userName: ""
    property string name: ""
    property string pluginName: ""
    property int type: 0
    property var chat: null
    property var replyMessageId: 0
    property var editMessageId: 0
    property variant selection: []
    property bool selectionActive: false
    readonly property double messageWidth: 1.5
    onSelectionActiveChanged: if (selectionActive === false) selection = [];

    allowedOrientations: Orientation.All

    onStatusChanged: {
        if (status === PageStatus.Deactivating) {
            console.log("huj")
            chat.foundChatMembers = []
            chatList.closeChat(chat.id)
        }
        if (status === PageStatus.Active) {
            chatList.openChat(chat.id)

            switch (chat.getChatType()) {
            case "private":
            case "secret":
                pageStack.pushAttached(Qt.resolvedUrl("../components/chatInfo/UserInfo.qml"), {chat: chat, userId: chat.getChatType() === "secret" ? chat.secretChatInfo.userId : chat.idFromType})
                break;
            case "group":
                pageStack.pushAttached(Qt.resolvedUrl("../components/chatInfo/BasicGroupInfo.qml"), {chat: chat})
                break;
            case "supergroup":
            case "channel":
                pageStack.pushAttached(Qt.resolvedUrl("../components/chatInfo/SupergroupInfo.qml"), {chat: chat})
                break;

            }
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: if (Qt.application.state === Qt.ApplicationInactive) chatList.closeChat(chat.id); else if (Qt.application.state === Qt.ApplicationActive) chatList.openChat(chat.id)
    }

    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property bool sendButton: false
        property bool animatedStickers: true
        property int fontSize: Theme.fontSizeMedium
    }

    ConfigurationGroup {
        id: chatSettings
        path: "/apps/yottagram/" + chat.id

        property var pinnedMessageId: 0
    }

    SortFilterProxyModel {
        id: chatProxyModel
        sourceModel: chat
        sorters: [
            RoleSorter { roleName: "messageId"; sortOrder: Qt.DescendingOrder}
        ]
    }

    SilicaFlickable {
        id: mainFlickable
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Clear history")
                onClicked: Remorse.popupAction(root, "", function() {
                    chat.clearHistory()
                })
                visible: !chatPage.selectionActive
            }

            MenuItem {
                text: qsTr("Open secret chat")
                visible: chat.getChatType() === "private" && !chatPage.selectionActive
                onClicked: {
                    chat.openSecretChat()
                    pageStack.pop()
                }
            }

            MenuItem {
                text: qsTr("Close secret chat")
                visible: chat.getChatType() === "secret" && !chatPage.selectionActive
                onClicked: chat.closeSecretChat()
            }

            MenuItem {
                text: qsTr("Set self-destruct timer")
                visible: chat.getChatType() === "secret" && !chatPage.selectionActive
                onClicked: pageStack.push(ttlDialog)
            }

            MenuItem {
                text: qsTr("Cancel selection")
                visible: chatPage.selectionActive
                onClicked: chatPage.selectionActive = false
            }

            MenuItem {
                text: qsTr("Forward")
                visible: chatPage.selectionActive
                onClicked: {
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
        }

        Rectangle {
            visible: chat.getChatType() === "secret" && chat.ttl > 0
            anchors.right: chatPhoto.right
            anchors.bottom: chatPhoto.bottom
            width: Theme.itemSizeExtraSmall/2
            height: width
            color: Theme.rgba(Theme.highlightBackgroundColor, 0.7)
            border.color: "transparent"
            border.width: 1
            radius: width*0.5
            z: 100
            Text {
                anchors.margins: Theme.paddingSmall/2
                anchors.fill: parent
                color: Theme.primaryColor
                text: TimeFormat.formatAutoDestruct(chat.ttl)
                font.pixelSize: Theme.fontSizeTiny
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
        }

        Avatar {
            id: chatPhoto
            width: Theme.itemSizeSmall
            height: width
            anchors.left: parent.left
            anchors.verticalCenter: pageHeader.verticalCenter
            anchors.leftMargin: Theme.paddingLarge
            userName: chat.title
            self: chat.isSelf
            avatarPhoto: if (!chat.isSelf) chat.smallPhoto
        }

        Row {
            anchors.top: parent.top
            anchors.topMargin: chatPage.isLandscape ? Theme.paddingMedium : Theme.paddingLarge
            anchors.right: parent.right
            anchors.rightMargin: Theme.horizontalPageMargin
            width: parent.width - (chatPhoto.width + Theme.paddingLarge*3)
            height: pageHeader.height/2
            layoutDirection: Qt.RightToLeft
            spacing: Theme.paddingSmall

            Label {
                verticalAlignment: Text.AlignVCenter
                text: chat.isSelf ? qsTr("Saved messages") : chat.title
                truncationMode: TruncationMode.Fade
                width: Math.min(implicitWidth, parent.width - (secretChatIndicator.visible ? (Theme.paddingSmall + secretChatIndicator.width) : 0))
                height: parent.height
                color: pageHeader.palette.highlightColor
                font.pixelSize: Theme.fontSizeLarge
                font.family: Theme.fontFamilyHeading
            }

            Icon {
                id: secretChatIndicator
                anchors.verticalCenter: parent.verticalCenter
                source: "image://theme/icon-s-secure"
                visible: chat.getChatType() === "secret"
                color: !visible ? "" : chat.secretChatInfo.state === "pending" ? "yellow" : (chat.secretChatInfo.state === "ready" ? "green" : "red")
            }
        }

        Row {
            anchors.top: parent.top
            anchors.topMargin: pageHeader.height/2 + Theme.paddingSmall
            anchors.right: parent.right
            anchors.rightMargin: Theme.horizontalPageMargin
            width: parent.width - (chatPhoto.width + Theme.paddingLarge*2)
            height: pageHeader.height/2
            layoutDirection: Qt.RightToLeft

            Label {
                text: {
                    var type = chat.getChatType()
                    if (type === "supergroup" || type === "channel") return qsTr("%1 members").arg(chat.supergroupFullInfo.memberCount)
                    else if (type === "group") return qsTr("%1 members").arg(chat.basicGroupFullInfo.rowCount())
                    else if (type === "secret") return users.getUserAsVariant(chat.secretChatInfo.userId).statusText
                    else return users.getUserAsVariant(chat.idFromType).statusText
                }
                truncationMode: TruncationMode.Fade
                width: Math.min(implicitWidth, parent.width - (secretChatIndicator.visible ? (Theme.paddingSmall + secretChatIndicator.width) : 0))
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Theme.fontSizeSmall
                color: palette.secondaryHighlightColor
            }
        }

        PageHeader {
            id: pageHeader
        }

        Loader {
            id: pinnedMessageLoader
            anchors.top: pageHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            asynchronous: true
            sourceComponent: pinnedComponent
            active: chat.pinnedMessageId !== 0 && chat.pinnedMessageId !== chatSettings.pinnedMessageId && typeof chat.pinnedMessage !== "undefined"

            Connections {
                target: chat
                onGotMessage: {
                    if (messageId === chat.pinnedMessageId) {
                        pinnedMessageLoader.active = true
                    }
                }

                onPinnedMessageIdChanged: {
                    pinnedMessageLoader.active = false
                    if (chat.pinnedMessageId !== 0 && chat.pinnedMessageId !== chatSettings.pinnedMessageId && typeof chat.pinnedMessage !== "undefined") {
                        pinnedMessageLoader.active = true
                    }
                }
            }
        }

        Component {
            id: pinnedComponent

            PinnedMessage {
                onClosed: {
                    chatSettings.pinnedMessageId = chat.pinnedMessageId
                    pinnedMessageLoader.active = false
                }
            }
        }

        SilicaFlickable {
            id: uploadFlickable
            anchors.top: chat.pinnedMessageId !== 0 ? pinnedMessageLoader.bottom : pageHeader.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            contentHeight: height + uploadMediaPanel.height
            clip: true

            onMovementEnded: {
                if (contentY <= (inputItem.height/2)) {
                    scrollToTop()
                    if (stickerPicker.visible) stickerPicker.visible = false
                } else {
                    scrollToBottom()
                }
            }

            SilicaSignalListView {
                id: messages
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: uploadFlickable.contentHeight - inputItem.height
                width: parent.width
                verticalLayoutDirection: ListView.BottomToTop
                clip: true
                model: chatProxyModel
                cacheBuffer: 0
                property var chatHistoryTime: (new Date()).getTime()
                property var chatMessagesTime: (new Date()).getTime()
                property var oldContentHeight: 0
                property var historyBrowsing: 0
                property var originMessage: 0

                Timer {
                    id: getHistoryTimer
                    interval: 1000
                    repeat: false
                    onTriggered: chat.getChatHistory(chat.getMessageIndex(chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId), 40, -20)
                }

                onBeforeScrollingToBottom: {
                    if (originMessage !== 0) {
                        var messageIndex = chat.getMessageIndex(originMessage)
                        if (messageIndex === -1) {
                            chat.scrollToMessage(originMessage)
                        } else {
                            messages.fastScrollEnabled = false
                            messages.positionViewAtIndex(chatProxyModel.mapFromSource(messageIndex), ListView.SnapPosition)
                        }
                    }
                }

                onScrollingToBottom: {
                    messages.fastScrollEnabled = true
                    if (originMessage !== 0) {
                        if (count !== 0) originMessage = 0
                    } else if (chat.lastMessageId !== chatProxyModel.get(0).messageId) {
                        historyBrowsing = 0
                        chat.scrollToMessage(chat.unreadCount > 0 && chat.lastMessageId !== 0 ? chat.lastReadInboxMessageId : chat.lastMessageId)
                    }
                }

                onContentHeightChanged: {
                    if (oldContentHeight === 0 && contentHeight !== 0) {
                        if (historyBrowsing !== 0) {
                            if (originMessage !== 0 && chat.getMessageIndex(originMessage) !== -1) {
                                messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(originMessage)), ListView.SnapPosition)
                                originMessage = 0
                            } else {
                                messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(historyBrowsing)), ListView.SnapPosition)
                            }
                        } else {
                            messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId)), ListView.SnapPosition)
                        }
                    }
                    oldContentHeight = contentHeight

                    if (count != 0 && count < 20 && historyBrowsing === 0) {
                        chat.getChatHistory(chat.getMessageIndex(chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId), 40, -20)
                    }
                }

                onAtYBeginningChanged: if (atYBeginning) chat.getMoreChatHistory()
                onAtYEndChanged: if (atYEnd) chat.getMoreChatMessages()

                Component.onCompleted: {
                    chat.getChatHistory((chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId), 40, -20)
                    getHistoryTimer.start()
                }

                onContentYChanged: {
                    var item = chatProxyModel.get(indexAt(0, contentY + height - Theme.paddingSmall))
                    if (!item.isRead && item.received) chat.setMessageAsRead(item.messageId)
                    if (((contentHeight + originY) - (contentY + height)) < Theme.itemSizeHuge*3 && ((new Date()).getTime() - chatMessagesTime) > 300 && count > 0) {
                        chatMessagesTime = (new Date()).getTime();
                        chat.getMoreChatMessages()
                    }

                    if ((contentY - originY) < 5000 && ((new Date()).getTime() - chatHistoryTime) > 300) {
                        chatHistoryTime = (new Date()).getTime();
                        chat.getMoreChatHistory()
                    }
                }

                delegate:
                ListItem {
                    id: item
                    width: chatPage.width
                    contentHeight: Math.max(column.height + Theme.paddingSmall, Theme.itemSizeExtraSmall)
                    contentWidth: width
                    readonly property var user: users.getUserAsVariant(authorId)
                    readonly property bool isService: messageType == "chatSetTtl" || messageType == "pinMessage" || messageType == "messageChatDeleteMember" || messageType == "messageChatAddMembers" || messageType == "messageChatJoinByLink"
                    property bool displayAvatar: received && (type === "group" || type === "supergroup") && !isService
                    highlighted: selection.indexOf(messageId) !== -1

                    onClicked: {
                        if (doubleClickTimer.running) {
                            doubleClicked(mouse)
                            return
                        } else {
                            doubleClickTimer.start()
                        }

                        if (!chatPage.selectionActive) return
                        if (selection.indexOf(messageId) === -1) {
                            selection.push(messageId)
                        } else {
                            selection.splice(selection.indexOf(messageId), 1)
                            if (chatPage.selectionActive === true && chatPage.selection.length == 0) chatPage.selectionActive = false
                        }
                        highlighted = selection.indexOf(messageId) !== -1
                    }

                    Timer {
                        id: doubleClickTimer
                        interval: 500
                        repeat: false
                    }

                    onDoubleClicked: {
                        chatPage.editMessageId = 0
                        chatPage.replyMessageId = messageId
                    }

                    function remove() {
                        remorseAction("Deleting", function() { chat.deleteMessage(messageId) })
                    }

                    Connections {
                        target: chatPage
                        onSelectionActiveChanged: {
                            if (chatPage.selectionActive === false) {
                                highlighted = false
                            }
                        }
                    }

                    menu: Component {
                        ContextMenu {
                            id: contextMenu

                            MenuItem {
                                text: qsTr("Reply")
                                visible: chatList.selection.length === 0
                                onClicked: {
                                    chatPage.editMessageId = 0
                                    chatPage.replyMessageId = messageId
                                }
                            }

                            MenuItem {
                                text: qsTr("Pin message")
                                visible: {
                                    if (messageId === chat.pinnedMessageId) return false

                                    var type = chat.getChatType()
                                    if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canPinMessages
                                    if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canPinMessages
                                    return chat.canPinMessages
                                }
                                onClicked: pageStack.push(pinMessageDialog, {messageId: messageId})
                            }

                            MenuItem {
                                text: qsTr("Edit")
                                visible: canBeEdited && chatList.selection.length === 0
                                onClicked: {
                                    textInput.text = messageText
                                    chatPage.editMessageId = messageId
                                    chatPage.replyMessageId = 0
                                }
                            }

                            MenuItem {
                                text: qsTr("Copy")
                                onClicked: Clipboard.text = messageText.trim()
                            }

                            MenuItem {
                                text: qsTr("Forward")
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

                            MenuItem {
                                text: qsTr("Delete")
                                visible: canBeDeleted
                                onClicked: remove()
                            }

                            MenuItem {
                                text: item.highlighted ? qsTr("Deselect") : qsTr("Select")
                                visible: chatList.selection.length === 0
                                onClicked: {
                                    if (selection.indexOf(messageId) === -1) {
                                        selection.push(messageId)
                                        if (chatPage.selectionActive === false) chatPage.selectionActive = true
                                    } else {
                                        selection.splice(selection.indexOf(messageId), 1)
                                        if (chatPage.selectionActive === true && chatPage.selection.length == 0) chatPage.selectionActive = false
                                    }
                                    item.highlighted = selection.indexOf(messageId) !== -1
                                }
                            }
                        }
                    }

                    Avatar {
                        id: avatarPhoto
                        width: Theme.itemSizeExtraSmall
                        height: width
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.paddingMedium
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: Theme.paddingMedium
                        visible: displayAvatar && chat.getAuthorByIndex(chatProxyModel.mapToSource(index-1)) !== authorId
                        userName: user.name
                        avatarPhoto: if (user && user.hasPhoto) user.smallPhoto
                    }

                    Column {
                        id: column
                        spacing: 0
                        anchors.left: if (received || isService) parent.left
                        anchors.leftMargin: isService ? 0 : (displayAvatar ? (Theme.itemSizeExtraSmall + Theme.paddingMedium + Theme.paddingMedium) : Theme.horizontalPageMargin)
                        anchors.right: if (!received || isService) parent.right
                        anchors.rightMargin: if (!received) Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width/chatPage.messageWidth

                        Label {
                            id: name
                            text: !isForwarded ? (chat.getChatType() === "channel" ? chat.title : user.name) : (qsTr("Forwarded from %1").arg(getName()) + " " + forwardTimestamp).trim()
                            font.pixelSize: Theme.fontSizeMedium
                            font.bold: true
                            horizontalAlignment: received ? Text.AlignLeft : Text.AlignRight
                            color: Theme.highlightColor
                            visible: ((displayAvatar && chat.getAuthorByIndex(chatProxyModel.mapToSource(index+1)) !== authorId) ||  isForwarded || chat.getChatType() === "channel") && !serviceMessage.visible
                            width: Math.min(implicitWidth, column.width)
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                            function getName() {
                                if (forwardUserId !== 0) return users.getUserAsVariant(forwardUserId).name;
                                if (forwardUsername !== "") return forwardUsername;
                                if (forwardChannelId !== 0) return chatList.getChatAsVariant(forwardChannelId).title
                            }
                        }

                        Loader {
                            id: replyLoader
                            asynchronous: true
                            height: (replyMessageId !== 0) ? Theme.itemSizeSmall : 0
                            anchors.right: if (!received) parent.right
                            anchors.left: if (received) parent.left
                            sourceComponent: if (replyMessageId !== 0) replyComponent
                            active: replyMessageId !== 0 && typeof chat.getMessage(replyMessageId) !== "undefined"
                            Component.onCompleted: {
                                if (replyMessageId !== 0 && typeof chat.getMessage(replyMessageId) === "undefined") chat.fetchMessage(replyMessageId)
                            }

                            Connections {
                                target: chat
                                onGotMessage: {
                                    if (messageId === replyMessageId) {
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
                                visible: replyMessageId !== 0

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
                                        text: replyMessageId !== 0 ? users.getUserAsVariant(chat.getMessage(replyMessageId).senderUserId).name : ""
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
                                        text: chat.getMessage(replyMessageId).type === "text" ? chat.getMessage(replyMessageId).text.trim().replace(/\r?\n|\r/g, " ")
                                                                                                       : chat.getMessage(replyMessageId).type
                                        truncationMode: TruncationMode.Fade

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
                                switch(messageType) {
                                case "photo":
                                    if (file.biggestPhotoSize.width === 0) return Theme.itemSizeHuge*2
                                    return Math.min(Theme.itemSizeHuge*2, file.biggestPhotoSize.width)
                                case "sticker":
                                    return Theme.itemSizeHuge*1.5
                                case "video":
                                    if (file.size.width === 0) return Theme.itemSizeHuge*2
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
                                }
                            }
                            height: {
                                switch(messageType) {
                                case "photo":
                                    if (file.biggestPhotoSize.height === 0) return Theme.itemSizeHuge*2
                                    return width * file.biggestPhotoSize.height/file.biggestPhotoSize.width;
                                case "sticker":
                                    return Theme.itemSizeHuge*1.5
                                case "video":
                                    if (file.size.height === 0) return Theme.itemSizeHuge*2
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

                        Label {
                            id: serviceMessage
                            width: chatPage.width
                            height: Theme.itemSizeMedium
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            text: messageText.trim()
                            color: Theme.highlightColor
                            visible: isService
                        }

                        LinkedLabel {
                            id: textField
                            plainText: messageText
                            font.pixelSize: settings.fontSize
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            width: Math.min(parent.width, implicitWidth)
                            anchors.right: if (!received) parent.right
                            anchors.left: if (received) parent.left
                            color: Theme.primaryColor
                            linkColor: Theme.highlightColor
                            visible: messageText !== "" && !serviceMessage.visible
                        }

                        Loader {
                            id: pollLoader
                            asynchronous: false
                            sourceComponent: {
                                if (messageType == "poll") {
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
                            asynchronous: true
                            sourceComponent: if (hasWebPage) webPageComponent
                        }

                        Component {
                            id: webPageComponent

                            Row {
                                id: webPagePreviewMessage

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
                                        id: webPageName
                                        text: webPage.name
                                        truncationMode: TruncationMode.Fade
                                        font.family: Theme.fontFamilyHeading
                                        font.bold: true
                                        font.pixelSize: Theme.fontSizeSmall
                                        color: Theme.highlightColor

                                        MouseArea {
                                            anchors.fill: parent
                                        }
                                    }

                                    Label {
                                        id: webPageTitle
                                        text: webPage.title
                                        width: webPageDescription.width
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        font.bold: true
                                        font.pixelSize: Theme.fontSizeSmall
                                        truncationMode: TruncationMode.Fade

                                        MouseArea {
                                            anchors.fill: parent
                                        }
                                    }

                                    Label {
                                        id: webPageDescription
                                        text: webPage.description
                                        width: chatPage.width/chatPage.messageWidth - Theme.paddingLarge
                                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                        maximumLineCount: 5
                                        font.pixelSize: Theme.fontSizeSmall

                                        MouseArea {
                                            anchors.fill: parent
                                        }
                                    }
                                }
                            }
                        }

                        Row {
                            id: stamp
                            spacing: Theme.paddingSmall
                            visible: !serviceMessage.visible

                            Item {
                                height: 1
                                width: column.width - time.width - readIcon.width - Theme.paddingSmall*2
                                visible: !received
                            }

                            Label {
                                id: time
                                text: (edited ? qsTr("Edited") + " ": "") + timestamp
                                anchors.verticalCenter: parent.verticalCenter
                                color: Theme.secondaryColor
                                font.pixelSize: Theme.fontSizeExtraSmall
                            }

                            Icon {
                                id: readIcon
                                width: Theme.iconSizeExtraSmall
                                height: Theme.iconSizeExtraSmall
                                anchors.verticalCenter: parent.verticalCenter
                                source: isRead ? "qrc:///icons/icon-s-read.svg" : "qrc:///icons/icon-s-sent.svg"
                            }
                        }
                    }
                }

                VerticalScrollDecorator {}
            }

            Column {
                id: inputItem
                anchors.top: messages.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                visible: !(chat.getChatType() === "secret" && (chat.secretChatState === "closed" || chat.secretChatState === "pending"))

                SilicaListView {
                    id: chatMembers
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    height: Math.min(Theme.itemSizeSmall*4, contentHeight)
                    spacing: Theme.paddingSmall
                    clip: true
                    model: chat.foundChatMembers.map(function (element) { return element; });
                    visible: count > 0

                    delegate: ListItem {
                        width: parent.width
                        contentHeight: Theme.itemSizeExtraSmall
                        readonly property var user: users.getUserAsVariant(modelData)

                        Row {
                            anchors.fill: parent
                            spacing: Theme.paddingLarge

                            Avatar {
                                id: avatar
                                anchors.verticalCenter: parent.verticalCenter
                                width: height
                                height: parent.height - Theme.paddingSmall
                                userName: user.name
                                avatarPhoto: user.smallPhoto
                            }

                            DetailItem {
                                width: parent.width - Theme.paddingLarge - avatar.width - Theme.paddingLarge
                                anchors.verticalCenter: parent.verticalCenter
                                label: user.name
                                value: "@" + user.username
                                alignment: Qt.AlignLeft
                                leftMargin: 0
                                rightMargin: 0
                            }
                        }

                        onClicked: {
                            var txtinput = textInput
                            var tmpChat = chat
                            var position = txtinput.cursorPosition

                            var cutText = txtinput.text.slice(0,position)
                            var result = (/(?:^|\s)\@\S*$/.exec(cutText))[0].trim().slice(1)

                            txtinput.text = txtinput.text.slice(0,position-result.length) + user.username + txtinput.text.slice(position,txtinput.text.length)
//                            txtinput.text += user.username
                            txtinput.cursorPosition = position + user.username.length - result.length

                            tmpChat.foundChatMembers = []
                        }
                    }

                    VerticalScrollDecorator { }
                }

                Row {
                    id: forwardRow
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatList.selection.length !== 0

                    Icon {
                        id: forwardIcon
                        source: "image://theme/icon-m-message-forward"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - forwardIcon.width - cancelForwardIcon.width - Theme.paddingLarge*2
                        text: qsTr("%L1 forwarded messages").arg(chatList.selection.length)
                        truncationMode: TruncationMode.Fade
                        color: Theme.highlightColor
                    }

                    IconButton {
                        id: cancelForwardIcon
                        icon.source: "image://theme/icon-m-cancel"
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: chatList.selection = []
                    }
                }

                Row {
                    id: replyRow
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatPage.replyMessageId !== 0

                    function getReplyData(roleName) {
                        return chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.replyMessageId)), 0), chatProxyModel.roleForName(roleName))
                    }

                    Icon {
                        id: replyIcon
                        source: "image://theme/icon-m-message-reply"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - replyIcon.width - cancelReplyIcon.width - Theme.paddingLarge*2

                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.replyMessageId === 0 ? "" : users.getUserAsVariant(replyRow.getReplyData("authorId")).name
                            truncationMode: TruncationMode.Fade
                            color: Theme.highlightColor
                        }
                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.replyMessageId === 0 ? "" :
                                          (replyRow.getReplyData("messageType") === "text" ? replyRow.getReplyData("messageText")
                                                                                               : replyRow.getReplyData("messageType"))
                            truncationMode: TruncationMode.Fade
                        }
                    }

                    IconButton {
                        id: cancelReplyIcon
                        icon.source: "image://theme/icon-m-cancel"
                        anchors.verticalCenter: parent.verticalCenter
                        visible: chatPage.replyMessageId !== 0
                        onClicked: chatPage.replyMessageId = 0
                    }
                }

                Row {
                    id: editRow
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatPage.editMessageId !== 0

                    function getEditData(roleName) {
                        return chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.editMessageId)), 0), chatProxyModel.roleForName(roleName))
                    }

                    Icon {
                        id: editIcon
                        source: "image://theme/icon-m-edit"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - editIcon.width - cancelEditIcon.width - Theme.paddingLarge*2

                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.editMessageId === 0 ? "" : users.getUserAsVariant(editRow.getEditData("authorId")).name
                            truncationMode: TruncationMode.Fade
                            color: Theme.highlightColor
                        }
                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.editMessageId === 0 ? "" :
                                          (editRow.getEditData("messageType") === "text" ? editRow.getEditData("messageText")
                                                                                               : editRow.getEditData("messageType"))
                            truncationMode: TruncationMode.Fade
                        }
                    }

                    IconButton {
                        id: cancelEditIcon
                        icon.source: "image://theme/icon-m-cancel"
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: {
                            textInput.text = ""
                            chatPage.editMessageId = 0
                        }
                    }
                }

                AudioRecorder {
                    id: audioRecorder
                    onStateChanged: voiceNoteTimer.start()
                }

                Timer {
                    id: voiceNoteTimer
                    interval: 100
                    repeat: false
                    onTriggered: if (audioRecorder.state == 0) {
                                     chat.sendVoiceNote(audioRecorder.location, audioRecorder.getWaveform(), voiceMessageButton.duration, chatPage.replyMessageId)
                                     uploadFlickable.contentY = 0
                                     chatPage.replyMessageId = 0
                                 }
                }

                Connections {
                    target: chatPage
                    onStatusChanged: {
                        if (chatPage.status === PageStatus.Deactivating) {
                            stickerPicker.visible = false
                            uploadFlickable.scrollToTop()
                        }
                    }
                }

                TextArea {
                    id: textInput
                    enabled: {
                        var type = chat.getChatType()
                        if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canSendMessages
                        if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canSendMessages
                        return chat.canSendMessages
                    }
                    anchors.right: sendButton.visible ? sendButton.left : parent.right
                    anchors.left: parent.left
                    placeholderText: qsTr("Type the text...")
                    color: Theme.primaryColor
                    focusOutBehavior: FocusBehavior.KeepFocus
                    height: if (!visible) 0
                    visible: !stickerPicker.visible

                    onTextChanged: {
                        var cutText = textInput.text.slice(0,textInput.cursorPosition)
                        var results = (/(?:^|\s)\@\S*$/.exec(cutText))
                        if (results !== null) {
                            chat.searchChatMembers(results[0].trim())
                        } else {
                            chat.foundChatMembers = []
                        }
                    }

                    EnterKey.onClicked: {
                        if (!settings.sendButton) {
                            textInput.text = textInput.text.slice(0,textInput.cursorPosition-1) + textInput.text.slice(textInput.cursorPosition,textInput.text.length)
                            sendMessage()
                        }
                    }

                    function sendMessage() {
                        if(textInput.text.length !== 0) {
                            if (chatPage.editMessageId !== 0) {
                                chat.editMessageText(chatPage.editMessageId, textInput.text)
                                chatPage.editMessageId = 0
                            } else {
                                chat.sendMessage(textInput.text, chatPage.replyMessageId)
                                chatPage.replyMessageId = 0
                            }
                            textInput.text = ""
                            textInput.focus = false
                            textInput.focus = true
                        }

                        if (chatList.selection.length !== 0) {
                            chat.sendForwardedMessages(chatList.selection, chatList.forwardedFrom)
                            chatList.selection = []
                            chatList.forwardedFrom = 0
                        }
                    }
                }

                IconButton {
                    id: sendButton
                    anchors.right: parent.right
                    icon.source: "image://theme/icon-m-send"
                    visible: settings.sendButton && !stickerPicker.visible
                    onClicked: textInput.sendMessage()
                }

                Column {
                    id: uploadMediaPanel
                    width: parent.width
                    spacing: Theme.paddingLarge
                    visible: {
                        if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canSendMediaMessages
                        if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canSendMediaMessages
                        return chat.canSendMediaMessages
                    }
                    height: if (!visible) 0
                    property int columns: 5
                    property real cellWidth: (width-Theme.paddingMedium*2)/columns
                    property bool hidePickers: stickerPicker.visible

                    StickerPicker {
                        id: stickerPicker
                        height: visible ? Screen.height/2 + Theme.paddingLarge : 0
                        width: parent.width
                        visible: false
                        page: chatPage
                        opacity: uploadFlickable.contentY/(inputItem.height- Theme.itemSizeLarge)
                        onStickerFileIdChanged: {
                            chat.sendSticker(stickerFileId, chatPage.replyMessageId)
                            stickerPicker.visible = false
                            chatPage.replyMessageId = 0
                            stickerFileId = 0
                            uploadFlickable.scrollToTop()
                        }
                    }

                    Row {
                        x: Theme.paddingMedium
                        width: parent.width - Theme.paddingMedium*2
                        id: firstRow
                        visible: !uploadMediaPanel.hidePickers

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Image")
                            source: "image://theme/icon-m-file-image"
                            onClicked: pageStack.push(imagePickerPage)
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Video")
                            source: "image://theme/icon-m-file-video"
                            onClicked: pageStack.push(videoPickerPage)
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Audio")
                            source: "image://theme/icon-m-file-audio"
                            onClicked: pageStack.push(musicPickerPage)
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Document")
                            source: "image://theme/icon-m-file-document-light"
                            onClicked: pageStack.push(documentPickerPage)
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("File")
                            source: "image://theme/icon-m-file-other-light"
                            onClicked: pageStack.push(filePickerPage)
                        }
                    }

                    Row {
                        x: Theme.paddingMedium
                        width: parent.width - Theme.paddingMedium*2
                        visible: !uploadMediaPanel.hidePickers

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Poll")
                            source: "image://theme/icon-m-events"
                            onClicked: pageStack.push(pollDialog)
                            visible: {
                                if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canSendPolls
                                if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canSendPolls
                                return chat.canSendPolls
                            }
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Sticker")
                            source: "image://theme/icon-m-other"
                            onClicked: {
                                textInput.focus = false
                                stickerPicker.visible = true
                                var timer = Qt.createQmlObject("import QtQuick 2.0; Timer {}", root);
                                timer.interval = 1
                                timer.repeat = false
                                timer.triggered.connect(function () {uploadFlickable.scrollToBottom()})
                                timer.start()
                            }
                        }

                        HighlightLabelIconButton {
                            id: voiceMessageButton
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Voice Note")
                            source: audioRecorder.recording ? (Theme.LightOnDark ? "image://theme/icon-m-call-recording-on-light" : "image://theme/icon-m-call-recording-on-dark"): "image://theme/icon-m-mic"
                            onPressed: audioRecorder.startRecording()
                            visible: !stickerPicker.visible
                            property var duration
                            onReleased: {
                                duration = audioRecorder.duration
                                audioRecorder.stopRecording()
                            }
                            onClicked: {
                                notification.publish()
                            }

                            Notification {
                                id: notification

                                isTransient: true
                                previewSummary: qsTr("Press and hold record button to record voice note.")
                            }
                        }
                    }

                    Row {
                        x: Theme.horizontalPageMargin
                        width: parent.width - Theme.horizontalPageMargin*2
                        height: 1
                        visible: !uploadMediaPanel.hidePickers
                    }
                }
            }
        }

        Component {
            id: filePickerPage
            MultiFilePickerDialog {
                allowedOrientations: Orientation.All
                title: "Select file"
                onAccepted: {
                    var urls = []
                    for (var i = 0; i < selectedContent.count; ++i) {
                        chat.sendFile(selectedContent.get(i).filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                    }
                    uploadFlickable.scrollToTop()
                    pageStack.navigateBack(PageStackAction.Immediate)
                }
            }
        }

        Component {
            id: imagePickerPage
            MultiImagePickerDialog {
                allowedOrientations: Orientation.All

                onAccepted: {
                    var urls = []
                    for (var i = 0; i < selectedContent.count; ++i) {
                        urls.push(selectedContent.get(i).filePath)
                    }
                    chat.sendPhotos(urls, chatPage.replyMessageId)
                    chatPage.replyMessageId = 0
                    uploadFlickable.scrollToTop()
                }
            }
        }

        Component {
            id: videoPickerPage
            MultiVideoPickerDialog {
                allowedOrientations: Orientation.All
                onAccepted: {
                    var urls = []
                    for (var i = 0; i < selectedContent.count; ++i) {
                        urls.push(selectedContent.get(i).filePath)
                    }
                    chat.sendVideos(urls, chatPage.replyMessageId)
                    chatPage.replyMessageId = 0
                    uploadFlickable.scrollToTop()
                }
            }
        }

        Component {
            id: musicPickerPage
            MultiMusicPickerDialog {
                allowedOrientations: Orientation.All
                onAccepted: {
                    var urls = []
                    for (var i = 0; i < selectedContent.count; ++i) {
                        chat.sendMusic(selectedContent.get(i).filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                    }
                    uploadFlickable.scrollToTop()
                }
            }
        }

        Component {
            id: documentPickerPage
            MultiDocumentPickerDialog {
                allowedOrientations: Orientation.All
                onAccepted: {
                    var urls = []
                    for (var i = 0; i < selectedContent.count; ++i) {
                        chat.sendFile(selectedContent.get(i).filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                    }
                    uploadFlickable.scrollToTop()
                }
            }
        }
    }

    Dialog {
        id: bigPhoto
        property string path

        allowedOrientations: Orientation.All

        SilicaFlickable {
            anchors.fill: parent

            PullDownMenu {
                MenuItem {
                    text: qsTr("Save to gallery")
                    onClicked: chat.saveToGallery(bigPhoto.path)
                }
            }

            Image {
                anchors.fill: parent
                source: bigPhoto.path
                fillMode: Image.PreserveAspectFit
            }
        }
    }

    Dialog {
        id: bigVideo
        property string path

        allowedOrientations: Orientation.All

        SilicaFlickable {
            anchors.fill: parent

            PullDownMenu {
                MenuItem {
                    text: qsTr("Save to gallery")
                    onClicked: chat.saveToGallery(bigVideo.path)
                }
            }

            Video {
                id: video
                anchors.fill: parent
                source: bigVideo.path
                autoPlay: true
                fillMode: Image.PreserveAspectFit
            }
        }

        onDone: {
            video.stop()
            bigVideo.path = ""
        }
    }

    Dialog {
        id: pinMessageDialog
        property var messageId
        onMessageIdChanged: notify.checked = true

        allowedOrientations: Orientation.All

        Column {
            anchors.fill: parent
            DialogHeader { }

            TextSwitch {
                id: notify
                width: parent.width
                text: qsTr("Notify members")
                checked: true
            }
        }

        onDone: {
            if (result == DialogResult.Accepted) {
                chat.pinMessage(messageId, notify.checked)
            }
        }
    }

    Component {
        id: pollDialog
        PollDialog {
            chatObject: chat
        }
    }

    Component {
        id: ttlDialog
        SetTtlDialog {
            id: setTtlDialog
            onAccepted: chat.ttl = setTtlDialog.seconds
        }
    }
}
