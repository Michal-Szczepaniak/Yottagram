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
import SortFilterProxyModel 0.2
import org.nemomobile.dbus 2.0
import QtGraphicalEffects 1.0
import "../components"
import "../components/functions/chatFolderIconHelper.js" as ChatFolderIconHelper

Page {
    id: page

    allowedOrientations: Orientation.All
    property int activeChatList: 0
    property var shareConfiguration: null

    DBusAdaptor {
        id: shareDBusInterface
        service: "com.verdanditeam.yottagram"
        path: "/"
        iface: "com.verdanditeam.yottagram"
        xml: '<interface name="com.verdanditeam.yottagram">
                  <method name="share"> <arg type="a" name="configuration" direction="in"/> </method>
                  <method name="openChat"> <arg type="x" name="chatId" direction="in" /> </method>
                  <method name="openApp" />
                  <method name="ping"> <arg name="id" direction="out" type="b"> </method>
              </interface>'

        function share(configuration) {
            page.shareConfiguration = configuration
            pageStack.pop(page, PageStackAction.Immediate)
            app.activate()
        }

        function openChat(chatId) {
            app.activate()
            var chat = chatList.openChat(chatId)
            if (typeof chat === "undefined" || !chat) return;
            pageStack.pop(page, PageStackAction.Immediate)
            pageStack.push(Qt.resolvedUrl("Chat.qml"), { chat: chat })
        }

        function openApp() {
            app.activate()
        }

        function ping() {
            return true;
        }
    }

    Connections {
        target: authorization
        onWaitingForPhoneNumber: {
            pageStack.replace(Qt.resolvedUrl("AuthorizationNumber.qml"));
        }

        onWaitingForCode: {
            pageStack.replace(Qt.resolvedUrl("AuthorizationCode.qml"));
        }

        onWaitingForPassword: {
            pageStack.replace(Qt.resolvedUrl("AuthorizationPassword.qml"));
        }

        onIsAuthorizedChanged: {
            if (!authorization.isAuthorized) {
                pageStack.replace(Qt.resolvedUrl("AuthorizationNumber.qml"));
            }
        }
    }

    PageBusyIndicator {
        id: loadingIndicator
        running: !telegramStatus.bootupComplete

        Label {
            anchors.top: parent.bottom
            anchors.topMargin: Theme.paddingLarge
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Loading")
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings.qml"))
            }

            MenuItem {
                text: qsTr("Contacts")
                onClicked: pageStack.push(Qt.resolvedUrl("Contacts.qml"))
            }

            MenuItem {
                text: activeChatList == 0 ? qsTr("Archive") : qsTr("Main")
                onClicked: {
                    activeChatList = activeChatList == 0 ? 1 : 0
                    chatList.switchChatList(activeChatList)
                }
            }
        }

        SilicaListView {
            id: chatListFilterList
            width: parent.width
            height: Theme.itemSizeExtraLarge
            model: chatListFilters
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            anchors.top: parent.top
            visible: count > 0
            delegate: BackgroundItem {
                width: Theme.itemSizeLarge
                height: Theme.itemSizeExtraLarge
                clip: true

                Icon {
                    id: icon
                    source: ChatFolderIconHelper.getIcon(iconName)
                    asynchronous: true
                    cache: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: Theme.paddingMedium
                }

                Label {
                    text: name
                    font.pixelSize: Theme.fontSizeExtraSmall
                    anchors.top: icon.bottom
                    anchors.topMargin: Theme.paddingSmall
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Theme.itemSizeMedium
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    lineHeight: 0.8
                }

                onClicked: {
                    activeChatList = 2;
                    chatList.switchChatList(2, id)
                }
            }
        }

        SearchField {
            id: searchField
            width: parent.width
            anchors.top: chatListFilterList.visible ? chatListFilterList.bottom : parent.top
            placeholderText: qsTr("Search")
        }

        SortFilterProxyModel {
            id: chatListProxyModel
            sourceModel: chatList
            sorters: [
                RoleSorter { roleName: "order"; sortOrder: Qt.DescendingOrder }
            ]
            filters: [
                ValueFilter {
                    roleName: "order"
                    value: false
                    inverted: true
                },
                RegExpFilter {
                    roleName: "name"
                    pattern: searchField.text
                    caseSensitivity: Qt.CaseInsensitive
                    enabled: searchField.text !== ""
                }
            ]
        }

        SilicaListView {
            id: listView
            anchors.top: searchField.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            spacing: 0
            model: page.status === PageStatus.Inactive ? null : chatListProxyModel
            contentHeight: Theme.itemSizeLarge * count
            delegate: ListItem {
                id: listItem
                contentHeight: Theme.itemSizeLarge
                contentWidth: parent.width

                onPressAndHold: {
                    contextMenuLoader.active = true;
                }

                Loader {
                    id: contextMenuLoader
                    active: false
                    asynchronous: true
                    onStatusChanged: {
                        if(status === Loader.Ready) {
                            listItem.menu = item;
                            listItem.openMenu();
                        }
                    }
                    sourceComponent: Component {
                        ContextMenu {
                            id: contextMenu

                            MenuItem {
                                text: isPinned ? qsTr("Unpin from top") : qsTr("Pin to top")
                                onClicked: {
                                    chatList.togglePinnedChat(id)
                                }
                            }

                            MenuItem {
                                text: qsTr("Set as read")
                                onClicked: {
                                    chatList.markChatAsRead(id)
                                }
                            }

                            MenuItem {
                                text: qsTr("Delete chat")

                                onClicked: {
                                    var list = chatList
                                    var chatId = id
                                    remorseAction("Deleting", function() {
                                        var chat = list.getChatAsVariant(chatId)
                                        chat.clearHistory(true)
                                        if (chat.getChatType() === "secret") chat.closeSecretChat()
                                    })
                                }
                            }
                        }
                    }
                }

                Avatar {
                    id: avatar
                    userName: name
                    avatarPhoto: (hasPhoto && !isSelf) ? photo.localPath : (isSelf ? Qt.resolvedUrl("qrc:/icons/icon-s-bookmark.png") : undefined)
                    forceBackground: isSelf
                    maskEnabled: true
                    height: Theme.itemSizeLarge - Theme.paddingMedium*2
                    width: height
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.topMargin: Theme.paddingMedium
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: avatar.right
                    anchors.leftMargin: Theme.paddingLarge
                    anchors.right: unread.left
                    anchors.rightMargin: Theme.paddingMedium

                    Row {
                        spacing: Theme.paddingSmall
                        width: parent.width

                        Icon {
                            id: secretChatIndicator
                            anchors.verticalCenter: parent.verticalCenter
                            source: "image://theme/icon-s-secure"
                            asynchronous: true
                            cache: true
                            color: secretChatState === "pending" ? "yellow" : (secretChatState === "ready" ? "green" : "red")
                            visible: secretChatState !== ""
                        }

                        Label {
                            textFormat: Text.PlainText
                            anchors.verticalCenter: parent.verticalCenter
                            text: name
                            truncationMode: TruncationMode.Fade
                            width: parent.width - (secretChatIndicator.visible ? (Theme.paddingSmall + secretChatIndicator.width) : 0)
                        }
                    }

                    Label {
                        id: lastMessageAuthorLabel
                        text: lastMessageAuthor + "<font color='" + Theme.secondaryColor + "'>" + lastMessage
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.highlightColor
                        truncationMode: TruncationMode.Fade
                        width: parent.width
                    }
                }

                Column {
                    id: unread
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingMedium
                    spacing: Theme.paddingMedium

                    Row {
                        spacing: Theme.paddingSmall

                        Icon {
                            id: readIcon
                            width: Theme.iconSizeExtraSmall
                            height: Theme.iconSizeExtraSmall
                            anchors.verticalCenter: parent.verticalCenter
                            fillMode: Image.PreserveAspectFit
                            cache: true
                            asynchronous: true
                            sourceSize.width: Theme.iconSizeExtraSmall
                            sourceSize.height: Theme.iconSizeExtraSmall
                            source: Qt.resolvedUrl(isRead ? "qrc:/icons/icon-s-read.png" : "qrc:/icons/icon-s-sent.png")
                        }

                        Label {
                            id: time
                            text: lastMessageTimestamp
                            anchors.verticalCenter: parent.verticalCenter
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        spacing: Theme.paddingSmall

                        Icon {
                            id: pinnedIcon
                            width: Theme.iconSizeExtraSmall
                            height: Theme.iconSizeExtraSmall
                            anchors.verticalCenter: parent.verticalCenter
                            fillMode: Image.PreserveAspectFit
                            source: Qt.resolvedUrl("qrc:/icons/icon-s-pinned.png")
                            opacity: 0.3
                            visible: unreadCount == 0 && isPinned
                        }

                        Label {
                            text: unreadCount > 0 ? unreadCount : " "
                            font.pixelSize: Theme.fontSizeSmall
                            height: implicitHeight
                            visible: unreadCount > 0 || !isPinned
                        }

                        Rectangle {
                            color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
                            width: Theme.iconSizeSmall
                            height: Theme.iconSizeSmall
                            anchors.verticalCenter: parent.verticalCenter
                            visible: unreadMentionCount > 0
                            radius: width*0.5

                            Icon {
                                id: mentionsIcon
                                anchors.fill: parent
                                fillMode: Image.PreserveAspectFit
                                source: "qrc:/icons/icon-splus-ping.png"
                            }
                        }
                    }
                }

                onClicked: {
                    var chat = chatList.getChatAsVariant(id)
                    var configuration = page.shareConfiguration
                    configuration = configuration
                    page.shareConfiguration = null

                    if (type === "supergroup" && chat.supergroupInfo.isForum) {
                        chat.fetchTopics()
                        pageStack.push(Qt.resolvedUrl("../dialogs/TopicPickerDialog.qml"), { chat: chat, shareConfiguration: configuration })
                    } else {
                        pageStack.push(Qt.resolvedUrl("Chat.qml"), { chat: chat, shareConfiguration: configuration })
                    }
                }
            }
        }
    }
}
