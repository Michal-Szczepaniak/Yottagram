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
import QtGraphicalEffects 1.0

Item {
    id: bubble

    property alias color: canvas.bubbleColor
    property alias bubbleOpacity: canvas.bubbleOpacity
    property alias radius: canvas.radius
    property alias canvas: canvas
    property bool leftSide: true
    property string source

    Canvas {
        id: canvas

        property color bubbleColor
        property real bubbleOpacity
        property color _color: Theme.rgba(bubbleColor, bubbleOpacity)
        property real radius: 30

        anchors.fill: parent

        onWidthChanged: update()
        onHeightChanged: update()
        onRadiusChanged: update()
        onBubbleColorChanged: update()
        onOpacityChanged: update()
        Component.onCompleted: update()

        onPaint: {
            var ctx = canvas.getContext("2d");
            ctx.strokeStyle = _color;
            ctx.fillStyle = _color;
            ctx.beginPath();
            ctx.moveTo(x + radius, y);
            ctx.lineTo(x + width - radius, y);
            ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
            if (leftSide) {
                ctx.lineTo(x + width, y + height - radius);
                ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
                ctx.lineTo(x, y + height);
            } else {
                ctx.lineTo(x + width, y + height);
                ctx.lineTo(x + radius, y + height);
                ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
            }

            ctx.lineTo(x, y + radius);
            ctx.quadraticCurveTo(x, y, x + radius, y);
            ctx.closePath();
            ctx.fill();
            ctx.stroke();
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
        color: canvas.bubbleColor
        opacity: canvas.bubbleOpacity
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
