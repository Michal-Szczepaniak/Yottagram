import QtQuick 2.0
import Sailfish.Silica 1.0

IconButton {
    property int __silica_menuitem
    signal earlyClick
    signal clicked
    signal delayedClick

    width: Theme.itemSizeSmall
    height: width
}
