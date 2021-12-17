include(upstream/plugins/plugin.pri)

TARGET = yottagram-voicecall-plugin
QT += dbus

enable-debug {
    DEFINES += WANT_TRACE
}

HEADERS += \
    yottagramvoicecallhandler.h  \
    yottagramvoicecallprovider.h \
    yottagramvoicecallproviderfactory.h

SOURCES += \
    yottagramvoicecallhandler.cpp \
    yottagramvoicecallprovider.cpp \
    yottagramvoicecallproviderfactory.cpp

DBUS_ADAPTORS += ../Yottagram/com.verdanditeam.yottagram.calls.xml

DEFINES += PLUGIN_NAME=\\\"yottagram-voicecall-plugin\\\"
