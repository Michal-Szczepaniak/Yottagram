/*

This file is part of Yottagram.
Copyright 2020, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Yottagram is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Yottagram is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

#include <QtQuick>
#include <sailfishapp.h>
#include <QDebug>
#include "core.h"
#include "tgsioplugin/tgsioplugin.h"
#include "ffmpegioplugin/ffmpegioplugin.h"
#include <execinfo.h>
#include <signal.h>

//Q_IMPORT_PLUGIN(TgsIOPlugin)
Q_IMPORT_PLUGIN(ffmpegIOPlugin)

void handler(int sig) {
    void *array[20] = {0};
    size_t size;

    size = backtrace(array, 20);

    fprintf(stderr, "Error: signal %d:\n", sig);
    char **symbols = backtrace_symbols(array, size);
    QRegularExpression re("\\(([+x0-9a-f]*)\\)");
    QStringList addresses;
    for (unsigned int i = 0; i < size; i++) {
        qDebug() << symbols[i];
        auto address = re.match(QString::fromLatin1(symbols[i])).captured(1);
        if (address.isEmpty()) {
            continue;
        }

        addresses << address;
    }

    qDebug() << addresses;

    exit(1);
}

int main(int argc, char *argv[])
{
    signal(SIGSEGV, handler);
    signal(SIGABRT, handler);
    signal(SIGBUS, handler);
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QSharedPointer<QQuickView> view(SailfishApp::createView());
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<QList<int64_t>>("QList<int64_t>");
    qmlRegisterType<ChatListTopics>("com.verdanditeam", 1, 0, "ChatListTopics");

    Core core;
    core.init();

    view->rootContext()->setContextProperty("authorization", &core._authorization);
    view->rootContext()->setContextProperty("telegramStatus", &core._telegramStatus);
    view->rootContext()->setContextProperty("chatList", core._chatList.get());
    view->rootContext()->setContextProperty("users", core._users.get());
    view->rootContext()->setContextProperty("stickerSets", &core._stickerSets);
    view->rootContext()->setContextProperty("savedAnimations", &core._savedAnimations);
    view->rootContext()->setContextProperty("calls", &core._calls);
    view->rootContext()->setContextProperty("chatListFilters", &core._chatListFilters);
    view->rootContext()->setContextProperty("proxyModel", &core._proxyModel);
    view->rootContext()->setContextProperty("contactsModel", &core._contacts);
    view->rootContext()->setContextProperty("wifiAutoDownloadSettings", &core._wifiAutoDownloadSettings);
    view->rootContext()->setContextProperty("mobileAutoDownloadSettings", &core._mobileAutoDownloadSettings);
    view->rootContext()->setContextProperty("roamingAutoDownloadSettings", &core._roamingAutoDownloadSettings);
    view->rootContext()->setContextProperty("otherAutoDownloadSettings", &core._otherAutoDownloadSettings);

    view->setSource(SailfishApp::pathTo("qml/yottagram.qml"));
    view->show();

    int ret = app->exec();

    view.reset();
    app.reset();

    return ret;
}
