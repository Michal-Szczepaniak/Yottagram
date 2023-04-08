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
#include <limits>

ChatList::ChatList() :
    _chats_ids(&_main_chats_ids),
    _channelNotificationSettings(nullptr),
    _groupNotificationSettings(nullptr),
    _privateNotificationSettings(nullptr),
    _selectedChatList(0),
    _selectedFilterChatList(-1)
{
    _secretChatsInfo = std::make_shared<SecretChatsInfo>();
    _basicGroupsInfo = std::make_shared<BasicGroupsInfo>();
    _supergroupsInfo = std::make_shared<SupergroupsInfo>();
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
    _secretChatsInfo->setTelegramManager(_manager);
    _basicGroupsInfo->setTelegramManager(_manager);
    _supergroupsInfo->setTelegramManager(_manager);

    connect(_manager.get(), &TelegramManager::updateNewChat, this, &ChatList::newChat);
    connect(_manager.get(), &TelegramManager::updateChatPhoto, this, &ChatList::updateChatPhoto);
    connect(_manager.get(), &TelegramManager::updateChatLastMessage, this, &ChatList::updateChatLastMessage);
    connect(_manager.get(), &TelegramManager::updateChatPosition, this, &ChatList::updateChatPosition);
    connect(_manager.get(), &TelegramManager::updateScopeNotificationSettings, this, &ChatList::updateScopeNotificationSettings);
    connect(_manager.get(), &TelegramManager::updateUnreadChatCount, this, &ChatList::updateUnreadChatCount);
    connect(_manager.get(), &TelegramManager::updateUnreadMessageCount, this, &ChatList::updateUnreadMessageCount);
    connect(_manager.get(), &TelegramManager::updateChatUnreadMentionCount, this, &ChatList::updateChatUnreadMentionCount);
    connect(_manager.get(), &TelegramManager::updateMessageMentionRead, this, &ChatList::updateMessageMentionRead);
    connect(_manager.get(), &TelegramManager::updateChatDraftMessage, this, &ChatList::updateChatDraftMessage);
    connect(_manager.get(), &TelegramManager::gotChats, this, &ChatList::onGotChats);
    connect(_manager.get(), &TelegramManager::gotScopeNotificationSettings, this, &ChatList::onGotScopeNotificationSettings);
    connect(_manager.get(), &TelegramManager::createdPrivateChat, this, &ChatList::onCreatedPrivateChat);
}

void ChatList::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void ChatList::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

bool ChatList::getDaemonEnabled() const
{
    return _manager->getDaemonEnabled();
}

void ChatList::setDaemonEnabled(bool daemonEnabled)
{
    _manager->setDaemonEnabled(daemonEnabled);
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

int64_t ChatList::getForwardedFrom() const
{
    return _forwardedFrom;
}

void ChatList::setForwardedFrom(int64_t forwardedFrom)
{
    _forwardedFrom = forwardedFrom;

    emit forwardedFromChanged();
}

int32_t ChatList::getChatUnreadCount() const
{
    return _unreadChatCount.unread_count_;
}

int32_t ChatList::getChatUnreadUnmutedCount() const
{
    return _unreadChatCount.unread_unmuted_count_;
}

int32_t ChatList::getMessageUnreadCount() const
{
    return _unreadMessageCount.unread_count_;
}

int32_t ChatList::getMessageUnreadUnmutedCount() const
{
    return _unreadMessageCount.unread_unmuted_count_;
}

void ChatList::createChat(td_api::chat *chat)
{
    if (chat != nullptr) {
        if (true == _chats.contains(chat->id_)) {
            qWarning() << "Deleting chat";
            delete _chats[chat->id_];
        }

        auto newChat = new Chat(chat, _files);
        newChat->setTelegramManager(_manager);
        newChat->setUsers(_users);
        newChat->setSecretChatsInfo(_secretChatsInfo);
        newChat->setBasicGroupsInfo(_basicGroupsInfo);
        newChat->setSupergroupsInfo(_supergroupsInfo);
        newChat->initialize();
        QQmlEngine::setObjectOwnership(newChat, QQmlEngine::CppOwnership);
        connect(newChat, &Chat::chatPhotoChanged, this, &ChatList::onChatPhotoChanged);
        connect(newChat, &Chat::unreadCountChanged, this, &ChatList::onUnreadCountChanged);
        connect(newChat, &Chat::secretChatChanged, this, &ChatList::secretChatStateChanged);
        connect(newChat, &Chat::lastReadInboxMessageIdChanged, this, &ChatList::lastReadInboxMessageIdChanged);
        connect(newChat, &Chat::lastReadOutboxMessageIdChanged, this, &ChatList::lastReadOutboxMessageIdChanged);

        if (newChat->getChatType() == "channel") {
            connect(this, &ChatList::channelNotificationSettingsChanged, newChat, &Chat::scopeNotificationSettingsChanged);
            newChat->scopeNotificationSettingsChanged(_channelNotificationSettings.getScopeNotificationSettings());
        }
        if (newChat->getChatType() == "group" || newChat->getChatType() == "supergroup") {
            connect(this, &ChatList::groupNotificationSettingsChanged, newChat, &Chat::scopeNotificationSettingsChanged);
            newChat->scopeNotificationSettingsChanged(_groupNotificationSettings.getScopeNotificationSettings());
        }
        if (newChat->getChatType() == "private" || newChat->getChatType() == "secret") {
            connect(this, &ChatList::privateNotificationSettingsChanged, newChat, &Chat::scopeNotificationSettingsChanged);
            newChat->scopeNotificationSettingsChanged(_privateNotificationSettings.getScopeNotificationSettings());
        }
        _chats[chat->id_] = newChat;

        updateChat(chat->id_, {});
        emit chatCreated(chat->id_);
    }
}

int ChatList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _chats_ids->length();
}

