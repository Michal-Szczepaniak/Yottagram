import QtQuick 2.0
import Sailfish.Silica 1.0
import QtMultimedia 5.0
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0

Item {
    width: chatPage.width/2.5
    height: videoLoader.active ? videoLoader.height : ((file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeLarge)

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
            source: file.video.localPath
            height: metaData.resolution ?
                        (width * (metaData.resolution.height/metaData.resolution.width)) :
                        ((file.size.width > 0 && file.size.height > 0) ? (width * (file.size.height/file.size.width)) : Theme.itemSizeLarge)
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
    }

    Rectangle {
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: downloadButton.visible
        radius: 90

        IconButton {
            id: downloadButton
            visible: !file.video.isDownloaded || videoLoader.item.playbackState === MediaPlayer.StoppedState
            icon.source: (!file.video.isDownloaded) ? "image://theme/icon-m-cloud-download" : "image://theme/icon-m-play"
            icon.asynchronous: true
            width: Theme.itemSizeMedium
            height: Theme.itemSizeMedium
            onClicked: {
                if (file && file.video) {
                    if (!file.video.isDownloading && !file.video.isDownloaded) file.video.download()
                    if (file.video.isDownloaded) {
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
}
