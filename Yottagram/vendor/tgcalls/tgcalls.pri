INCLUDEPATH += \
    $$PWD/upstream \
    $$PWD/upstream/tgcalls \
    $$PWD/../tg_owt/upstream/src \

DEFINES += \
    TGCALLS_USE_STD_OPTIONAL

SOURCES += \
    $$PWD/upstream/tgcalls/AudioDeviceHelper.cpp \
    $$PWD/upstream/tgcalls/CodecSelectHelper.cpp \
    $$PWD/upstream/tgcalls/CryptoHelper.cpp \
    $$PWD/upstream/tgcalls/EncryptedConnection.cpp \
    $$PWD/upstream/tgcalls/FakeAudioDeviceModule.cpp \
    $$PWD/upstream/tgcalls/FakeVideoTrackSource.cpp \
    $$PWD/upstream/tgcalls/Instance.cpp \
    $$PWD/upstream/tgcalls/InstanceImpl.cpp \
    $$PWD/upstream/tgcalls/LogSinkImpl.cpp \
    $$PWD/upstream/tgcalls/Manager.cpp \
    $$PWD/upstream/tgcalls/MediaManager.cpp \
    $$PWD/upstream/tgcalls/Message.cpp \
    $$PWD/upstream/tgcalls/NetworkManager.cpp \
    $$PWD/upstream/tgcalls/StaticThreads.cpp \
    $$PWD/upstream/tgcalls/ThreadLocalObject.cpp \
    $$PWD/upstream/tgcalls/VideoCaptureInterface.cpp \
    $$PWD/upstream/tgcalls/VideoCaptureInterfaceImpl.cpp \
    $$PWD/upstream/tgcalls/platform/fake/FakeInterface.cpp \
    $$PWD/upstream/tgcalls/TurnCustomizerImpl.cpp

HEADERS += \
    $$PWD/upstream/tgcalls/AudioDeviceHelper.h \
    $$PWD/upstream/tgcalls/AudioFrame.h \
    $$PWD/upstream/tgcalls/CodecSelectHelper.h \
    $$PWD/upstream/tgcalls/CryptoHelper.h \
    $$PWD/upstream/tgcalls/EncryptedConnection.h \
    $$PWD/upstream/tgcalls/FakeAudioDeviceModule.h \
    $$PWD/upstream/tgcalls/FakeVideoTrackSource.h \
    $$PWD/upstream/tgcalls/Instance.h \
    $$PWD/upstream/tgcalls/InstanceImpl.h \
    $$PWD/upstream/tgcalls/LogSinkImpl.h \
    $$PWD/upstream/tgcalls/Manager.h \
    $$PWD/upstream/tgcalls/MediaManager.h \
    $$PWD/upstream/tgcalls/Message.h \
    $$PWD/upstream/tgcalls/NetworkManager.h \
    $$PWD/upstream/tgcalls/PlatformContext.h \
    $$PWD/upstream/tgcalls/StaticThreads.h \
    $$PWD/upstream/tgcalls/Stats.h \
    $$PWD/upstream/tgcalls/ThreadLocalObject.h \
    $$PWD/upstream/tgcalls/VideoCaptureInterface.h \
    $$PWD/upstream/tgcalls/VideoCaptureInterfaceImpl.h \
    $$PWD/upstream/tgcalls/VideoCapturerInterface.h \
    $$PWD/upstream/tgcalls/platform/PlatformInterface.h \
    $$PWD/upstream/tgcalls/platform/fake/FakeInterface.h \
    $$PWD/upstream/tgcalls/TurnCustomizerImpl.h
