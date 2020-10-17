import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

Item {
    id: root

    property var avatarPhoto
    property string userName
    property bool maskEnabled: true

    Rectangle {
        id: altChatPhoto
        anchors.fill: parent
        color: Theme.highlightColor
        width: height
        radius: maskEnabled ? 90 : 0
        visible: !root.avatarPhoto

        Label {
            anchors.centerIn: parent
            text: {
                var n = root.userName.split(' ')
                if (n.length === 2 && n[1].length > 0) {
                    return n[0][0].toUpperCase() + n[1][0].toUpperCase()
                }

                return n[0][0].toUpperCase()
            }
        }
    }

    Image {
        id: chatPhoto
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: root.avatarPhoto ? root.avatarPhoto.localPath : ""
        visible: typeof root.avatarPhoto !== "undefined"
        asynchronous: true
        cache: true
        layer.enabled: root.maskEnabled
        layer.effect: OpacityMask {
            maskSource: chatPhotoMask
        }
    }

    Rectangle {
        id: chatPhotoMask
        width: chatPhoto.height
        height: chatPhoto.height
        radius: 90
        visible: false
    }
}
