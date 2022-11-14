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

Item {
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (videoLoader.item.playbackState === MediaPlayer.PlayingState) {
                videoLoader.item.pause();
            } else {
                videoLoader.item.play();
            }
        }
    }


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
        source: thumbnail
        radius: 32
        cached: true
    }

    Rectangle {
        id: background
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !webPage.video.isDownloading && !webPage.video.isUploading
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        width: Theme.itemSizeMedium
        visible: webPage.video.isDownloading || webPage.video.isUploading
        value: webPage.video.isUploading ? webPage.video.uploadedSize / webPage.video.downloadedSize : webPage.video.downloadedSize / webPage.video.uploadedSize
    }

    IconButton {
        id: downloadButton
        anchors.centerIn: background
        icon.source: webPage.video.isDownloading || webPage.video.isUploading ? "image://theme/icon-m-cancel" : (!webPage.video.isDownloaded ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play")
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (webPage && webPage.video) {
                if (webPage.video.isDownloading) {
                    webPage.video.cancelDownload();
                } else if (webPage.video.isUploading) {
                    webPage.video.cancelUpload();
                    chat.deleteMessage(messageId)
                } else if (!webPage.video.isDownloaded || !chatList.fileExists(webPage.video.localPath)) {
                    webPage.video.download()
                } else if (webPage.video.isDownloaded) {
                    pageStack.push(bigVideo, {path: webPage.video.localPath})
                }
            }
        }
    }
}
