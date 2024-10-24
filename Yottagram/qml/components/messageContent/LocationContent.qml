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
import Nemo.DBus 2.0

Image {
    source: "https://api.mapbox.com/styles/v1/mapbox/streets-v11/static/geojson(" + encodeURIComponent(JSON.stringify({"type":"Point","coordinates":[location.longitude,location.latitude]})) + ")/" + location.longitude + "," + location.latitude + ",12,0,0/" + width + "x" + height + "@2x?access_token=pk.eyJ1IjoibWlzdGVyLW1hZ2lzdGVyIiwiYSI6ImNrdG1kM2xhMTI0YmkydXBldzFuNjdkZTIifQ.IQx3FyHbkgYmcWpPiJnDow"
    asynchronous: true
    cache: true

    MouseArea {
        anchors.fill: parent
        onClicked: {
            puremapsDbus.call('ShowPoi', [qsTr("Telegram location"), location.latitude, location.longitude])
        }
    }

    DBusInterface {
        id: puremapsDbus

        service: 'io.github.rinigus.PureMaps'
        iface: 'io.github.rinigus.PureMaps'
        path: '/io/github/rinigus/PureMaps'
    }
}
