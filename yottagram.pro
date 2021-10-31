include(vendor/vendor.pri)

TARGET = yottagram

QT += dbus multimedia

CONFIG +=  c++11 c++14 link_pkgconfig sailfishapp iostream

QMAKE_CXXFLAGS += -std=c++14 -O0

PKGCONFIG += zlib openssl nemonotifications-qt5 connman-qt5 vorbisfile systemsettings

DEFINES += QT_STATICPLUGIN

CONFIG -= debug_and_release
CONFIG(debug, debug|release): {
    message(Building for debug)
}
CONFIG(release, debug|release): {
    message(Building for release)
    DEFINES *= QT_NO_DEBUG_OUTPUT
}

SOURCES += src/core.cpp \
    src/components/audiorecorder.cpp \
    src/components/autodownloadsettings.cpp \
    src/components/basicgroupfullinfo.cpp \
    src/components/basicgroupinfo.cpp \
    src/components/basicgroupsinfo.cpp \
    src/components/scopenotificationsettings.cpp \
    src/components/secretchatinfo.cpp \
    src/components/secretchatsinfo.cpp \
    src/components/supergroupfullinfo.cpp \
    src/components/supergroupinfo.cpp \
    src/components/supergroupsinfo.cpp \
    src/components/thumbnail.cpp \
    src/components/userfullinfo.cpp \
    src/files/animation.cpp \
    src/files/audio.cpp \
    src/files/contentfile.cpp \
    src/files/document.cpp \
    src/files/file.cpp \
    src/files/files.cpp \
    src/files/photo.cpp \
    src/files/sticker.cpp \
    src/files/video.cpp \
    src/files/videonote.cpp \
    src/files/voicenote.cpp \
    src/message.cpp \
    src/notifications.cpp \
    src/poll.cpp \
    src/stickerset.cpp \
    src/stickersets.cpp \
    src/tgsioplugin/tgsioplugin.cpp \
    src/tgsioplugin/tgsiohandler.cpp \
    src/user.cpp \
    src/users.cpp \
    src/webpage.cpp \
    src/yottagram.cpp \
    src/authorization.cpp \
    src/core/telegramreceiver.cpp \
    src/core/telegrammanager.cpp \
    src/chatlist.cpp \
    src/chat.cpp

