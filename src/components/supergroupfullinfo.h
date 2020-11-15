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

#ifndef SUPERGROUPFULLINFO_H
#define SUPERGROUPFULLINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class SupergroupFullInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString description READ getDescription NOTIFY supergroupFullInfoChanged)
    Q_PROPERTY(qint32 memberCount READ getMemberCount NOTIFY supergroupFullInfoChanged)
    Q_PROPERTY(QString inviteLink READ getInviteLink NOTIFY supergroupFullInfoChanged)
    /** Add rest of the properties, im too sleepy and they don't matter much to us */
public:
    explicit SupergroupFullInfo(QObject *parent = nullptr);

    void setSupergroupFullInfo(td_api::object_ptr<td_api::supergroupFullInfo> supergroupFullInfo);

    QString getDescription() const;
    qint32 getMemberCount() const;
    QString getInviteLink() const;
signals:
    void supergroupFullInfoChanged();

public slots:

private:
    td_api::object_ptr<td_api::supergroupFullInfo> _supergroupFullInfo;
};
Q_DECLARE_METATYPE(SupergroupFullInfo*)

#endif // SUPERGROUPFULLINFO_H
