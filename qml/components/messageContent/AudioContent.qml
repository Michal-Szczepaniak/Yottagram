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

        FastBlur {
            anchors.fill: parent
            source: thumbnail
            radius: 32
            cached: true

            Rectangle {
                anchors.fill: parent
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.2)


                IconButton {
                    anchors.fill: parent
                    icon.asynchronous: true
                    icon.source: {
                        if (!file.audio.isDownloaded && !file.audio.isDownloading) {
                            return "image://theme/icon-m-cloud-download"
                        }

                        if (!audioLoader.active || audioLoader.item.playbackState !== MediaPlayer.PlayingState) {
                            return "image://theme/icon-m-play"
                        } else {
                            return "image://theme/icon-m-pause"
                        }
                    }
                    onClicked: {
                        audioLoader.active = true
                        if (file && file.audio) {
                            if (file.audio.isDownloaded || file.audio.isDownloading) {
                                if (audioLoader.item.playbackState === MediaPlayer.PlayingState) {
                                    audioLoader.item.pause()
                                } else {
                                    audioLoader.item.play()
                                }
                            } else {
                                file.audio.download()
                            }
                        }
                    }
                }
            }
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
