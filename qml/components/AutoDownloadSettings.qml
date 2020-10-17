import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    property var settings


    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Load slow preset")
                onClicked: settings.loadPreset("low")
            }

            MenuItem {
                text: qsTr("Load medium preset")
                onClicked: settings.loadPreset("medium")
            }

            MenuItem {
                text: qsTr("Load high preset")
                onClicked: settings.loadPreset("high")
            }
        }

        Column {
            anchors.fill: parent

            PageHeader {
                title: qsTr("Auto download settings")
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin*2
                height: parent.height

                TextSwitch {
                    width: parent.width
                    text: qsTr("Enable auto download")
                    checked: settings.isAutoDownloadEnabled
                    onCheckedChanged: settings.isAutoDownloadEnabled = checked
                }

                Slider {
                    width: parent.width
                    label: qsTr("Maximum Photo File Size")
                    value: settings.maxPhotoFileSize
                    valueText: {
                        if (value < 1024) return qsTr("%L1B").arg(Math.floor(value))
                        if (value < 1024*1024) return qsTr("%L1KB").arg(Math.floor(value/1024))
                        if (value <= 1024*1024*1024) return qsTr("%L1MB").arg(Math.floor(value/1024/1024))
                    }

                    minimumValue: 0
                    maximumValue: 1024*1024*20
                    stepSize: 1
                    onReleased: settings.maxPhotoFileSize = value
                }

                Slider {
                    width: parent.width
                    label: qsTr("Maximum Video File Size")
                    value: settings.maxVideoFileSize
                    valueText: {
                        if (value < 1024) return qsTr("%L1B").arg(Math.floor(value))
                        if (value < 1024*1024) return qsTr("%L1KB").arg(Math.floor(value/1024))
                        if (value <= 1024*1024*1024) return qsTr("%L1MB").arg(Math.floor(value/1024/1024))
                    }

                    minimumValue: 0
                    maximumValue: 1024*1024*20
                    stepSize: 1
                    onReleased: settings.maxVideoFileSize = value
                }

                Slider {
                    width: parent.width
                    label: qsTr("Maximum Other File Size")
                    value: settings.maxOtherFileSize
                    valueText: {
                        if (value < 1024) return qsTr("%L1B").arg(Math.floor(value))
                        if (value < 1024*1024) return qsTr("%L1KB").arg(Math.floor(value/1024))
                        if (value <= 1024*1024*1024) return qsTr("%L1MB").arg(Math.floor(value/1024/1024))
                    }

                    minimumValue: 0
                    maximumValue: 1024*1024*20
                    stepSize: 1
                    onReleased: settings.maxOtherFileSize = value
                }
            }
        }
    }
}
