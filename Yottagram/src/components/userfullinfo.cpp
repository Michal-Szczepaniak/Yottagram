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

#include "userfullinfo.h"

UserFullInfo::UserFullInfo(QObject *parent) : QObject(parent)
{

}

void UserFullInfo::setUserFullInfo(td_api::object_ptr<td_api::userFullInfo> userFullInfo)
{
    _userFullInfo = std::move(userFullInfo);

    emit userFullInfoChanged();
}

bool UserFullInfo::hasInfo() const
{
    return (bool)_userFullInfo;
}

bool UserFullInfo::isBlocked() const
{
    return (bool)_userFullInfo->block_list_;
}

bool UserFullInfo::canBeCalled() const
{
    return _userFullInfo->can_be_called_;
}

bool UserFullInfo::hasPrivateCalls() const
{
    return _userFullInfo->has_private_calls_;
}

bool UserFullInfo::needPhoneNumberPrivacyException() const
{
    return _userFullInfo->need_phone_number_privacy_exception_;
}

QString UserFullInfo::getBio() const
{
    if (!_userFullInfo || !_userFullInfo->bio_) return "";
    return QString::fromStdString(_userFullInfo->bio_->text_);
}

int32_t UserFullInfo::getGroupInCommonCount() const
{
    return _userFullInfo->group_in_common_count_;
}
