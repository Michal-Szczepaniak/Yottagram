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

#ifndef USER_H
#define USER_H

#include <QObject>
#include <QVariant>
#include <QQmlEngine>
#include "core/telegrammanager.h"
#include "files/file.h"
#include "files/files.h"
#include "components/userfullinfo.h"

class User : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int64_t id READ getId NOTIFY userChanged)
    Q_PROPERTY(bool hasPhoto READ hasPhoto NOTIFY hasPhotoChanged)
    Q_PROPERTY(QString name READ getName NOTIFY userChanged)
    Q_PROPERTY(QString username READ getUserame NOTIFY userChanged)
    Q_PROPERTY(QString phoneNumber READ getPhoneNumber NOTIFY userChanged)
    Q_PROPERTY(Status status READ getStatus NOTIFY userStatusChanged)
    Q_PROPERTY(QString statusText READ getStatusText NOTIFY userStatusChanged)
    Q_PROPERTY(bool isContact READ isContact NOTIFY userChanged)
    Q_PROPERTY(bool isMutualContact READ isMutualContact NOTIFY userChanged)
    Q_PROPERTY(bool isVerified READ isVerified NOTIFY userChanged)
    Q_PROPERTY(bool isSupport READ isSupport NOTIFY userChanged)
    Q_PROPERTY(QString restrictionReason READ getRestrictionReason NOTIFY userChanged)
    Q_PROPERTY(bool isScam READ isScam NOTIFY userChanged)
    Q_PROPERTY(bool haveAccess READ haveAccess NOTIFY userChanged)
    Q_PROPERTY(QString type READ getType NOTIFY userChanged)
    Q_PROPERTY(File* smallPhoto READ getSmallPhoto NOTIFY smallPhotoChanged)
    Q_PROPERTY(File* bigPhoto READ getBigPhoto NOTIFY bigPhotoChanged)
    Q_PROPERTY(UserFullInfo* userFullInfo READ getUserFullInfo NOTIFY userFullInfoChanged)
public:
    enum Status {
        Empty,
        LastMonth,
        LastWeek,
        Offline,
        Online,
        Recently
    };
    Q_ENUM(Status)

    enum Type {
        Bot,
        Deleted,
        Regular,
        Unknown
    };
    Q_ENUM(Type)

    explicit User(QObject *parent = nullptr);
    ~User();

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);
    td_api::user *user() const;
    void setUser(td_api::user *user);

    int64_t getId() const;
    QString getFirstName() const;
    QString getLastName() const;
    QString getName() const;
    QString getUserame() const;
    QString getPhoneNumber() const;
    User::Status getStatus() const;
    QString getStatusText() const;
    bool isContact() const;
    bool isMutualContact() const;
    bool isVerified() const;
    bool isSupport() const;
    QString getRestrictionReason() const;
    bool isScam() const;
    bool haveAccess() const;
    User::Type getType() const;
    bool hasPhoto() const;
    File* getSmallPhoto() const;
    File* getBigPhoto() const;
    UserFullInfo* getUserFullInfo() const;
    void setUserFullInfo(td_api::object_ptr<td_api::userFullInfo> UserFullInfo);
    void setUserStatus(td_api::object_ptr<td_api::UserStatus> userStatus);

signals:
    void fileUpdated(int32_t fileId);
    void userChanged();
    void hasPhotoChanged(bool hasPhoto);
    void smallPhotoChanged();
    void bigPhotoChanged();
    void userFullInfoChanged();
    void userStatusChanged();

public slots:

private:
    td_api::user* _user;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Files> _files;
    int32_t _smallPhotoId;
    int32_t _bigPhotoId;
    UserFullInfo* _userFullInfo;
};
Q_DECLARE_METATYPE(User*)

#endif // USER_H
