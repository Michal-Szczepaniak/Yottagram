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
import com.verdanditeam.contacts 1.0

Dialog {
    id: dialog

    signal selectAll()
    signal unselectAll()

    onStatusChanged: {
        if (status === PageStatus.Active) {
            pimContacts.load()
        }
    }

    onAccepted: contactsModel.importJson(pimContacts.exportAsJSON())

    SortFilterProxyModel {
        id: contactsListProxyModel
        sourceModel: PIMContacts {
            id: pimContacts
        }
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

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Unselect all")
                onClicked: dialog.unselectAll()
            }

            MenuItem {
                text: qsTr("Select all")
                onClicked: dialog.selectAll()
            }
        }

        DialogHeader {
            id: dialogHeader
            title: qsTr("Select contacts to import")
        }

        SearchField {
            id: searchField
            width: parent.width
            anchors.top: parent.top
            placeholderText: qsTr("Search")
        }


        SilicaListView {
            anchors {
                top: dialogHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            clip: true
            model: contactsListProxyModel
            delegate: ListItem {
                width: parent.width
                contentHeight: Theme.itemSizeExtraSmall

                Connections {
                    target: dialog

                    onSelectAll: {
                        highlighted = true
                        pimContacts.selectContact(firstName, lastName, phoneNumber)
                    }

                    onUnselectAll: {
                        highlighted = false
                        pimContacts.unselectContact(firstName, lastName, phoneNumber)
                    }
                }

                DetailItem {
                    width: parent.width
                    anchors.verticalCenter: parent.verticalCenter
                    label: name
                    value: phoneNumber
                    alignment: Qt.AlignLeft
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }

                onClicked: {
                    if (highlighted) {
                        highlighted = false
                        pimContacts.unselectContact(firstName, lastName, phoneNumber)
                    } else {
                        highlighted = true
                        pimContacts.selectContact(firstName, lastName, phoneNumber)
                    }
                }
            }

            VerticalScrollDecorator { }
        }
    }
}
