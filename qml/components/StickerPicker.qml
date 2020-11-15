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
import SortFilterProxyModel 0.2
import org.nemomobile.configuration 1.0

Item {
    id: root
    property var page
    property int stickerFileId: 0

    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property real stickerPickerPostition: 0
        property int stickerPickerPickedPack: 0

        Component.onCompleted: stickerSetGridView.model = stickerSets.data(stickerSets.index(settings.stickerPickerPickedPack, 0), 267)
    }

    Connections {
        target: authorization
        onIsAuthorizedChanged: {
            stickerSets.getStickerSets()
        }
    }

    Column {
        id: column
        anchors.fill: parent
        spacing: 0

        Item {
            width: parent.width
            height: Theme.paddingLarge
        }

        SilicaListView {
            id: stickerSetsListView
            width: parent.width
            height: Theme.itemSizeLarge
            model: stickerSets
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            contentX: settings.stickerPickerPostition
            onContentXChanged: settings.stickerPickerPostition = contentX
            Component.onCompleted: contentX = contentX
            delegate: ListItem {
                width: Theme.itemSizeLarge
                height: Theme.itemSizeLarge
                Image {
                    sourceSize.width: Theme.itemSizeMedium - Theme.paddingSmall*2
                    sourceSize.height: Theme.itemSizeMedium - Theme.paddingSmall*2
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    asynchronous: true
                    source: thumbnail.localPath
                }

                onClicked: {
                    settings.stickerPickerPickedPack = index
                    stickerSetGridView.model = stickerSet
                }
            }
        }

        SilicaGridView {
            id: stickerSetGridView
            width: root.width
            height: root.height - stickerSetsListView.height - Theme.paddingLarge
            cellWidth: Math.floor(page.isLandscape ? root.width/(Math.floor(root.width/(root.height/5))) : root.width/5)
            cellHeight: cellWidth
            clip: true
            cacheBuffer: 0

            delegate: GridItem {
                width: stickerSetGridView.cellWidth
                height: width
                onClicked: root.stickerFileId = sticker.id

                Image {
                    anchors.centerIn: parent
                    sourceSize.width: parent.width - Theme.paddingSmall*2
                    sourceSize.height: parent.width - Theme.paddingSmall*2
                    cache: true
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    source: sticker.localPath
                }
            }
        }
    }
}
