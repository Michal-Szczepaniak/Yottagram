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
        var sessionToken = token.text.trim()
        var sessionCookie = cookie.text.trim()
        if (sessionToken === "" || sessionCookie === "") return

        if (sessionToken.indexOf("xoxc-") !== 0) {
            errorLabel.text = qsTr("The session token starts with xoxc-")
            return
        }

        errorLabel.text = ""
        busy = true
        authorization.setSlackCookie(sessionCookie)
        authorization.sendSlackToken(sessionToken)
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
                text: qsTr("Slack browser session")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("1. Log in to your workspace at app.slack.com in a desktop browser.\n2. In the developer tools console, run:\nObject.values(JSON.parse(localStorage.localConfig_v2).teams).map(t => t.name + \": \" + t.token)\nand copy your workspace's xoxc- token.\n3. In the developer tools storage tab, copy the value of the cookie named d for slack.com. It starts with xoxd-.")
            }

            TextField {
                id: token
                width: column.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                label: qsTr("Session token")
                placeholderText: "xoxc-..."
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: cookie.focus = true
            }

            TextField {
                id: cookie
                width: column.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                label: qsTr("d cookie")
                placeholderText: "xoxd-..."
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: page.submit()
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
                enabled: !page.busy && token.text.trim() !== "" && cookie.text.trim() !== ""
                onClicked: page.submit()
            }
        }
    }
}
