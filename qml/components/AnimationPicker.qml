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
import QtMultimedia 5.0
import org.nemomobile.configuration 1.0

Item {
    id: root
    property var page
    property var animationPreview
    property int animationFileId: 0
    property int animationWidth: 0
    property int animationHeight: 0

    Column {
        id: column
        anchors.fill: parent
        spacing: 0

        SilicaGridView {
            id: savedAnimationsGridView
            width: root.width
            height: root.height - Theme.paddingLarge
            cellWidth: Math.floor(page.isLandscape ? root.width/(Math.floor(root.width/(root.height/4))) : root.width/4)
            cellHeight: cellWidth
            clip: true
            cacheBuffer: 0
            model: savedAnimations

            delegate: GridItem {
                width: savedAnimationsGridView.cellWidth
                height: width
                onClicked: {
                    root.animationWidth = animationWidth
                    root.animationHeight = animationHeight
                    root.animationFileId = file.id
                }

                onPressAndHold: animationPreview.path = file.localPath
                onReleased: animationPreview.path = ""

                Loader {
                    active: true
                    sourceComponent: videoThumbnail ? videoComponent : imageComponent
                    asynchronous: true

                    anchors.centerIn: parent
                    width: parent.width - Theme.paddingSmall*2
                    height: parent.width - Theme.paddingSmall*2
                }

                Component {
                    id: imageComponent

                    Image {
                        visible: !videoThumbnail
                        anchors.fill: parent
                        cache: true
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        source: thumbnail.localPath
                    }
                }


                Component {
                    id: videoComponent

                    Video {
                        visible: videoThumbnail
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        source: thumbnail.localPath
                        autoLoad: true
                        autoPlay: true
                        Component.onCompleted: pause()
                    }
                }
            }
        }
    }
}
