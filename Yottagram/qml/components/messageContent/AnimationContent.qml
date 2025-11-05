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
import QtMultimedia 5.0
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0

Item {
    width: chatPage.width/2.5
    height: videoLoader.active ? videoLoader.height : ((file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeHuge*1.5)

    Loader {
        id: videoLoader
        asynchronous: true
        active: false
        sourceComponent: videoComponent
    }

    Component {
        id: videoComponent
        AnimatedImage {
            id: videoPlayer
            fillMode: VideoOutput.PreserveAspectFit
            width: chatPage.width/2.5
            source: file.animation.localPath
            cache: false
            asynchronous: true
            height: (file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeLarge

            function play() {
                playing = true;
            }

            function pause() {
                playing = false;
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (videoLoader.item.playing) {
                videoLoader.item.pause();
            } else {
                videoLoader.item.play();
            }
        }
    }

    Image {
        id: thumbnail
        Binding on source { value: file.thumbnail.localPath; when: visible }
        visible: !videoLoader.active || !videoLoader.item.playing
        anchors.fill: parent
        asynchronous: true
        cache: true
    }

    FastBlur {
        anchors.fill: parent
        source: thumbnail
        radius: 32
        cached: true
        visible: !videoLoader.active || !videoLoader.item.playing
    }

    Rectangle {
        id: background
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.animation.isDownloading && !file.animation.isUploading && (!videoLoader.active || !videoLoader.item.playing)
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        visible: file.animation.isDownloading || file.animation.isUploading
        value : file.animation.isUploading ? file.animation.uploadedSize / file.animation.downloadedSize : file.animation.downloadedSize / file.animation.uploadedSize
    }

    IconButton {
        id: downloadButton
        visible: !file.animation.isDownloaded || !file.animation.isUploaded || !videoLoader.active || !videoLoader.item.playing
        icon.source: (file.animation.isDownloading || file.animation.isUploading) ? "image://theme/icon-m-cancel" : (!file.animation.isDownloaded ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play")
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        anchors.centerIn: background
        onClicked: {
            if (file && file.animation) {
                if (file.animation.isDownloading) {
                    file.animation.cancelDownload()
                } else if (file.animation.isUploading) {
                    file.animation.cancelUpload()
                    chat.deleteMessage(messageId)
                } else if (!file.animation.isDownloaded) {
                    file.animation.download()
                } else if (file.animation.isDownloaded) {
                    if (!videoLoader.active) {
                        videoLoader.active = true
                        return
                    } else {
                        videoLoader.item.play();
                    }
                }
            }
        }
    }
}
