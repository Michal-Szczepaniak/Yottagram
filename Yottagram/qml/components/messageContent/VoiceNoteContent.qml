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
    Audio {
        id: audioPlayer
        source: file.voicenote.localPath
    }
    Thumbnail {
        id: thumbnail
        image: if (typeof file.thumbnail !== "undefined") file.thumbnail
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

        Rectangle {
            id: background
            anchors.fill: parent
            color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
            visible: !file.voicenote.isDownloading && !file.voicenote.isUploading
        }

        IconButton {
            id: downloadButton
            anchors.fill: background
            icon.source: {
                if (file.voicenote.isDownloaded) {
                    if (audioPlayer.playbackState == MediaPlayer.PlayingState) {
                        return "image://theme/icon-m-pause"
                    } else {
                        return "image://theme/icon-m-play"
                    }
                } else if (file.voicenote.isDownloading || file.voicenote.isUploading) {
                    return "image://theme/icon-m-cancel"
                } else {
                    return "image://theme/icon-m-cloud-download"
                }
            }

            onClicked: {
                if (file && file.voicenote) {
                    if (file.voicenote.isDownloading) {
                        file.voicenote.cancelDownload()
                    } else if (file.voicenote.isUploading) {
                        file.voicenote.cancelUpload()
                        chat.deleteMessage(messageId)
                    } else if (!file.voicenote.isDownloaded || !chatList.fileExists(file.voicenote.localPath)) {
                        file.voicenote.download()
                    } else {
                        if (audioPlayer.playbackState == MediaPlayer.PlayingState) {
                            audioPlayer.pause()
                        } else {
                            audioPlayer.play()
                        }
                    }
                }
            }

            ProgressCircle {
                id: progress
                anchors.centerIn: parent
                width: Theme.itemSizeMedium
                visible: file.voicenote.isDownloading || file.voicenote.isUploading
                value : file.voicenote.isUploading ? file.voicenote.uploadedSize / file.voicenote.downloadedSize : file.voicenote.downloadedSize / file.voicenote.uploadedSize
            }
        }
    }

    Item {
        width: Theme.paddingMedium
        height: 1
    }

    Column {
        Label {
            text: Format.formatDuration(file.duration, ((file.duration) > 3600 ? Formatter.DurationLong : Formatter.DurationShort))
            width: Theme.itemSizeHuge*2 - Theme.paddingMedium
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor
        }
    }
}
