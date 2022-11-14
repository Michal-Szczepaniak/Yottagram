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

#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <td/telegram/Client.h>
#include <QObject>
#include "core/telegrammanager.h"

using namespace std;
using namespace td;

class Authorization : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAuthorized READ isAuthorized WRITE setIsAuthorized NOTIFY isAuthorizedChanged)
public:
    explicit Authorization(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> _manager);
    void updateAuthorizationState(td_api::updateAuthorizationState &updateAuthorizationState);

    bool isAuthorized();
    void setIsAuthorized(bool isAuthorized);

    Q_INVOKABLE void sendNumber(QString number);
    Q_INVOKABLE void sendCode(QString code);
    Q_INVOKABLE void sendPassword(QString password);

public slots:
    void messageReceived(uint64_t id, td_api::Object* object);

signals:
    void waitingForCode();
    void waitingForPassword();
    void waitingForPhoneNumber();

    void isAuthorizedChanged(bool);

private:
    shared_ptr<TelegramManager> _manager;
    td_api::object_ptr<td_api::AuthorizationState> _authorizationState;
    bool _isAuthorized = false;

    void authorizationStateReady();
    void authorizationStateLoggingOut();
    void authorizationStateClosing();
    void authorizationStateClosed();
    void authorizationStateWaitCode();
    void authorizationStateWaitPassword();
    void authorizationStateWaitPhoneNumber();
    void authorizationStateWaitTdlibParameters();
};

#endif // AUTHORIZATION_H
