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
import "../functions/muteFormat.js" as MuteFormat
import "../"

Page {
    id: chatInfo

    allowedOrientations: Orientation.All
    property var chat: null

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height + header.height + Theme.paddingLarge

        PageHeader {
            id: header
            title: (chat.getChatType() === "channel") ? qsTr("Channel info") : qsTr("Group info")
        }

        Column {
            id: column
//            spacing: Theme.paddingLarge
            anchors.top: parent.top
            anchors.topMargin: header.height
            width: parent.width


            Rectangle {
                width: parent.width
                height: Theme.itemSizeHuge
                color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)

                Row {
                    anchors.fill: parent
                    spacing: Theme.horizontalPageMargin

                    Avatar {
                        id: chatPhoto
                        width: height
                        height: parent.height
                        userName: chat.title
                        avatarPhoto: chat.smallPhoto
                        maskEnabled: false
//                        onAvatarPhotoChanged: if (chat.smallPhoto && !chat.smallPhoto.isDownloading && !chat.smallPhoto.isDownloaded) chat.smallPhoto.download()
                    }

                    Column {
                        height: parent.height - Theme.paddingLarge
                        width: parent.width - chatPhoto.width
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            text: chat.title
                            font.family: Theme.fontFamilyHeading
                            font.pixelSize: Theme.fontSizeLarge
                            width: parent.width - Theme.horizontalPageMargin*2
                            anchors.horizontalCenter: parent.horizontalCenter
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                        }

                        Label {
                            text: qsTr("%1 members").arg(chat.basicGroupFullInfo.rowCount())
                            color: Theme.secondaryColor
                            width: parent.width - Theme.horizontalPageMargin*2
                            anchors.horizontalCenter: parent.horizontalCenter
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Info")
                visible: infoColumn.height > 0
            }

            Column {
                id: infoColumn
                width: parent.width

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: chat.basicGroupFullInfo.description
                    visible: chat.basicGroupFullInfo.description !== ""
                }

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    visible: chat.basicGroupFullInfo.description !== ""
                    text: qsTr("Description")
                }

                Item {
                    width: 1
                    height: chat.basicGroupFullInfo.description !== "" ? Theme.paddingLarge : 0
                }

                LabelWithMenu {
                    width: parent.width
                    visible: chat.basicGroupFullInfo.inviteLink !== ""
                    text: chat.basicGroupFullInfo.inviteLink
                    description: qsTr("Invite link")

                    menu: Component {
                        ContextMenu {
                            MenuItem {
                                text: qsTr("Copy")
                                onClicked: Clipboard.text = user.username
                            }
                        }
                    }
                }

                Item {
                    width: 1
                    height: chat.basicGroupFullInfo.inviteLink !== "" ? Theme.paddingLarge : 0
                }

                TextSwitchWithMenu {
                    anchors.left: parent.left
                    width: parent.width
                    checked: chat.muteFor === 0
                    onCheckedChanged: {
                        if (checked) {
                            chat.muteFor = 0
                        } else {
                            chat.muteFor = 2147483647
                        }
                    }

                    text: qsTr("Notifications")
                    description: MuteFormat.appendSpace(MuteFormat.formatMute(chat.muteFor)) + (chat.defaultMuteFor ? qsTr("Default") : "")
                    menu: Component {
                        ContextMenu {
                            MenuItem {
                                text: qsTr("Mute for 1 hour")
                                onClicked: chat.muteFor = 3600
                            }
                            MenuItem {
                                text: qsTr("Mute for 8 hours")
                                onClicked: chat.muteFor = 28800
                            }
                            MenuItem {
                                text: qsTr("Mute for 2 days")
                                onClicked: chat.muteFor = 172800
                            }
                            MenuItem {
                                text: qsTr("Customize")
                                onClicked: pageStack.push(Qt.resolvedUrl("../ChatNotifications.qml"), {chat: chat})
                            }
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Users")
            }

            SilicaListView {
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin*2
                height: (contentHeight < chatInfo.height) ? contentHeight :  chatInfo.height
                model: chat.basicGroupFullInfo
                spacing: Theme.paddingLarge
                delegate: BackgroundItem {
                    width: parent.width
                    height: Theme.itemSizeSmall
                    readonly property var user: users.getUserAsVariant(userId)

                    Row {
                        anchors.fill: parent

                        Avatar {
                            width: height
                            height: parent.height
                            avatarPhoto: user.smallPhoto
                        }

                        Item {
                            height: parent.height
                            width: Theme.horizontalPageMargin
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: Theme.paddingSmall

                            Label {
                                text: user.name
                                font.family: Theme.fontFamilyHeading
                            }

                            Label {
                                text: user.statusText
                                font.pixelSize: Theme.fontSizeSmall
                                color: Theme.secondaryColor
                            }
                        }
                    }
                }
            }

       }
       VerticalScrollDecorator {}
    }
}
