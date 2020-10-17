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

#ifndef CONTENTFILE_H
#define CONTENTFILE_H

#include <QObject>
#include <td/telegram/Client.h>
#include "../core/telegrammanager.h"
#include "files.h"

class ContentFile : public QObject
{
    Q_OBJECT
public:
    explicit ContentFile(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

protected:
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Files> _files;
};

#endif // CONTENTFILE_H
