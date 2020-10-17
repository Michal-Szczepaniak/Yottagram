import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0

Image {
    id: img
    source: file.sticker.localPath
    width: Theme.itemSizeHuge*1.5
    height: width
    fillMode: Image.PreserveAspectFit
    asynchronous: true

    MouseArea {
        anchors.fill: parent

        onClicked: pageStack.push(bigPhoto, {path: file.sticker.localPath})
    }

    Rectangle {
        color: downloadButton.down ? Theme.rgba(Theme.highlightColor, Theme.highlightBackgroundOpacity) : Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        anchors.centerIn: parent
        width: downloadButton.width
        height: downloadButton.height
        visible: !file.sticker.isDownloaded
        radius: 90

        IconButton {
            id: downloadButton
            visible: !file.sticker.isDownloaded
            icon.source: "image://theme/icon-m-cloud-download"
            icon.asynchronous: true
            width: Theme.itemSizeMedium
            height: Theme.itemSizeMedium
            onClicked: {
                if (file && file.sticker && !file.sticker.isDownloading && !file.sticker.isDownloaded) file.sticker.download()
            }
        }
    }
}
