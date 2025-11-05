
DEFINES += QPM_INIT\\(E\\)=\"E.addImportPath(QStringLiteral(\\\"qrc:/\\\"));\"
DEFINES += QPM_USE_NS
INCLUDEPATH += $$PWD
QML_IMPORT_PATH += $$PWD


include($$PWD/fr/grecko/sortfilterproxymodel/SortFilterProxyModel.pri)
include($$PWD/webp-plugin/libwebp.pri)
include($$PWD/tg_owt/abseil-cpp.pri)
include($$PWD/tg_owt/tg_owt.pri)
include($$PWD/tgcalls/tgcalls.pri)
include($$PWD/tdlib/tdlib.pri)
include($$PWD/picoplayer/picoplayer.pri)

