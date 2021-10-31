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

#include "telegrammanager.h"
#include "../overloaded.h"
#include <QDebug>
#include <QSettings>
#include <QGuiApplication>

TelegramManager::TelegramManager(): _messageId(1)
{
}

void TelegramManager::init()
{
    _networkManager = NetworkManager::instance();

    connect(&receiver, SIGNAL(messageReceived(uint64_t, td_api::Object*)),
            this, SLOT(messageReceived(uint64_t, td_api::Object*)), Qt::DirectConnection);
    connect(this, SIGNAL(updateOption(td_api::updateOption*)), this, SLOT(onUpdateOption(td_api::updateOption*)));

    connect(_networkManager, SIGNAL(defaultRouteChanged(NetworkService*)), this, SLOT(defaultRouteChanged(NetworkService*)));
    defaultRouteChanged(_networkManager->defaultRoute());

    receiver.start();
}

void TelegramManager::sendQuery(td_api::Function* message)
{
    receiver.client->send({(std::uint64_t)1, td_api::object_ptr<td_api::Function>(message)});
}

void TelegramManager::sendQueryWithRespone(int64_t chatId, int32_t type, int32_t subType, td_api::Function *message)
{
    _messageId++;
    _messages[_messageId] = {chatId, type, subType};

    receiver.client->send({_messageId, td_api::object_ptr<td_api::Function>(message)});
}

int32_t TelegramManager::getMyId() const
{
    return _myId;
}

bool TelegramManager::getDaemonEnabled() const
{
    QSettings settings;
    return settings.value("daemonEnabled", false).toBool();
}

void TelegramManager::setDaemonEnabled(bool daemonEnabled)
{
    QSettings settings;
    settings.setValue("daemonEnabled", daemonEnabled);
    QGuiApplication::setQuitOnLastWindowClosed(!daemonEnabled);
    sendQuery(new td_api::getAutoDownloadSettingsPresets());
}

void TelegramManager::setNetworkType(QString networkType)
{
    td_api::object_ptr<td_api::NetworkType> tdNetworkType;
    _networkType = networkType;

    if (networkType == "none") tdNetworkType = td_api::make_object<td_api::networkTypeNone>();
    else if (networkType == "wifi") tdNetworkType = td_api::make_object<td_api::networkTypeWiFi>();
    else if (networkType == "cellular") tdNetworkType = td_api::make_object<td_api::networkTypeMobile>();
    else if (networkType == "roaming") tdNetworkType = td_api::make_object<td_api::networkTypeMobileRoaming>();
    else tdNetworkType = td_api::make_object<td_api::networkTypeOther>();

    sendQuery(new td_api::setNetworkType(move(tdNetworkType)));
}

QString TelegramManager::getNetworkType() const
{
    return _networkType;
}

void TelegramManager::handleMessageWithResponse(uint64_t id, td_api::Object *message)
{
    if (!_messages.contains(id)) return;

    MessageWithResponse response = _messages.take(id);
    switch (response.type) {
    case td_api::getChatHistory::ID:
        emit gotChatHistory(response.chatId, static_cast<td_api::messages*>(message));
        break;
    case td_api::getChats::ID:
        emit gotChats(static_cast<td_api::chats*>(message));
        break;
    }
}