QVariant ChatList::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount() || index.row() < 0 || !_chats.contains((*_chats_ids)[index.row()])) return QVariant();

    auto chatNode = _chats[(*_chats_ids)[index.row()]];
    switch (role) {
    case ChatElementRoles::TypeRole:
        return chatNode->getChatType();
    case ChatElementRoles::IdRole:
        return QVariant::fromValue(chatNode->getId());
    case ChatElementRoles::NameRole:
        return chatNode->getTitle();
    case ChatElementRoles::OrderRole:
        return QVariant::fromValue(chatNode->getOrder(getSelectedChatList().get()));
    case ChatElementRoles::PhotoRole:
        if (chatNode->hasPhoto()) {
            return QVariant::fromValue(chatNode->getSmallPhoto());
        }
        return QVariant();
    case ChatElementRoles::HasPhotoRole:
        return chatNode->hasPhoto();
    case ChatElementRoles::UnreadCountRole:
        return chatNode->getUnreadCount();
    case ChatElementRoles::UnreadMentionCountRole:
        return chatNode->getUnreadMentionCount();
    case ChatElementRoles::LastMessageRole:
        return getMessageText(chatNode->lastMessage());
    case ChatElementRoles::LastMessageAuthorRole:
    {
        td_api::message* message = chatNode->getLastMessage();
        if (message == nullptr) return "";

        QString lastMessageInfo = "";

        if (message->is_outgoing_) {
            lastMessageInfo += "You: ";
        } else if (chatNode->getChatType() == "group" || chatNode->getChatType() == "supergroup") {
            QString name;
            if (message->sender_id_->get_id() == td_api::messageSenderUser::ID) {
                shared_ptr<User> user = _users->getUser(static_cast<td_api::messageSenderUser*>(message->sender_id_.get())->user_id_);
                if(user != nullptr) name = user->getName();
            } else if (message->sender_id_->get_id() == td_api::messageSenderChat::ID){
                Chat* chat = getChat(static_cast<td_api::messageSenderChat*>(message->sender_id_.get())->chat_id_);
                if (chat != nullptr) name = chat->getTitle();
            }
            if(name != "") {
                lastMessageInfo += name + ": ";
            }
        }

        return lastMessageInfo;
    }
    case ChatElementRoles::IsSelfRole:
        return chatNode->isSelf();
    case ChatElementRoles::SecretChatStateRole:
    {
        auto chat = chatNode->getSecretChatInfo();
        if (chat == nullptr) return "";
        return chat->getState();
    }
    case ChatElementRoles::IsPinnedRole:
        return chatNode->isPinned(getSelectedChatList());
    case ChatElementRoles::IsReadRole:
    {
        td_api::message* message = chatNode->getLastMessage();
        if (message == nullptr) return false;

        if (message->is_outgoing_) {
            return message->id_ <= chatNode->lastReadOutboxMessageId();
        }  else {
            return message->id_ <= chatNode->lastReadInboxMessageId();
        }
    }
    case ChatElementRoles::LastMessageTimestampRole:
    {
        td_api::message* message = chatNode->getLastMessage();
        if (message == nullptr) return "";

        return message->date_;
    }
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
    roles[PhotoRole] = "photo";
    roles[HasPhotoRole] = "hasPhoto";
    roles[UnreadCountRole] = "unreadCount";
    roles[UnreadMentionCountRole] = "unreadMentionCount";
    roles[LastMessageAuthorRole] = "lastMessageAuthor";
    roles[LastMessageRole] = "lastMessage";
    roles[IsSelfRole] = "isSelf";
    roles[SecretChatStateRole] = "secretChatState";
    roles[IsPinnedRole] = "isPinned";
    roles[IsReadRole] = "isRead";
    roles[LastMessageTimestampRole] = "lastMessageTimestamp";
    return roles;
}

