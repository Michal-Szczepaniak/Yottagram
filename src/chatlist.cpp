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

#include "chatlist.h"
#include <QDebug>
#include <QQmlEngine>
#include "overloaded.h"

ChatList::ChatList() : _channelNotificationSettings(nullptr), _groupNotificationSettings(nullptr), _privateNotificationSettings(nullptr)
{
}

ChatList::~ChatList()
{
    for (auto &key: _chats.keys()) {
        delete _chats[key];
    }
}

void ChatList::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    _privateNotificationSettings.setTelegramManager(_manager);
    _groupNotificationSettings.setTelegramManager(_manager);
    _channelNotificationSettings.setTelegramManager(_manager);

    connect(_manager.get(), SIGNAL(chats(td_api::chats*)), this, SLOT(newChats(td_api::chats*)));
    connect(_manager.get(), SIGNAL(updateNewChat(td_api::updateNewChat*)), this, SLOT(newChat(td_api::updateNewChat*)));
    connect(_manager.get(), SIGNAL(updateChatPhoto(td_api::updateChatPhoto*)), this, SLOT(updateChatPhoto(td_api::updateChatPhoto*)));
    connect(_manager.get(), SIGNAL(updateChatLastMessage(td_api::updateChatLastMessage*)), this, SLOT(updateChatLastMessage(td_api::updateChatLastMessage*)));
    connect(_manager.get(), SIGNAL(updateChatOrder(td_api::updateChatOrder*)), this, SLOT(updateChatOrder(td_api::updateChatOrder*)));
    connect(_manager.get(), SIGNAL(updateSecretChat(td_api::updateSecretChat*)), this, SLOT(updateSecretChat(td_api::updateSecretChat*)));
    connect(_manager.get(), SIGNAL(updateScopeNotificationSettings(td_api::updateScopeNotificationSettings*)), this, SLOT(updateScopeNotificationSettings(td_api::updateScopeNotificationSettings*)));
}

void ChatList::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void ChatList::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

QStringList ChatList::getSelection() const
{
    return _selection;
}

void ChatList::setSelection(QStringList selection)
{
    _selection = selection;

    emit selectionChanged();
}

qint64 ChatList::getForwardedFrom() const
{
    return _forwardedFrom;
}

void ChatList::setForwardedFrom(qint64 forwardedFrom)
{
    _forwardedFrom = forwardedFrom;

    emit forwardedFromChanged();
}

int ChatList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _chats_ids.length();
}

