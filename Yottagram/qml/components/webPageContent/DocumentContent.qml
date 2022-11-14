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

Row {
    id: documentRow

    IconButton {
        icon.source: {
            if (webPage.document.isDownloaded) {
                return "image://theme/icon-m-file-document-light"
            } else if (webPage.document.isDownloading || webPage.document.isUploading) {
                return "image://theme/icon-m-cancel"
            } else {
                return "image://theme/icon-m-cloud-download"
            }
        }
        width: Theme.itemSizeMedium
        height: width
        anchors.verticalCenter: parent.verticalCenter
        onClicked: {
            if (webPage && webPage.document) {
                if (webPage.document.isDownloading) {
                    webPage.document.cancelDownload()
                } else if (webPage.document.isUploading) {
                    webPage.document.cancelUpload()
                    chat.deleteMessage(messageId)
                } else if (!webPage.document.isDownloaded || !chatList.fileExists(webPage.document.localPath)) {
                    webPage.document.download()
                } else {
                    chat.open(webPage.document.localPath)
                }
            }
        }

        ProgressCircle {
            id: progress
            anchors.centerIn: parent
            width: Theme.itemSizeMedium
            visible: webPage.document.isDownloading || webPage.document.isUploading
            value : webPage.document.isUploading ? webPage.document.uploadedSize / webPage.document.downloadedSize : webPage.document.downloadedSize / webPage.document.uploadedSize
        }
    }

    Column {
        Label {
            text: webPage.mediaTitle
            width: Theme.itemSizeHuge*2
            truncationMode: TruncationMode.Fade
        }

        Label {
            text: bytesToSize(webPage.document.expectedSize)
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
