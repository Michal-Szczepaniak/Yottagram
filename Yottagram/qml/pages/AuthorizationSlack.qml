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

    property bool busy: false

    function submit() {
        if (token.text.trim() === "") return

        errorLabel.text = ""
        busy = true
        authorization.sendSlackToken(token.text)
    }

    Connections {
        target: authorization
        onSlackLoginFailed: {
            busy = false
            errorLabel.text = message
        }
        onIsAuthorizedChanged: {
            if (page.status === PageStatus.Active)
                pageStack.replace(Qt.resolvedUrl("ChatList.qml"))
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Login")
            }

            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("Slack user token")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            TextField {
                id: token
                width: column.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                placeholderText: "xoxp-..."
                Keys.onReturnPressed: page.submit()
            }

            Label {
                id: errorLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: text !== ""
                wrapMode: Text.Wrap
                color: Theme.errorColor
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Submit")
                enabled: !page.busy
                onClicked: page.submit()
            }
        }
    }
}
