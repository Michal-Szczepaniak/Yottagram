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
import com.verdanditeam.manager 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    property var settings

    Component.onCompleted: proxyModel.getProxies()

    Connections {
        target: proxyModel
        onTestError: Notices.show(message, Notice.Short, Notice.Center)
        onProxyTestSuccessful: Notices.show("Test successful", Notice.Short, Notice.Center)
        onAddError: Notices.show(message, Notice.Short, Notice.Center)
        onEditError: Notices.show(message, Notice.Short, Notice.Center)
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Add server")
                onClicked: proxyModel.addServer()
            }
            MenuItem {
                text: qsTr("Disable proxy")
                onClicked: proxyModel.disable()
            }
        }

        Column {
            anchors.fill: parent

            PageHeader {
                id: header
                title: qsTr("Proxy settings")
            }

            SilicaListView {
                width: parent.width
                height: parent.height - header.height
                model: proxyModel
                delegate: ListItem {
                    TextSwitch {
                        width: Theme.itemSizeExtraSmall
                        height: Theme.itemSizeSmall
                        checked: isEnabled
                        busy: isEnabled && proxyModel.connectionState === TelegramManager.ConnectingToProxy
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: Theme.horizontalPageMargin
                    }

                    DetailItem {
                        label: server
                        value: port
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Enable")
                            onClicked: proxyModel.enable(index)
                        }
                        MenuItem {
                            text: qsTr("Test")
                            onClicked: proxyModel.test(index)
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            onClicked: Remorse.popupAction(
                                           root, "Removed",
                                           function() {
                                               proxyModel.remove(index)
                                           })
                        }
                    }

                    Component {
                        id: dialogComponent
                        Dialog {
                            id: serverEditDialog
                            width: Screen.width

                            onAccepted: proxyModel.saveServer(index)

                            Column {
                                anchors.fill: parent

                                spacing: Theme.paddingMedium

                                DialogHeader {
                                    id: header
                                    title: qsTr("Server settings")
                                }

                                ComboBox {
                                    id: typeSwitch
                                    width: parent.width
                                    currentIndex: type
                                    onCurrentIndexChanged: type = currentIndex
                                    menu: ContextMenu {
                                        MenuItem { text: "Socks5" }
                                        MenuItem { text: "HTTP" }
                                        MenuItem { text: "MTProto" }
                                    }
                                }

                                TextField {
                                    width: parent.width
                                    label: qsTr("Server")
                                    text: server
                                    onTextChanged: server = text
                                }

                                TextField {
                                    width: parent.width
                                    label: qsTr("Port")
                                    text: port
                                    onTextChanged: port = text
                                }

                                TextField {
                                    width: parent.width
                                    label: qsTr("Username")
                                    text: username
                                    onTextChanged: username = text
                                }

                                TextField {
                                    width: parent.width
                                    label: qsTr("Password")
                                    text: password
                                    onTextChanged: password = text
                                    visible: typeSwitch.currentIndex === 0 || typeSwitch.currentIndex === 1
                                }
                            }
                        }
                    }

                    onClicked: {
                        pageStack.push(dialogComponent)
                    }
                }
            }
        }
    }
}
