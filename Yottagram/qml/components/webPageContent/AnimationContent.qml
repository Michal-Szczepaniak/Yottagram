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
    id: item

    Loader {
        id: videoLoader
        anchors.fill: parent
        asynchronous: true
        active: false
        sourceComponent: videoComponent
    }

    Component {
        id: videoComponent
        Video {
            id: videoPlayer
            fillMode: VideoOutput.PreserveAspectFit
            muted: true
            autoPlay: true
            onStopped: {
                videoPlayer.seek(0)
                videoPlayer.play()
            }

            anchors.fill: parent
            source: webPage.animation.localPath
        }
    }

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
        visible: !webPage.thumbnail.isDownloaded && (videoLoader.active ? videoLoader.item.playbackState === MediaPlayer.StoppedState : true)
        anchors.fill: parent
    }

    Image {
        id: thumbnail
        source: webPage.thumbnail.localPath
        visible: videoLoader.active ? videoLoader.item.playbackState === MediaPlayer.StoppedState : true
        anchors.fill: parent
    }

    FastBlur {
        anchors.fill: parent
        source: thumbnail
        radius: 32
        cached: true
        visible: videoLoader.active ? videoLoader.item.playbackState === MediaPlayer.StoppedState : true
    }

    Rectangle {
        id: background
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !webPage.animation.isDownloading && !webPage.animation.isUploading && (!videoLoader.active || videoLoader.item.playbackState !== MediaPlayer.PlayingState)
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: background
        visible: webPage.animation.isDownloading || webPage.animation.isUploading
        value : webPage.animation.isUploading ? webPage.animation.uploadedSize / webPage.animation.downloadedSize : webPage.animation.downloadedSize / webPage.animation.uploadedSize
    }

    IconButton {
        id: downloadButton
        visible: !webPage.animation.isDownloaded || !webPage.animation.isUploaded || !videoLoader.active || videoLoader.item.playbackState !== MediaPlayer.PlayingState
        icon.source: (webPage.animation.isDownloading || webPage.animation.isUploading) ? "image://theme/icon-m-cancel" : (!webPage.animation.isDownloaded ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play")
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        anchors.centerIn: background
        onClicked: {
            if (webPage.animation.isDownloading) {
                webPage.animation.cancelDownload()
            } else if (webPage.animation.isUploading) {
                webPage.animation.cancelUpload()
                chat.deleteMessage(messageId)
            } else if (!webPage.animation.isDownloaded) {
                webPage.animation.download()
            } else if (webPage.animation.isDownloaded) {
                if (!videoLoader.active) {
                    videoLoader.active = true
                    return
                } else {
                    videoLoader.item.play()
                }
            }
        }
    }
}
