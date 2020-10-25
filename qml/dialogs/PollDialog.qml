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
    canAccept: questionField.text !== "" && options.itemAt(0).text !== ""
    property var chatObject

    onDone: {
        if (result == DialogResult.Accepted) {
            chatObject.sendPoll(questionField.text, getOptions(), anonymous.checked, multipleAnswers.checked, quizMode.checked, options.checkedOption)
        }
    }

    function getOptions() {
        var selected = []

        for(var i = 0; i < listModel.rowCount(); i++) {
            if (options.itemAt(i).text !== "") selected.push(options.itemAt(i).text);
        }

        return selected
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height
        Column {
            id: column
            width: parent.width

            DialogHeader { }

            SectionHeader {
                text: qsTr("Question")
            }

            TextField {
                id: questionField
                width: parent.width
                placeholderText: qsTr("Ask a question")
                label: "Question"
            }

            SectionHeader {
                text: qsTr("Options")
            }

            Repeater {
                id: options
                model: ListModel {
                    id: listModel
                    ListElement { }
                }
                property int checkedOption: 0
                delegate: Item {
                    property alias text: textField.text
                    width: parent.width
                    height: textField.height

                    Connections {
                        target: options
                        onCheckedOptionChanged: optionSwitch.checked = (index === options.checkedOption)
                    }

                    Switch {
                        id: optionSwitch
                        visible: quizMode.checked
                        enabled: textField.text !== ""
                        onCheckedChanged: {
                            if (checked) options.checkedOption = index
                        }
                    }

                    TextField {
                        id: textField
                        anchors.left: quizMode.checked ? optionSwitch.right : parent.left
                        width: parent.width
                        placeholderText: qsTr("Add an option")
                        label: "Option"
                        validator: RegExpValidator { regExp: /^.{1,100}$/ }
                        onTextChanged: if (text !== "" && listModel.rowCount()-1 === index && listModel.rowCount() <= 10) listModel.append({})
                        onFocusChanged: if (focus === false && text === "" && listModel.rowCount() > 1 && options.itemAt(index-1).text === "") listModel.remove(index)
                    }
                }
            }

            SectionHeader {
                text: qsTr("Settings")
            }

            TextSwitch {
                id: anonymous
                checked: true
                text: qsTr("Anonymous voting")
                width: parent.width
            }

            TextSwitch {
                id: multipleAnswers
                text: qsTr("Multiple answers")
                width: parent.width
                enabled: !quizMode.checked
            }

            TextSwitch {
                id: quizMode
                text: qsTr("Quiz mode")
                width: parent.width
                onCheckedChanged: multipleAnswers.checked = false
            }
        }
    }
}
