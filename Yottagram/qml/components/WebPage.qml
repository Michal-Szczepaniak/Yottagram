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
import QtGraphicalEffects 1.0
import com.verdanditeam.thumbnail 1.0
import "./webPageContent"

Row {
    id: webPagePreviewMessage
    width: chatPage.width / chatPage.messageWidth

    Rectangle {
        id: verticalRectangle
        width: 3
        height: parent.height
        color: Theme.highlightColor
    }

    Item {
        height: 1
        width: Theme.paddingMedium
    }

    Column {
        id: column
        width: (chatPage.width/chatPage.messageWidth) - Theme.paddingMedium - verticalRectangle.width

        Label {
            id: webPageName
            text: webPage.name
            visible: text !== ""
            truncationMode: TruncationMode.Fade
            font.family: Theme.fontFamilyHeading
            font.bold: true
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.highlightColor

            MouseArea {
                anchors.fill: parent
            }
        }

        Label {
            id: webPageTitle
            text: webPage.title
            width: parent.width
            visible: text !== ""
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.bold: true
            font.pixelSize: Theme.fontSizeSmall
            truncationMode: TruncationMode.Fade

            MouseArea {
                anchors.fill: parent
            }
        }

        Row {
            id: row
            width: parent.width
            spacing: webPageIcon.visible ? Theme.paddingMedium : 0

            Label {
                id: webPageDescription
                text: webPage.description
                width: webPageIcon.visible ? parent.width - Theme.itemSizeLarge - Theme.paddingMedium : parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                maximumLineCount: 3
                font.pixelSize: Theme.fontSizeSmall
                visible: text !== ""

                MouseArea {
                    anchors.fill: parent
                }
            }

            Image {
                id: webPageIcon
                source: webPage.photo ? webPage.photo.localPath : ""
                width: Theme.itemSizeLarge
                height: Theme.itemSizeLarge
                visible: webPage.type === "article" && webPage.photo
            }
        }

        Item {
            width: 1
            height: webPageLoader.height > 0 ? Theme.paddingMedium : 0
        }

        Loader {
            id: webPageLoader
            width: parent.width
            height: switch (webPage.type) {
                    case "photo":
                    case "video":
                    case "animation":
                        return Math.min(Theme.itemSizeHuge*1.5, width*webPage.mediaRatio);
                    case "audio":
                    case "document":
                        return Theme.itemSizeSmall;
                    case "article":
                    default:
                        return 0;
                    }

            asynchronous: true
            sourceComponent: switch(webPage.type) {
                             case "photo":
                                 return imageComponent;
                             case "video":
                                 return videoComponent;
                             case "audio":
                                 return audioComponent;
                             case "document":
                                 return documentComponent;
                             case "animation":
                                 return animationComponent;
                             }
        }

        Component {
            id: imageComponent
            ImageContent {}
        }

        Component {
            id: videoComponent
            VideoContent {}
        }

        Component {
            id: audioComponent
            AudioContent {}
        }

        Component {
            id: documentComponent
            DocumentContent {}
        }

        Component {
            id: animationComponent
            AnimationContent {}
        }
    }

}
