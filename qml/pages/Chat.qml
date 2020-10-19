import QtQuick 2.6
import Sailfish.Silica 1.0
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
//        bubble.canvas.requestPaint()
        if (status === PageStatus.Deactivating) chatList.closeChat(chat.id)
        if (status === PageStatus.Active) {
            chatList.openChat(chat.id)
            chat.getChatHistory(0)

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

    function addZero(i) {
        if (i < 10) {
            i = "0" + i;
        }
        return i;
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
                    onClicked: chat.clearHistory()
                    visible: !chatPage.selectionActive
                }

                MenuItem {
                    text: qsTr("Open secret chat")
                    visible: chat.getChatType() === "private" && !chatPage.selectionActive
                    onClicked: chat.openSecretChat()
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
                    font.pixelSize: Theme.fontSizeExtraSmall
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
//                onAvatarPhotoChanged: if (chat.smallPhoto && !chat.smallPhoto.isDownloading && !chat.smallPhoto.isDownloaded) chat.smallPhoto.download()
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
                sourceComponent: if (chat.pinnedMessageId !== 0) pinnedComponent
            }

            Component {
                id: pinnedComponent

                PinnedMessage { }
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
                    spacing: Theme.paddingSmall
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: uploadFlickable.height - (stickerPicker.visible ? (Theme.itemSizeLarge + inputItem.editReplyPanel) : (textInput.height + inputItem.editReplyPanel))
                    width: parent.width
                    verticalLayoutDirection: ListView.BottomToTop
                    clip: true
                    model: chatProxyModel
                    cacheBuffer: 0

                    onCountChanged: if(count < 10) chat.getChatHistory(0)

                    onAtYBeginningChanged: if (atYBeginning) chat.getMoreChatHistory()
                    onAtYEndChanged: contentY--
                    Component.onCompleted: contentY--

                    onContentYChanged: if ((contentY - originY) < 5000) chat.getMoreChatHistory()

                    delegate:
                    ListItem {
                        id: item
                        width: parent.width
                        contentHeight: Math.max(column.height, Theme.itemSizeMedium)
                        contentWidth: width
                        property bool displayAvatar: received && (type === "group" || type === "supergroup")
                        readonly property var user: users.getUserAsVariant(authorId)
                        readonly property bool isService: messageType == "chatSetTtl" || messageType == "pinMessage" || messageType == "messageChatDeleteMember" || messageType == "messageChatAddMembers" || messageType == "messageChatJoinByLink"
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

                        Component.onCompleted: {
                            if (!isRead && received) chat.setMessageAsRead(messageId)
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

        //                MessageBubble {
        //                    id: bubble
        //                    anchors.fill: parent
        //                    leftSide: received
        //                    source: "qrc:///icons/icons/corner.svg"
        //                    color: Theme.highlightBackgroundColor
        //                    bubbleOpacity: Theme.highlightBackgroundOpacity
        //                }

                        Avatar {
                            id: avatarPhoto
                            width: Theme.itemSizeExtraSmall
                            height: width
                            anchors.left: parent.left
                            anchors.leftMargin: Theme.horizontalPageMargin
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: Theme.paddingMedium
                            visible: displayAvatar && chat.getAuthorByIndex(chatProxyModel.mapToSource(index-1)) !== authorId
                            userName: user.name
                            avatarPhoto: if (user && user.hasPhoto) user.smallPhoto
                        }

                        Column {
                            id: column
                            spacing: 0
                            padding: Theme.paddingSmall
                            anchors.left: if (received) parent.left
                            anchors.leftMargin: Theme.horizontalPageMargin + (displayAvatar ? (Theme.itemSizeExtraSmall + Theme.paddingMedium) : 0)
                            anchors.right: if (!received) parent.right
                            anchors.rightMargin: if (!received) Theme.horizontalPageMargin

                            Label {
                                id: name
                                text: !isForwarded ? (chat.getChatType() === "channel" ? chat.title : user.name) : qsTr("Forwarded from %1").arg(getName())
                                font.pixelSize: Theme.fontSizeMedium
                                horizontalAlignment: received ? Text.AlignLeft : Text.AlignRight
                                color: Theme.highlightColor
                                visible: ((displayAvatar && chat.getAuthorByIndex(chatProxyModel.mapToSource(index+1)) !== authorId) ||  isForwarded || chat.getChatType() === "channel") && !serviceMessage.visible

                                function getName() {
                                    if (forwardUserId !== 0) return users.getUserAsVariant(forwardUserId).name;
                                    if (forwardUsername !== "") return forwardUsername;
                                    if (forwardChannelId !== 0) return chatList.getChatAsVariant(forwardChannelId).title
                                }
                            }

                            Loader {
                                id: replyLoader
                                asynchronous: true
                                sourceComponent: if (replyMessageId !== 0) replyComponent
                            }

                            Component {
                                id: replyComponent

                                Row {
                                    id: replyMessage
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
                                            text: replyMessageId !== 0 ? users.getUserAsVariant(replyMessage.getReplyData("authorId")).name : ""
                                            truncationMode: TruncationMode.Fade

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(replyMessageId)), ListView.SnapPosition)
                                            }
                                        }
                                        Label {
                                            id: replyText
                                            text: replyMessageId === 0 ? "" :
                                                      (replyMessage.getReplyData("messageType") === "text" ? replyMessage.getReplyData("messageText").trim().replace(/\r?\n|\r/g, " ")
                                                                                                           : replyMessage.getReplyData("messageType"))
                                            truncationMode: TruncationMode.Fade

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: messages.positionViewAtIndex(chatProxyModel.mapFromSource(chat.getMessageIndex(replyMessageId)), ListView.SnapPosition)
                                            }
                                        }
                                    }

                                    states: [
                                        State {
                                            name: "wide reply"
                                            when: replyText.text.length > 28
                                            PropertyChanges {
                                                target: replyMessage
                                                width: chatPage.width/chatPage.messageWidth
                                            }
                                            PropertyChanges {
                                                target: replyText
                                                width: chatPage.width/chatPage.messageWidth - Theme.paddingLarge
                                            }
                                        }
                                    ]

                                    function getReplyData(roleName) {
                                        var result = chatProxyModel.data(chatProxyModel.index(chatProxyModel.mapFromSource(chat.getMessageIndex(replyMessageId)), 0), chatProxyModel.roleForName(roleName))
                                        if (result === void(0)) return "";
                                        return result
                                    }
                                }
                            }

                            Loader {
                                id: contentLoader
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
                                width: chatPage.width/chatPage.messageWidth
                                horizontalAlignment: Text.AlignHCenter
                                text: messageText.trim()
                                color: Theme.highlightColor
                                visible: isService
                            }

                            LinkedLabel {
                                id: textField
                                plainText: messageText.trim()
                                font.pixelSize: Theme.fontSizeMedium
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                color: Theme.primaryColor
                                linkColor: Theme.highlightColor
                                visible: messageText !== "" && !serviceMessage.visible
                            }

                            Text {
                                id: dummy_text
                                text: messageText.trim()
                                width: messageType === "photo" ? contentLoader.width: undefined
                                font.pixelSize: Theme.fontSizeMedium
                                visible: false
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
                                            truncationMode: TruncationMode.Fade

                                            MouseArea {
                                                anchors.fill: parent
                                            }
                                        }

                                        Label {
                                            id: webPageDescription
                                            text: webPage.description
                                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                                            maximumLineCount: 5

                                            MouseArea {
                                                anchors.fill: parent
                                            }
                                        }
                                    }

                                    states: [
                                        State {
                                            name: "wide reply"
                                            when: webPageDescription.text.length > 28
                                            PropertyChanges {
                                                target: webPageDescription
                                                width: chatPage.width/chatPage.messageWidth
                                            }
                                            PropertyChanges {
                                                target: webPageDescription
                                                width: chatPage.width/chatPage.messageWidth - Theme.paddingLarge
                                            }
                                        }
                                    ]
                                }
                            }

                            Row {
        //                        width: Math.max(textField.width, replyLoader.width, contentLoader.width, time.width + readIcon.width)
                                anchors.right: if (!received) parent.right
                                anchors.left: if (received) parent.left
                                spacing: Theme.paddingSmall
                                visible: !serviceMessage.visible
                                Label {
                                    id: time
                                    text: (edited ? qsTr("Edited") + " ": "") + timestamp
        //                            horizontalAlignment: received ? Text.AlignLeft : Text.AlignRight
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

                            states: [
                                State {
                                    name: "wide text"
                                    when: dummy_text.text.length > 30
                                    PropertyChanges {
                                        target: textField
                                        width: chatPage.width/chatPage.messageWidth
                                        height: textField.paintedHeight
                                    }
                                },
                                State {
                                    name: "not wide text"
                                    when: dummy_text.text.length <= 30
                                    PropertyChanges {
                                        target: textField
                                        width: dummy_text.paintedWidth
                                        height: dummy_text.paintedHeight
                                    }
                                }
                            ]
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
                        anchors.top: parent.top
                        anchors.topMargin: inputItem.editReplyPanel
                        anchors.right: voiceMessageButton.left
                        anchors.left: parent.left
                        placeholderText: qsTr("Type the text...")
                        color: Theme.primaryColor
                        focusOutBehavior: FocusBehavior.KeepFocus
                        height: if (!visible) 0
                        visible: !stickerPicker.visible
                        EnterKey.onClicked: {
                            if(textInput.text.length !== 0) {
                                if (chatPage.editMessageId !== 0) {
                                    chat.editMessageText(chatPage.editMessageId, textInput.text.replace('\n', ''))
                                    chatPage.editMessageId = 0
                                    textInput.text = ""
                                    textInput.focus = false
                                    textInput.focus = true
                                } else {
                                    chat.sendMessage(textInput.text.replace('\n', ''), chatPage.replyMessageId)
                                    textInput.text = ""
                                    textInput.focus = false
                                    textInput.focus = true
                                    chatPage.replyMessageId = 0
                                }
                            }

                            if (chatList.selection.length !== 0) {
                                chat.sendForwardedMessages(chatList.selection, chatList.forwardedFrom)
                                chatList.selection = []
                                chatList.forwardedFrom = 0
                            }
                        }
                    }

                    IconButton {
                        id: voiceMessageButton
                        anchors.top: parent.top
                        anchors.topMargin: inputItem.editReplyPanel
                        anchors.right: parent.right
                        icon.source: "image://theme/icon-m-mic"
                        onPressed: audioRecorder.startRecording()
                        visible: !stickerPicker.visible
                        property var duration
                        onReleased: {
                            duration = audioRecorder.duration
                            audioRecorder.stopRecording()
                        }
                    }

                    Column {
                        id: uploadMediaPanel
                        width: parent.width
                        anchors.top: textInput.bottom
                        spacing: Theme.paddingLarge
                        padding: Theme.paddingLarge
                        visible: !stickerPicker.visible
                        height: if (!visible) 0
                        property int columns: 5
                        property real cellWidth: (width-Theme.paddingLarge*2)/columns
                        Row {
                            width: parent.width
                            id: firstRow

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Image"
                                source: "image://theme/icon-m-file-image"
                                onClicked: pageStack.push(imagePickerPage)
                            }

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Video"
                                source: "image://theme/icon-m-file-video"
                                onClicked: pageStack.push(videoPickerPage)
                            }

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Audio"
                                source: "image://theme/icon-m-file-audio"
                                onClicked: pageStack.push(musicPickerPage)
                            }

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Document"
                                source: "image://theme/icon-m-file-document-light"
                                onClicked: pageStack.push(documentPickerPage)
                            }

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "File"
                                source: "image://theme/icon-m-file-other-light"
                                onClicked: pageStack.push(filePickerPage)
                            }
                        }
                        Row {
                            width: parent.width

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Poll"
                                source: "image://theme/icon-m-events"
                                onClicked: pageStack.push(pollDialog)
                                visible: chat.getChatType() !== "private"
                            }

                            HighlightLabelIconButton {
                                width: uploadMediaPanel.cellWidth
                                text: "Sticker"
                                source: "image://theme/icon-m-other"
                                onClicked: {
                                    textInput.focus = false
                                    stickerPicker.visible = true
                                    uploadFlickable.scrollToBottom()
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

        Image {
            anchors.fill: parent
            source: parent.path
            fillMode: Image.PreserveAspectFit
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
