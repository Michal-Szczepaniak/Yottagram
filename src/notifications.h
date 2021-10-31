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

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <td/telegram/Client.h>
#include "core/telegrammanager.h"
#include "message.h"
#include "users.h"
#include "chatlist.h"
#include <notification.h>
#include "files/files.h"

class Notifications : public QObject
{
    Q_OBJECT
public:
    explicit Notifications(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setChatList(shared_ptr<ChatList> chatList);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    QString getGroupTypeAsString(td_api::NotificationGroupType* notificationGroupType) const;
signals:

public slots:
    void updateActiveNotifications(td_api::updateActiveNotifications* updateActiveNotifications);
    void updateNotificationGroup(td_api::updateNotificationGroup* updateNotificationGroup);
    void updateNotification(td_api::updateNotification* updateNotification);

private:
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
    shared_ptr<ChatList> _chatList;
    shared_ptr<Files> _files;
    QHash<int32_t, Notification*> _notifications;
};

#endif // NOTIFICATIONS_H
