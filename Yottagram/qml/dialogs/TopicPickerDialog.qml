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
import SortFilterProxyModel 0.2
import com.verdanditeam 1.0
import "../components"

Dialog {
    id: dialog

    property var chat: null
    property var shareConfiguration: null

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        DialogHeader {
            id: dialogHeader
            title: qsTr("Pick topic")
        }

        SortFilterProxyModel {
            id: topicListProxyModel
            sourceModel: chat.getTopicModel()
            sorters: [
                RoleSorter { roleName: "isPinned"; sortOrder: Qt.DescendingOrder },
                RoleSorter { roleName: "order"; sortOrder: Qt.DescendingOrder }
            ]
            filters: [
                ValueFilter {
                    roleName: "order"
                    value: false
                    inverted: true
                }
            ]
        }


        SilicaListView {
            anchors {
                top: dialogHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            clip: true
            model: topicListProxyModel
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
                                    chatList.markTopicAsRead(chatId)
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
                    forceBackground: true
                    color: iconColor
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
                            visible: isLocked
                            opacity: 0.3
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
                    chat.setTopic(id)
                    pageStack.push(Qt.resolvedUrl("../pages/Chat.qml"), { chat: chat, shareConfiguration: shareConfiguration })
                }
            }
        }
    }
}
