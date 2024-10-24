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

Image {
    id: img
    source: file.biggestPhoto.localPath
    width: chatPage.width/2.5
    height: (file.biggestPhotoSize.width > 0 && file.biggestPhotoSize.height > 0) ?
                width * file.biggestPhotoSize.height/file.biggestPhotoSize.width :
                Math.max(Theme.itemSizeHuge*1.5, width * sourceSize.height/sourceSize.width)
    fillMode: Image.PreserveAspectFit
    asynchronous: true
    cache: true

    Image {
        id: thumbnail
        source: file.thumbnail.localPath
        anchors.fill: parent
        asynchronous: true
        visible: !file.biggestPhoto.isDownloaded
        cache: true
    }

    FastBlur {
        anchors.fill: parent
        source: thumbnail
        radius: 32
        cached: true
        visible: !file.biggestPhoto.isDownloaded
    }

    MouseArea {
        anchors.fill: parent

        onClicked: pageStack.push(bigPhoto, {path: file.biggestPhoto.localPath})
    }

    Rectangle {
        id: background
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.biggestPhoto.isDownloading && !file.biggestPhoto.isUploading && !file.biggestPhoto.isDownloaded && !!file.biggestPhoto.isUploaded
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        visible: file.biggestPhoto.isDownloading || file.biggestPhoto.isUploading
        value : (file.biggestPhoto.isUploading ? file.biggestPhoto.uploadedSize / file.biggestPhoto.downloadedSize : file.biggestPhoto.downloadedSize / file.biggestPhoto.uploadedSize)

    }

    IconButton {
        id: downloadButton
        anchors.centerIn: background
        visible: !file.biggestPhoto.isDownloaded || !file.biggestPhoto.isUploaded
        icon.source: file.biggestPhoto.isDownloading || file.biggestPhoto.isUploading ? "image://theme/icon-m-cancel" : "image://theme/icon-m-cloud-download"
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (file && file.biggestPhoto) {
                if (file.biggestPhoto.isDownloading) {
                    file.biggestPhoto.cancelDownload();
                } else if (file.biggestPhoto.isUploading) {
                    file.biggestPhoto.cancelUpload();
                    chat.deleteMessage(messageId)
                } else if (!file.biggestPhoto.isDownloaded || !chatList.fileExists(file.biggestPhoto.localPath)) {
                    file.biggestPhoto.download()
                }
            }
        }
    }
}
