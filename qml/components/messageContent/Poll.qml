import QtQuick 2.6
import Sailfish.Silica 1.0
import "../"

Column {
    id: column

    Row {
        spacing: Theme.paddingSmall

        Label {
            text: (poll.isAnonymous ? qsTr("Anonymous") : "") + " " + (poll.typeName === "quiz" ? qsTr("Quiz") : qsTr("Poll"))
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
        }

        Item { width: 1 }

        Repeater {
            model: if (!poll.isAnonymous && poll.totalVoterCount > 0) poll.recentVoters.split(',')
            delegate: Avatar {
                width: Theme.itemSizeExtraSmall/2
                height: width
                avatarPhoto: if (user.hasPhoto) user.smallPhoto
                userName: user.name
                readonly property var user: users.getUserAsVariant(modelData)
            }
        }
    }

    Repeater {
        id: options
        width: parent.width
        model: poll
        signal selectionChanged

        onSelectionChanged: {
            selectButton.visible = getOptions() !== "" && !poll.isAnswered
        }

        function getOptions() {
            var selected = []

            for(var i = 0; i < model.rowCount(); i++) {
                if (options.itemAt(i).selected) selected.push(i)
            }

            return selected.join(',')
        }

        delegate: BackgroundItem {
            id: item
            width: parent.width
            height: percentage.visible ? percentage.height : (option.height + Theme.paddingMedium)
            property bool selected: false

            onSelectedChanged:
                if (!poll.isAnswered) {
                    if (poll.allowMultipleAnswers) {
                        options.selectionChanged()
                    } else {
                        chat.chose(messageId, index)
                    }
                }

            MouseArea {
                width: parent.width
                height: parent.height
                onClicked: if (!poll.isAnswered) item.selected = !item.selected

                GlassItem {
                    id: selectItem
                    color: poll.typeName === "quiz" ? ((poll.correctOptionId === index) ? "green" : (isChosen ? "red" : "white")) : "white"
                    enabled: !poll.isAnswered
                    dimmed: !isChosen && !item.selected
                    height: percentage.height
                    width: height
                }

                Label {
                    id: option
                    anchors.left: selectItem.right
                    anchors.top: parent.top
                    anchors.topMargin: Theme.paddingMedium
                    visible: !poll.isAnswered
                    text: !poll.isAnswered ? optionText : ""
                    width: parent.width - selectItem.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                Slider {
                    id: percentage
                    enabled: false
                    value: votePercentage
                    minimumValue: 0
                    maximumValue: 100
                    handleVisible: false
                    height: Theme.itemSizeExtraSmall
                    width: parent.width
                    visible: poll.isAnswered
                    label: (Math.round(votePercentage * 100) / 100) + "% " + optionText
                }
            }
        }
    }

    Button {
        id: selectButton
        text: qsTr("Select answers")
        visible: false
        onClicked: chat.chose(messageId, options.getOptions())

        Connections {
            target: poll
            onPollChanged: {
                selectButton.visible = options.getOptions() !== "" && !poll.isAnswered
            }
        }
    }
}
