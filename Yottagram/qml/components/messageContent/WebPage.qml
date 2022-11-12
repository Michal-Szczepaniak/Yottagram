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
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0

Row {
    id: webPagePreviewMessage

    Rectangle {
        width: 3
        height: parent.height
        color: Theme.highlightColor
    }

    Item {
        height: 1
        width: Theme.paddingMedium
    }

    Component.onCompleted: console.log(webPage.type)

    Column {
        Label {
            id: webPageName
            text: webPage.name
            truncationMode: TruncationMode.Fade
            font.family: Theme.fontFamilyHeading
            font.bold: true
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.highlightColor

            MouseArea {
                anchors.fill: parent
            }
        }

        Label {
            id: webPageTitle
            text: webPage.title
            width: webPageDescription.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.bold: true
            font.pixelSize: Theme.fontSizeSmall
            truncationMode: TruncationMode.Fade

            MouseArea {
                anchors.fill: parent
            }
        }

        Label {
            id: webPageDescription
            text: webPage.description
            width: chatPage.width/chatPage.messageWidth - Theme.paddingLarge
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            maximumLineCount: 5
            font.pixelSize: Theme.fontSizeSmall

            MouseArea {
                anchors.fill: parent
            }
        }
    }
}
