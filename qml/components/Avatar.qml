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

Item {
    id: root

    property var avatarPhoto
    property string userName
    property bool self: false
    property bool maskEnabled: true

    Rectangle {
        id: altChatPhoto
        anchors.fill: parent
        color: Theme.highlightColor
        width: height
        radius: maskEnabled ? 90 : 0
        visible: !root.avatarPhoto || root.self

        Label {
            anchors.centerIn: parent
            visible: !root.self
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

    Image {
        id: chatPhoto
        anchors.centerIn: parent
        width: root.self ? Theme.itemSizeMedium/1.5 : parent.width
        height: width
        sourceSize.width: width
        sourceSize.height: height
        fillMode: Image.PreserveAspectFit
        source: root.avatarPhoto ? root.avatarPhoto.localPath : (root.self ? "qrc:///icons/icon-s-bookmark.svg" : "")
        visible: typeof root.avatarPhoto !== "undefined" || root.self
        asynchronous: true
        cache: true
        layer.enabled: root.maskEnabled
        layer.effect: OpacityMask {
            cached: true
            maskSource: chatPhotoMask
        }
    }

    Rectangle {
        id: chatPhotoMask
        width: chatPhoto.height
        height: chatPhoto.height
        radius: 90
        visible: false
    }
}
