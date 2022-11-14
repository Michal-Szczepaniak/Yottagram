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

#ifndef USERFULLINFO_H
#define USERFULLINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class UserFullInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasInfo READ hasInfo NOTIFY userFullInfoChanged)
    Q_PROPERTY(bool isBlocked READ isBlocked NOTIFY userFullInfoChanged)
    Q_PROPERTY(bool canBeCalled READ canBeCalled NOTIFY userFullInfoChanged)
    Q_PROPERTY(bool hasPrivateCalls READ hasPrivateCalls NOTIFY userFullInfoChanged)
    Q_PROPERTY(bool needPhoneNumberPrivacyException READ needPhoneNumberPrivacyException NOTIFY userFullInfoChanged)
    Q_PROPERTY(QString bio READ getBio NOTIFY userFullInfoChanged)
    Q_PROPERTY(int32_t groupInCommonCount READ getGroupInCommonCount NOTIFY userFullInfoChanged)
public:
    explicit UserFullInfo(QObject *parent = nullptr);

    void setUserFullInfo(td_api::object_ptr<td_api::userFullInfo> userFullInfo);

    bool hasInfo() const;
    bool isBlocked() const;
    bool canBeCalled() const;
    bool hasPrivateCalls() const;
    bool needPhoneNumberPrivacyException() const;
    QString getBio() const;
    int32_t getGroupInCommonCount() const;

signals:
    void userFullInfoChanged();

public slots:

private:
    td_api::object_ptr<td_api::userFullInfo> _userFullInfo;
};
Q_DECLARE_METATYPE(UserFullInfo*)

#endif // USERFULLINFO_H
