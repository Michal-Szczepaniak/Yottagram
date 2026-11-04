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

#ifndef TDLIBBACKEND_H
#define TDLIBBACKEND_H

#include "backend.h"
#include "telegramreceiver.h"

class TdlibBackend : public Backend
{
    Q_OBJECT
public:
    TdlibBackend();

    void start() override;
    void send(uint64_t id, td_api::object_ptr<td_api::Function> function) override;
    void close() override;

private:
    TelegramReceiver _receiver;
};

#endif // TDLIBBACKEND_H
