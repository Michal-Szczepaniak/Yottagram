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

#include "users.h"
#include <QDebug>

Users::Users(QObject *parent) : QAbstractListModel(parent)
{

}

Users::~Users()
{

}

void Users::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), SIGNAL(updateUser(td_api::updateUser*)), this, SLOT(updateUser(td_api::updateUser*)));
    connect(_manager.get(), SIGNAL(updateUserFullInfo(td_api::updateUserFullInfo*)), this, SLOT(onUpdateUserFullInfo(td_api::updateUserFullInfo*)));
}

void Users::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int Users::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return _users.size();
}

QVariant Users::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0) return QVariant();

    auto chatNode = _users[index.row()];
    switch (role) {
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Users::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[IdRole]   = "id";
    roles[NameRole] = "name";
    return roles;
}

std::shared_ptr<User> Users::getUser(int32_t userId) const
{
    if (_users.contains(userId)) {
        return _users[userId];
    } else {
        return nullptr;
    }
}

QVariant Users::getUserAsVariant(int32_t userId) const
{
    if (_users.contains(userId)) {
        auto user = _users[userId].get();
        QQmlEngine::setObjectOwnership(user, QQmlEngine::CppOwnership);
        return QVariant::fromValue(user);
    } else {
        return QVariant();
    }
}

void Users::updateUser(td_api::updateUser *updateUser)
{
    auto userId = updateUser->user_->id_;

    if (true == _users.contains(userId)) {
        _users[userId]->setUser(updateUser->user_.release());
    } else {
        auto user = std::make_shared<User>();
        user->setFiles(_files);
        user->setUser(updateUser->user_.release());
        user->setTelegramManager(_manager);

        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        _users[userId] = user;
        endInsertRows();
    }
}

void Users::onUpdateUserFullInfo(td_api::updateUserFullInfo *updateUserFullInfo)
{
    auto userId = updateUserFullInfo->user_id_;

    if (false == _users.contains(userId)) return;

    _users[userId]->setUserFullInfo(std::move(updateUserFullInfo->user_full_info_));
}
