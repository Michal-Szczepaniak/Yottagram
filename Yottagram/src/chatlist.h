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

#ifndef CHATLIST_H
#define CHATLIST_H

#include <QAbstractListModel>
#include <QMap>
#include <QSet>
#include "core/telegrammanager.h"
#include "files/files.h"
#include "chat.h"
#include "users.h"
#include "components/scopenotificationsettings.h"
#include "components/basicgroupsinfo.h"
#include "components/supergroupsinfo.h"

class ChatList : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool daemonEnabled READ getDaemonEnabled WRITE setDaemonEnabled)
    Q_PROPERTY(QStringList selection READ getSelection WRITE setSelection NOTIFY selectionChanged)
    Q_PROPERTY(int64_t forwardedFrom READ getForwardedFrom WRITE setForwardedFrom NOTIFY forwardedFromChanged)
    Q_PROPERTY(int32_t chatUnreadCount READ getChatUnreadCount NOTIFY unreadChatCountChanged)
    Q_PROPERTY(int32_t chatUnreadUnmutedCount READ getChatUnreadUnmutedCount NOTIFY unreadChatCountChanged)
    Q_PROPERTY(int32_t messageUnreadCount READ getMessageUnreadCount NOTIFY unreadMessageCountChanegd)
    Q_PROPERTY(int32_t messageUnreadUnmutedCount READ getMessageUnreadUnmutedCount NOTIFY unreadMessageCountChanegd)
public:
    enum ChatElementRoles {
        TypeRole = Qt::UserRole + 1,
        IdRole,
        NameRole,
        OrderRole,
        HasPhotoRole,
        PhotoRole,
        UnreadCountRole,
        UnreadMentionCountRole,
        LastMessageRole,
        LastMessageAuthorRole,
        IsSelfRole,
        SecretChatStateRole,
        IsPinnedRole,
        IsReadRole,
        LastMessageTimestampRole,
        IsTopicBasedRole
    };

    struct UnreadChatCount {
        int32_t total_count_;
        int32_t unread_count_;
        int32_t unread_unmuted_count_;
        int32_t marked_as_unread_count_;
        int32_t marked_as_unread_unmuted_count_;
    };

    struct UnreadMessageCount {
        int32_t unread_count_;
        int32_t unread_unmuted_count_;
    };

    ChatList();
    ~ChatList();
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    bool getDaemonEnabled() const;
    void setDaemonEnabled(bool daemonEnabled);
    QStringList getSelection() const;
    void setSelection(QStringList selection);
    int64_t getForwardedFrom() const;
    void setForwardedFrom(int64_t forwardedFrom);
    int32_t getChatUnreadCount() const;
    int32_t getChatUnreadUnmutedCount() const;
    int32_t getMessageUnreadCount() const;
    int32_t getMessageUnreadUnmutedCount() const;
    void createChat(td_api::chat *chat);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    QHash<int, QByteArray> roleNames() const;

    void fetchChatList();
    void fetchAllScopeNotificationSettings();

    Q_INVOKABLE QVariant openChat(int64_t chatId);
    Q_INVOKABLE void closeChat(int64_t chatId);
    Chat* getChat(int64_t chatId) const;
    Q_INVOKABLE QVariant getChatAsVariant(int64_t chatId) const;
    Q_INVOKABLE QVariant getChatAsVariantForUser(int64_t userId);
    Q_INVOKABLE void markChatAsRead(int64_t chatId);
    Q_INVOKABLE void markTopicAsRead(int64_t chatId);
    Q_INVOKABLE QVariant getChannelNotificationSettings();
    Q_INVOKABLE QVariant getGroupNotificationSettings();
    Q_INVOKABLE QVariant getPrivateNotificationSettings();
    Q_INVOKABLE void togglePinnedChat(int64_t chatId);
    Q_INVOKABLE bool autoDownloadEnabled();
    Q_INVOKABLE void autoDownload(int32_t fileId, QString type);
    Q_INVOKABLE bool fileExists(QString path);
    Q_INVOKABLE void switchChatList(int chatList, int64_t filterId = 0);

