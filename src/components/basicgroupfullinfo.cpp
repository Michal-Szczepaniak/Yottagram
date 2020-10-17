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

#include "basicgroupfullinfo.h"

BasicGroupFullInfo::BasicGroupFullInfo(QObject *parent) : QAbstractListModel(parent)
{

}

void BasicGroupFullInfo::setBasicGroupFullInfo(td_api::object_ptr<td_api::basicGroupFullInfo> basicGroupFullInfo)
{
    _basicGroupFullInfo = std::move(basicGroupFullInfo);

    emit basicGroupFullInfoChanged();
}

QString BasicGroupFullInfo::getDescription() const
{
    if (_basicGroupFullInfo == nullptr) return "";
    return QString::fromStdString(_basicGroupFullInfo->description_);
}

qint32 BasicGroupFullInfo::getCreatorUserId() const
{
    if (_basicGroupFullInfo == nullptr) return 0;
    return _basicGroupFullInfo->creator_user_id_;
}

QString BasicGroupFullInfo::getInviteLink() const
{
    if (_basicGroupFullInfo == nullptr) return "";
    return QString::fromStdString(_basicGroupFullInfo->invite_link_);
}

int BasicGroupFullInfo::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (_basicGroupFullInfo == nullptr) return 0;
    return _basicGroupFullInfo->members_.size();
}

QVariant BasicGroupFullInfo::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() < 0 || index.row() >= rowCount()) return QVariant();

    auto& chatMember = _basicGroupFullInfo->members_[index.row()];
    switch (role) {
    case ChatMemberRoles::UserIdRole:
        return chatMember->user_id_;
    case ChatMemberRoles::InviteUserIdRole:
        return chatMember->inviter_user_id_;
    case ChatMemberRoles::JoinedChatDateRole:
        return chatMember->joined_chat_date_;
//    case ChatMemberRoles::BotInfoRole:
//        return chatMember->status_
//    case ChatMemberRoles::BotInfoRole:
//        return chatMember->bot_info_
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> BasicGroupFullInfo::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[UserIdRole] = "userId";
    roles[InviteUserIdRole] = "inviteUserId";
    roles[JoinedChatDateRole] = "joinedChatDate";
    roles[StatusRole] = "status";
    roles[BotInfoRole] = "botInfo";
    return roles;
}
