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
    property var chat
    property var userId
    readonly property var user: users.getUserAsVariant(userId)

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height + header.height + Theme.paddingLarge

        PageHeader {
            id: header
            title: qsTr("User info")
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("Clear history")
                onClicked: Remorse.popupAction(root, "", function() {
                    chat.clearHistory()
                })
            }
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
                        userName: user.name
                        avatarPhoto: user.smallPhoto.localPath
                        maskEnabled: false
//                        onAvatarPhotoChanged: if (user.smallPhoto && !user.smallPhoto.isDownloading && !user.smallPhoto.isDownloaded) user.smallPhoto.download()
                    }

                    Column {
                        height: parent.height - Theme.paddingLarge
                        width: parent.width - chatPhoto.width
                        anchors.verticalCenter: parent.verticalCenter

                        Label {
                            text: user.name
                            font.family: Theme.fontFamilyHeading
                            font.pixelSize: Theme.fontSizeLarge
                            width: parent.width - Theme.horizontalPageMargin*2
                            anchors.horizontalCenter: parent.horizontalCenter
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                        }

                        Label {
                            text: user.statusText
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
            }
            Column {
                width: parent.width

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    visible: user.phoneNumber !== ""
                    text: "+" + user.phoneNumber
                }

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    visible: user.phoneNumber !== ""
                    text: qsTr("Number")
                }

                Item {
                    width: 1
                    height: if (user.phoneNumber !== "") Theme.paddingLarge
                }

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text: user.userFullInfo.bio
                    visible: user.userFullInfo.bio !== ""
                }

                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin*2
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    visible: user.userFullInfo.bio !== ""
                    text: qsTr("Bio")
                }

                Item {
                    width: 1
                    height: if (user.userFullInfo.bio !== "") Theme.paddingLarge
                }

                LabelWithMenu {
                    width: parent.width
                    visible: user.username !== ""
                    text: "@" + user.username
                    description: qsTr("Username")

                    menu: Component {
                        ContextMenu {
                            MenuItem {
                                text: qsTr("Copy")
                                onClicked: Clipboard.text = "@" + user.username
                            }
                        }
                    }
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

                Item {
                    width: 1
                    height: Theme.paddingLarge
                }
            }
       }
       VerticalScrollDecorator {}
    }
}
