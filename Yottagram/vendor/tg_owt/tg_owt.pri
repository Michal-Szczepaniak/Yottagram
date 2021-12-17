INCLUDEPATH += \
    $$PWD/upstream/src \
    $$PWD/upstream/src/third_party/libyuv/include \
    $$PWD/upstream/src/modules/audio_coding/codecs/opus/ \
    $$PWD/upstream/src/rtc_base/ \
    $$PWD/upstream/src/api/

DEFINES += \
    HAVE_WEBRTC_VIDEO \
    RTC_DISABLE_TRACE_EVENTS \
    RTC_ENABLE_VP9 \
    WEBRTC_APM_DEBUG_DUMP=0 \
    WEBRTC_ENABLE_LINUX_ALSA \
    WEBRTC_ENABLE_LINUX_PULSE \
    WEBRTC_ENABLE_PROTOBUF=0 \
    WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE \
    WEBRTC_LIBRARY_IMPL \
    WEBRTC_LINUX \
    WEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=1 \
    WEBRTC_OPUS_SUPPORT_120MS_PTIME=1 \
    WEBRTC_OPUS_VARIABLE_COMPLEXITY=0 \
    WEBRTC_POSIX \
    WEBRTC_USE_BUILTIN_ISAC_FLOAT \
    WEBRTC_ENABLE_SYMBOL_EXPORT

LIBS += $$PWD/prebuilds/$$QT_ARCH/libtg_owt.a -ldl
