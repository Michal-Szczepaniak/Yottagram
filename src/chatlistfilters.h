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

#ifndef CHATLISTFILTERS_H
#define CHATLISTFILTERS_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"

class ChatListFilters : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ChatListRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        IconNameRole
    };

    struct ChatList {
        int32_t id;
        QString name;
        QString iconName;
    };

    explicit ChatListFilters(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void updateChatFilters(td_api::updateChatFilters *updateChatFilters);

private:
    shared_ptr<TelegramManager> _manager;
    QVector<ChatList> _filterChatLists;
};

#endif // CHATLISTFILTERS_H