private:
    QVector<int64_t>* getChatList(td_api::ChatList* list);
    td_api::object_ptr<td_api::ChatList> getSelectedChatList() const;
    bool setChatPosition(int64_t chatId, td_api::chatPosition* position);

signals:
    void channelNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void groupNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void privateNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void selectionChanged();
    void forwardedFromChanged();
    void unreadChatCountChanged();
    void unreadMessageCountChanegd();
    void chatCreated(int64_t chatId);

public slots:
    void onIsAuthorizedChanged(bool isAuthorized);
    void onChatPhotoChanged(int64_t chatId);
    void onUnreadCountChanged(int64_t chatId, int32_t unreadCount);
    void onGotChats(td_api::chats *chats);
    void onGotScopeNotificationSettings(int32_t scope, td_api::scopeNotificationSettings *scopeNotificationSettings);
    void newChat(td_api::updateNewChat *updateNewChat);
    void updateChatPhoto(td_api::updateChatPhoto *updateChatPhoto);
    void updateChatLastMessage(td_api::updateChatLastMessage *updateChatLastMessage);
    void updateChatPosition(td_api::updateChatPosition *updateChatPosition);
    void secretChatStateChanged(int64_t chatId);
    void updateScopeNotificationSettings(td_api::updateScopeNotificationSettings *updateScopeNotificationSettings);
    void lastReadInboxMessageIdChanged(int64_t chatId, int64_t lastReadInboxMessageIdChanged);
    void lastReadOutboxMessageIdChanged(int64_t chatId, int64_t lastReadOutboxMessageIdChanged);
    void updateUnreadChatCount(td_api::updateUnreadChatCount *updateUnreadChatCount);
    void updateUnreadMessageCount(td_api::updateUnreadMessageCount *updateUnreadMessageCount);
    void updateChatUnreadMentionCount(td_api::updateChatUnreadMentionCount *updateChatUnreadMentionCount);
    void updateMessageMentionRead(td_api::updateMessageMentionRead *updateMessageMentionRead);
    void updateChatDraftMessage(td_api::updateChatDraftMessage *updateChatDraftMessage);
    void onCreatedPrivateChat(td_api::chat *chat);
    void onBootupComplete();

protected:
    void updateChat(int64_t chat, QVector<int> roles = {IdRole, NameRole});

private:
    QString getLastMessageAuthor(Chat* chat);
    QString getLastMessageTimestamp(Chat* chat);

private:
    int64_t openedChat = -1;
    bool _isAuthorized = false;
    QHash<int64_t, Chat*> _chats;
    QVector<int64_t> *_chats_ids;
    QVector<int64_t> _main_chats_ids;
    QVector<int64_t> _archive_chats_ids;
    QHash<int32_t, QVector<int64_t>> _filter_chats_ids;
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Users> _users;
    std::shared_ptr<Files> _files;
    std::shared_ptr<SecretChatsInfo> _secretChatsInfo;
    std::shared_ptr<BasicGroupsInfo> _basicGroupsInfo;
    std::shared_ptr<SupergroupsInfo> _supergroupsInfo;
    ScopeNotificationSettings _channelNotificationSettings;
    ScopeNotificationSettings _groupNotificationSettings;
    ScopeNotificationSettings _privateNotificationSettings;
    QStringList _selection;
    int64_t _forwardedFrom;
    UnreadChatCount _unreadChatCount;
    UnreadMessageCount _unreadMessageCount;
    quint8 _selectedChatList;
    int32_t _selectedFolderChatList;
    QTimer _updateListTimer;
    QMap<int64_t, QSet<int>> _chatsToUpdate;

    struct ChatCache {
        QString lastAuthor;
        QString lastMessage;
        QString lastTimestamp;
    };

    QMap<int64_t, ChatCache> _chatCache;
};

#endif // CHATLIST_H
