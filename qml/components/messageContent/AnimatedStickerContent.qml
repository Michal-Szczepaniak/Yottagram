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

AnimatedImage {
    id: img
    source: file.sticker.localPath
    width: Theme.itemSizeHuge*1.5
    height: width
    fillMode: Image.PreserveAspectFit
    asynchronous: true
    paused: !Qt.application.active
    cache: false
    Component.onCompleted: console.log(paused)

    MouseArea {
        anchors.fill: parent

        onClicked: img.paused = !img.paused;
    }

    Rectangle {
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.sticker.isDownloaded
        radius: 90

        IconButton {
            id: downloadButton
            visible: !file.sticker.isDownloaded
            icon.source: "image://theme/icon-m-cloud-download"
            icon.asynchronous: true
            width: Theme.itemSizeMedium
            height: Theme.itemSizeMedium
            onClicked: {
                if (file && file.sticker && !file.sticker.isDownloading && !file.sticker.isDownloaded) file.sticker.download()
            }
        }
    }
}
