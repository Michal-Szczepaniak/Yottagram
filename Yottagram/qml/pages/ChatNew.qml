import QtQuick 2.6
import Sailfish.Silica 1.0
import org.nemomobile.notifications 1.0
import org.nemomobile.configuration 1.0
import SortFilterProxyModel 0.2
import com.verdanditeam.user 1.0
import com.verdanditeam.audiorecorder 1.0
import "../components/functions/muteFormat.js" as TimeFormat
import "../components"

Page {
    id: chatPage
    property var chat: null
    property variant selection: []
    property bool selectionActive: false

    allowedOrientations: Orientation.All

    Connections {
        target: Qt.application
        onStateChanged: if (Qt.application.state === Qt.ApplicationInactive) chatList.closeChat(chat.id); else if (Qt.application.state === Qt.ApplicationActive) chatList.openChat(chat.id)
    }

    onStatusChanged: {
        if (status === PageStatus.Deactivating) chatList.closeChat(chat.id)
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
                visible: chat.unreadMentionCount > 0 && chat.firstUnreadMention !== 0 && !chatPage.selectionActive
                onClicked: scrollToMention()
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
                    width: parent.width - (secretChatIndicator.visible ? (Theme.paddingSmall + secretChatIndicator.width) : 0)
                    color: pageHeader.palette.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    font.family: Theme.fontFamilyHeading
                }

                Icon {
                    id: secretChatIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    source: "image://theme/icon-s-secure"
                    color: chat.secretChatInfo.state === "pending" ? "yellow" : (chat.secretChatInfo.state === "ready" ? "green" : "red")
                    visible: chat.getChatType() === "secret"
                }
            }

            Label {
                text: {
                    var type = chat.getChatType()
                    if (type === "supergroup" || type === "channel") return qsTr("%1 members").arg(chat.supergroupFullInfo.memberCount)
                    else if (type === "group") return qsTr("%1 members").arg(chat.basicGroupFullInfo.rowCount())
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
            active: chat.pinnedMessageId !== 0 && chat.pinnedMessageId !== chatSettings.pinnedMessageId && typeof chat.pinnedMessage !== "undefined"
            visible: active
            height: active ? Theme.fontSizeMedium*2 : 0

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
            anchors{
                top: pinnedMessageLoader.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
//            contentHeight: messages.height + inputItem.height
            clip: true

//            onMovementEnded: {
//                if (contentY <= (inputItem.height/2)) {
//                    scrollToTop()
//                    if (stickerPicker.visible) stickerPicker.visible = false
//                } else {
//                    scrollToBottom()
//                }
//            }

            SilicaSignalListView {
                id: messages
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: uploadFlickable.height - (stickerPicker.visible ? (Theme.itemSizeLarge + inputItem.editReplyPanel) : (textInput.height + inputItem.editReplyPanel))
                verticalLayoutDirection: ListView.BottomToTop
                clip: true
                model: chatProxyModel
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
                    height: if (!visible) 0
                    visible: !stickerPicker.visible
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
                    visible: {
                        if (stickerPicker.visible) return false
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
    }
}
