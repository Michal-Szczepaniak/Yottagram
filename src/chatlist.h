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

class ChatList : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool daemonEnabled READ getDaemonEnabled WRITE setDaemonEnabled)
    Q_PROPERTY(QStringList selection READ getSelection WRITE setSelection NOTIFY selectionChanged)
    Q_PROPERTY(qint64 forwardedFrom READ getForwardedFrom WRITE setForwardedFrom NOTIFY forwardedFromChanged)
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
        IsPinnedRole
    };

    ChatList();
    ~ChatList();
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    QStringList getSelection() const;
    void setSelection(QStringList selection);
    qint64 getForwardedFrom() const;
    void setForwardedFrom(qint64 forwardedFrom);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    QHash<int, QByteArray> roleNames() const;

    void newChatID(int64_t chat);
    void setChatOrder(int64_t chat, int64_t order);
    void getMainChatList(Chat::ChatList chatList = Chat::ChatList::Main);
    bool getDaemonEnabled() const;
    void setDaemonEnabled(bool daemonEnabled);

    Q_INVOKABLE QVariant openChat(qint64 chatId);
    Q_INVOKABLE void closeChat(qint64 chatId);
    Chat* getChat(int64_t chatId) const;
    Q_INVOKABLE QVariant getChatAsVariant(qint64 chatId) const;
    Q_INVOKABLE void markChatAsRead(qint64 chatId);
    Q_INVOKABLE QVariant getChannelNotificationSettings();
    Q_INVOKABLE QVariant getGroupNotificationSettings();
    Q_INVOKABLE QVariant getPrivateNotificationSettings();
    Q_INVOKABLE void togglePinnedChat(qint64 chatId);

signals:
    void channelNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void groupNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void privateNotificationSettingsChanged(td_api::scopeNotificationSettings* scopeNotificationSettings);
    void selectionChanged();
    void forwardedFromChanged();

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
    void updateSecretChat(td_api::updateSecretChat *updateSecretChat);
    void secretChatStateChanged(qint64 chatId);
    void updateScopeNotificationSettings(td_api::updateScopeNotificationSettings *updateScopeNotificationSettings);

protected:
    void updateChat(int64_t chat, const QVector<int> &roles = {IdRole, NameRole});

private:
    int64_t openedChat = -1;
    bool _isAuthorized = false;
    QHash<int64_t, Chat*> _chats;
    QHash<qint32, td_api::secretChat*> _secretChats;
    QVector<int64_t> _chats_ids;
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Users> _users;
    std::shared_ptr<Files> _files;
    ScopeNotificationSettings _channelNotificationSettings;
    ScopeNotificationSettings _groupNotificationSettings;
    ScopeNotificationSettings _privateNotificationSettings;
    QStringList _selection;
    qint64 _forwardedFrom;
};

#endif // CHATLIST_H
