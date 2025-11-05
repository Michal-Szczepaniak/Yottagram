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
import SortFilterProxyModel 0.2
import org.nemomobile.configuration 1.0
import com.verdanditeam.chat 1.0
import "functions/twemoji.js" as Twemoji

Item {
    id: root
    property var page
    property string emoji: ""
    property var reactionMessage: 0

    ConfigurationGroup {
        id: settings
        path: "/apps/yottagram"

        property real emojiPickerPostition: 0
        property int emojiPickerPickedPack: 0

        Component.onCompleted: {
            emojiProxyModel.sourceModel = emojiSets.data(emojiSets.index(settings.emojiPickerPickedPack, 0), 266)
        }
    }

    Connections {
        target: emojiSets
        onGotStickerSet: {
            if (stickerSetId == emojiSets.data(emojiSets.index(settings.emojiPickerPickedPack, 0), 257)) {
                emojiProxyModel.sourceModel = emojiSets.data(emojiSets.index(settings.emojiPickerPickedPack, 0), 266)
            }
        }
    }

    SortFilterProxyModel {
        id: emojiProxyModel
        filters: [
            RegExpFilter {
                roleName: "emoji"
                pattern: chat.availableReactions.length === 0 ? "0" : chat.availableReactions.join('|')
                enabled: reactionMessage != 0
            },
            ValueFilter {
                roleName: "type"
                enabled: reactionMessage != 0 && !currentUser.hasPremium
                value: "customEmoji"
                inverted: true
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
            id: emojiSetsListView
            width: parent.width
            height: Theme.itemSizeLarge
            model: emojiSets
            orientation: Qt.Horizontal
            layoutDirection: Qt.LeftToRight
            contentX: settings.emojiPickerPostition
            onContentXChanged: settings.emojiPickerPostition = contentX
            Component.onCompleted: contentX = contentX
            delegate: MouseArea {
                height: Theme.itemSizeLarge
                width: Theme.itemSizeLarge
                Image {
                    width: Theme.itemSizeLarge - Theme.paddingLarge
                    height: Theme.itemSizeLarge - Theme.paddingLarge
                    sourceSize.width: width
                    sourceSize.height: height
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    asynchronous: true
                    source: {
                        switch (type) {
                        case "smileys":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("🙂") + '.svg';
                        case "nature":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("🐱") + '.svg';
                        case "food":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("🍎") + '.svg';
                        case "activities":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("⚽️") + '.svg';
                        case "travel":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("🚗") + '.svg';
                        case "objects":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("💡") + '.svg';
                        case "symbols":
                            return 'qrc:///emoji/' + Twemoji.emojifyRaw("❤️") + '.svg';
                        }

                        return thumbnail && thumbnail.isDownloaded ? thumbnail.localPath : "image://theme/icon-m-other"
                    }

                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        anchors.centerIn: parent
                        running: (!thumbnail || !thumbnail.isDownloaded || thumbnail.isDownloading) && type === "customEmoji"
                    }
                }

                onClicked: {
                    settings.emojiPickerPickedPack = index
                    emojiProxyModel.sourceModel = stickerSet
                    chat.sendAction(Chat.Typing)
                }
            }
        }

        SilicaGridView {
            id: stickerSetGridView
            width: root.width
            height: root.height - emojiSetsListView.height - Theme.paddingLarge
            cellWidth: Math.floor(page.isLandscape ? root.width/(Math.floor(root.width/(root.height/8))) : root.width/8)
            cellHeight: cellWidth
            clip: true
            cacheBuffer: cellWidth*4
            model: emojiProxyModel

            delegate: GridItem {
                width: stickerSetGridView.cellWidth
                height: width
                onClicked: {
                    if (reactionMessage != 0) {
                        chat.addReaction(reactionMessage, reactionId)
                        root.emoji = "0"
                    } else {
                        root.emoji = (type === "customEmoji" ? "<a href=\"emoji://" + id + "\"><img src=\"" + thumbnail.localPath + "\" width=\"" + Theme.fontSizeSmall + "\" height=\"" + Theme.fontSizeSmall + "\"/></a>" : emoji)
                    }
                }

                Image {
                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingLarge
                    height: parent.width - Theme.paddingLarge
                    sourceSize.width: width
                    sourceSize.height: height
                    cache: false
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    source: type === "customEmoji" ? thumbnail.localPath : 'qrc:///emoji/' + Twemoji.emojifyRaw(emoji) + '.svg'
                }
            }
        }
    }
}
