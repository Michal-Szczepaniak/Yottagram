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

import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Silica.private 1.0
import Sailfish.Silica.Background 1.0
import QtGraphicalEffects 1.0

Item {
    id: bubble

    property bool leftSide: true
    property string source

    ColorBackground {
        id: colorBackground
        anchors.fill: parent

        radius: Math.min(Theme.paddingLarge, height / 2)
        roundedCorners: {
            var result = Corners.None
            result |= Corners.TopLeft | Corners.TopRight;
            result |= leftSide ? Corners.BottomRight : Corners.BottomLeft
            return result
        }

        color: {
            if (Theme.colorScheme === Theme.DarkOnLight) {
                return Theme.rgba(Theme.highlightColor, Theme.opacityFaint)
            }

            return Theme.rgba(Theme.primaryColor, Theme.opacityFaint)
        }
    }

    Image {
        id: messageCorner
        anchors.left: if(!leftSide) parent.right
        anchors.right: if(leftSide) parent.left
        anchors.bottom: parent.bottom
        width: sourceSize.width*2
        height: sourceSize.height*2
        source: bubble.source
        antialiasing: true
        visible: false
    }

    ColorOverlay {
        anchors.fill: messageCorner
        source: messageCorner
        color: Theme.colorScheme === Theme.DarkOnLight ? Theme.highlightColor : Theme.primaryColor
        opacity: Theme.opacityFaint
        antialiasing: true
        transform: leftSide ? matrixFlipped : dummyScale
        Matrix4x4 {
            id: matrixFlipped
            matrix: Qt.matrix4x4( -1, 0, 0, messageCorner.width, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
        }
        Scale {
            id: dummyScale
        }
    }
}
