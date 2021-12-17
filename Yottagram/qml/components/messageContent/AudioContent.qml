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

Row {
    spacing: Theme.paddingMedium

    Loader {
        id: audioLoader
        sourceComponent: audioPlayerComponent
        active: false
    }

    Component {
        id: audioPlayerComponent

        Audio {
            id: audioPlayer
            autoPlay: true
            source: file.audio.localPath
        }
    }

    Thumbnail {
        id: thumbnail
        image: file.thumbnail
        width: Theme.itemSizeSmall
        height: Theme.itemSizeSmall
        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: chatPhotoMask
        }

        Rectangle {
            id: chatPhotoMask
            width: chatPhoto.height
            height: chatPhoto.height
            radius: 90
            visible: false
        }

        IconButton {
            id: downloadButton
            anchors.centerIn: background
            icon.asynchronous: true
            icon.source: {
                if (file.audio.isUploading) {
                    return "image://theme/icon-m-cancel"
                }

                if (!file.audio.isDownloaded && !file.audio.isDownloading) {
                    return "image://theme/icon-m-cloud-download"
                }

                if (!audioLoader.active || audioLoader.item.playbackState !== MediaPlayer.PlayingState) {
                    return "image://theme/icon-m-play"
                } else {
                    return "image://theme/icon-m-pause"
                }
            }
            width: Theme.itemSizeMedium
            height: width

            onClicked: {
                audioLoader.active = true
                if (file && file.audio) {
                    if (chatList.fileExists(file.audio.localPath) && (file.audio.isDownloaded || file.audio.isDownloading)) {
                        if (audioLoader.item.playbackState === MediaPlayer.PlayingState) {
                            audioLoader.item.pause()
                        } else {
                            audioLoader.item.play()
                        }
                    } else if (file.audio.isUploading) {
                        file.audio.cancelUpload()
                        chat.deleteMessage(messageId)
                    } else {
                        file.audio.download()
                    }
                }
            }
        }

        ProgressCircle {
            id: progress
            anchors.centerIn: background
            width: Theme.itemSizeMedium
            visible: file.audio.isDownloading || file.audio.isUploading
            value: file.audio.isUploading ? file.audio.uploadedSize / file.audio.downloadedSize : file.audio.downloadedSize / file.audio.uploadedSize
        }

        Rectangle {
            id: background
            anchors.fill: parent
            color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)

        }
    }

    Column {
        Label {
            text: file.performer + " - " + file.title
            width: Theme.itemSizeHuge*2
            truncationMode: TruncationMode.Fade
        }

        Label {
            text: Format.formatDuration(file.duration, ((file.duration) > 3600 ? Formatter.DurationLong : Formatter.DurationShort))
            width: Theme.itemSizeHuge*2
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
        }
    }
}