QVariant ChatList::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0) return QVariant();

    auto chatNode = _chats[_chats_ids[index.row()]];
    switch (role) {
    case ChatElementRoles::TypeRole:
        return chatNode->getChatType();;
    case ChatElementRoles::IdRole:
        return chatNode->getId();
    case ChatElementRoles::NameRole:
        return chatNode->getTitle();
    case ChatElementRoles::OrderRole:
        return chatNode->getOrder();
    case ChatElementRoles::ChatListRole:
        return chatNode->getChatList();
    case ChatElementRoles::PhotoRole:
        if (chatNode->hasPhoto()) {
            return QVariant::fromValue(chatNode->getSmallPhoto());
        }
        return QVariant();
    case ChatElementRoles::HasPhotoRole:
        return chatNode->hasPhoto();
    case ChatElementRoles::UnreadCountRole:
        return chatNode->getUnreadCount();
    case ChatElementRoles::LastMessageRole:
    {
        td_api::message* message = chatNode->getLastMessage();
        if (message == nullptr) return "";

        QString lastMessageInfo = "";

        switch (message->content_->get_id()) {
        case td_api::messageText::ID:
            lastMessageInfo += QString::fromStdString(static_cast<const td_api::messageText &>(*message->content_).text_->text_);
            break;
        case td_api::messageChatDeleteMember::ID:
            return tr("%1 left").arg(_users->getUser(static_cast<const td_api::messageChatDeleteMember &>(*message->content_).user_id_)->getName());
        case td_api::messageChatAddMembers::ID:
        {
            auto userIds = static_cast<const td_api::messageChatAddMembers &>(*message->content_).member_user_ids_;
            QStringList messages;
            for(auto userId: userIds) {
                messages << tr("%1 joined").arg(_users->getUser(userId)->getName());
            }
            return messages.join('\n');
        }
        case td_api::messageChatJoinByLink::ID:
            return tr("%1 joined").arg(_users->getUser(message->sender_user_id_)->getName());
        case td_api::messageAudio::ID:
            lastMessageInfo += tr("Audio");
            break;
        case td_api::messageVideoNote::ID:
            lastMessageInfo += tr("Video note");
            break;
        case td_api::messageVoiceNote::ID:
            lastMessageInfo += tr("Voice note");
            break;
        case td_api::messagePhoto::ID:
            lastMessageInfo += tr("Photo");
            break;
        case td_api::messageSticker::ID:
            lastMessageInfo += tr("Sticker");
            break;
        case td_api::messageVideo::ID:
            lastMessageInfo += tr("Video");
            break;
        case td_api::messageDocument::ID:
            lastMessageInfo += tr("Document");
            break;
        case td_api::messageAnimation::ID:
            lastMessageInfo += tr("GIF");
            break;
        case td_api::messagePoll::ID:
            lastMessageInfo += tr("Poll");
            break;
        case td_api::messageChatSetTtl::ID:
            lastMessageInfo += tr("Self-destruct timer set to %n second(s)", "", static_cast<td_api::messageChatSetTtl*>(message->content_.get())->ttl_);
            break;
        default:
            lastMessageInfo += "Message UNSUPPORTED >:3";
            break;
        }

        return lastMessageInfo;
    }
    case ChatElementRoles::LastMessageAuthorRole:
    {
        td_api::message* message = chatNode->getLastMessage();
        if (message == nullptr) return "";

        QString lastMessageInfo = "";

        if (message->is_outgoing_) {
            lastMessageInfo += "You: ";
        } else if (chatNode->getChatType() == "group" || chatNode->getChatType() == "supergroup"){
            auto user = _users->getUser(message->sender_user_id_);
            if(user != nullptr) {
                lastMessageInfo += user->getName() + ": ";
            }
        }

        return lastMessageInfo;
    }
    case ChatElementRoles::IsSelfRole:
        return chatNode->isSelf();
    case ChatElementRoles::SecretChatStateRole:
        return chatNode->getSecretChatState();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[OrderRole] = "order";
    roles[ChatListRole] = "chatlist";
    roles[PhotoRole] = "photo";
    roles[HasPhotoRole] = "hasPhoto";
    roles[UnreadCountRole] = "unreadCount";
    roles[LastMessageAuthorRole] = "lastMessageAuthor";
    roles[LastMessageRole] = "lastMessage";
    roles[IsSelfRole] = "isSelf";
    roles[SecretChatStateRole] = "secretChatState";
    return roles;
}

void ChatList::updateChat(int64_t chat, const QVector<int> &roles) {
    if(rowCount() <= 0) return;

    auto it = std::find(_chats_ids.begin(), _chats_ids.end(), chat);
    if (it != _chats_ids.end()) {
        auto index = std::distance(_chats_ids.begin(), it);
        emit dataChanged(createIndex(index, 0), createIndex(index, 0), roles);
    }
}

void ChatList::newChatID(int64_t chat)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _chats_ids.push_back(chat);
    endInsertRows();
}

void ChatList::setChatOrder(int64_t chat, int64_t order)
{
    if (false == _chats_ids.contains(chat)) {
        newChatID(chat);
    }

    getChat(chat)->setOrder(order);
    updateChat(chat, {OrderRole});
}

void ChatList::getMainChatList(Chat::ChatList chatList)
{
    qint64 offsetOrder = std::numeric_limits<std::int64_t>::max();
    qint64 offsetChatId = 0;
    if (rowCount() > 0) {
        if (_chats.size() == _chats_ids.length()) return;

        auto chat = _chats[_chats_ids[rowCount()-1]]->getChat();
        offsetOrder = chat->order_;
        offsetChatId = chat->id_;
    }

    td_api::object_ptr<td_api::ChatList> list;
    if (chatList == Chat::ChatList::Archive) {
        list = td_api::make_object<td_api::chatListArchive>();
    } else {
        list = td_api::make_object<td_api::chatListMain>();
    }

    _manager->sendQuery(new td_api::getChats(move(list), offsetOrder, offsetChatId, 20));
}

