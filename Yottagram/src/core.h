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

#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QThread>
#include "core/telegrammanager.h"
#include "authorization.h"
#include "chatlist.h"
#include "users.h"
#include "files/files.h"
#include "notifications.h"
#include "components/autodownloadsettings.h"
#include "stickersets.h"
#include "savedanimations.h"
#include "calls.h"
#include "dbushelper.h"
#include "chatlistfilters.h"
#include "proxymodel.h"
#include "contacts.h"
#include "telegramstatus.h"

using namespace std;

Q_DECLARE_METATYPE(shared_ptr<td_api::Object>)
class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(QObject *parent = nullptr);
    void init();

    Authorization _authorization;
    TelegramStatus _telegramStatus;
    Notifications _notifications;
    StickerSets _stickerSets;
    SavedAnimations _savedAnimations;
    Calls _calls;
    ChatListFilters _chatListFilters;
    ProxyModel _proxyModel;
    Contacts _contacts;
    shared_ptr<ChatList> _chatList;
    shared_ptr<Users> _users;
    shared_ptr<Files> _files;
    shared_ptr<DBusHelper> _dbusHelper;
    AutoDownloadSettings _wifiAutoDownloadSettings;
    AutoDownloadSettings _mobileAutoDownloadSettings;
    AutoDownloadSettings _roamingAutoDownloadSettings;
    AutoDownloadSettings _otherAutoDownloadSettings;

private:
    shared_ptr<TelegramManager> _manager;

};

#endif // CORE_H
