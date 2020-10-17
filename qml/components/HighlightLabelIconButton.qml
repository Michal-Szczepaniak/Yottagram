import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    id: root
    height: column.height

    property alias source: icon.source
    property alias text: label.text
    property bool highlighted: pressed

    Column {
        id: column
        anchors.centerIn: parent
        spacing: Theme.paddingMedium

        Icon {
            id: icon
            highlighted: root.highlighted
            anchors.horizontalCenter: parent.horizontalCenter
            width: Theme.iconSizeMedium
            height: Theme.iconSizeMedium
        }

        Label {
            id: label
            truncationMode: TruncationMode.Fade
            color: root.highlighted ? Theme.highlightColor : Theme.primaryColor
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeSmallBase
            width: root.width
        }
    }
}
