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

    // Keep in sync with vendor/yottaslack/slack-app-manifest.yml
    readonly property var manifest: ({
        display_information: {
            name: "Yottagram",
            description: "Slack on Sailfish OS through Yottagram"
        },
        oauth_config: {
            scopes: {
                user: [
                    "channels:read", "groups:read", "im:read", "mpim:read",
                    "channels:history", "groups:history", "im:history", "mpim:history",
                    "users:read", "chat:write",
                    "channels:write", "groups:write", "im:write", "mpim:write",
                    "files:read", "files:write", "reactions:read", "reactions:write", "emoji:read"
                ]
            }
        },
        settings: {
            event_subscriptions: {
                user_events: [
                    "message.channels", "message.groups", "message.im", "message.mpim",
                    "reaction_added", "reaction_removed"
                ]
            },
            org_deploy_enabled: false,
            socket_mode_enabled: true,
            token_rotation_enabled: false
        }
    })

    function createApp(readOnly) {
        var app = JSON.parse(JSON.stringify(manifest))
        if (readOnly) {
            app.display_information.name = "Yottagram read-only"
            app.oauth_config.scopes.user = app.oauth_config.scopes.user.filter(function(scope) {
                return scope.indexOf(":write") === -1
            })
        }

        Qt.openUrlExternally("https://api.slack.com/apps?new_app=1&manifest_json=" + encodeURIComponent(JSON.stringify(app)))
    }

    function submit() {
        if (token.text.trim() === "") return

        errorLabel.text = ""
        busy = true
        if (appToken.text.trim() !== "") authorization.setSlackAppToken(appToken.text)
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

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("1. Create the app below, then install it to your workspace.\n2. Copy the User OAuth Token from OAuth & Permissions.\n3. For instant updates, generate an app-level token with the connections:write scope in Basic Information.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Create Slack app")
                onClicked: page.createApp(false)
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Create read-only Slack app")
                onClicked: page.createApp(true)
            }

            TextField {
                id: token
                width: column.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                label: qsTr("User OAuth Token")
                placeholderText: "xoxp-..."
                Keys.onReturnPressed: page.submit()
            }

            TextField {
                id: appToken
                width: column.width
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                label: qsTr("App token, optional")
                placeholderText: "xapp-..."
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
