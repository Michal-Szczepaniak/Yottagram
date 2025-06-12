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
import org.nemomobile.configuration 1.0
import "../components"
import "../dialogs"

Page {
    id: settingsPage

    allowedOrientations: Orientation.All

    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property bool sendButton: false
        property bool animatedStickers: true
        property bool animatedEmoji: true
        property bool combineWithMuted: false
        property int fontSize: Theme.fontSizeMedium
        property bool chatBubbles: false
        property bool textSwitchWithMenuHintShown: false
        property real voiceMessageBoost: 1.0
        Component.onCompleted: if (!textSwitchWithMenuHintShown) hint.start()
    }

    Component {
        id: moveFilesDialog
        MoveFilesDialog {
        }
    }

    InteractionHintLabel {
        z: 10001
        anchors.bottom: parent.bottom
        text: qsTr("Long press to select mute duration")
        opacity: hint.running ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation {} }
    }

    SilicaFlickable {
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        contentHeight: column.height + header.height

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("About.qml"))
            }
        }

        PageHeader {
            id: header
            title: qsTr("Settings")
        }

        Column {
            id: column
            spacing: Theme.paddingLarge
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            TextSwitch {
                width: parent.width
                text: qsTr("Daemon")
                checked: chatList.daemonEnabled
                onCheckedChanged: chatList.daemonEnabled = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Send button")
                checked: settings.sendButton
                onCheckedChanged: settings.sendButton = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Chat bubbles")
                checked: settings.chatBubbles
                onCheckedChanged: settings.chatBubbles = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Animated stickers")
                checked: settings.animatedStickers
                onCheckedChanged: settings.animatedStickers = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Animated emoji")
                checked: settings.animatedEmoji
                onCheckedChanged: settings.animatedEmoji = checked
            }

            TextSwitch {
                width: parent.width
                text: qsTr("Include muted chats in unread chats and messages counts")
                checked: settings.combineWithMuted
                onCheckedChanged: settings.combineWithMuted = checked
            }

            ComboBox {
                width: parent.width
                label: qsTr("Message font size")
                currentIndex: {
                    switch (settings.fontSize) {
                    case Theme.fontSizeHuge:
                        return 0;
                    case Theme.fontSizeExtraLarge:
                        return 1;
                    case Theme.fontSizeLarge:
                        return 2;
                    case Theme.fontSizeMedium:
                        return 3;
                    case Theme.fontSizeSmall:
                        return 4;
                    case Theme.fontSizeExtraSmall:
                        return 5;
                    case Theme.fontSizeTiny:
                        return 6;
                    default:
                        return 3;
                    }
                }

                menu: ContextMenu {
                    MenuItem { text: "Huge" }
                    MenuItem { text: "Extra large" }
                    MenuItem { text: "Large" }
                    MenuItem { text: "Medium" }
                    MenuItem { text: "Small" }
                    MenuItem { text: "Extra small" }
                    MenuItem { text: "Tiny" }
                }

                onCurrentIndexChanged: {
                    switch (currentIndex) {
                    case 0:
                        settings.fontSize = Theme.fontSizeHuge
                        break;
                    case 1:
                        settings.fontSize = Theme.fontSizeExtraLarge
                        break;
                    case 2:
                        settings.fontSize = Theme.fontSizeLarge
                        break;
                    case 3:
                        settings.fontSize = Theme.fontSizeMedium
                        break;
                    case 4:
                        settings.fontSize = Theme.fontSizeSmall
                        break;
                    case 5:
                        settings.fontSize = Theme.fontSizeExtraSmall
                        break;
                    case 6:
                        settings.fontSize = Theme.fontSizeTiny
                        break;
                    }
                }
            }

            Slider {
                width: parent.width
                label: qsTr("Voice message volume boost")
                valueText: value
                minimumValue: 1.0
                maximumValue: 10.0
                stepSize: 1.0
                value: settings.voiceMessageBoost
                onReleased: settings.voiceMessageBoost = value
            }

            TextSwitchWithMenu {
                id: privateChatNotificationGlobalSettingsMenu
                width: parent.width
                checked: chatList.getPrivateNotificationSettings().muteFor === 0
                onCheckedChanged: {
                    if (checked) {
                        chatList.getPrivateNotificationSettings().muteFor = 0
                    } else {
                        chatList.getPrivateNotificationSettings().muteFor = 2147483647
                    }
                }

                text: qsTr("Private chat notification global settings")
                menu: Component {
                    ContextMenu {
                        MenuItem {
                            text: qsTr("Mute for 1 hour")
                            onClicked: chatList.getPrivateNotificationSettings().muteFor = 3600
                        }
                        MenuItem {
                            text: qsTr("Mute for 8 hours")
                            onClicked: chatList.getPrivateNotificationSettings().muteFor = 28800
                        }
                        MenuItem {
                            text: qsTr("Mute for 2 days")
                            onClicked: chatList.getPrivateNotificationSettings().muteFor = 172800
                        }
                        MenuItem {
                            text: qsTr("Customize")
                            onClicked: pageStack.push(Qt.resolvedUrl("../components/ScopeNotificationSettings.qml"), {chat: chatList.getPrivateNotificationSettings()})
                        }
                    }
                }

                TapInteractionHint {
                    id: hint
                    loops: 3
                    z: 10000
                    running: false
                    anchors.centerIn: privateChatNotificationGlobalSettingsMenu
                    onRunningChanged: if (!running) settings.textSwitchWithMenuHintShown = true
                }
            }

            TextSwitchWithMenu {
                width: parent.width
                checked: chatList.getGroupNotificationSettings().muteFor === 0
                onCheckedChanged: {
                    if (checked) {
                        chatList.getGroupNotificationSettings().muteFor = 0
                    } else {
                        chatList.getGroupNotificationSettings().muteFor = 2147483647
                    }
                }

                text: qsTr("Group notification global settings")
                menu: Component {
                    ContextMenu {
                        MenuItem {
                            text: qsTr("Mute for 1 hour")
                            onClicked: chatList.getGroupNotificationSettings().muteFor = 3600
                        }
                        MenuItem {
                            text: qsTr("Mute for 8 hours")
                            onClicked: chatList.getGroupNotificationSettings().muteFor = 28800
                        }
                        MenuItem {
                            text: qsTr("Mute for 2 days")
                            onClicked: chatList.getGroupNotificationSettings().muteFor = 172800
                        }
                        MenuItem {
                            text: qsTr("Customize")
                            onClicked: pageStack.push(Qt.resolvedUrl("../components/ScopeNotificationSettings.qml"), {chat: chatList.getGroupNotificationSettings()})
                        }
                    }
                }
            }

            TextSwitchWithMenu {
                width: parent.width
                checked: chatList.getChannelNotificationSettings().muteFor === 0
                onCheckedChanged: {
                    if (checked) {
                        chatList.getChannelNotificationSettings().muteFor = 0
                    } else {
                        chatList.getChannelNotificationSettings().muteFor = 2147483647
                    }
                }

                text: qsTr("Channel notification global settings")
                menu: Component {
                    ContextMenu {
                        MenuItem {
                            text: qsTr("Mute for 1 hour")
                            onClicked: chatList.getChannelNotificationSettings().muteFor = 3600
                        }
                        MenuItem {
                            text: qsTr("Mute for 8 hours")
                            onClicked: chatList.getChannelNotificationSettings().muteFor = 28800
                        }
                        MenuItem {
                            text: qsTr("Mute for 2 days")
                            onClicked: chatList.getChannelNotificationSettings().muteFor = 172800
                        }
                        MenuItem {
                            text: qsTr("Customize")
                            onClicked: pageStack.push(Qt.resolvedUrl("../components/ScopeNotificationSettings.qml"), {chat: chatList.getChannelNotificationSettings()})
                        }
                    }
                }
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Wifi auto download settings")
                onClicked: pageStack.push(Qt.resolvedUrl("../components/AutoDownloadSettings.qml"), {settings: wifiAutoDownloadSettings})
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Cellular auto download settings")
                onClicked: pageStack.push(Qt.resolvedUrl("../components/AutoDownloadSettings.qml"), {settings: mobileAutoDownloadSettings})
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Roaming auto download settings")
                onClicked: pageStack.push(Qt.resolvedUrl("../components/AutoDownloadSettings.qml"), {settings: roamingAutoDownloadSettings})
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Other auto download settings")
                onClicked: pageStack.push(Qt.resolvedUrl("../components/AutoDownloadSettings.qml"), {settings: otherAutoDownloadSettings})
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Move yottagram's data")
                onClicked: pageStack.push(moveFilesDialog)
            }

            SubpageElement {
                width: parent.width
                text: qsTr("Proxy settings")
                onClicked: pageStack.push(Qt.resolvedUrl("../components/ProxySettings.qml"))
            }
        }
        VerticalScrollDecorator {}
    }
}