void TelegramManager::messageReceived(uint64_t id, td_api::Object* message)
{
    emit onMessageReceived(id, message);
    if (id > 1) handleMessageWithResponse(id, message);

    downcast_call(
        *message, overloaded(
            [this](td_api::updateNewChat &newChat) {
                emit this->updateNewChat(&newChat);
            },
            [this](td_api::updateChatTitle &updateChatTitle) {
                emit this->updateChatTitle(&updateChatTitle);
            },
            [this](td_api::updateChatPhoto &updateChatPhoto) {
                emit this->updateChatPhoto(&updateChatPhoto);
            },
            [this](td_api::updateChatLastMessage &updateChatLastMessage) {
                emit this->updateChatLastMessage(&updateChatLastMessage);
            },
            [this](td_api::updateChatPosition &updateChatPosition) {
                emit this->updateChatPosition(&updateChatPosition);
            },
            [this](td_api::updateFile &updateFile) {
                emit this->updateFile(&updateFile);
            },
            [this](td_api::updateChatReadInbox &updateChatReadInbox) {
                emit this->updateChatReadInbox(&updateChatReadInbox);
            },
            [this](td_api::updateChatReadOutbox &updateChatReadOutbox) {
                emit this->updateChatReadOutbox(&updateChatReadOutbox);
            },
            [this](td_api::updateNewMessage &updateNewMessage) {
                emit this->updateNewMessage(&updateNewMessage);
            },
            [this](td_api::updateUser &updateUser) {
                emit this->updateUser(&updateUser);
            },
            [this](td_api::updateMessageSendSucceeded &updateMessageSendSucceeded) {
                emit this->updateMessageSendSucceeded(&updateMessageSendSucceeded);
            },
            [this](td_api::updateMessageContent &updateMessageContent) {
                emit this->updateMessageContent(&updateMessageContent);
            },
            [this](td_api::updateDeleteMessages &updateDeleteMessages) {
                emit this->updateDeleteMessages(&updateDeleteMessages);
            },
            [this](td_api::updateNotification &updateNotification) {
                emit this->updateNotification(&updateNotification);
            },
            [this](td_api::updateNotificationGroup &updateNotificationGroup) {
                emit this->updateNotificationGroup(&updateNotificationGroup);
            },
            [this](td_api::updateActiveNotifications &updateActiveNotifications) {
                emit this->updateActiveNotifications(&updateActiveNotifications);
            },
            [this](td_api::updateHavePendingNotifications &updateHavePendingNotifications) {
                emit this->updateHavePendingNotifications(&updateHavePendingNotifications);
            },
            [this](td_api::updateUserFullInfo &updateUserFullInfo) {
                emit this->updateUserFullInfo(&updateUserFullInfo);
            },
            [this](td_api::updateBasicGroupFullInfo &updateBasicGroupFullInfo) {
                emit this->updateBasicGroupFullInfo(&updateBasicGroupFullInfo);
            },
            [this](td_api::updateSupergroupFullInfo &updateSupergroupFullInfo) {
                emit this->updateSupergroupFullInfo(&updateSupergroupFullInfo);
            },
            [this](td_api::updateOption &updateOption) {
                emit this->updateOption(&updateOption);
            },
            [this](td_api::updateBasicGroup &updateBasicGroup) {
                emit this->updateBasicGroup(&updateBasicGroup);
            },
            [this](td_api::updateSupergroup &updateSupergroup) {
                emit this->updateSupergroup(&updateSupergroup);
            },
            [this](td_api::updateSecretChat &updateSecretChat) {
                emit this->updateSecretChat(&updateSecretChat);
            },
            [this](td_api::updateChatNotificationSettings &updateChatNotificationSettings) {
                emit this->updateChatNotificationSettings(&updateChatNotificationSettings);
            },
            [this](td_api::updateScopeNotificationSettings &updateScopeNotificationSettings) {
                emit this->updateScopeNotificationSettings(&updateScopeNotificationSettings);
            },
            [this](td_api::autoDownloadSettingsPresets &autoDownloadSettingsPresets) {
                emit this->autoDownloadSettingsPresets(&autoDownloadSettingsPresets);
            },
            [this](td_api::updateInstalledStickerSets &updateInstalledStickerSets) {
                emit this->updateInstalledStickerSets(&updateInstalledStickerSets);
            },
            [this](td_api::updateChatPermissions &updateChatPermissions) {
                emit this->updateChatPermissions(&updateChatPermissions);
            },
            [this](td_api::updateUnreadChatCount &updateUnreadChatCount) {
                emit this->updateUnreadChatCount(&updateUnreadChatCount);
            },
            [this](td_api::updateUnreadMessageCount &updateUnreadMessageCount) {
                emit this->updateUnreadMessageCount(&updateUnreadMessageCount);
            },
            [this](td_api::updateChatUnreadMentionCount &updateChatUnreadMentionCount) {
                emit this->updateChatUnreadMentionCount(&updateChatUnreadMentionCount);
            },
            [](auto &update) { Q_UNUSED(update) }
        )
    );
}

void TelegramManager::onUpdateOption(td_api::updateOption *updateOption)
{
    if (updateOption->name_ == "my_id") {
        _myId = static_cast<td_api::optionValueInteger*>(updateOption->value_.get())->value_;
        emit myIdChanged(_myId);
    }
}

void TelegramManager::defaultRouteChanged(NetworkService *networkService)
{
    if (networkService == nullptr) {
        setNetworkType("none");
        return;
    }

    if (networkService->type() == "wifi") {
        setNetworkType("wifi");
        return;
    }
    if (networkService->type() == "cellular") {
        if (networkService->roaming()) {
            setNetworkType("cellular");
        } else {
            setNetworkType("roaming");
        }
        return;
    }

    setNetworkType("other");
}
