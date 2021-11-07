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

#ifndef PINNEDMESSAGES_H
#define PINNEDMESSAGES_H

#include <QObject>
#include "../core/telegrammanager.h"
#include "../files/files.h"
#include "../users.h"
#include "../message.h"

class PinnedMessages : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int64_t chatId READ chatId WRITE setChatId NOTIFY chatIdChanged)
    Q_PROPERTY(int64_t id READ id NOTIFY idChanged)
    Q_PROPERTY(int32_t senderUserId READ senderUserId NOTIFY idChanged)
    Q_PROPERTY(QString type READ type NOTIFY idChanged)
    Q_PROPERTY(QString text READ text NOTIFY idChanged)
public:
    explicit PinnedMessages(QObject *parent = nullptr);
    ~PinnedMessages();

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);
    void setUsers(shared_ptr<Users> users);
    int64_t chatId() const;
    void setChatId(int64_t chatId);
    int64_t id();
    int32_t senderUserId();
    QString type();
    QString text();

    Q_INVOKABLE void cycleMessage();
    Q_INVOKABLE void cycleMessageBack();
    Q_INVOKABLE bool contains(int64_t messageId);

private:
    void fetchMessages(int64_t fromMessage = 0);

public slots:
    void onGotSearchChatMessagesFilterPinned(int64_t chatId, td_api::messages* messages);
    void onUpdateMessageIsPinned(td_api::updateMessageIsPinned *updateMessageIsPinned);

signals:
    void chatIdChanged();
    void idChanged();

private:
    unsigned int _currentMessage;
    int64_t _chatId;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Files> _files;
    shared_ptr<Users> _users;
    std::vector<Message*> _messages;
};

#endif // PINNEDMESSAGES_H
