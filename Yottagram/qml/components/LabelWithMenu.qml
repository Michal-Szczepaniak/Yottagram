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

ListItem {
    id: root
    property alias text: label.text
    property alias description: description.text
    contentHeight: column.height + Theme.paddingMedium*2

    Column {
        id: column
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter

        Label {
            id: label
            anchors.left: parent.left
            anchors.leftMargin: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin*2
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: description
            anchors.left: parent.left
            anchors.leftMargin: Theme.horizontalPageMargin
            width: parent.width - Theme.horizontalPageMargin*2
            truncationMode: TruncationMode.Fade
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.secondaryColor
        }
    }
}
