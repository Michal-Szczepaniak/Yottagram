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
        //                                autoLoad: true
            onStopped: {
                videoPlayer.seek(0)
                videoPlayer.play()
            }

            width: chatPage.width/2.5
            source: file.videonote.localPath
            height: width
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
        id: downloadButtonBackground
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.videonote.isDownloaded || videoLoader.item.playbackState === MediaPlayer.StoppedState
        radius: 90

        IconButton {
            id: downloadButton
            visible: downloadButtonBackground.visible
            icon.source: (!file.videonote.isDownloaded) ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play"
            icon.asynchronous: true
            width: Theme.itemSizeMedium
            height: Theme.itemSizeMedium
            onClicked: {
                if (file && file.videonote) {
                    if (!file.videonote.isDownloading && !file.videonote.isDownloaded) file.videonote.download()
                    if (file.videonote.isDownloaded) videoLoader.item.play()
                }
            }
        }
    }

    Rectangle {
        id: videonoteMask
        width: parent.width
        height: parent.height
        radius: parent.height
        visible: false
    }
}