bool ChatList::getDaemonEnabled() const
{
    return _manager->getDaemonEnabled();
}

void ChatList::setDaemonEnabled(bool daemonEnabled)
{
    _manager->setDaemonEnabled(daemonEnabled);
}

QVariant ChatList::openChat(qint64 chatId)
{
    auto chat = getChat(chatId);
    if (chat == nullptr) return QVariant();

    _manager->sendQuery(new td_api::openChat(chat->getId()));
    chat->setIsOpen(true);

    return QVariant::fromValue(chat);
}

void ChatList::closeChat(qint64 chatId)
{
    auto chat = getChat(chatId);
    _manager->sendQuery(new td_api::closeChat(chat->getId()));
    chat->setIsOpen(false);
}

Chat *ChatList::getChat(int64_t chatId) const
{
    if (_chats.contains(chatId)) {
        auto chat = _chats.value(chatId);
        return chat;
    } else {
        qWarning() << "Chat doesn't exist!";
        return nullptr;
    }
}

QVariant ChatList::getChatAsVariant(qint64 chatId) const
{
    if (_chats.contains(chatId)) {
        auto chat = _chats.value(chatId);
        QQmlEngine::setObjectOwnership(chat, QQmlEngine::CppOwnership);
        return QVariant::fromValue(chat);
    } else {
        qWarning() << "Chat doesn't exist!";
        return QVariant();
    }
}

void ChatList::markChatAsRead(qint64 chatId)
{

    _manager->sendQuery(new td_api::viewMessages(getChat(chatId)->getId(), {getChat(chatId)->getLastMessage()->id_}, false));
    _manager->sendQuery(new td_api::toggleChatIsMarkedAsUnread(chatId, false));
}

QVariant ChatList::getChannelNotificationSettings()
{
    auto* settings = &_channelNotificationSettings;
    QQmlEngine::setObjectOwnership(settings, QQmlEngine::CppOwnership);
    return QVariant::fromValue(settings);
}

QVariant ChatList::getGroupNotificationSettings()
{
    auto* settings = &_groupNotificationSettings;
    QQmlEngine::setObjectOwnership(settings, QQmlEngine::CppOwnership);
    return QVariant::fromValue(settings);
}

QVariant ChatList::getPrivateNotificationSettings()
{
    auto* settings = &_privateNotificationSettings;
    QQmlEngine::setObjectOwnership(settings, QQmlEngine::CppOwnership);
    return QVariant::fromValue(settings);
}

void ChatList::onIsAuthorizedChanged(bool isAuthorized)
{
    _isAuthorized = isAuthorized;

    this->getMainChatList();
}

void ChatList::onChatPhotoChanged(qint64 chatId)
{
    this->updateChat(chatId, {PhotoRole, HasPhotoRole});
}

void ChatList::onUnreadCountChanged(qint64 chatId, qint32 unreadCount)
{
    Q_UNUSED(unreadCount)
    this->updateChat(chatId, {UnreadCountRole});
}

void ChatList::newChats(td_api::chats *chats)
{
    if (chats->chat_ids_.size() != 0) {
        this->getMainChatList();
    }
}

