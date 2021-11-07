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

#include "core.h"
#include <QDebug>
#include "overloaded.h"
#include "user.h"
#include <QtQml>
#include "components/thumbnail.h"
#include "components/audiorecorder.h"
#include "components/pinnedmessages.h"

Core::Core(QObject *parent) : QObject(parent)
{
    _manager = make_shared<TelegramManager>();
    _chatList = make_shared<ChatList>();
    _users = make_shared<Users>();
    _files = make_shared<Files>();
    qRegisterMetaType<shared_ptr<td_api::Object>>();
    qmlRegisterType<User>("com.verdanditeam.user", 1, 0, "User");
    qmlRegisterType<Thumbnail>("com.verdanditeam.thumbnail", 1, 0, "Thumbnail");
    qmlRegisterType<AudioRecorder>("com.verdanditeam.audiorecorder", 1, 0, "AudioRecorder");
    qmlRegisterType<PinnedMessages>("com.verdanditeam.pinnedmessages", 1, 0, "PinnedMessages");

    _files->setTelegramManager(_manager);
    _files->setWifiAutoDownloadSettings(&_wifiAutoDownloadSettings);
    _files->setMobileAutoDownloadSettings(&_wifiAutoDownloadSettings);
    _files->setRoamingAutoDownloadSettings(&_wifiAutoDownloadSettings);
    _files->setOtherAutoDownloadSettings(&_wifiAutoDownloadSettings);
    _authorization.setTelegramManager(_manager);
    _chatList->setTelegramManager(_manager);
    _chatList->setUsers(_users);
    _chatList->setFiles(_files);
    _users->setTelegramManager(_manager);
    _users->setFiles(_files);
    _notifications.setTelegramManager(_manager);
    _notifications.setChatList(_chatList);
    _notifications.setUsers(_users);
    _notifications.setFiles(_files);

    _wifiAutoDownloadSettings.setTelegramManager(_manager);
    _wifiAutoDownloadSettings.setConnectionType("wifi");
    connect(_manager.get(), SIGNAL(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)), &_wifiAutoDownloadSettings, SLOT(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)));
    _wifiAutoDownloadSettings.loadSettings();
    _mobileAutoDownloadSettings.setTelegramManager(_manager);
    _mobileAutoDownloadSettings.setConnectionType("cellular");
    connect(_manager.get(), SIGNAL(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)), &_mobileAutoDownloadSettings, SLOT(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)));
    _mobileAutoDownloadSettings.loadSettings();
    _roamingAutoDownloadSettings.setTelegramManager(_manager);
    _roamingAutoDownloadSettings.setConnectionType("roaming");
    connect(_manager.get(), SIGNAL(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)), &_roamingAutoDownloadSettings, SLOT(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)));
    _roamingAutoDownloadSettings.loadSettings();
    _otherAutoDownloadSettings.setTelegramManager(_manager);
    _otherAutoDownloadSettings.setConnectionType("other");
    connect(_manager.get(), SIGNAL(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)), &_otherAutoDownloadSettings, SLOT(autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets*)));
    _otherAutoDownloadSettings.loadSettings();

    _stickerSets.setTelegramManager(_manager);
    _stickerSets.setFiles(_files);
    connect(&_authorization, &Authorization::isAuthorizedChanged, &_stickerSets, &StickerSets::onIsAuthorizedChanged);
}

void Core::init()
{
    connect(&_authorization, SIGNAL(isAuthorizedChanged(bool)), _chatList.get(), SLOT(onIsAuthorizedChanged(bool)));

    _manager->init();

    auto objectValue = td_api::make_object<td_api::optionValueInteger>(1);
    _manager->sendQuery(new td_api::setOption("notification_group_count_max", std::move(objectValue)));
}
