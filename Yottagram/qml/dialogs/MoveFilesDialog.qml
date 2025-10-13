/*

This file is part of Yottagram.
Copyright 2024, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

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
import com.verdanditeam.data 1.0

Dialog {
    allowedOrientations: Orientation.All

    canAccept: !dataManager.newPathExists

    onDone: {
        if (result == DialogResult.Accepted) {
            dataManager.commitNewPath();
            Qt.quit();
        }
    }

    DataManager {
        id: dataManager
    }

    Column {
        id: column
        width: parent.width

        DialogHeader {
            title: qsTr("Move yottagram's data")
        }

        Item {
            width: 1
            height: Theme.paddingLarge
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Theme.horizontalPageMargin*2
            color: 'red'
            font.pixelSize: Theme.fontSizeLarge
            font.bold: true
            text: qsTr("App will be closed upon accepting")
        }

        Item {
            width: 1
            height: Theme.paddingMedium
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Theme.horizontalPageMargin*2
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Old path: %1").arg(dataManager.getOldPath());
        }

        Item {
            width: 1
            height: Theme.paddingLarge
        }

        TextField {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: dataManager.newPath
            onTextChanged: dataManager.newPath = text
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Theme.horizontalPageMargin*2
            color: "red"
            visible: dataManager.newPathExists
            text: qsTr("Path already exists")
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Theme.horizontalPageMargin*2
            color: "red"
            visible: dataManager.noPermission
            text: qsTr("No permission to create directory")
        }
    }
}
