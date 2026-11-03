INCLUDEPATH += \
    $$PWD/upstream/include

SOURCES += \
    $$PWD/upstream/src/http.c \
    $$PWD/upstream/src/info.c \
    $$PWD/upstream/src/internal.c \
    $$PWD/upstream/src/opusfile.c \
    $$PWD/upstream/src/stream.c \

HEADERS += \
    $$PWD/upstream/src/internal.h \
    $$PWD/upstream/include/opusfile.h
