TEMPLATE = subdirs

SUBDIRS = \
    YottagramVoicecallPlugin \
    YottagramTransferEnginePlugin \
    Yottagram \

DISTFILES += rpm/yottagram.spec \
    Yottagram/qml/dialogs/MoveFilesDialog.qml
