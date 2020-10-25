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
import "../components"


Page {
    id: settingsPage

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        flickableDirection: Flickable.VerticalFlick
        contentHeight: column.height + header.height

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

            TextSwitchWithMenu {
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
        }
        VerticalScrollDecorator {}
    }
}