void ChatList::updateChat(int64_t chat, const QVector<int> &roles) {
    if(rowCount() <= 0) return;

    auto it = std::find(_chats_ids->begin(), _chats_ids->end(), chat);
    if (it != _chats_ids->end()) {
        auto index = std::distance(_chats_ids->begin(), it);
        emit dataChanged(createIndex(index, 0), createIndex(index, 0), roles);
    }
}

QVector<int64_t> *ChatList::getChatList(td_api::ChatList *list)
{
    switch (list->get_id()) {
    case td_api::chatListMain::ID:
        return &_main_chats_ids;
        break;
    case td_api::chatListArchive::ID:
        return &_archive_chats_ids;
        break;
    case td_api::chatListFilter::ID:
        return &_filter_chats_ids[static_cast<const td_api::chatListFilter *>(list)->chat_filter_id_];
        break;
    }
}

td_api::object_ptr<td_api::ChatList> ChatList::getSelectedChatList() const
{
    switch (_selectedChatList) {
    case 0:
        return td_api::make_object<td_api::chatListMain>();
    case 1:
        return td_api::make_object<td_api::chatListArchive>();
    case 2:
        return td_api::make_object<td_api::chatListFilter>(_selectedFilterChatList);
    default:
        qCritical() << "Wrong _selectedChatList!";
        return nullptr;
    }
}

void ChatList::setChatPosition(int64_t chatId, td_api::chatPosition *position)
{
    QVector<int64_t> *list = getChatList(position->list_.get());
    int64_t order = position->order_;
    if (order == 0) {
        int indexOf = list->indexOf(chatId);
        if (indexOf != -1) {
            if (list == getChatList(getSelectedChatList().get())) beginRemoveRows(QModelIndex(), indexOf, indexOf);
            list->remove(indexOf);
            if (list == getChatList(getSelectedChatList().get())) endRemoveRows();
        }
    } else {
        if (list->indexOf(chatId) == -1) {
            if (list == _chats_ids) beginInsertRows(QModelIndex(), rowCount(), rowCount());
            list->append(chatId);
            if (list == _chats_ids) endInsertRows();
        }
    }
    getChat(chatId)->updatePosition(position);
    if (list == _chats_ids) updateChat(chatId, {OrderRole});
}

QString ChatList::getMessageText(shared_ptr<Message> message) const
{
    if (message == nullptr) return "";

    switch (message->getContentType()) {
    case td_api::messagePhoto::ID:
        return tr("Photo");
    case td_api::messageVideo::ID:
        return tr("Video");
    case td_api::messageDocument::ID:
        return tr("Document");
    case td_api::messageAudio::ID:
        return tr("Audio");
    case td_api::messageAnimation::ID:
        return tr("GIF");
    case td_api::messageVoiceNote::ID:
        return tr("VoiceNote");
    case td_api::messagePoll::ID:
        return tr("Poll");
    case td_api::messageLocation::ID:
        return tr("Location");
    case td_api::messageContact::ID:
        return tr("Contact");
    case td_api::messageSticker::ID:
        return tr("Sticker");
    case td_api::messageVideoNote::ID:
        return tr("VideoNote");
    }

    return message->getText();
}

void ChatList::fetchChatList()
{
    td_api::object_ptr<td_api::ChatList> list = static_cast<td_api::object_ptr<td_api::ChatList>>(getSelectedChatList());

    _manager->sendQueryWithResponse(0, td_api::getChats::ID, 0, new td_api::loadChats(move(list), std::numeric_limits<int32_t>::max()));
}

