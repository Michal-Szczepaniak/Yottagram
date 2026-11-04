YOTTASLACK_LIB = $$OUT_PWD/yottaslack/libyottaslack.so
YOTTASLACK_PREBUILT = $$PWD/prebuilds/$$QT_ARCH/libyottaslack.so

yottaslack_lib.target = $$YOTTASLACK_LIB
yottaslack_lib.commands = sh $$PWD/build.sh $$QT_ARCH $$YOTTASLACK_LIB
yottaslack_lib.depends = $$PWD/build.sh $$PWD/go.mod $$files($$PWD/*.go, true)
exists($$YOTTASLACK_PREBUILT): yottaslack_lib.depends += $$YOTTASLACK_PREBUILT

QMAKE_EXTRA_TARGETS += yottaslack_lib
PRE_TARGETDEPS += $$YOTTASLACK_LIB

yottaslack.files = $$YOTTASLACK_LIB
yottaslack.path = /usr/share/yottagram/lib
yottaslack.CONFIG += no_check_exist

INSTALLS += yottaslack
