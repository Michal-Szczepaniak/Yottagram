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
import QtGraphicalEffects 1.0
import Nemo.Notifications 1.0
import "../components"
import "../dialogs"

Page {
    id: page

    allowedOrientations: Orientation.All
    property int activeChatList: 0
    property var shareConfiguration: null


    onStatusChanged: {
        if (status === PageStatus.Active) {
            contactsModel.load()
        }
    }

    Connections {
        target: contactsModel
        onImportDone: {
            importNotification.summary = qsTr("Imported %1 contacts").arg(importedCount)
            importNotification.previewSummary = importNotification.summary
            importNotification.publish()
        }
    }

    Notification {
        id: importNotification
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Import contacts")
                onClicked: pageStack.push(Qt.resolvedUrl("../dialogs/ContactsImport.qml"))
            }
        }

        SearchField {
            id: searchField
            width: parent.width
            anchors.top: parent.top
            placeholderText: qsTr("Search")
        }

        SortFilterProxyModel {
            id: contactsListProxyModel
            sourceModel: contactsModel
            sorters: [
                RoleSorter { roleName: "name"; sortOrder: Qt.AscendingOrder }
            ]
            filters: [
                RegExpFilter {
                    roleName: "name"
                    pattern: searchField.text
                    caseSensitivity: Qt.CaseInsensitive
                    enabled: searchField.text !== ""
                }
            ]
        }

        SilicaListView {
            id: contactsList
            anchors.top: searchField.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            spacing: 0
            model: page.status === PageStatus.Inactive ? null : contactsListProxyModel
            contentHeight: Theme.itemSizeLarge * count
            cacheBuffer: Theme.itemSizeLarge*100

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
                        }
                    }
                }

                Avatar {
                    id: avatar
                    userName: user.name
                    avatarPhoto: user.hasPhoto ? user.smallPhoto.localPath : undefined
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
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.paddingMedium

                    Row {
                        spacing: Theme.paddingSmall
                        width: parent.width

                        Label {
                            textFormat: Text.PlainText
                            anchors.verticalCenter: parent.verticalCenter
                            text: user.name
                            truncationMode: TruncationMode.Fade
                            width: parent.width
                        }
                    }

                    Row {
                        width: parent.width

                        Label {
                            text: "+" + user.phoneNumber
                            maximumLineCount: 1
                            truncationMode: TruncationMode.Fade
                            clip: true
                            width: parent.width
                            font.pixelSize: Theme.fontSizeSmall
                            color: Theme.secondaryColor
                        }
                    }
                }

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../components/chatInfo/UserInfo.qml"), { userId: user.id, chat: chatList.getChatAsVariantForUser(user.id) })
                }
            }
        }
    }
}
