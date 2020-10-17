import QtQuick 2.6
import Sailfish.Silica 1.0
import SortFilterProxyModel 0.2

Item {
    id: root
    property var page
    property int stickerFileId: 0

    Connections {
        target: authorization
        onIsAuthorizedChanged: {
            stickerSets.getStickerSets()
        }
    }

    SortFilterProxyModel {
        id: stickerSetsProxyModel
        sourceModel: stickerSets
        filters: [
            ValueFilter {
                roleName: "isAnimated"
                value: false
            }
        ]
    }

    Column {
        id: column
        anchors.fill: parent
        spacing: 0

        Item {
            width: parent.width
            height: Theme.paddingLarge
        }

        SilicaListView {
            id: stickerSetsListView
            width: parent.width
            height: Theme.itemSizeLarge
            model: stickerSetsProxyModel
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            onCountChanged: if (count > 0) stickerSetGridView.model = stickerSetsProxyModel.data(stickerSetsProxyModel.index(0, 0), 267)
            delegate: ListItem {
                width: Theme.itemSizeLarge
                height: Theme.itemSizeLarge
                Image {
                    sourceSize.width: Theme.itemSizeMedium - Theme.paddingSmall*2
                    sourceSize.height: Theme.itemSizeMedium - Theme.paddingSmall*2
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    asynchronous: true
                    source: thumbnail.localPath
                }

                onClicked: stickerSetGridView.model = stickerSet
            }
        }

        SilicaGridView {
            id: stickerSetGridView
            width: root.width
            height: root.height - stickerSetsListView.height - Theme.paddingLarge
            cellWidth: Math.floor(page.isLandscape ? root.width/(Math.floor(root.width/(root.height/5))) : root.width/5)
            cellHeight: cellWidth
            clip: true
            cacheBuffer: 0

            delegate: GridItem {
                width: stickerSetGridView.cellWidth
                height: width
                onClicked: root.stickerFileId = sticker.id

                Image {
                    anchors.centerIn: parent
                    sourceSize.width: parent.width - Theme.paddingSmall*2
                    sourceSize.height: parent.width - Theme.paddingSmall*2
                    cache: true
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    source: sticker.localPath
                }
            }
        }
    }
}
