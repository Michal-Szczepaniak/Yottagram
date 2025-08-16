INCLUDEPATH += \
    $$PWD/includes

LIBS += -lssl -pthread $$PWD/prebuilds/$$QT_ARCH/libtdclient.a $$PWD/prebuilds/$$QT_ARCH/libtdapi.a $$PWD/prebuilds/$$QT_ARCH/libtdcore.a $$PWD/prebuilds/$$QT_ARCH/libtde2e.a $$PWD/prebuilds/$$QT_ARCH/libtdmtproto.a $$PWD/prebuilds/$$QT_ARCH/libtdnet.a $$PWD/prebuilds/$$QT_ARCH/libtdutils.a $$PWD/prebuilds/$$QT_ARCH/libtddb.a $$PWD/prebuilds/$$QT_ARCH/libtdactor.a $$PWD/prebuilds/$$QT_ARCH/libtdsqlite.a
