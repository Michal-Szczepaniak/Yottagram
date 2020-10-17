import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0

Row {
    spacing: Theme.paddingMedium
    Audio {
        id: audioPlayer
        source: file.audio.localPath
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
                    icon.source: {
                        if (!file.audio.isDownloaded && !file.audio.isDownloading) {
                            return "image://theme/icon-m-cloud-download"
                        }

                        if (audioPlayer.playbackState == MediaPlayer.PlayingState) {
                            return "image://theme/icon-m-pause"
                        } else {
                            return "image://theme/icon-m-play"
                        }
                    }
                    onClicked: {
                        if (file && file.audio) {
                            if (file.audio.isDownloaded || file.audio.isDownloading) {
                                if (audioPlayer.playbackState == MediaPlayer.PlayingState) {
                                    audioPlayer.pause()
                                } else {
                                    audioPlayer.play()
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