void ChatList::fetchAllScopeNotificationSettings()
{
    auto settingsQuerty = new td_api::getScopeNotificationSettings(td_api::make_object<td_api::notificationSettingsScopePrivateChats>());
    _manager->sendQueryWithResponse(0, td_api::getScopeNotificationSettings::ID, td_api::notificationSettingsScopePrivateChats::ID, settingsQuerty);
    settingsQuerty = new td_api::getScopeNotificationSettings(td_api::make_object<td_api::notificationSettingsScopeGroupChats>());
    _manager->sendQueryWithResponse(0, td_api::getScopeNotificationSettings::ID, td_api::notificationSettingsScopeGroupChats::ID, settingsQuerty);
    settingsQuerty = new td_api::getScopeNotificationSettings(td_api::make_object<td_api::notificationSettingsScopeChannelChats>());
    _manager->sendQueryWithResponse(0, td_api::getScopeNotificationSettings::ID, td_api::notificationSettingsScopeChannelChats::ID, settingsQuerty);
}

QVariant ChatList::openChat(int64_t chatId)
{
    auto chat = getChat(chatId);
    if (chat == nullptr) return QVariant();

    _manager->sendQuery(new td_api::openChat(chat->getId()));
    chat->setIsOpen(true);
    QQmlEngine::setObjectOwnership(chat, QQmlEngine::CppOwnership);

    return QVariant::fromValue(chat);
}

void ChatList::closeChat(int64_t chatId)
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

QVariant ChatList::getChatAsVariant(int64_t chatId) const
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

QVariant ChatList::getChatAsVariantForUser(int64_t userId)
{
    int64_t chatId;

    auto result = std::find_if(_chats.begin(), _chats.end(), [&](Chat* chat){ return chat->getChatType() == "private" && chat->getIdFromType() == userId; });
    if (result != _chats.end()) {
        Chat* chat = *result;
        chatId = chat->getId();
        return getChatAsVariant(chatId);
    } else {
        _manager->sendQueryWithResponse(td_api::createPrivateChat::ID, new td_api::createPrivateChat(userId, false));

        return QVariant();
    }
}

