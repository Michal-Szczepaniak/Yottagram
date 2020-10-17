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
