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
    width: chatPage.width/2.5
    height: (file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeHuge*1.5

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
        id: thumbnail
        image: file.thumbnail
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
        visible: !file.video.isDownloading && !file.video.isUploading
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        width: Theme.itemSizeMedium
        visible: file.video.isDownloading || file.video.isUploading
        value: file.video.isUploading ? file.video.uploadedSize / file.video.downloadedSize : file.video.downloadedSize / file.video.uploadedSize
    }

    IconButton {
        id: downloadButton
        anchors.centerIn: background
        icon.source: file.video.isDownloading || file.video.isUploading ? "image://theme/icon-m-cancel" : (!file.video.isDownloaded ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play")
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (file && file.video) {
                if (file.video.isDownloading) {
                    file.video.cancelDownload();
                } else if (file.video.isUploading) {
                    file.video.cancelUpload();
                    chat.deleteMessage(messageId)
                } else if (!file.video.isDownloaded || !chatList.fileExists(file.video.localPath)) {
                    file.video.download()
                } else if (file.video.isDownloaded) {
                    pageStack.push(bigVideo, {path: file.video.localPath})
                }
            }
        }
    }
}
