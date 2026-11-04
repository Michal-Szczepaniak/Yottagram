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

#ifndef SLACKMAPPER_H
#define SLACKMAPPER_H

#include <QByteArray>
#include <td/telegram/Client.h>
#include <vector>

using namespace std;
using namespace td;

namespace SlackMapper {

struct Output {
    uint64_t id;
    td_api::Object* object;
};

// One library event can expand to several td_api objects, the order is significant
vector<Output> map(const QByteArray &json);

td_api::Object* authorizationWaitToken();

}

#endif // SLACKMAPPER_H
