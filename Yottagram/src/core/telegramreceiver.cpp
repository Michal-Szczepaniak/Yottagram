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
    client = std::make_unique<td::Client>();
}

void TelegramReceiver::run() {
    while(true) {
        auto response = client->receive(WAIT_TIMEOUT);

        if (response.object) {
            emit messageReceived(response.id, response.object.release());

            msleep(10);
        }
    }
}
