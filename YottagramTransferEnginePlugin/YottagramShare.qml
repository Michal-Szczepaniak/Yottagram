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
import Nemo.DBus 2.0

Item {
    property var shareAction

    Component.onCompleted: {
        yottagram.call("share", [shareAction.toConfiguration()])
        shareAction.done()
    }

    DBusInterface {
        id: yottagram

        service: "com.verdanditeam.yottagram"
        path: "/"
        iface: "com.verdanditeam.yottagram"
    }
}