void ChatList::markChatAsRead(int64_t chatId)
{
    _manager->sendQuery(new td_api::viewMessages(getChat(chatId)->getId(), 0, {getChat(chatId)->getLastMessage()->id_}, true));
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

void ChatList::togglePinnedChat(int64_t chatId)
{
    _manager->sendQuery(new td_api::toggleChatIsPinned(getSelectedChatList(), chatId, !getChat(chatId)->isPinned(getSelectedChatList())));
}

bool ChatList::autoDownloadEnabled()
{
    return _files->getActiveAutoDownloadSetting()->getIsAutoDownloadEnabled();
}

void ChatList::autoDownload(int32_t fileId, QString type)
{
    _files->considerAutoDownloading(fileId, type);
}

bool ChatList::fileExists(QString path)
{
    return QFile::exists(path);
}

void ChatList::switchChatList(int chatList, int64_t filterId)
{
    beginResetModel();
    _selectedChatList = chatList;
    if (chatList == 2) _selectedFilterChatList = filterId;
    _chats_ids = getChatList(getSelectedChatList().get());
    if (_chats_ids->empty()) fetchChatList();
    endResetModel();
}

void ChatList::onIsAuthorizedChanged(bool isAuthorized)
{
    _isAuthorized = isAuthorized;

    fetchAllScopeNotificationSettings();
    fetchChatList();
}

void ChatList::onChatPhotoChanged(int64_t chatId)
{
    this->updateChat(chatId, {PhotoRole, HasPhotoRole});
}

void ChatList::onUnreadCountChanged(int64_t chatId, int32_t unreadCount)
{
    Q_UNUSED(unreadCount)
    this->updateChat(chatId, {UnreadCountRole});
}

void ChatList::onGotChats(td_api::chats *chats)
{
}

void ChatList::onGotScopeNotificationSettings(int32_t scope, td_api::scopeNotificationSettings *scopeNotificationSettings)
{
    switch (scope) {
    case td_api::notificationSettingsScopeChannelChats::ID:
        _channelNotificationSettings.setScopeNotificationSettings(td_api::object_ptr<td_api::scopeNotificationSettings>(scopeNotificationSettings), "channel");
        emit channelNotificationSettingsChanged(_channelNotificationSettings.getScopeNotificationSettings());
        return;
    case td_api::notificationSettingsScopeGroupChats::ID:
        _groupNotificationSettings.setScopeNotificationSettings(td_api::object_ptr<td_api::scopeNotificationSettings>(scopeNotificationSettings), "group");
        emit groupNotificationSettingsChanged(_groupNotificationSettings.getScopeNotificationSettings());
        return;
    case td_api::notificationSettingsScopePrivateChats::ID:
        _privateNotificationSettings.setScopeNotificationSettings(td_api::object_ptr<td_api::scopeNotificationSettings>(scopeNotificationSettings), "private");
        emit privateNotificationSettingsChanged(_privateNotificationSettings.getScopeNotificationSettings());
        return;
    }
}

void ChatList::newChat(td_api::updateNewChat *updateNewChat)
{
    auto chat = updateNewChat->chat_.release();

    createChat(chat);
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
    for (td_api::object_ptr<td_api::chatPosition> &position: updateChatLastMessage->positions_) {
        setChatPosition(updateChatLastMessage->chat_id_, position.release());
    }

    if (updateChatLastMessage->last_message_) {
        _chats[updateChatLastMessage->chat_id_]->setLastMessage(move(updateChatLastMessage->last_message_));
        updateChat(updateChatLastMessage->chat_id_, {LastMessageRole, LastMessageAuthorRole});
    }
}

void ChatList::updateChatPosition(td_api::updateChatPosition *updateChatPosition)
{
    setChatPosition(updateChatPosition->chat_id_, updateChatPosition->position_.release());
}

void ChatList::secretChatStateChanged(int64_t chatId)
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

void ChatList::lastReadInboxMessageIdChanged(int64_t chatId, int64_t lastReadInboxMessageIdChanged)
{
    updateChat(chatId, {IsReadRole});
}

void ChatList::lastReadOutboxMessageIdChanged(int64_t chatId, int64_t lastReadOutboxMessageIdChanged)
{
    updateChat(chatId, {IsReadRole});
}

void ChatList::updateUnreadChatCount(td_api::updateUnreadChatCount *updateUnreadChatCount)
{
    if (updateUnreadChatCount->chat_list_->get_id() != td_api::chatListMain::ID) return;

    _unreadChatCount.total_count_ = updateUnreadChatCount->total_count_;
    _unreadChatCount.unread_count_ = updateUnreadChatCount->unread_count_;
    _unreadChatCount.unread_unmuted_count_ = updateUnreadChatCount->unread_unmuted_count_;
    _unreadChatCount.marked_as_unread_count_ = updateUnreadChatCount->marked_as_unread_count_;
    _unreadChatCount.marked_as_unread_unmuted_count_ = updateUnreadChatCount->marked_as_unread_unmuted_count_;
    emit unreadChatCountChanged();
}

void ChatList::updateUnreadMessageCount(td_api::updateUnreadMessageCount *updateUnreadMessageCount)
{
    if (updateUnreadMessageCount->chat_list_->get_id() != td_api::chatListMain::ID) return;
    _unreadMessageCount.unread_count_ = updateUnreadMessageCount->unread_count_;
    _unreadMessageCount.unread_unmuted_count_ = updateUnreadMessageCount->unread_unmuted_count_;
    emit unreadMessageCountChanegd();
}

void ChatList::updateChatUnreadMentionCount(td_api::updateChatUnreadMentionCount *updateChatUnreadMentionCount)
{
    getChat(updateChatUnreadMentionCount->chat_id_)->setUnreadMentionCount(updateChatUnreadMentionCount->unread_mention_count_);
    updateChat(updateChatUnreadMentionCount->chat_id_, {UnreadMentionCountRole});
}

void ChatList::updateMessageMentionRead(td_api::updateMessageMentionRead *updateMessageMentionRead)
{
    getChat(updateMessageMentionRead->chat_id_)->updateMessageMentionRead(updateMessageMentionRead->unread_mention_count_, updateMessageMentionRead->message_id_);
    updateChat(updateMessageMentionRead->chat_id_, {UnreadMentionCountRole});
}

void ChatList::updateChatDraftMessage(td_api::updateChatDraftMessage *updateChatDraftMessage)
{
    for (td_api::object_ptr<td_api::chatPosition> &position: updateChatDraftMessage->positions_) {
        setChatPosition(updateChatDraftMessage->chat_id_, position.release());
    }

    _chats[updateChatDraftMessage->chat_id_]->setDraftMessage(move(updateChatDraftMessage->draft_message_));
}

void ChatList::onCreatedPrivateChat(td_api::chat *chat)
{
    if (chat != nullptr && !_chats.contains(chat->id_)) {
        createChat(chat);
    }
}
