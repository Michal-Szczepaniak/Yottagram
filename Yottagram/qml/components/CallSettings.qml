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

Page {
    id: page

    allowedOrientations: Orientation.All
    property var settings


    SilicaFlickable {
        anchors.fill: parent

        Column {
            anchors.fill: parent

            PageHeader {
                title: qsTr("Call settings")
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin*2
                height: parent.height

                TextSwitch {
                    width: parent.width
                    text: qsTr("Echo cancellation")
                    checked: calls.echoCancellation
                    onCheckedChanged: calls.echoCancellation = checked
                }

                TextSwitch {
                    width: parent.width
                    text: qsTr("Noise suppression")
                    checked: calls.noiseSuppression
                    onCheckedChanged: calls.noiseSuppression = checked
                }

                TextSwitch {
                    width: parent.width
                    text: qsTr("Auto gain control")
                    checked: calls.autoGainControl
                    onCheckedChanged: calls.autoGainControl = checked
                }

                TextSwitch {
                    width: parent.width
                    text: qsTr("Highpass filter")
                    checked: calls.highpassFilter
                    onCheckedChanged: calls.highpassFilter = checked
                }

                TextSwitch {
                    width: parent.width
                    text: qsTr("Typing detection")
                    checked: calls.typingDetection
                    onCheckedChanged: calls.typingDetection = checked
                }
            }
        }
    }
}
