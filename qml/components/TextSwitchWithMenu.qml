import QtQuick 2.0
import Sailfish.Silica 1.0

TextSwitch {
    id: root
    propagateComposedEvents: true
    property alias menu: listItem.menu
    property alias description: description.text
    height: listItem.height

    Label {
        id: description
        anchors.top: parent.top
        anchors.topMargin: Theme.paddingMedium*4
        x: Theme.itemSizeExtraSmall + Theme.paddingMedium - Theme.paddingSmall
        width: parent.width
        height: implicitHeight + Theme.paddingSmall
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: Theme.fontSizeSmall
        color: Theme.secondaryColor
    }

    ListItem {
        id: listItem
        width: parent.width
        propagateComposedEvents: true
        contentHeight: Theme.itemSizeSmall + (description.text != "" ? description.height : 0)
    }
}
