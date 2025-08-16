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

#include "telegramreceiver.h"
#include <td/telegram/Log.h>
#include <td/telegram/Client.h>
#include <QDebug>
#include <QDateTime>

TelegramReceiver::TelegramReceiver()
{
    td::Log::set_verbosity_level(0);
    client = manager.create_client_id();
    manager.send(client, 0, td_api::make_object<td_api::setLogVerbosityLevel>(0));
}

void TelegramReceiver::run() {
    while(true) {
        auto response = manager.receive(WAIT_TIMEOUT);

        qDebug() << "Message: " << response.object->get_id();
        if (!response.object) continue;

        emit messageReceived(response.request_id, response.object.release());
        msleep(10);
    }
}
