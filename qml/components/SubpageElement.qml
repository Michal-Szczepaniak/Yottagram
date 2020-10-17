import QtQuick 2.6
import Sailfish.Silica 1.0

BackgroundItem {
    id: barButton
    property alias text: label.text

    height: Theme.itemSizeSmall

    highlightedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)

    Label {
        id: label
        anchors.left: parent.left
        anchors.leftMargin: Theme.paddingLarge
        anchors.verticalCenter: parent.verticalCenter
        color: barButton.highlighted ? Theme.highlightColor : Theme.primaryColor
    }

    Image {
        id: rightIcon
        anchors.right: parent.right
        anchors.rightMargin: Theme.paddingMedium
        anchors.verticalCenter: parent.verticalCenter
        source: "image://theme/icon-m-right"
    }
}
