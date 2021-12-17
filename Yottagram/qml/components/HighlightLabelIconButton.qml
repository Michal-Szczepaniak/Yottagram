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

MouseArea {
    id: root
    height: column.height

    property alias source: icon.source
    property alias text: label.text
    property bool highlighted: pressed

    Column {
        id: column
        anchors.centerIn: parent
        spacing: Theme.paddingMedium

        Icon {
            id: icon
            highlighted: root.highlighted
            anchors.horizontalCenter: parent.horizontalCenter
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
        }

        Label {
            id: label
            truncationMode: TruncationMode.Fade
            color: root.highlighted ? Theme.highlightColor : Theme.primaryColor
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeSmallBase
            width: root.width
        }
    }
}
