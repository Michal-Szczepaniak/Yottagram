import QtQuick 2.6
import Sailfish.Silica 1.0
import QtMultimedia 5.6
import com.verdanditeam.yt 1.0
import Sailfish.Media 1.0
import org.nemomobile.mpris 1.0
import com.jolla.settings.system 1.0
import org.nemomobile.systemsettings 1.0
import org.nemomobile.configuration 1.0
import Nemo.Notifications 1.0
import Nemo.KeepAlive 1.2
import QtGraphicalEffects 1.0
import QtSensors 5.0
import Sailfish.Pickers 1.0
import Sailfish.Share 1.0

Page {
    id: page

    allowedOrientations: app.videoCover && Qt.application.state === Qt.ApplicationInactive ? Orientation.Portrait : Orientation.All
    showNavigationIndicator: _controlsVisible

    property string url
    property bool isLocal
    property bool _controlsVisible: true
    property bool videoChanging: false
    property bool landscape: ( page.orientation === Orientation.Landscape || page.orientation === Orientation.LandscapeInverted )
    property bool landscapeCover: (_orientation === OrientationReading.LeftUp || _orientation === OrientationReading.RightUp) && app.videoCover && Qt.application.state === Qt.ApplicationInactive
    property int inactiveBrightness: -1
    property int activeBrightness: -1
    property bool fillMode: false
    property bool playlistMode: false
    Keys.onRightPressed: videoPlayer.seek(videoPlayer.position + 5000)
    Keys.onLeftPressed: videoPlayer.seek(videoPlayer.position - 5000)
    property int _orientation: OrientationReading.TopUp

    DisplaySettings {
        id: displaySettings
        onBrightnessChanged: {
            if (inactiveBrightness === -1) {
                inactiveBrightness = brightness
                activeBrightness = brightness
            }
        }
    }

    OrientationSensor {
        id: orientationSensor
        active: true

        onReadingChanged: {
            if (reading.orientation >= OrientationReading.TopUp && reading.orientation <= OrientationReading.RightUp) {
                _orientation = reading.orientation
            }
        }
    }

    Timer {
        id: hideControlsAutomatically
        interval: 3000
        running: false
        repeat: false
        onTriggered: _controlsVisible = false
    }

    Timer {
        id: hideVolumeSlider
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            volumeSlider.visible = false
            volumeIndicator.visible = false
        }
    }

    Timer {
        id: hideBrightnessSlider
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            brightnessSlider.visible = false
            brightnessIndicator.visible = false
        }
    }

    function showHideControls() {
//        jupii.ping()

        if (_controlsVisible) {
            showAnimation.start()
            hideControlsAutomatically.restart()
            dimPane.show()
        } else {
            hideAnimation.start()
            dimPane.hide()
        }

        if ((_controlsVisible && page.landscape) || page.orientation === Orientation.Portrait) {
            showAnimation3.start()
        } else {
            hideAnimation3.start()
        }
    }

    onOrientationChanged: {
        page.landscape = ( page.orientation === Orientation.Landscape || page.orientation === Orientation.LandscapeInverted )

        if ( Qt.application.state === Qt.ApplicationActive && page.status === PageStatus.Active ) {
            if (landscape) {
                hideAnimation3.start()
                displaySettings.brightness = activeBrightness
            } else {
                showAnimation3.start()
                displaySettings.brightness = inactiveBrightness
            }
        }
    }

    on_ControlsVisibleChanged: {
        showHideControls()
    }

    onStatusChanged: {
        if(status === PageStatus.Active) {
            Theme.setColorScheme("dark")
            showHideControls()
            pacontrol.update()
            hideControlsAutomatically.restart()
        } else if (status === PageStatus.Inactive) {
            displaySettings.brightness = inactiveBrightness
        }
    }

    Component.onDestruction: {
        displaySettings.brightness = inactiveBrightness
    }

    Connections {
        target: pacontrol
        onVolumeChanged: {
            volumeSlider.value = volume
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if ( state === Qt.ApplicationInactive ) {
                displaySettings.brightness = inactiveBrightness
            } else if ( state === Qt.ApplicationActive && landscape ) {
                displaySettings.brightness = activeBrightness
            }
        }
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent

        Rectangle {
            x: 0
            y: 0
            width: page.width
            height: landscapeCover ? page.width*1.6 : page.height
            color: "black"

            VideoPlayer {
                id: videoPlayer
                width : landscapeCover ? page.width*1.6 : (landscape ? page.width : (page.fillMode ? page.height : page.width))
                videoSource: url
                anchors.centerIn: parent
                height: landscapeCover
                          ? page.width
                          : (page.fillMode ? page.width : page.height)
                transform: Rotation {
                    origin.x: (page.width*1.6)/2
                    origin.y: page.width/2
                    angle: landscapeCover
                           ? (_orientation === OrientationReading.LeftUp ? -90 : 90)
                           : 0
                }

                Behavior on width { PropertyAnimation { duration: pinchArea.pinching ? 250 : 0 } }
                Behavior on height { PropertyAnimation { duration: pinchArea.pinching ? 250 : 0 } }

                onStateChanged: {
                    if (state === VideoPlayer.StatePaused) {
                    } else if (state === VideoPlayer.StateStopped) {
                    } else if (state === VideoPlayer.StatePlaying) {
                        videoPlayer.setPlaybackSpeed(playbackSpeedSlider.value)
                        showHideControls()
                    }

                    mprisPlayer.playbackState = state === VideoPlayer.StatePlaying ?
                                Mpris.Playing : state === VideoPlayer.StatePaused ?
                                    Mpris.Paused : Mpris.Stopped
                }

                onPositionChanged: {
                    progressSlider.value = position
                }

                Rectangle {
                    id: dimPane
                    anchors.fill: parent
                    property double colorOpacity: 0
                    color: Theme.rgba("black", colorOpacity)
                    Behavior on colorOpacity {
                        NumberAnimation {}
                    }
                    visible: true

                    function show() {
                        colorOpacity = 0.5
                    }

                    function hide() {
                        colorOpacity = 0
                    }

                    Label {
                        id: errorText
                        visible: parent.visible
                        anchors.centerIn: parent
                        font.pointSize: Theme.fontSizeExtraLarge
                        font.family: Theme.fontFamilyHeading
                    }
                }


                BusyIndicator {
                    size: BusyIndicatorSize.Large
                    anchors.centerIn: parent
                    running: !isLocal && videoPlayer.bufferProgress != 1
                }

                PinchArea {
                    id: pinchArea
                    anchors.fill: parent
                    enabled: true
                    property bool pinching: false
                    onPinchUpdated: {
                        if (pinch.scale < 0.9) {
                            page.fillMode = false
                        } else if (pinch.scale > 1.5) {
                            page.fillMode = true
                        }
                    }
                    onPinchStarted: pinching = true
                    onPinchFinished: pinching = false
                    MouseArea {
                        id: mousearea
                        anchors.fill: parent
                        property int offset: page.height/20
                        property int offsetHeight: height - (offset*2)
                        property int step: offsetHeight / 10
                        property bool stepChanged: false
                        property int brightnessStep: displaySettings.maximumBrightness / 10
                        property int lambdaVolumeStep: -1
                        property int lambdaBrightnessStep: -1
                        property int currentVolume: -1

                        Timer{
                            id: doubleClickTimer
                            interval: 200
                        }


                        function calculateStep(mouse) {
                            return Math.round((offsetHeight - (mouse.y-offset)) / step)
                        }

                        onReleased: {
                            if (doubleClickTimer.running) doubleClicked(mouse)
                            if (!doubleClickTimer.running) doubleClickTimer.start()
                            if (!stepChanged) _controlsVisible = !_controlsVisible

                            flickable.flickableDirection = Flickable.VerticalFlick
                            lambdaVolumeStep = -1
                            lambdaBrightnessStep = -1
                            stepChanged = false
                        }

                        onPressed: {
                            pacontrol.update()
                            flickable.flickableDirection = Flickable.HorizontalFlick
                            lambdaBrightnessStep = lambdaVolumeStep = calculateStep(mouse)
                        }

                        function doubleClicked(mouse) {
                            var newPos = null
                            if(mouse.x < mousearea.width/2 ) {
                                newPos = videoPlayer.position - 5000
                                if(newPos < 0) newPos = 0
                                videoPlayer.seek(newPos)
                                backwardIndicator.visible = true
                            } else if (mouse.x > mousearea.width/2) {
                                newPos = videoPlayer.position + 5000
                                if(newPos > videoPlayer.duration) {
                                    return
                                }
                                videoPlayer.seek(newPos)
                                forwardIndicator.visible = true
                            }
                        }

                        Connections {
                            target: pacontrol
                            onVolumeChanged: {
                                mousearea.currentVolume = volume
                                if (volume > 10) {
                                    mousearea.currentVolume = 10
                                } else if (volume < 0) {
                                    mousearea.currentVolume = 0
                                }
                            }
                        }

                        onPositionChanged: {
                            var step = calculateStep(mouse)
                            if((mouse.y - offset) > 0 && (mouse.y + offset) < offsetHeight && mouse.x < mousearea.width/2 && lambdaVolumeStep !== step) {
                                pacontrol.setVolume(currentVolume - (lambdaVolumeStep - step))
                                volumeSlider.value = currentVolume - (lambdaVolumeStep - step)
                                lambdaVolumeStep = step
                                volumeSlider.visible = true
                                volumeIndicator.visible = true
                                hideVolumeSlider.restart()
                                pacontrol.update()
                                stepChanged = true
                            } else if ((mouse.y - offset) > 0 && (mouse.y + offset) < offsetHeight && mouse.x > mousearea.width/2 && lambdaBrightnessStep !== step) {
                                var relativeStep = Math.round(displaySettings.brightness/brightnessStep) - (lambdaBrightnessStep - step)
                                if (relativeStep > 10) relativeStep = 10;
                                if (relativeStep < 0) relativeStep = 0;
                                displaySettings.brightness = relativeStep * brightnessStep
                                activeBrightness = relativeStep * brightnessStep
                                lambdaBrightnessStep = step
                                brightnessSlider.value = relativeStep
                                brightnessSlider.visible = true
                                brightnessIndicator.visible = true
                                hideBrightnessSlider.restart()
                                stepChanged = true
                            }
                        }
                    }
                }
            }

            DisplayBlanking {
                preventBlanking: videoPlayer.state == VideoPlayer.StatePlaying
            }

            Row {
                id: volumeIndicator
                anchors.centerIn: parent
                visible: false
                spacing: Theme.paddingLarge

                Image {
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    source: "image://theme/icon-m-speaker-on"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    text: (mousearea.currentVolume * 10) + "%"
                    font.pixelSize: Theme.fontSizeHuge
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                id: brightnessIndicator
                anchors.centerIn: parent
                visible: false
                spacing: Theme.paddingLarge

                Image {
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    source: "image://theme/icon-m-light-contrast"
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    text: (Math.round(displaySettings.brightness/mousearea.brightnessStep) * 10) + "%"
                    font.pixelSize: Theme.fontSizeHuge
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                id: backwardIndicator
                anchors.centerIn: parent
                visible: false
                spacing: -Theme.paddingLarge*2

                Image {
                    id: prev1
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "image://theme/icon-cover-play"

                    transform: Rotation{
                        angle: 180
                        origin.x: prev1.width/2
                        origin.y: prev1.height/2
                    }
                }
                Image {
                    id: prev2
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "image://theme/icon-cover-play"

                    transform: Rotation{
                        angle: 180
                        origin.x: prev2.width/2
                        origin.y: prev2.height/2
                    }
                }

                Timer {
                    id: hideBackward
                    interval: 300
                    onTriggered: backwardIndicator.visible = false
                }

                onVisibleChanged: if (backwardIndicator.visible) hideBackward.start()
            }

            Row {
                id: forwardIndicator
                anchors.centerIn: parent
                visible: false
                spacing: -Theme.paddingLarge*2

                Image {
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "image://theme/icon-cover-play"

                }
                Image {
                    width: Theme.itemSizeLarge
                    height: Theme.itemSizeLarge
                    anchors.verticalCenter: parent.verticalCenter
                    fillMode: Image.PreserveAspectFit
                    source: "image://theme/icon-cover-play"
                }

                Timer {
                    id: hideForward
                    interval: 300
                    onTriggered: forwardIndicator.visible = false
                }

                onVisibleChanged: if (forwardIndicator.visible) hideForward.start()
            }

            NumberAnimation {
                id: showAnimation
                targets: [progress, duration, playButton, streamsButton, playbackSpeedButton]
                properties: "opacity"
                to: 1
                duration: 100
            }
            NumberAnimation {
                id: hideAnimation
                targets: [progress, duration, playButton, streamsButton, playbackSpeedButton, playbackSpeedSlider]
                properties: "opacity"
                to: 0
                duration: 100
            }

            IconButton {
                id: playButton
                enabled: opacity != 0
                icon.source: videoPlayer.state == VideoPlayer.StatePlaying ? "image://theme/icon-m-pause" : "image://theme/icon-m-play"
                anchors.centerIn: parent
                onClicked: videoPlayer.state == VideoPlayer.StatePlaying ? videoPlayer.pause() : videoPlayer.play()
            }

            IconButton {
                id: streamsButton
                visible: false
                icon.source: "qrc:///images/icon-m-source.svg"
                width: Theme.itemSizeExtraSmall
                height: width
                anchors.right: playbackSpeedButton.left
                anchors.top: parent.top
                anchors.margins: Theme.paddingMedium
                icon.width: width
                icon.height: width
                onClicked: pageStack.push(streamsDialog)
            }

            Component {
                id: streamsDialog

                Dialog {
                    forwardNavigation: false
                    allowedOrientations: Orientation.All

                    Component {
                        id: filePickerPage
                        FilePickerPage {
                            allowedOrientations: Orientation.All

                            onSelectedContentPropertiesChanged: {
                                videoPlayer.subtitle = "file://" + selectedContentProperties.filePath
                            }
                        }
                    }
                    Column {
                        anchors.fill: parent

                        DialogHeader {
                            id: header
                        }

                        ComboBox {
                            width: parent.width
                            label: qsTr("Video track")

                            Component.onCompleted: {
                                currentIndex = videoPlayer.selectedVideoStream + 1
                                currentIndex = currentIndex
                            }

                            onCurrentIndexChanged: if (!_menuOpen) videoPlayer.selectVideoStream(currentIndex - 1)

                            menu: ContextMenu {
                                Repeater {
                                    model: videoPlayer.videoStreams
                                    MenuItem {
                                        text: qsTr("%1. %2").arg(index).arg(modelData)
                                    }
                                }
                            }
                        }

                        ComboBox {
                            width: parent.width
                            label: qsTr("Audio track")

                            Component.onCompleted: {
                                currentIndex = videoPlayer.selectedAudioStream + 1
                                currentIndex = currentIndex
                            }

                            onCurrentIndexChanged: if (!_menuOpen) videoPlayer.selectAudioStream(currentIndex - 1)

                            menu: ContextMenu {
                                Repeater {
                                    model: videoPlayer.audioStreams
                                    MenuItem {
                                        text: qsTr("%1. %2").arg(index).arg(modelData)
                                    }
                                }
                            }
                        }

                        ComboBox {
                            width: parent.width
                            label: qsTr("Subtitle track")


                            Component.onCompleted: {
                                currentIndex = videoPlayer.selectedSubtitleStream + 1
                                currentIndex = currentIndex
                            }

                            onCurrentIndexChanged: videoPlayer.selectSubtitle(currentIndex - 1)

                            menu: ContextMenu {
                                Repeater {
                                    model: videoPlayer.subtitleStreams
                                    MenuItem {
                                        text: qsTr("%1. %2").arg(index).arg(modelData)
                                    }
                                }
                            }
                        }

                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                            height: implicitHeight + Theme.paddingLarge+2
                            text: qsTr("Select subtitle file")
                            onClicked: pageStack.push(filePickerPage)
                        }
                    }
                }

            }

            IconButton {
                id: playbackSpeedButton
                visible: opacity != 0 && !playbackSpeedSlider.visible
                icon.source: "qrc:///images/icon-m-playback-speed.svg"
                width: Theme.itemSizeExtraSmall
                height: width
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Theme.paddingMedium
                icon.width: width
                icon.height: width
                onClicked: {
                    playbackSpeedSlider.opacity = 1.0
                    playbackSpeedSliderTimer.start()
                }

                Timer {
                    id: playbackSpeedSliderTimer
                    interval: 3000
                    repeat: false
                    onTriggered: playbackSpeedSlider.opacity = 0
                }
            }

            Slider {
                id: playbackSpeedSlider
                visible: opacity !== 0
                opacity: 0
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: width - height + Theme.paddingMedium*2
                anchors.rightMargin: -(width) + height + Theme.paddingMedium
                width: Theme.itemSizeHuge*2
                value: 1.0
                minimumValue: 0.25
                maximumValue: 2.0
                stepSize: 0.25
                transform: Rotation { angle: -90 }
                enabled: visible
                onDownChanged: if (down) {
                                   playbackSpeedSliderTimer.stop()
                               } else {
                                   playbackSpeedSliderTimer.start()
                                   videoPlayer.setPlaybackSpeed(value)
                               }

                Behavior on opacity {
                    PropertyAction {}
                }
            }

            Label {
                anchors.top: parent.top
                anchors.topMargin: playbackSpeedSlider.width - playbackSpeedSlider.height - Theme.paddingMedium*2
                anchors.left: playbackSpeedSlider.left
                anchors.leftMargin: playbackSpeedSlider.height/2 - width/2
                text: playbackSpeedSlider.value
                opacity: playbackSpeedSlider.opacity
                visible: opacity !== 0
            }

            Slider {
                id: volumeSlider
                visible: false
                x: page.width - height
                y: page.height
                width: page.height
                minimumValue: 0
                maximumValue: 10
                transform: Rotation { angle: -90}
                enabled: false

                Behavior on opacity {
                    PropertyAction {}
                }
            }

            Slider {
                id: brightnessSlider
                visible: false
                x: 0
                y: page.height
                width: page.height
                transform: Rotation { angle: -90}
                enabled: false
                maximumValue: 10
                minimumValue: 0

                Behavior on opacity {
                    PropertyAction {}
                }
            }

            Label {
                id: progress
                width: Theme.itemSizeExtraSmall
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: Theme.paddingLarge
                text:  Format.formatDuration(Math.round(videoPlayer.position/1000), ((videoPlayer.duration/1000) > 3600 ? Formatter.DurationLong : Formatter.DurationShort))
            }

            Label {
                id: duration
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Theme.paddingLarge
                text: Format.formatDuration(Math.round(videoPlayer.duration/1000), ((videoPlayer.duration/1000) > 3600 ? Formatter.DurationLong : Formatter.DurationShort))
            }

            Slider {
                id: progressSlider
                value: videoPlayer.position
                minimumValue: 0
                maximumValue: videoPlayer.duration
                anchors.bottom: parent.bottom
                x: progress.width
                width: parent.width - progress.width - duration.width
                visible: _controlsVisible

                NumberAnimation on opacity {
                    id: showAnimation3
                    to: 1
                    duration: 100

                }
                NumberAnimation on opacity {
                    id: hideAnimation3
                    to: 0
                    duration: 100
                }

                onReleased: videoPlayer.seek(progressSlider.value)
            }
        }

        Label {
            id: subtitles
            text: videoPlayer.displaySubtitle
            width: page.width - Theme.paddingLarge*2
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            font.weight: Font.Bold
            color: "white"
            style: Text.Outline
            styleColor: "black"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.paddingMedium
            anchors.horizontalCenter: videoPlayer.horizontalCenter
        }
    }

    CoverActionList {
        id: coverAction
        enabled: videoPlayer.state !== VideoPlayer.StateStopped

        CoverAction {
            iconSource: videoPlayer.state == VideoPlayer.StatePlaying ? "image://theme/icon-cover-pause" : "image://theme/icon-cover-play"
            onTriggered: {
                videoPlayer.state == VideoPlayer.StatePlaying ? videoPlayer.pause() : videoPlayer.play()
            }
        }
    }

    MprisPlayer {
        id: mprisPlayer

        serviceName: "picoplayer"
//        property string title: videoPlayer.metaData.title ? videoPlayer.metaData.title : "picoplayer"
        property var playbackState: Mpris.Playing


//        Component.onCompleted: {
//            title = videoPlayer.metaData.title !== undefined ? videoPlayer.metaData.title : "picoplayer"
//        }

//        onTitleChanged: {
//            var metadata = mprisPlayer.metadata

//            metadata[Mpris.metadataToString(Mpris.Title)] = title // String

//            mprisPlayer.metadata = metadata
//        }

        identity: "picoplayer"

        canControl: true

        canGoNext: false
        canGoPrevious: false
        canPause: true
        canPlay: true
        canSeek: true

        playbackStatus: playbackState

        loopStatus: Mpris.None
        shuffle: false
        volume: 1

        onPauseRequested: {
            console.log("pause")
            videoPlayer.pause()
        }

        onPlayRequested: {
            console.log("play")
            videoPlayer.play()
        }

        onPlayPauseRequested: {
            console.log("pauseplay")
            videoPlayer.state == VideoPlayer.StatePlaying ? videoPlayer.pause() : videoPlayer.play()
        }

        onStopRequested: {
            console.log("stop")
            videoPlayer.stop()
        }

        onSeekRequested: {
            videoPlayer.seek(offset)
        }
    }
}
