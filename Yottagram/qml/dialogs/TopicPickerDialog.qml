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

Dialog {
    id: dialog

    property var chat: null
    property var shareConfiguration: null

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        DialogHeader {
            id: dialogHeader
            title: qsTr("Pick topic")
        }


        SilicaListView {
            anchors {
                top: dialogHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            clip: true
            model: chat.getTopicNames()
            delegate: ListItem {
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - Theme.horizontalPageMargin*2
                    x: Theme.horizontalPageMargin
                    text: modelData
                }

                onClicked: {
                    chat.setTopic(index)
                    pageStack.push(Qt.resolvedUrl("../pages/Chat.qml"), { chat: chat, shareConfiguration: shareConfiguration })
                }
            }
        }
    }
}
