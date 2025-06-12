TEMPLATE = lib

TARGET = $$qtLibraryTarget(YottagramTransferEnginePlugin)
CONFIG += plugin
DEPENDPATH += .

CONFIG += link_pkgconfig
PKGCONFIG += nemotransferengine-qt5

CONFIG += c++11

HEADERS += \
    $$PWD/YottagramSharePluginInfo.h \
    $$PWD/YottagramMediaTransfer.h \
    $$PWD/YottagramSharePlugin.h

SOURCES += \
    $$PWD/YottagramSharePluginInfo.cpp \
    $$PWD/YottagramMediaTransfer.cpp \
    $$PWD/YottagramSharePlugin.cpp

DISTFILES += \
    $$PWD/YottagramShare.qml

qml.files = YottagramShare.qml
qml.path = /usr/share/nemo-transferengine/plugins/sharing

INSTALLS += qml

target.path = $$[QT_INSTALL_LIBS]/nemo-transferengine/plugins/sharing
INSTALLS += target

