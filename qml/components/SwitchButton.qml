import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    id: switchButton

    property bool checked

    onClicked: {
        checked = !checked
    }

    width: indicator.width
    height: indicator.height

    GlassItem {
        id: indicator
        opacity: switchButton.enabled ? 1.0 : Theme.opacityLow
        color: highlighted ? Theme.highlightColor : Theme.lightPrimaryColor
        backgroundColor: checked ? switchButton.palette.backgroundGlowColor : "transparent"
        anchors.horizontalCenter: parent.horizontalCenter
        dimmed: !checked
    }
}
