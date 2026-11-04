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

#ifndef SLACKBACKEND_H
#define SLACKBACKEND_H

#include "../core/backend.h"
#include "slacklibrary.h"
#include "slackreceiver.h"
#include <QJsonObject>
#include <QLibrary>

class SlackBackend : public Backend
{
    Q_OBJECT
public:
    SlackBackend();

    void start() override;
    void send(uint64_t id, td_api::object_ptr<td_api::Function> function) override;
    void close() override;

private slots:
    void onReceived(uint64_t id, td_api::Object* object);

private:
    // TelegramManager starts its ids at 2, so this one never reaches it
    static constexpr uint64_t STORED_TOKEN_LOGIN_ID = 1;

    void loadLibrary();
    void call(uint64_t id, const QString &method, const QJsonObject &params = QJsonObject());
    void reply(uint64_t id, td_api::Object* object);
    void replyError(uint64_t id, int32_t code, const string &message);
    void handleViewMessages(uint64_t id, td_api::viewMessages *query);
    void handleSendMessage(uint64_t id, td_api::sendMessage *query);
    void handleEditMessage(uint64_t id, td_api::editMessageText *query);
    void handleDeleteMessages(uint64_t id, td_api::deleteMessages *query);

    QLibrary _library;
    SlackLibrary _api;
    SlackReceiver _receiver;
    uint64_t _loginRequestId = 0;
    QString _pendingToken;
    bool _closed = false;
};

#endif // SLACKBACKEND_H
