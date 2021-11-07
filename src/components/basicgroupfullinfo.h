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

#ifndef GROUPFULLINFO_H
#define GROUPFULLINFO_H

#include <QAbstractListModel>
#include <td/telegram/Client.h>

using namespace td;

class BasicGroupFullInfo : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString description READ getDescription NOTIFY basicGroupFullInfoChanged)
    Q_PROPERTY(int32_t creatorUserId READ getCreatorUserId NOTIFY basicGroupFullInfoChanged)
    Q_PROPERTY(QString inviteLink READ getInviteLink NOTIFY basicGroupFullInfoChanged)
public:
    enum ChatMemberRoles {
        UserIdRole = Qt::UserRole + 1,
        InviteUserIdRole,
        JoinedChatDateRole,
        StatusRole,
        BotInfoRole
    };

    explicit BasicGroupFullInfo(QObject *parent = nullptr);

    void setBasicGroupFullInfo(td_api::object_ptr<td_api::basicGroupFullInfo> basicGroupFullInfo);
    QString getDescription() const;
    int32_t getCreatorUserId() const;
    QString getInviteLink() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = UserIdRole) const;
    QHash<int, QByteArray> roleNames() const;

signals:
    void basicGroupFullInfoChanged();

public slots:

private:
    td_api::object_ptr<td_api::basicGroupFullInfo> _basicGroupFullInfo;
};
Q_DECLARE_METATYPE(BasicGroupFullInfo*)

#endif // GROUPFULLINFO_H
