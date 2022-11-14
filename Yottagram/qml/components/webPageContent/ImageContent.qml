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
    source: webPage.photo.localPath
    fillMode: Image.PreserveAspectFit
    asynchronous: true
    cache: true

    Thumbnail {
        id: miniThumbnail
        image: webPage.miniThumbnail
        visible: !webPage.thumbnail.isDownloaded
        anchors.fill: parent
    }

    Image {
        id: thumbnail
        source: webPage.thumbnail.localPath
        visible: !webPage.photo.isDownloaded
        anchors.fill: parent
    }

    FastBlur {
        anchors.fill: parent
        source: webPage.thumbnail.isDownloaded ? thumbnail : miniThumbnail
        radius: 32
        cached: true
        visible: !webPage.photo.isDownloaded
    }

    MouseArea {
        anchors.fill: parent

        onClicked: pageStack.push(bigPhoto, {path: webPage.photo.localPath})
    }

    Rectangle {
        id: background
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !webPage.photo.isDownloading && !webPage.photo.isUploading && !webPage.photo.isDownloaded && !!webPage.photo.isUploaded
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        visible: webPage.photo.isDownloading || webPage.photo.isUploading
        value : (webPage.photo.isUploading ? webPage.photo.uploadedSize / webPage.photo.downloadedSize : webPage.photo.downloadedSize / webPage.photo.uploadedSize)

    }

    IconButton {
        id: downloadButton
        anchors.centerIn: background
        visible: !webPage.photo.isDownloaded || !webPage.photo.isUploaded
        icon.source: webPage.photo.isDownloading || webPage.photo.isUploading ? "image://theme/icon-m-cancel" : "image://theme/icon-m-cloud-download"
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (webPage.photo.isDownloading) {
                webPage.photo.cancelDownload();
            } else if (webPage.photo.isUploading) {
                webPage.photo.cancelUpload();
                chat.deleteMessage(messageId)
            } else if (!webPage.photo.isDownloaded || !chatList.fileExists(webPage.photo.localPath)) {
                webPage.photo.download()
            }
        }
    }
}
