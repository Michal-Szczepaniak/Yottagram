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
    property var seconds

    onDone: {
        if (result == DialogResult.Accepted) {
            seconds = durationComboBox.currentItem.ttl
        }
    }

    function getOptions() {
        var selected = []

        for(var i = 0; i < listModel.rowCount(); i++) {
            if (options.itemAt(i).text !== "") selected.push(options.itemAt(i).text);
        }

        return selected
    }
    Column {
        id: column
        width: parent.width

        DialogHeader { }

        ComboBox {
            id: durationComboBox
            width: parent.width
            label: "Self-destruct timer"

            menu: ContextMenu {
                Repeater {
                    model: 16

                    MenuItem {
                        text: qsTr("%L1 second(s)").arg(index)
                        property int ttl: index
                    }
                }

                MenuItem {
                    text: qsTr("30 seconds")
                    property int ttl: 30
                }

                MenuItem {
                    text: qsTr("1 minute")
                    property int ttl: 60
                }

                MenuItem {
                    text: qsTr("1 hour")
                    property int ttl: 3600
                }

                MenuItem {
                    text: qsTr("1 day")
                    property int ttl: 86400
                }

                MenuItem {
                    text: qsTr("1 week")
                    property int ttl: 604800
                }
            }
        }
    }
}
