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
    id: root
    width: chatPage.width/2.5
    height: width

    Loader {
        id: videoLoader
        active: false
        sourceComponent: videoComponent
    }

    Component {
        id: videoComponent
        Video {
            id: videoPlayer
            fillMode: VideoOutput.PreserveAspectFit
            muted: false
            autoPlay: true
        //                                autoLoad: true
            onStopped: {
                videoPlayer.seek(0)
                videoPlayer.play()
            }

            width: chatPage.width/2.5
            source: file.videonote.localPath
            height: width
            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: videonoteMask
            }
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
        id: thumbnail
        image: file.thumbnail
        visible: videoLoader.item.playbackState === MediaPlayer.StoppedState
        anchors.fill: parent
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: videonoteMask
        }
    }

    FastBlur {
        anchors.fill: parent
        source: thumbnail
        radius: 32
        cached: true
        visible: videoLoader.item.playbackState === MediaPlayer.StoppedState
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: videonoteMask
        }
    }

    Rectangle {
        id: videonoteMask
        width: parent.width
        height: parent.height
        radius: parent.height
        visible: false
    }

    Rectangle {
        id: downloadButtonBackground
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.videonote.isDownloading && !file.videonote.isUploading && (!videoLoader.active || videoLoader.item.playbackState !== MediaPlayer.PlayingState)
        radius: 90
    }

    ProgressCircle {
        id: progress
        anchors.centerIn: downloadButtonBackground
        width: Theme.itemSizeMedium
        visible: file.videonote.isDownloading || file.videonote.isUploading
        value: file.videonote.isUploading ? file.videonote.uploadedSize / file.videonote.downloadedSize : file.videonote.downloadedSize / file.videonote.uploadedSize
    }

    IconButton {
        id: downloadButton
        anchors.centerIn: downloadButtonBackground
        visible: !file.videonote.isDownloaded || !file.videonote.isUploaded || !videoLoader.active || videoLoader.item.playbackState !== MediaPlayer.PlayingState
        icon.source: file.videonote.isDownloading || file.videonote.isUploading ? "image://theme/icon-m-cancel" : (!file.videonote.isDownloaded ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play")
        icon.asynchronous: true
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (file && file.videonote) {
                if (file.videonote.isDownloading) {
                    file.videonote.cancelDownload();
                } else if (file.videonote.isUploading) {
                    file.videonote.cancelUpload();
                    chat.deleteMessage(messageId)
                } else if (!file.videonote.isDownloaded || !chatList.fileExists(file.videonote.localPath)) {
                    file.videonote.download()
                } else if (file.videonote.isDownloaded) {
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
}
