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
    Q_PROPERTY(qint64 forwardedFrom READ getForwardedFrom WRITE setForwardedFrom NOTIFY forwardedFromChanged)
    Q_PROPERTY(qint32 chatUnreadCount READ getChatUnreadCount NOTIFY unreadChatCountChanged)
    Q_PROPERTY(qint32 chatUnreadUnmutedCount READ getChatUnreadUnmutedCount NOTIFY unreadChatCountChanged)
    Q_PROPERTY(qint32 messageUnreadCount READ getMessageUnreadCount NOTIFY unreadMessageCountChanegd)
    Q_PROPERTY(qint32 messageUnreadUnmutedCount READ getMessageUnreadUnmutedCount NOTIFY unreadMessageCountChanegd)
public:
    enum ChatElementRoles {
        TypeRole = Qt::UserRole + 1,
        IdRole,
        NameRole,
        OrderRole,
        ChatListRole,
        HasPhotoRole,
        PhotoRole,
        UnreadCountRole,
        LastMessageRole,
        LastMessageAuthorRole,
        IsSelfRole,
        SecretChatStateRole,
        IsPinnedRole,
        IsReadRole,
        LastMessageTimestampRole
    };

    struct UnreadChatCount {
        qint32 total_count_;
        qint32 unread_count_;
        qint32 unread_unmuted_count_;
        qint32 marked_as_unread_count_;
        qint32 marked_as_unread_unmuted_count_;
    };

    struct UnreadMessageCount {
        qint32 unread_count_;
        qint32 unread_unmuted_count_;
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
    qint64 getForwardedFrom() const;
    void setForwardedFrom(qint64 forwardedFrom);
    qint32 getChatUnreadCount() const;
    qint32 getChatUnreadUnmutedCount() const;
    qint32 getMessageUnreadCount() const;
    qint32 getMessageUnreadUnmutedCount() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    QHash<int, QByteArray> roleNames() const;

    void newChatID(int64_t chat);
    void setChatOrder(int64_t chat, int64_t order);
    void getMainChatList(Chat::ChatList chatList = Chat::ChatList::Main);

    Q_INVOKABLE QVariant openChat(qint64 chatId);
    Q_INVOKABLE void closeChat(qint64 chatId);
    Chat* getChat(int64_t chatId) const;
    Q_INVOKABLE QVariant getChatAsVariant(qint64 chatId) const;
    Q_INVOKABLE void markChatAsRead(qint64 chatId);
    Q_INVOKABLE QVariant getChannelNotificationSettings();
    Q_INVOKABLE QVariant getGroupNotificationSettings();
    Q_INVOKABLE QVariant getPrivateNotificationSettings();
    Q_INVOKABLE void togglePinnedChat(qint64 chatId);
    Q_INVOKABLE bool autoDownloadEnabled();
    Q_INVOKABLE void autoDownload(qint32 fileId, QString type);
    Q_INVOKABLE bool fileExists(QString path);

signals:
    void channelNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void groupNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void privateNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void selectionChanged();
    void forwardedFromChanged();
    void unreadChatCountChanged();
    void unreadMessageCountChanegd();

public slots:
    void onIsAuthorizedChanged(bool isAuthorized);
    void onChatPhotoChanged(qint64 chatId);
    void onUnreadCountChanged(qint64 chatId, qint32 unreadCount);
    void newChats(td_api::chats *_chats);
    void newChat(td_api::updateNewChat *updateNewChat);
    void updateChatPhoto(td_api::updateChatPhoto *updateChatPhoto);
    void updateChatLastMessage(td_api::updateChatLastMessage *updateChatLastMessage);
    void updateChatIsPinned(td_api::updateChatIsPinned *updateChatIsPinned);
    void updateChatOrder(td_api::updateChatOrder *updateChatOrder);
    void secretChatStateChanged(qint64 chatId);
    void updateScopeNotificationSettings(td_api::updateScopeNotificationSettings *updateScopeNotificationSettings);
    void lastReadInboxMessageIdChanged(qint64 chatId, qint64 lastReadInboxMessageIdChanged);
    void lastReadOutboxMessageIdChanged(qint64 chatId, qint64 lastReadOutboxMessageIdChanged);
    void updateUnreadChatCount(td_api::updateUnreadChatCount *updateUnreadChatCount);
    void updateUnreadMessageCount(td_api::updateUnreadMessageCount *updateUnreadMessageCount);

protected:
    void updateChat(int64_t chat, const QVector<int> &roles = {IdRole, NameRole});

private:
    int64_t openedChat = -1;
    bool _isAuthorized = false;
    QHash<int64_t, Chat*> _chats;
    QVector<int64_t> _chats_ids;
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
    qint64 _forwardedFrom;
    UnreadChatCount _unreadChatCount;
    UnreadMessageCount _unreadMessageCount;
};

#endif // CHATLIST_H
