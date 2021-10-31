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

#include "supergroupfullinfo.h"

SupergroupFullInfo::SupergroupFullInfo(QObject *parent) : QObject(parent)
{

}

void SupergroupFullInfo::setSupergroupFullInfo(td_api::object_ptr<td_api::supergroupFullInfo> supergroupFullInfo)
{
    _supergroupFullInfo = std::move(supergroupFullInfo);

    emit supergroupFullInfoChanged();
}

QString SupergroupFullInfo::getDescription() const
{
    if (_supergroupFullInfo == nullptr) return "";
    return QString::fromStdString(_supergroupFullInfo->description_);
}

int32_t SupergroupFullInfo::getMemberCount() const
{
    if (_supergroupFullInfo == nullptr) return 0;
    return _supergroupFullInfo->member_count_;
}

QString SupergroupFullInfo::getInviteLink() const
{
    if (_supergroupFullInfo == nullptr) return "";
    return QString::fromStdString(_supergroupFullInfo->invite_link_);
}