DISTFILES += qml/yottagram.qml \
    com.verdanditeam.yottagram.service \
    qml/components/AutoDownloadSettings.qml \
    qml/components/ChatNotifications.qml \
    qml/components/PinnedMessage.qml \
    qml/components/ScopeNotificationSettings.qml \
    qml/components/SilicaSignalListView.qml \
    qml/components/StickerPicker.qml \
    qml/components/SubpageElement.qml \
    qml/components/SwitchButton.qml \
    qml/components/chatInfo/BasicGroupInfo.qml \
    qml/components/chatInfo/SupergroupInfo.qml \
    qml/components/chatInfo/UserInfo.qml \
    qml/components/functions/muteFormat.js \
    qml/components/messageContent/AnimatedStickerContent.qml \
    qml/components/messageContent/AnimationContent.qml \
    qml/components/messageContent/AudioContent.qml \
    qml/components/Avatar.qml \
    qml/components/messageContent/DocumentContent.qml \
    qml/components/HighlightLabelIconButton.qml \
    qml/components/messageContent/ImageContent.qml \
    qml/components/LabelWithMenu.qml \
    qml/components/MessageBubble.qml \
    qml/components/MessageContextMenu.qml \
    qml/components/messageContent/Poll.qml \
    qml/components/messageContent/StickerContent.qml \
    qml/components/TextSwitchWithMenu.qml \
    qml/components/messageContent/VideoContent.qml \
    qml/components/messageContent/VideoNoteContent.qml \
    qml/components/messageContent/VoiceNoteContent.qml \
    qml/components/private/BoundsBehavior.qml \
    qml/components/private/FastScrollAnimation.js \
    qml/components/private/FastScrollAnimation.qml \
    qml/components/private/QuickScroll.qml \
    qml/components/private/QuickScrollArea.qml \
    qml/components/private/QuickScrollButton.qml \
    qml/components/private/HighlightImage.qml \
    qml/cover/CoverPage.qml \
    qml/dialogs/PollDialog.qml \
    qml/dialogs/SetTtlDialog.qml \
    qml/pages/About.qml \
    qml/pages/AuthorizationCode.qml \
    qml/pages/AuthorizationNumber.qml \
    qml/pages/AuthorizationPassword.qml \
    qml/pages/Chat.qml \
    qml/pages/ChatList.qml \
    qml/pages/Loading.qml \
    qml/pages/Settings.qml \
    rpm/yottagram.spec \
    translations/*.ts \
    yottagram.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

CONFIG += sailfishapp_i18n

TRANSLATIONS += \
    translations/yottagram-es.ts \
    translations/yottagram-sv.ts \
    translations/yottagram-zh_CN.ts

HEADERS += \
    src/components/audiorecorder.h \
    src/components/autodownloadsettings.h \
    src/components/basicgroupfullinfo.h \
    src/components/basicgroupinfo.h \
    src/components/basicgroupsinfo.h \
    src/components/scopenotificationsettings.h \
    src/components/secretchatinfo.h \
    src/components/secretchatsinfo.h \
    src/components/supergroupfullinfo.h \
    src/components/supergroupinfo.h \
    src/components/supergroupsinfo.h \
    src/components/thumbnail.h \
    src/components/userfullinfo.h \
    src/core.h \
    src/files/animation.h \
    src/files/audio.h \
    src/files/contentfile.h \
    src/files/document.h \
    src/files/file.h \
    src/files/files.h \
    src/files/photo.h \
    src/files/sticker.h \
    src/files/video.h \
    src/files/videonote.h \
    src/files/voicenote.h \
    src/message.h \
    src/notifications.h \
    src/overloaded.h \
    src/authorization.h \
    src/core/telegramreceiver.h \
    src/core/telegrammanager.h \
    src/chatlist.h \
    src/chat.h \
    src/poll.h \
    src/stickerset.h \
    src/stickersets.h \
    src/tgsioplugin/tgsioplugin.h \
    src/tgsioplugin/tgsiohandler.h \
    src/user.h \
    src/users.h \
    src/webpage.h

LIBS = -lssl -pthread /usr/lib/libtdclient.so.1.7.0 /usr/lib/libtdapi.a /usr/lib/libtdcore.a /usr/lib/libtdutils.a

RESOURCES += \
    qml/resources/icons.qrc

dbus.files = com.verdanditeam.yottagram.service
dbus.path = /usr/share/dbus-1/services/

notificationcategories.files=x-verdanditeam.yottagram.im.conf
notificationcategories.path=/usr/share/lipstick/notificationcategories

INSTALLS += dbus notificationcategories

# https://github.com/Samsung/rlottie.git

RLOTTIE_CONFIG = $${PWD}/vendor/rlottie/src/vector/config.h
PRE_TARGETDEPS += $${RLOTTIE_CONFIG}
QMAKE_EXTRA_TARGETS += rlottie_config

rlottie_config.target = $${RLOTTIE_CONFIG}
rlottie_config.commands = touch $${RLOTTIE_CONFIG} # Empty config is fine

DEFINES += LOTTIE_THREAD_SUPPORT

INCLUDEPATH += \
    vendor/rlottie/inc \
    vendor/rlottie/src/vector \
    vendor/rlottie/src/vector/freetype

SOURCES += \
    vendor/rlottie/src/lottie/lottieanimation.cpp \
    vendor/rlottie/src/lottie/lottieitem.cpp \
    vendor/rlottie/src/lottie/lottieitem_capi.cpp \
    vendor/rlottie/src/lottie/lottiekeypath.cpp \
    vendor/rlottie/src/lottie/lottieloader.cpp \
    vendor/rlottie/src/lottie/lottiemodel.cpp \
    vendor/rlottie/src/lottie/lottieparser.cpp

SOURCES += \
    vendor/rlottie/src/vector/freetype/v_ft_math.cpp \
    vendor/rlottie/src/vector/freetype/v_ft_raster.cpp \
    vendor/rlottie/src/vector/freetype/v_ft_stroker.cpp \
    vendor/rlottie/src/vector/stb/stb_image.cpp \
    vendor/rlottie/src/vector/varenaalloc.cpp \
    vendor/rlottie/src/vector/vbezier.cpp \
    vendor/rlottie/src/vector/vbitmap.cpp \
    vendor/rlottie/src/vector/vbrush.cpp \
    vendor/rlottie/src/vector/vdasher.cpp \
    vendor/rlottie/src/vector/vdrawable.cpp \
    vendor/rlottie/src/vector/vdrawhelper.cpp \
    vendor/rlottie/src/vector/vdrawhelper_common.cpp \
    vendor/rlottie/src/vector/vdrawhelper_neon.cpp \
    vendor/rlottie/src/vector/vdrawhelper_sse2.cpp \
    vendor/rlottie/src/vector/vmatrix.cpp \
    vendor/rlottie/src/vector/vimageloader.cpp \
    vendor/rlottie/src/vector/vinterpolator.cpp \
    vendor/rlottie/src/vector/vpainter.cpp \
    vendor/rlottie/src/vector/vpath.cpp \
    vendor/rlottie/src/vector/vpathmesure.cpp \
    vendor/rlottie/src/vector/vraster.cpp \
    vendor/rlottie/src/vector/vrle.cpp

NEON = $$system(g++ -dM -E -x c++ - < /dev/null | grep __ARM_NEON__)
SSE2 = $$system(g++ -dM -E -x c++ - < /dev/null | grep __SSE2__)

!isEmpty(NEON) {
    message(Using NEON render functions)
    SOURCES += vendor/rlottie/src/vector/pixman/pixman-arm-neon-asm.S
} else {
    !isEmpty(SSE2) {
        message(Using SSE2 render functions)
        SOURCES += vendor/rlottie/src/vector/vdrawhelper_sse2.cpp
    } else {
        message(Using default render functions)
    }
}
