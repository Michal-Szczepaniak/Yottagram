import QtQuick 2.0
import Sailfish.Silica 1.0

Row {
    id: documentRow

    IconButton {
        icon.source: {
            if (file.document.isDownloaded) {
                return "image://theme/icon-m-file-document-light"
            } else {
                return "image://theme/icon-m-cloud-download"
            }
        }
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        onClicked: {
            if (file && file.document) {
                if (file.document.isDownloaded) {
                    chat.open(file.document.localPath)
                } else {
                    file.document.download()
                }
            }
        }
    }

    Column {
        Label {
            text: file.name
            width: Theme.itemSizeHuge*2
            truncationMode: TruncationMode.Fade
        }

        Label {
            text: bytesToSize(file.document.expectedSize)
            width: Theme.itemSizeHuge*2
            truncationMode: TruncationMode.Fade
            color: Theme.secondaryColor

            function bytesToSize(bytes) {
               var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
               if (bytes == 0) return '0 Byte';
               var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
               return Math.round(bytes / Math.pow(1024, i), 2) + ' ' + sizes[i];
            }
        }
    }
}
