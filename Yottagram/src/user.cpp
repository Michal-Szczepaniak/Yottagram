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

#include "user.h"
#include <QtQml>
#include <QDebug>

User::User(QObject *parent) : QObject(parent), _user(nullptr)
{
    _userFullInfo = new UserFullInfo();
}

User::~User()
{
    delete _userFullInfo;
    delete _user;
}

void User::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void User::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

td_api::user *User::user() const
{
    return _user;
}

void User::setUser(td_api::user *user)
{
    if (_user != nullptr) delete _user;
    _user = user;

    emit userChanged();

    if (hasPhoto()) {
        _smallPhotoId = _user->profile_photo_->small_->id_;
        _bigPhotoId = _user->profile_photo_->big_->id_;

        _files->appendFile(std::move(_user->profile_photo_->small_), "avatar");
        _files->appendFile(std::move(_user->profile_photo_->big_), "");

        emit smallPhotoChanged();
        emit bigPhotoChanged();
    }
    emit hasPhotoChanged(hasPhoto());
}

QString User::getName() const
{
    return QString::fromStdString(_user->first_name_) + " " + QString::fromStdString(_user->last_name_);
}

QString User::getUserame() const
{
    return QString::fromStdString(_user->username_);
}

QString User::getPhoneNumber() const
{
    return QString::fromStdString(_user->phone_number_);
}

User::Status User::getStatus() const
{
    switch (_user->status_->get_id()) {
    case td_api::userStatusEmpty::ID:
        return Status::Empty;
    case td_api::userStatusLastMonth::ID:
        return Status::LastMonth;
    case td_api::userStatusLastWeek::ID:
        return Status::LastWeek;
    case td_api::userStatusOffline::ID:
        return Status::Offline;
    case td_api::userStatusOnline::ID:
        return Status::Online;
    case td_api::userStatusRecently::ID:
        return Status::Recently;
    default:
        return Status::Empty;
    }
}

QString User::getStatusText() const
{
    switch (getStatus()) {
    case Status::LastMonth:
        return tr("Last seen within month");
    case Status::LastWeek:
        return tr("Last seen within week");
    case Status::Offline:
        return tr("Offline");
    case Status::Online:
        return tr("Online");
    case Status::Recently:
        return tr("Last seen recently");
    case Status::Empty:
        return tr("Never seen");

    }
}

bool User::isContact() const
{
    return _user->is_contact_;
}

bool User::isMutualContact() const
{
    return _user->is_mutual_contact_;
}

bool User::isVerified() const
{
    return _user->is_verified_;
}

bool User::isSupport() const
{
    return _user->is_support_;
}

QString User::getRestrictionReason() const
{
    return QString::fromStdString(_user->restriction_reason_);
}

bool User::isScam() const
{
    return _user->is_scam_;
}

bool User::haveAccess() const
{
    return _user->have_access_;
}

User::Type User::getType() const
{
    switch (_user->type_->get_id()) {
    case td_api::userTypeBot::ID:
        return Type::Bot;
    case td_api::userTypeDeleted::ID:
        return Type::Deleted;
    case td_api::userTypeRegular::ID:
        return Type::Regular;
    case td_api::userTypeUnknown::ID:
    default:
        return Type::Unknown;

    }
}

bool User::hasPhoto() const
{
    return _user->profile_photo_ != nullptr;
}

File* User::getSmallPhoto() const
{
    auto file = _files->getFile(_smallPhotoId).get();
    QQmlEngine::setObjectOwnership(file, QQmlEngine::CppOwnership);
    return file;
}

File* User::getBigPhoto() const
{
    auto file = _files->getFile(_bigPhotoId).get();
    QQmlEngine::setObjectOwnership(file, QQmlEngine::CppOwnership);
    return file;
}

UserFullInfo *User::getUserFullInfo() const
{
    return _userFullInfo;
}

void User::setUserFullInfo(td_api::object_ptr<td_api::userFullInfo> userFullInfo)
{
    _userFullInfo->setUserFullInfo(std::move(userFullInfo));

    emit userFullInfoChanged();
}
