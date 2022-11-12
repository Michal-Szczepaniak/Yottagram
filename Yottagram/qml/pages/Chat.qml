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
import com.verdanditeam.chat 1.0
import com.verdanditeam.user 1.0
import com.verdanditeam.audiorecorder 1.0
import com.verdanditeam.pinnedmessages 1.0
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
    property var shareConfiguration: null
    property var newReplyMessageId: 0
    property var newEditMessageId: 0
    property variant selection: []
    property bool selectionActive: false
    property var startMessage: (chat.firstUnreadMention > 0 ? chat.firstUnreadMention : (chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId))
    readonly property double messageWidth: 1.5
    onSelectionActiveChanged: if (selectionActive === false) selection = [];

    allowedOrientations: Orientation.All

    onShareConfigurationChanged: {
        if (!shareConfiguration || shareConfiguration === null) return;

        if (shareConfiguration.mimeType.lastIndexOf("audio", 0) === 0) {
            chat.sendMusic(shareConfiguration.resources[0], 0)
        } else if (shareConfiguration.mimeType.lastIndexOf("video", 0) === 0) {
            chat.sendVideo(shareConfiguration.resources[0], 0)
        } else if (shareConfiguration.mimeType.lastIndexOf("image", 0) === 0) {
            chat.sendPhoto(shareConfiguration.resources[0], 0)
        } else {
            chat.sendFile(shareConfiguration.resources[0], 0)
        }
    }

    Connections {
        target: pageStack
        onDepthChanged: if (depth === 1) chat.clearCachedHistory()
    }

    Connections {
        target: chat
        onDraftMessageChanged: {
            textInput.text = chat.draftMessage
            textInput.text = textInput.text

            if (chat.draftMessageReplyId != 0) {
                chatPage.newReplyMessageId = chat.draftMessageReplyId
                chatPage.newReplyMessageId = chatPage.newReplyMessageId
            }
        }
    }

    onNewReplyMessageIdChanged: {
        if (chat.draftMessage !== "") {
            chat.setDraftMessage(textInput.text, chatPage.newReplyMessageId)
        }
    }


    onStatusChanged: {
        if (status === PageStatus.Deactivating) {
            chatList.closeChat(chat.id)
        }
        if (status === PageStatus.Active) {
            chatList.openChat(chat.id)

            if (!chat.isSelf) {
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

            if (messages.count < 20) {
                console.log("GETCHATHISTORY")
                chat.getChatHistory(startMessage, 40, -20)
            }
            if (chat.draftMessage !== "") {
                textInput.text = chat.draftMessage
                textInput.text = textInput.text
            }
            if (chat.draftMessageReplyId != 0) {
                console.log(chat.draftMessageReplyId)
                chatPage.newReplyMessageId = chat.draftMessageReplyId
                chatPage.newReplyMessageId = chatPage.newReplyMessageId
            }
        }
    }

    function scrollToMention() {
        messages.originMessage = Math.max(0, messages.indexAt(0, messages.contentY + messages.height - Theme.paddingSmall))
        var messageIndex = chat.getMessageIndex(chat.firstUnreadMention)
        if (messageIndex === -1) {
            messages.historyBrowsing = chat.firstUnreadMention
            chat.scrollToMessage(chat.firstUnreadMention)
        } else {
            messages.positionViewAtIndex(chatProxyModel.mapFromSource(messageIndex), ListView.SnapPosition)
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationInactive) chatList.closeChat(chat.id); else if (Qt.application.state === Qt.ApplicationActive) chatList.openChat(chat.id)
        }
    }

    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property bool sendButton: false
        property bool animatedStickers: true
        property int fontSize: Theme.fontSizeMedium
        property bool uploadHintShown: false
        Component.onCompleted: if (!uploadHintShown) hint.start()
    }

    ConfigurationGroup {
        id: chatSettings
        path: "/apps/yottagram/" + chat.id
    }

    TouchInteractionHint {
        id: hint
        loops: 3
        z: 10000
        startY: chatPage.height - textInput.height
        interactionMode: TouchInteraction.Swipe
        direction: TouchInteraction.Up
        onRunningChanged: if (!running) settings.uploadHintShown = true
    }
    InteractionHintLabel {
        z: 10001
        anchors.bottom: parent.bottom
        text: qsTr("Swipe from bottom to access upload options")
        opacity: hint.running ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation {} }
    }

    SortFilterProxyModel {
        id: chatProxyModel
        sourceModel: chat
        sorters: [
            RoleSorter { roleName: "messageId"; sortOrder: Qt.DescendingOrder}
        ]
    }

    PinnedMessages {
        id: pinnedMessages
        chatId: chat.id
        Component.onCompleted: chat.injectDependencies(pinnedMessages)
    }

    AnimationPreview {
        id: animationPreview
        anchors.fill: parent
        opacity: path !== "" ? 1 : 0
        Behavior on opacity { PropertyAnimation {} }

        visible: opacity > 0
        z: 10000
    }

    SilicaFlickable {
        id: mainFlickable
        anchors.fill: parent

        PullDownMenu {
            busy: chat.unreadMentionCount > 0

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

            MenuItem {
                text: qsTr("Scroll to first unread mention")
                visible: chat.unreadMentionCount > 0 && chat.firstUnreadMention != 0 && !chatPage.selectionActive
                onClicked: scrollToMention()
            }

            MenuItem {
                text: qsTr("Call")
                visible: chat.getChatType() === "private" && !chatPage.selectionActive
                onClicked: calls.call(chat.idFromType)
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
            avatarPhoto: chat.isSelf ? Qt.resolvedUrl("qrc:/icons/icon-s-bookmark.png") : chat.smallPhoto.localPath
        }


        Rectangle {
            id: unreadMentionsButton
            anchors.left: chatPhoto.right
            anchors.leftMargin: Theme.paddingLarge
            anchors.verticalCenter: pageHeader.verticalCenter
            color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
            width: Theme.itemSizeSmall
            height: width
            visible: chat.unreadMentionCount > 0
            radius: width*0.5

            IconButton {
                id: mentionsIcon
                anchors.fill: parent
                icon.fillMode: Image.PreserveAspectFit
                icon.source: "image://theme/icon-m-health"
                onClicked: scrollToMention()
            }
        }

        Column {
            anchors {
                right: parent.right
                left: unreadMentionsButton.visible ? unreadMentionsButton.right : chatPhoto.right
                rightMargin: Theme.paddingMedium
                verticalCenter: pageHeader.verticalCenter
            }
            rightPadding: Theme.paddingLarge

            Row {
                width: parent.width
                layoutDirection: Qt.RightToLeft
                spacing: Theme.paddingSmall

                Label {
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignRight
                    text: chat.isSelf ? qsTr("Saved messages") : chat.title
                    truncationMode: TruncationMode.Fade
                    width: parent.width - (secretChatIndicator.visible ? (Theme.paddingLarge + secretChatIndicator.width) : 0)
                    color: pageHeader.palette.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    font.family: Theme.fontFamilyHeading
                }

                Icon {
                    id: secretChatIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    source: "image://theme/icon-s-secure"
                    color: if (chat.getChatType() === "secret") chat.secretChatInfo.state === "pending" ? "yellow" : (chat.secretChatInfo.state === "ready" ? "green" : "red")
                    visible: chat.getChatType() === "secret"
                }
            }

            Label {
                text: {
                    if (chat.actionText !== "") return chat.actionText
                    var type = chat.getChatType()
                    if (type === "supergroup" || type === "channel") return qsTr("%1 members").arg(chat.supergroupFullInfo.memberCount) +
                                                                     (chat.onlineMemberCount > 0 ? ", " + qsTr("%1 online").arg(chat.onlineMemberCount) : "")
                    else if (type === "group") return qsTr("%1 members").arg(chat.basicGroupFullInfo.rowCount()) +
                                               (chat.onlineMemberCount > 0 ? ", " + qsTr("%1 online").arg(chat.onlineMemberCount) : "")
                    else if (type === "secret") return users.getUserAsVariant(chat.secretChatInfo.userId).statusText
                    else return users.getUserAsVariant(chat.idFromType).statusText
                }
                truncationMode: TruncationMode.Fade
                width: parent.width
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignRight
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
            active: pinnedMessages.id != 0
            onActiveChanged: console.log(pinnedMessages.id, pinnedMessages.id != 0)
            visible: active
            height: active ? Theme.fontSizeSmall*2 + Theme.paddingLarge : 0
        }

        Component {
            id: pinnedComponent

            PinnedMessage {
                onClosed: {
                    pinnedMessageLoader.active = false
                }
            }
        }

        SilicaFlickable {
            id: uploadFlickable
            anchors{
                top: pinnedMessageLoader.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            contentHeight: messages.height + inputItem.height
            clip: true

            onMovementEnded: {
                if (contentY <= (inputItem.height/2)) {
                    scrollToTop()
                    if (stickerPicker.visible) stickerPicker.visible = false
                    if (animationPicker.visible) animationPicker.visible = false
                } else {
                    scrollToBottom()
                }
            }

            SilicaSignalListView {
                id: messages
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: uploadFlickable.height - ((stickerPicker.visible || animationPicker.visible) ? (Theme.itemSizeLarge + inputItem.editReplyPanel) : (textInput.height + inputItem.editReplyPanel))
                width: parent.width
                verticalLayoutDirection: ListView.BottomToTop
                clip: true
                model: chatProxyModel
                cacheBuffer: 0
                enabled: initDone
                property bool initDone: false
                property bool creationDone: false
                property var chatHistoryTime: (new Date()).getTime()
                property var chatMessagesTime: (new Date()).getTime()
                property var oldContentHeight: 0
                property var historyBrowsing: 0
                property var originMessage: 0
                property var blinkMessage: -1

                Component.onCompleted: {
                    creationDone = true

                    countChanged(count)
                }

                Connections {
                    target: chat
                    onGotChatHistory: {
                        console.log("got chat history ", chat.rowCount(), messages.creationDone)
                        messages.countChanged(chat.rowCount())
                    }

                    onLastMessageIdChanged: {
                        var highlightMessage = Math.max(0, messages.indexAt(0, messages.contentY + messages.height -1))
                        var messageItem = chatProxyModel.get(highlightMessage)
                        if ((!messageItem.isRead || messageItem.containsUnreadMention) && messageItem.received && messages.initDone) {
                            if (messageItem.containsUnreadMention || messageItem.containsUnreadReply) {
                                messages.blinkMessage = highlightMessage
                                messages.blinkMessage = messages.blinkMessage
                                console.log(messages.blinkMessage)
                            }
                            chat.setMessageAsRead(messageItem.messageId)
                        }
                    }
                }

                function countChanged(chagedCount) {
                    console.log(initDone, chagedCount, creationDone, chagedCount === 1)
                    if (initDone || !creationDone) return;
                    if (chagedCount <= 1) {
                        chat.getChatHistory(chatPage.startMessage, 40, -20)
                    } else {
                        messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.startMessage)), ListView.SnapPosition)
                        initDone = true
                    }
                }

                Timer {
                    id: getHistoryTimer
                    interval: 1000
                    repeat: false
                    onTriggered: chat.getChatHistory(chat.getMessageIndex(chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId), 40, -20)
                }

                onBeforeScrollingToBottom: {
                    if (originMessage != 0) {
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
                    if (originMessage != 0) {
                        if (count != 0) originMessage = 0
                    } else if (chat.lastMessageId !== chatProxyModel.get(0).messageId) {
                        historyBrowsing = 0
                        chat.scrollToMessage(chat.unreadCount > 0 && chat.lastMessageId != 0 ? chat.lastReadInboxMessageId : chat.lastMessageId)
                    }
                }

                onContentHeightChanged: {
                    if (oldContentHeight == 0 && contentHeight != 0) {
                        if (historyBrowsing != 0) {
                            if (originMessage != 0 && chat.getMessageIndex(originMessage) !== -1) {
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

                    if (count != 0 && count < 20 && historyBrowsing == 0) {
                        chat.getChatHistory(chat.getMessageIndex(chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.latestMessageId), 40, -20)
                    }
                }

                onAtYBeginningChanged: if (atYBeginning) chat.getMoreChatHistory()
                onAtYEndChanged: if (atYEnd && chatProxyModel.get(0).messageId !== chat.lastMessageId) chat.getMoreChatMessages()

                onContentYChanged: {
                    var highlightMessage = Math.max(0, indexAt(0, contentY + height - Theme.paddingSmall))
                    var messageItem = chatProxyModel.get(highlightMessage)
                    if ((!messageItem.isRead || messageItem.containsUnreadMention) && messageItem.received && initDone) {
                        if ((messageItem.containsUnreadMention || messageItem.containsUnreadReply) && messages.blinkMessage == -1) {
                            blinkMessage = highlightMessage
                            blinkMessage = blinkMessage
                        }
                        chat.setMessageAsRead(messageItem.messageId)
                    }
                    if (((contentHeight + originY) - (contentY + height)) < Theme.itemSizeHuge*3 && ((new Date()).getTime() - chatMessagesTime) > 300 && count > 0 && chatProxyModel.get(0).messageId !== chat.lastMessageId) {
                        chatMessagesTime = (new Date()).getTime();
                        chat.getMoreChatMessages()
                    }

                    if ((contentY - originY) < 5000 && ((new Date()).getTime() - chatHistoryTime) > 300) {
                        chatHistoryTime = (new Date()).getTime();
                        chat.getMoreChatHistory()
                    }
                }

//                add: Transition {
//                    to: item.height
//                    PauseAnimation { duration: 500 }
//                    NumberAnimation { properties: "y"; duration: 200; easing.type: Easing.InOutQuad }
//                }

                delegate:
                ListItem {
                    id: listItem
                    width: chatPage.width
                    contentHeight: Math.max(message.height + Theme.paddingSmall, Theme.itemSizeExtraSmall)
                    contentWidth: width
                    highlighted: selection.indexOf(messageId) !== -1

                    Rectangle {
                        id: blinkBackground
                        anchors.fill: parent
                        color: "transparent"
                    }

                    Connections {
                        target: messages
                        onBlinkMessageChanged: {
                            if (index === messages.blinkMessage) {
                                if (!flashAnimation.running) flashAnimation.start()
                            }
                        }
                    }

                    SequentialAnimation {
                        id: flashAnimation
                        running: false
                        onRunningChanged: if (!running) messages.blinkMessage = -1
                        ColorAnimation { target: blinkBackground; property: "color"; from: Theme.rgba(Theme.highlightBackgroundColor, 0); to: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity);  duration: 500}
                        PauseAnimation { duration: 500 }
                        ColorAnimation { target: blinkBackground; property: "color"; from: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity); to: Theme.rgba(Theme.highlightBackgroundColor, 0);  duration: 500}
                    }

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
                        chatPage.newEditMessageId = 0
                        chatPage.newReplyMessageId = messageId
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

                    onPressAndHold: {
                        contextMenuLoader.active = true;
                    }

                    Loader {
                        id: contextMenuLoader
                        active: false
                        asynchronous: true
                        onStatusChanged: {
                            if(status === Loader.Ready) {
                                listItem.menu = contextMenuLoader.item;
                                listItem.openMenu();
                            }
                        }
                        sourceComponent: Component {
                            ContextMenu {
                                id: contextMenu

                                MenuItem {
                                    text: qsTr("Reply")
                                    visible: chatList.selection.length == 0
                                    onClicked: {
                                        chatPage.newEditMessageId = 0
                                        chatPage.newReplyMessageId = messageId
                                    }
                                }

                                MenuItem {
                                    text: qsTr("Pin message")
                                    visible: {
                                        if (pinnedMessages.contains(messageId)) return false

                                        var type = chat.getChatType()
                                        if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canPinMessages
                                        if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canPinMessages
                                        return chat.canPinMessages
                                    }
                                    onClicked: pageStack.push(pinMessageDialog, {messageId: messageId, priv: (type === "private" || type === "secret")})
                                }

                                MenuItem {
                                    text: qsTr("Unpin message")
                                    visible: {
                                        if (!pinnedMessages.contains(messageId)) return false

                                        var type = chat.getChatType()
                                        if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canPinMessages
                                        if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canPinMessages
                                        return chat.canPinMessages
                                    }
                                    onClicked: {
                                        chat.unpinMessage(messageId)
                                    }
                                }

                                MenuItem {
                                    text: qsTr("Edit")
                                    visible: canBeEdited && chatList.selection.length == 0
                                    onClicked: {
                                        textInput.text = messageText
                                        chatPage.newEditMessageId = messageId
                                        chatPage.newReplyMessageId = 0
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
                                    text: listItem.highlighted ? qsTr("Deselect") : qsTr("Select")
                                    visible: chatList.selection.length == 0
                                    onClicked: {
                                        if (selection.indexOf(messageId) === -1) {
                                            selection.push(messageId)
                                            if (chatPage.selectionActive === false) chatPage.selectionActive = true
                                        } else {
                                            selection.splice(selection.indexOf(messageId), 1)
                                            if (chatPage.selectionActive === true && chatPage.selection.length == 0) chatPage.selectionActive = false
                                        }
                                        listItem.highlighted = selection.indexOf(messageId) !== -1
                                        listItem.selectionChanged()
                                    }
                                }

                                MenuItem {
                                    text: qsTr("Set as read")
                                    onClicked: chat.setMessageAsRead(messageId)
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
                        visible: displayAvatar
                        userName: sender === "chat" ? chatList.getChatAsVariant(senderId).title : users.getUserAsVariant(senderId).name
                        avatarPhoto: if (sender === "chat" && chatList.getChatAsVariant(senderId).hasPhoto) chatList.getChatAsVariant(senderId).smallPhoto.localPath
                                     else if (users.getUserAsVariant(senderId).hasPhoto) users.getUserAsVariant(senderId).smallPhoto.localPath
                    }

                    Message {
                        id: message
                        width: parent.width/chatPage.messageWidth

                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: isService ? 0 : ((received && displayAvatar ? (Theme.itemSizeExtraSmall + Theme.paddingMedium + Theme.paddingMedium) : Theme.horizontalPageMargin) + (received ? 0 : parent.width/3))
                            rightMargin: isService ? 0 : (Theme.horizontalPageMargin + (received ? parent.width/3 : 0))
                        }
                    }
                }

                VerticalScrollDecorator {}
            }

            Item {
                id: inputItem
                anchors.top: messages.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: visible ? uploadMediaPanel.height + textInput.height + editReplyPanel + stickerPicker.height + animationPicker.height : 0
                property var editReplyPanel: (replyRow.visible ? replyRow.height : 0) + (editRow.visible ? editRow.height : 0) + (forwardRow.visible ? forwardRow.height : 0)
                visible: !(chat.getChatType() === "secret" && (chat.secretChatState === "closed" || chat.secretChatState === "pending"))

                Row {
                    id: forwardRow
                    anchors.top: parent.topz
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatList.selection.length != 0

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
                    anchors.top: parent.topz
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatPage.newReplyMessageId != 0

                    function getReplyData(roleName) {
                        return chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.newReplyMessageId)), 0), chatProxyModel.roleForName(roleName))
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
                            text: chatPage.newReplyMessageId == 0 ? "" : users.getUserAsVariant(replyRow.getReplyData("senderId")).name
                            truncationMode: TruncationMode.Fade
                            color: Theme.highlightColor
                        }
                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.newReplyMessageId == 0 ? "" :
                                          (replyRow.getReplyData("messageType") === "text" ? replyRow.getReplyData("messageText")
                                                                                               : replyRow.getReplyData("messageType"))
                            truncationMode: TruncationMode.Fade
                        }
                    }

                    IconButton {
                        id: cancelReplyIcon
                        icon.source: "image://theme/icon-m-cancel"
                        anchors.verticalCenter: parent.verticalCenter
                        visible: chatPage.newReplyMessageId != 0
                        onClicked: chatPage.newReplyMessageId = 0
                    }
                }

                Row {
                    id: editRow
                    anchors.bottom: textInput.top
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingLarge
                    spacing: Theme.paddingLarge
                    visible: chatPage.newEditMessageId != 0

                    function getEditData(roleName) {
                        return chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.newEditMessageId)), 0), chatProxyModel.roleForName(roleName))
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
                            text: chatPage.newEditMessageId == 0 ? "" : users.getUserAsVariant(editRow.getEditData("senderId")).name
                            truncationMode: TruncationMode.Fade
                            color: Theme.highlightColor
                        }
                        Label {
                            width: parent.width - Theme.paddingLarge
                            text: chatPage.newEditMessageId == 0 ? "" :
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
                            chatPage.newEditMessageId = 0
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
                                     chat.sendVoiceNote(audioRecorder.location, audioRecorder.getWaveform(), voiceMessageButton.duration, chatPage.newReplyMessageId)
                                     uploadFlickable.contentY = 0
                                     chatPage.newReplyMessageId = 0
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

                StickerPicker {
                    id: stickerPicker
                    height: visible ? Screen.height/2 + Theme.paddingLarge : 0
                    width: parent.width
                    anchors.top: parent.top
                    anchors.topMargin: inputItem.editReplyPanel
                    visible: false
                    page: chatPage
                    opacity: uploadFlickable.contentY/(inputItem.height- Theme.itemSizeLarge)
                    onVisibleChanged: chat.sendAction(visible ? Chat.ChoosingSticker : Chat.None)

                    onStickerFileIdChanged: {
                        chat.sendSticker(stickerFileId, chatPage.newReplyMessageId)
                        stickerPicker.visible = false
                        chatPage.newReplyMessageId = 0
                        stickerFileId = 0
                        uploadFlickable.scrollToTop()
                    }
                }

                AnimationPicker {
                    id: animationPicker
                    height: visible ? Screen.height/2 + Theme.paddingLarge : 0
                    width: parent.width
                    anchors.top: parent.top
                    visible: false
                    animationPreview: animationPreview
                    page: chatPage
                    opacity: uploadFlickable.contentY/(inputItem.height- Theme.itemSizeLarge)
                    onAnimationFileIdChanged: {
                        chat.sendAnimation(animationFileId, animationWidth, animationHeight, chatPage.newReplyMessageId)
                        animationPicker.visible = false
                        chatPage.newReplyMessageId = 0
                        animationFileId = 0
                        uploadFlickable.scrollToTop()
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
                    anchors.top: parent.top
                    anchors.topMargin: inputItem.editReplyPanel
                    anchors.right: sendButton.visible ? sendButton.left : parent.right
                    anchors.left: parent.left
                    placeholderText: qsTr("Type the text...")
                    color: Theme.primaryColor
                    focusOutBehavior: FocusBehavior.KeepFocus
                    height: visible ? Math.min(implicitHeight, Theme.itemSizeExtraLarge) : 0
                    visible: !(stickerPicker.visible || animationPicker.visible)
                    onFocusChanged: {
                        if (!focus) chat.setDraftMessage(textInput.text, chatPage.newReplyMessageId)
                        messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(chatPage.startMessage)), ListView.SnapPosition)
                    }
                    EnterKey.onClicked: {
                        if (!settings.sendButton) {
                            textInput.text = textInput.text.slice(0,textInput.cursorPosition-1) + textInput.text.slice(textInput.cursorPosition,textInput.text.length)
                            sendMessage()
                        }
                    }

                    onTextChanged: chat.sendAction(text.length > 0 ? Chat.Typing : Chat.None)

                    function sendMessage() {
                        if(textInput.text.length != 0) {
                            if (chatPage.newEditMessageId != 0) {
                                chat.editMessageText(chatPage.newEditMessageId, textInput.text)
                                chatPage.newEditMessageId = 0
                            } else {
                                chat.sendMessage(textInput.text, chatPage.newReplyMessageId)
                                chatPage.newReplyMessageId = 0
                            }
                            textInput.text = ""
                            textInput.focus = false
                            textInput.focus = true
                        }

                        if (chatList.selection.length != 0) {
                            chat.sendForwardedMessages(chatList.selection, chatList.forwardedFrom)
                            chatList.selection = []
                            chatList.forwardedFrom = 0
                        }
                    }
                }

                IconButton {
                    id: sendButton
                    anchors.top: parent.top
                    anchors.topMargin: inputItem.editReplyPanel
                    anchors.right: parent.right
                    icon.source: "image://theme/icon-m-send"
                    visible: settings.sendButton && !(stickerPicker.visible || animationPicker.visible)
                    onClicked: textInput.sendMessage()
                }

                Column {
                    id: uploadMediaPanel
                    width: parent.width
                    anchors.top: textInput.bottom
                    spacing: Theme.paddingLarge
                    padding: Theme.paddingLarge
                    visible: {
                        if (stickerPicker.visible || animationPicker.visible) return false
                        if ((type === "supergroup" || type === "channel") && chat.supergroupInfo.memberType !== "member") return chat.supergroupInfo.canSendMediaMessages
                        if (type === "group" && chat.basicGroupInfo.memberType !== "member") return chat.basicGroupInfo.canSendMediaMessages
                        return chat.canSendMediaMessages
                    }
                    height: if (!visible) 0
                    property int columns: 5
                    property real cellWidth: (width-Theme.paddingLarge*2)/columns
                    Row {
                        width: parent.width
                        id: firstRow

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
                        width: parent.width

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
                                uploadFlickable.scrollToBottom()
                            }
                        }

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("GIF")
                            source: "image://theme/icon-m-play"
                            onClicked: {
                                textInput.focus = false
                                animationPicker.visible = true
                                uploadFlickable.scrollToBottom()
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

                        HighlightLabelIconButton {
                            width: uploadMediaPanel.cellWidth
                            text: qsTr("Location")
                            source: "image://theme/icon-m-location"
                            onClicked: {
                                pageStack.push(locationPickerDialog)
                            }
                        }
                    }
                }
            }
        }

        Component {
            id: locationPickerDialog

            LocationPicker {
                id: locationPicker

                page: chatPage
            }
        }

        Component {
            id: filePickerPage
            MultiFilePickerDialog {
                allowedOrientations: Orientation.All
                title: "Select file"
                property bool doneOnce: false

                onOpened: doneOnce = false
                onAccepted: {
                    if (doneOnce === false) {
                        doneOnce = true;
                        var urls = []
                        for (var i = 0; i < selectedContent.count; ++i) {
                            chat.sendFile(selectedContent.get(i).filePath, chatPage.newReplyMessageId)
                            chatPage.newReplyMessageId = 0
                        }
                        uploadFlickable.scrollToTop()
//                        pageStack.navigateBack(PageStackAction.Immediate)
                    }
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
                    chat.sendPhotos(urls, chatPage.newReplyMessageId)
                    chatPage.newReplyMessageId = 0
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
                    chat.sendVideos(urls, chatPage.newReplyMessageId)
                    chatPage.newReplyMessageId = 0
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
                        chat.sendMusic(selectedContent.get(i).filePath, chatPage.newReplyMessageId)
                        chatPage.newReplyMessageId = 0
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
                        chat.sendFile(selectedContent.get(i).filePath, chatPage.newReplyMessageId)
                        chatPage.newReplyMessageId = 0
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
        onOpened: {
            photo.x = photo.y = 0
            photo.scale = 1
        }

        SilicaFlickable {
            anchors.fill: parent

            Image {
                id: photo
                width: chatPage.width
                height: chatPage.height
                source: bigPhoto.path
                fillMode: Image.PreserveAspectFit

                PinchArea {
                    anchors.fill: parent
                    pinch.target: parent
                    pinch.minimumScale: 1
                    pinch.maximumScale: 4

                    MouseArea{
                        anchors.fill: parent
                        drag.target: photo
                        drag.axis: Drag.XAndYAxis
                        propagateComposedEvents: true
                    }
                }
            }
        }
    }

    Dialog {
        id: bigVideo
        property string path

        allowedOrientations: Orientation.All

        SilicaFlickable {
            anchors.fill: parent

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
        property var priv
        onMessageIdChanged: notify.checked = true

        allowedOrientations: Orientation.All

        Column {
            anchors.fill: parent
            DialogHeader { }

            TextSwitch {
                id: notify
                width: parent.width
                text: pinMessageDialog.priv ? qsTr("Also pin for %1").arg(chat.title) : qsTr("Notify members")
                checked: true
            }
        }

        onDone: {
            if (result == DialogResult.Accepted) {
                chat.pinMessage(messageId, priv ? true : notify.checked, priv ? !notify.checked : false)
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
