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
        if (status === PageStatus.Deactivating) chatList.closeChat(chat.id)
        if (status === PageStatus.Active) {
            chatList.openChat(chat.id)

            switch (chat.getChatType()) {
            case "private":
            case "secret":
                pageStack.pushAttached(Qt.resolvedUrl("../components/chatInfo/UserInfo.qml"), {chat: chat})
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

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: pageComponent
        asynchronous: true

        BusyIndicator {
            size: BusyIndicatorSize.Large
            anchors.centerIn: parent
            running: pageLoader.status != Loader.Ready
        }
    }

    Component {
        id: pageComponent
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
                avatarPhoto: chat.smallPhoto
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
                    color: chat.secretChatState === "pending" ? "yellow" : (chat.secretChatState === "ready" ? "green" : "red")
                    visible: chat.secretChatState !== ""
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
                    text: (chat.getChatType() === "supergroup" || chat.getChatType() === "channel") ? qsTr("%1 members").arg(chat.supergroupFullInfo.memberCount) :
                                                                                                             (chat.getChatType() === "group") ? qsTr("%1 members").arg(chat.basicGroupFullInfo.rowCount()) :
                                                                                                                                                users.getUserAsVariant(chat.idFromType).statusText
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
                contentHeight: messages.height + inputItem.height
                clip: true

                onMovementEnded: {
                    if (contentY <= (inputItem.height/2)) {
                        scrollToTop()
                        if (stickerPicker.visible) stickerPicker.visible = false
                    } else {
                        scrollToBottom()
                    }
                }

                SilicaListView {
                    id: messages
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: uploadFlickable.height - (stickerPicker.visible ? (Theme.itemSizeLarge + inputItem.editReplyPanel) : (textInput.height + inputItem.editReplyPanel))
                    width: parent.width
                    verticalLayoutDirection: ListView.BottomToTop
                    clip: true
                    model: chatProxyModel
                    cacheBuffer: 0
                    property var chatHistoryTime: (new Date()).getTime()
                    property var chatMessagesTime: (new Date()).getTime()
                    property var oldCount: 0

                    onCountChanged: {
                        if (oldCount === 0 && count !== 0) {
                            console.log((chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.lastMessageId))
                            messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex((chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.lastMessageId))), ListView.SnapPosition)
                            oldCount = count
                        }
                        if (count < 20) chat.getChatHistory(Math.max(chat.lastReadInboxMessageId, chat.lastReadOutboxMessageId), 40, -20)

                        if (atYEnd) {
                            var item = chatProxyModel.get(indexAt(0, contentY + height - Theme.paddingSmall))
                            if (!item.isRead && item.received) chat.setMessageAsRead(item.messageId)
                        }
                    }

                    onAtYBeginningChanged: if (atYBeginning) chat.getMoreChatHistory()

                    Component.onCompleted: {
                        chat.getChatHistory((chat.unreadCount > 0 ? chat.lastReadInboxMessageId : chat.lastMessageId), 40, -20)
                    }

                    onContentYChanged: {
                        var item = chatProxyModel.get(indexAt(0, contentY + height - Theme.paddingSmall))
                        if (!item.isRead && item.received) chat.setMessageAsRead(item.messageId)
                        if (((contentHeight + originY) - (contentY + height)) < Theme.itemSizeHuge && ((new Date()).getTime() - chatMessagesTime) > 300) {
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
                        width: parent.width
                        contentHeight: Math.max(column.height + Theme.paddingSmall, Theme.itemSizeExtraSmall)
                        contentWidth: width
                        readonly property var user: users.getUserAsVariant(authorId)
                        readonly property bool isService: messageType == "chatSetTtl" || messageType == "pinMessage" || messageType == "messageChatDeleteMember" || messageType == "messageChatAddMembers" || messageType == "messageChatJoinByLink"
                        property bool displayAvatar: received && (type === "group" || type === "supergroup") && !isService
                        highlighted: selection.indexOf(messageId) !== -1

                        onClicked: {
                            if (!chatPage.selectionActive) return
                            if (selection.indexOf(messageId) === -1) {
                                selection.push(messageId)
                            } else {
                                selection.splice(selection.indexOf(messageId), 1)
                                if (chatPage.selectionActive === true && chatPage.selection.length == 0) chatPage.selectionActive = false
                            }
                            highlighted = selection.indexOf(messageId) !== -1
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
                                    text: qsTr("Edit")
                                    visible: chat.canBeEdited && chatList.selection.length === 0
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
                                    text: qsTr("Delete")
                                    visible: chat.canBeDeleted
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

                                MenuItem {
                                    text: qsTr("Pin message")
                                    visible: chat.canPinMessages && messageId !== chat.pinnedMessageId
                                    onClicked: pageStack.push(pinMessageDialog, {messageId: messageId})
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

                                        Label {
                                            id: replyName
                                            text: replyMessageId !== 0 ? users.getUserAsVariant(chat.getMessage(replyMessageId).senderUserId).name : ""
                                            font.bold: true
                                            font.pixelSize: Theme.fontSizeSmall
                                            truncationMode: TruncationMode.Fade

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(replyMessageId)), ListView.SnapPosition)
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
                                                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(replyMessageId)), ListView.SnapPosition)
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
                                        if (file.biggestPhotoSize.width === 0) return Theme.itemSizeLarge
                                        return Math.min(Theme.itemSizeHuge, file.biggestPhotoSize.width)
                                    case "sticker":
                                        return Theme.itemSizeHuge*1.5
                                    case "video":
                                        if (file.size.width === 0) return Theme.itemSizeHuge
                                        return Math.min(Theme.itemSizeHuge, file.size.width)
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
                                        if (file.biggestPhotoSize.height === 0) return Theme.itemSizeLarge
                                        return width * file.biggestPhotoSize.height/file.biggestPhotoSize.width;
                                    case "sticker":
                                        return Theme.itemSizeHuge*1.5
                                    case "video":
                                        if (file.size.height === 0) return Theme.itemSizeHuge
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
                                        return imageComponent;
                                    case "sticker":
                                        return stickerComponent;
                                    case "video":
                                        return videoComponent;
                                    case "animation":
                                        return animationComponent;
                                    case "audio":
                                        return audioComponent;
                                    case "voiceNote":
                                        return voicenoteComponent;
                                    case "videoNote":
                                        return videonoteComponent;
                                    case "document":
                                        return documentComponent;
                                    }
                                }
                            }

        //                    LottieAnimation {
        //                        id: animatedSticker
        //                        source:  sticker
        //                        visible: (sticker !== "" && stickerIsAnimated)
        //                        width: chatPage.width/chatPage.messageWidth
        ////                        running: true
        //                        fillMode: Image.PreserveAspectFit
        //                        loops: Animation.Infinite
        //                        speed: 0.001
        //                        clearBeforeRendering: false

        //                        MouseArea {
        //                            anchors.fill: parent

        //                            onClicked: animatedSticker.start()
        //                        }
        //                    }

                            Component {
                                id: imageComponent

                                ImageContent { }
                            }

                            Component {
                                id: stickerComponent

                                StickerContent { }
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
                                font.pixelSize: Theme.fontSizeMedium
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                width: Math.min(parent.width, implicitWidth)
                                anchors.right: if (!received) parent.right
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
                                anchors.right: if (!received) parent.right
                                anchors.left: if (received) parent.left
                                spacing: Theme.paddingSmall
                                visible: !serviceMessage.visible
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
                                    source: isRead ? "image://theme/icon-s-checkmark" : "image://theme/icon-s-time"
                                }
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
                    height: visible ? uploadMediaPanel.height + textInput.height + editReplyPanel + stickerPicker.height : 0
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
                        anchors.top: parent.topz
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
                        anchors.bottom: textInput.top
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
                                              (replyRow.getEditData("messageType") === "text" ? replyRow.getEditData("messageText")
                                                                                                   : replyRow.getEditData("messageType"))
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

                    StickerPicker {
                        id: stickerPicker
                        height: visible ? Screen.height/2 + Theme.paddingLarge : 0
                        width: parent.width
                        anchors.top: parent.top
                        anchors.topMargin: inputItem.editReplyPanel
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

                    TextArea {
                        id: textInput
                        enabled: chat.canSendMessages
                        anchors.top: parent.top
                        anchors.topMargin: inputItem.editReplyPanel
                        anchors.right: sendButton.visible ? sendButton.left : parent.right
                        anchors.left: parent.left
                        placeholderText: qsTr("Type the text...")
                        color: Theme.primaryColor
                        focusOutBehavior: FocusBehavior.KeepFocus
                        height: if (!visible) 0
                        visible: !stickerPicker.visible
                        EnterKey.onClicked: {
                            textInput.text = textInput.text.slice(0,textInput.cursorPosition-1) + textInput.text.slice(textInput.cursorPosition,textInput.text.length)
                            if (!settings.sendButton) sendMessage()
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
                        anchors.top: parent.top
                        anchors.topMargin: inputItem.editReplyPanel
                        anchors.right: parent.right
                        icon.source: "image://theme/icon-m-send"
                        visible: settings.sendButton && !stickerPicker.visible
                        onClicked: textInput.sendMessage()
                    }

                    Column {
                        id: uploadMediaPanel
                        width: parent.width
                        anchors.top: textInput.bottom
                        spacing: Theme.paddingLarge
                        padding: Theme.paddingLarge
                        visible: !stickerPicker.visible && chat.canSendMediaMessages
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
                                visible: chat.getChatType() !== "private" && chat.canSendPolls
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
                    }
                }
            }

            Component {
                id: filePickerPage
                FilePickerPage {
                    allowedOrientations: Orientation.All
                    title: "Select file"
                    onSelectedContentPropertiesChanged: {
                        chat.sendFile(selectedContentProperties.filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                        uploadFlickable.scrollToTop()
                    }
                }
            }

            Component {
                id: imagePickerPage
                ImagePickerPage {
                    allowedOrientations: Orientation.All
                    title: "Select image"
                    onSelectedContentPropertiesChanged: {
                        chat.sendPhoto(selectedContentProperties.filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                        uploadFlickable.scrollToTop()
                    }
                }
            }

            Component {
                id: videoPickerPage
                VideoPickerPage {
                    allowedOrientations: Orientation.All
                    title: "Select video"
                    onSelectedContentPropertiesChanged: {
                        chat.sendVideo(selectedContentProperties.filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                        uploadFlickable.scrollToTop()
                    }
                }
            }

            Component {
                id: musicPickerPage
                MusicPickerPage {
                    allowedOrientations: Orientation.All
                    title: "Select music"
                    onSelectedContentPropertiesChanged: {
                        chat.sendMusic(selectedContentProperties.filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                        uploadFlickable.scrollToTop()
                    }
                }
            }

            Component {
                id: documentPickerPage
                DocumentPickerPage {
                    allowedOrientations: Orientation.All
                    title: "Select document"
                    onSelectedContentPropertiesChanged: {
                        chat.sendFile(selectedContentProperties.filePath, chatPage.replyMessageId)
                        chatPage.replyMessageId = 0
                        uploadFlickable.scrollToTop()
                    }
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

        Video {
            id: video
            anchors.fill: parent
            source: parent.path
            autoPlay: true
            fillMode: Image.PreserveAspectFit
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