void ChatList::newChat(td_api::updateNewChat *updateNewChat)
{
    auto chat = updateNewChat->chat_.release();

    if (chat != nullptr) {
        if (true == _chats.contains(chat->id_)) {
            qWarning() << "Deleting chat";
            delete _chats[chat->id_];
        }

        auto newChat = new Chat(chat, _files);
        newChat->setTelegramManager(_manager);
        newChat->setUsers(_users);
        if (newChat->getChatType() == "secret") newChat->setSecretChat(_secretChats[newChat->getSecretChatId()]);
        QQmlEngine::setObjectOwnership(newChat, QQmlEngine::CppOwnership);
        connect(newChat, SIGNAL(chatPhotoChanged(qint64)), this, SLOT(onChatPhotoChanged(qint64)));
        connect(newChat, SIGNAL(unreadCountChanged(qint64,qint32)), this, SLOT(onUnreadCountChanged(qint64,qint32)));
        connect(newChat, SIGNAL(secretChatChanged(qint64)), this, SLOT(secretChatStateChanged(qint64)));

        if (newChat->getChatType() == "channel") {
            connect(this, SIGNAL(channelNotificationSettingsChanged(td_api::scopeNotificationSettings*)), newChat, SLOT(scopeNotificationSettingsChanged(td_api::scopeNotificationSettings*)));
            newChat->scopeNotificationSettingsChanged(_channelNotificationSettings.getScopeNotificationSettings());
        }
        if (newChat->getChatType() == "group" || newChat->getChatType() == "supergroup") {
            connect(this, SIGNAL(groupNotificationSettingsChanged(td_api::scopeNotificationSettings*)), newChat, SLOT(scopeNotificationSettingsChanged(td_api::scopeNotificationSettings*)));
            newChat->scopeNotificationSettingsChanged(_groupNotificationSettings.getScopeNotificationSettings());
        }
        if (newChat->getChatType() == "private" || newChat->getChatType() == "secret") {
            connect(this, SIGNAL(privateNotificationSettingsChanged(td_api::scopeNotificationSettings*)), newChat, SLOT(scopeNotificationSettingsChanged(td_api::scopeNotificationSettings*)));
            newChat->scopeNotificationSettingsChanged(_privateNotificationSettings.getScopeNotificationSettings());
        }
        _chats[chat->id_] = newChat;

        this->updateChat(chat->id_, {});
    }
}

void ChatList::updateChatPhoto(td_api::updateChatPhoto *updateChatPhoto)
{
    auto chat = updateChatPhoto->chat_id_;
    auto photo = move(updateChatPhoto->photo_);
    _chats[chat]->setChatPhoto(move(photo));
    this->updateChat(chat, {PhotoRole, HasPhotoRole});
}

void ChatList::updateChatLastMessage(td_api::updateChatLastMessage *updateChatLastMessage)
{
    setChatOrder(updateChatLastMessage->chat_id_, updateChatLastMessage->order_);

    if (updateChatLastMessage->last_message_ != nullptr) {
        _chats[updateChatLastMessage->chat_id_]->setLastMessage(move(updateChatLastMessage->last_message_));
        updateChat(updateChatLastMessage->chat_id_, {LastMessageRole, LastMessageAuthorRole});
    }
}

void ChatList::updateChatOrder(td_api::updateChatOrder *updateChatOrder)
{
    setChatOrder(updateChatOrder->chat_id_, updateChatOrder->order_);
}

void ChatList::updateSecretChat(td_api::updateSecretChat *updateSecretChat)
{
    if (!_secretChats.contains(updateSecretChat->secret_chat_->id_)) {
        _secretChats[updateSecretChat->secret_chat_->id_] = updateSecretChat->secret_chat_.release();
    }
}

void ChatList::secretChatStateChanged(qint64 chatId)
{
    updateChat(chatId, {SecretChatStateRole});
}

void ChatList::updateScopeNotificationSettings(td_api::updateScopeNotificationSettings *updateScopeNotificationSettings)
{
    switch (updateScopeNotificationSettings->scope_->get_id()) {
    case td_api::notificationSettingsScopeChannelChats::ID:
        _channelNotificationSettings.setScopeNotificationSettings(std::move(updateScopeNotificationSettings->notification_settings_), "channel");
        emit channelNotificationSettingsChanged(_channelNotificationSettings.getScopeNotificationSettings());
        return;
    case td_api::notificationSettingsScopeGroupChats::ID:
        _groupNotificationSettings.setScopeNotificationSettings(std::move(updateScopeNotificationSettings->notification_settings_), "group");
        emit groupNotificationSettingsChanged(_groupNotificationSettings.getScopeNotificationSettings());
        return;
    case td_api::notificationSettingsScopePrivateChats::ID:
        _privateNotificationSettings.setScopeNotificationSettings(std::move(updateScopeNotificationSettings->notification_settings_), "private");
        emit privateNotificationSettingsChanged(_privateNotificationSettings.getScopeNotificationSettings());
        return;
    }
}
