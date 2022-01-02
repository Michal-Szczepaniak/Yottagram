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
    id: root

    property var avatarPhoto
    property string userName
    property bool maskEnabled: true
    property bool forceBackground: false

    Rectangle {
        id: altChatPhoto
        anchors.fill: parent
        color: Theme.highlightColor
        radius: maskEnabled ? width/2 : 0
        visible: !root.avatarPhoto || root.forceBackground

        Label {
            anchors.centerIn: parent
            visible: !root.avatarPhoto
            text: {
                if (root.userName === "") return "DA";
                var n = root.userName.split(' ')
                if (n.length === 2 && n[1].length > 0) {
                    return n[0][0].toUpperCase() + n[1][0].toUpperCase()
                }

                return n[0][0].toUpperCase()
            }
        }
    }

    ShaderEffect {
        anchors.fill: parent
        visible: maskEnabled && typeof root.avatarPhoto !== "undefined"
        property variant src: chatPhoto
        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform sampler2D src;
            uniform lowp float qt_Opacity;
            void main() {
                lowp vec4 tex = texture2D(src, qt_TexCoord0);
                lowp float is_outside_circle = step(distance(qt_TexCoord0, vec2(0.5)), 0.5);
                gl_FragColor = mix(vec4(0.0),tex, is_outside_circle);
                }"
    }

    Image {
        id: chatPhoto
        anchors.centerIn: parent
        width: parent.width
        height: width
        fillMode: Image.PreserveAspectFit
        source: root.avatarPhoto ? root.avatarPhoto : ""
        sourceSize.width: width
        sourceSize.height: height
        visible: !maskEnabled && !altChatPhoto.visible
        asynchronous: true
        cache: true
    }
}
