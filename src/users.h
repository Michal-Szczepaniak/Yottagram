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

#ifndef USERS_H
#define USERS_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"
#include "user.h"

class Users : public QAbstractListModel
{
    Q_OBJECT
public:
    enum UsersRoles {
        TypeRole = Qt::UserRole + 1,
        IdRole,
        NameRole,
    };

    explicit Users(QObject *parent = nullptr);
    ~Users();

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    QHash<int, QByteArray> roleNames() const;

    std::shared_ptr<User> getUser(int64_t userId) const;
    Q_INVOKABLE QVariant getUserAsVariant(int64_t userId) const;

public slots:
    void updateUser(td_api::updateUser *updateUser);
    void onUpdateUserFullInfo(td_api::updateUserFullInfo* updateUserFullInfo);

private:
    QHash<int64_t, std::shared_ptr<User>> _users;
    std::shared_ptr<TelegramManager> _manager;
    shared_ptr<Files> _files;

};

#endif // USERS_H
