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

#include "tdlibbackend.h"

TdlibBackend::TdlibBackend()
{
    connect(&_receiver, &TelegramReceiver::messageReceived, this, &Backend::messageReceived, Qt::DirectConnection);
}

void TdlibBackend::start()
{
    _receiver.start();
}

void TdlibBackend::send(uint64_t id, td_api::object_ptr<td_api::Function> function)
{
    _receiver.manager.send(_receiver.client, id, std::move(function));
}

void TdlibBackend::close()
{
    _receiver.manager.execute(td_api::make_object<td_api::close>());
}
