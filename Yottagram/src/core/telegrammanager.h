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

#ifndef TELEGRAMSENDER_H
#define TELEGRAMSENDER_H

#include "telegramreceiver.h"
#include <QObject>
#include <QTimer>
#include <connman-qt5/networkmanager.h>

using namespace std;

class TelegramManager : public QObject
{
    Q_OBJECT
public:
    TelegramManager();

    struct MessageWithResponse {
        int64_t chatId;
        int32_t type;
        int32_t subType;
    };

    enum ConnectionState {
        WaitingForNetwork,
        ConnectingToProxy,
        Connecting,
        Updating,
        Ready
    };
    Q_ENUMS(ConnectionState)

    void init();
    void sendQuery(td_api::Function* message);
    void sendQuerySync(td_api::Function* message);
    td_api::object_ptr<td_api::Object> sendQuerySyncWithResponse(td_api::Function* message);
    void sendQueryWithResponse(int64_t chatId, int32_t type, int32_t subType, td_api::Function* message);
    void sendQueryWithResponse(int64_t chatId, int32_t type, td_api::Function* message);
    void sendQueryWithResponse(int32_t type, int32_t subType, td_api::Function* message);
    void sendQueryWithResponse(int32_t type, td_api::Function* message);
    int32_t getMyId() const;
    bool getDaemonEnabled() const;
    void setDaemonEnabled(bool daemonEnabled);
    void setNetworkType(QString networkType);
    QString getNetworkType() const;
    ConnectionState getConnectionState() const;
    bool isBootupComplete() const;

private:
    void handleMessageWithResponse(uint64_t id, td_api::Object* message);

signals:
    void onMessageReceived(uint64_t id, td_api::Object* message);
    void bootupComplete();
    void send(td_api::Function* message);
    void updateNewChat(td_api::updateNewChat *newChat);
    void updateChatTitle(td_api::updateChatTitle *updateChatTitle);
    void updateChatPhoto(td_api::updateChatPhoto *updateChatPhoto);
    void updateChatLastMessage(td_api::updateChatLastMessage *updateChatLastMessage);
    void updateChatPosition(td_api::updateChatPosition *updateChatPosition);
    void updateFile(td_api::updateFile *updateFile);
    void updateChatReadInbox(td_api::updateChatReadInbox *updateChatReadInbox);
    void updateChatReadOutbox(td_api::updateChatReadOutbox *updateChatReadOutbox);
    void updateNewMessage(td_api::updateNewMessage *updateNewMessage);
    void updateUser(td_api::updateUser *updateUser);
    void updateMessageSendSucceeded(td_api::updateMessageSendSucceeded *updateMessageSendSucceeded);
    void updateMessageContent(td_api::updateMessageContent *updateMessageContent);
    void updateDeleteMessages(td_api::updateDeleteMessages *updateDeleteMessages);
    void updateNotification(td_api::updateNotification *updateNotification);
    void updateNotificationGroup(td_api::updateNotificationGroup *updateNotificationGroup);
    void updateActiveNotifications(td_api::updateActiveNotifications *updateActiveNotifications);
    void updateHavePendingNotifications(td_api::updateHavePendingNotifications *updateHavePendingNotifications);
    void updateUserFullInfo(td_api::updateUserFullInfo *updateUserFullInfo);
    void updateBasicGroupFullInfo(td_api::updateBasicGroupFullInfo *updateBasicGroupFullInfo);
    void updateSupergroupFullInfo(td_api::updateSupergroupFullInfo *updateSupergroupFullInfo);
    void updateOption(td_api::updateOption *updateOption);
    void updateSecretChat(td_api::updateSecretChat *updateSecretChat);
    void updateBasicGroup(td_api::updateBasicGroup *updateBasicGroup);
    void updateSupergroup(td_api::updateSupergroup *updateSupergroup);
    void updateChatNotificationSettings(td_api::updateChatNotificationSettings *updateChatNotificationSettings);
    void updateScopeNotificationSettings(td_api::updateScopeNotificationSettings *updateScopeNotificationSettings);
    void autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets *autoDownloadSettingsPresets);
    void updateInstalledStickerSets(td_api::updateInstalledStickerSets *updateInstalledStickerSets);
    void updateChatPermissions(td_api::updateChatPermissions *updateChatPermissions);
    void updateUnreadChatCount(td_api::updateUnreadChatCount *updateUnreadChatCount);
    void updateUnreadMessageCount(td_api::updateUnreadMessageCount *updateUnreadMessageCount);
    void updateChatUnreadMentionCount(td_api::updateChatUnreadMentionCount *updateChatUnreadMentionCount);
    void updateMessageIsPinned(td_api::updateMessageIsPinned *updateMessageIsPinned);
    void updateMessageMentionRead(td_api::updateMessageMentionRead *updateMessageMentionRead);
    void updateSavedAnimations(td_api::updateSavedAnimations *updateSavedAnimations);
    void updateNewCallSignalingData(td_api::updateNewCallSignalingData *updateNewCallSignalingData);
    void updateCall(td_api::updateCall *updateCall);
    void updateChatMessageAutoDeleteTime(td_api::updateChatMessageAutoDeleteTime *updateChatMessageAutoDeleteTime);
    void updateChatFolders(td_api::updateChatFolders *updateChatFolders);
    void updateChatDraftMessage(td_api::updateChatDraftMessage *updateChatDraftMessage);
    void updateChatAction(td_api::updateChatAction *updateChatAction);
    void updateUserStatus(td_api::updateUserStatus *updateUserStatus);

// Responses
    void gotChatHistory(int64_t chatId, td_api::messages *messages);
    void gotChats(td_api::chats *chats);
    void gotStickerSet(td_api::stickerSet *stickerSet);
    void gotSearchChatMessages(int64_t chatId, td_api::messages *messages);
    void gotSearchChatMessagesFilterPinned(int64_t chatId, td_api::messages *messages);
    void gotSearchChatMessagesFilterUnreadMention(int64_t chatId, td_api::messages *messages);
    void gotSavedAnimations(td_api::animations *animations);
    void gotMessage(td_api::message *message);
    void gotProxies(td_api::proxies *proxies);
    void proxyTestSuccessful();
    void proxyAdded();
    void gotScopeNotificationSettings(int32_t scope, td_api::scopeNotificationSettings *scopeNotificationSettings);
    void gotSearchChatMembers(int64_t chatId, td_api::chatMembers *chatMembers);
    void gotRecentInlineBots(int64_t chatId, td_api::users *users);
    void gotContacts(td_api::users *contacts);
    void createdPrivateChat(td_api::chat *chat);
    void importedContacts(td_api::importedContacts *importedContacts);

    void myIdChanged(int32_t myId);
    void error(int64_t chatId, int32_t type, int32_t subType, int32_t code, QString message);

    void connectionStateChanged();

public slots:
    void messageReceived(uint64_t id, td_api::Object* message);
    void onUpdateOption(td_api::updateOption *updateOption);
    void defaultRouteChanged(NetworkService *networkService);
    void updateConnectionState(td_api::updateConnectionState *updateConnectionState);

private:
    TelegramReceiver receiver;
    QThread receiverThread;
    QTimer incomingMessageCheckTimer;
    int32_t _myId;
    NetworkManager* _networkManager;
    QString _networkType;
    QHash<uint64_t, MessageWithResponse> _messages;
    uint64_t _messageId;
    ConnectionState _connectionState;
    bool _bootupComplete = false;
};

#endif // TELEGRAMSENDER_H
