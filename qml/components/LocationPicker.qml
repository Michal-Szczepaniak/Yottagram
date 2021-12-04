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
import QtPositioning 5.2

Dialog {
    id: root
    property var page

    onAccepted: {
        chat.sendLocation(positionSource.position.coordinate.latitude, positionSource.position.coordinate.longitude, positionSource.position.horizontalAccuracy, page.replyMessageId)
    }

    canAccept: positionSource.valid

    DialogHeader {
        title: qsTr("Location Picker")
    }

    PositionSource {
        id: positionSource
        updateInterval: 1000
        active: true
    }

    Column {
        anchors.centerIn: parent

        Property {
            name: qsTr("Latitude:")
            value: positionSource.position.coordinate.latitude
        }

        Property {
            name: qsTr("Longitude:")
            value: positionSource.position.coordinate.longitude
        }
    }
}
