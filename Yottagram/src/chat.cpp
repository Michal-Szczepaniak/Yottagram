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

#include "chat.h"

#include <QDateTime>
#include <QFileInfo>
#include <QDesktopServices>
#include <QQmlEngine>
#include "overloaded.h"
#include <contents/animatedemoji.h>

Chat::Chat() :
    _smallPhotoId(0),
    _bigPhotoId(0),
    _chat(nullptr),
    _files(nullptr),
    _notificationSettings(nullptr),
    _scopeNotificationSettings(nullptr),
    _mainPosition(nullptr),
    _archivePosition(nullptr)
{
}

Chat::Chat(td_api::chat* chat, shared_ptr<Files> files) :
    _smallPhotoId(0),
    _bigPhotoId(0),
    _chat(chat),
    _files(files),
    _notificationSettings(nullptr),
    _scopeNotificationSettings(nullptr),
    _mainPosition(nullptr),
    _archivePosition(nullptr)
{
    _basicGroupFullInfo = new BasicGroupFullInfo();
    _supergroupFullInfo = new SupergroupFullInfo();
    _notificationSettings = move(chat->notification_settings_);
    setLastReadInboxMessageId(chat->last_read_inbox_message_id_);
    setLastReadOutboxMessageId(chat->last_read_outbox_message_id_);
    setUnreadCount(_chat->unread_count_);
    setUnreadMentionCount(_chat->unread_mention_count_);
    if (_chat->last_message_) setLastMessage(move(_chat->last_message_));

    for (td_api::object_ptr<td_api::chatPosition> &position: _chat->positions_) {
        updatePosition(position.release());
    }

    if (this->hasPhoto()) {
        setChatPhoto(std::move(chat->photo_));
    }
}

Chat::~Chat()
{
    delete _chat;
}

int64_t Chat::getId() const
{
    return _chat->id_;
}

int64_t Chat::getIdFromType() const
{
    switch (_chat->type_->get_id()) {
    case td_api::chatTypeBasicGroup::ID:
        return static_cast<td_api::chatTypeBasicGroup*>(_chat->type_.get())->basic_group_id_;
    case td_api::chatTypePrivate::ID:
        return static_cast<td_api::chatTypePrivate*>(_chat->type_.get())->user_id_;
    case td_api::chatTypeSecret::ID:
        return static_cast<td_api::chatTypeSecret*>(_chat->type_.get())->secret_chat_id_;
    case td_api::chatTypeSupergroup::ID:
        return static_cast<td_api::chatTypeSupergroup*>(_chat->type_.get())->supergroup_id_;
    default:
        return -1;
    }
}

int32_t Chat::getSecretChatId() const
{
    return static_cast<td_api::chatTypeSecret*>(_chat->type_.get())->secret_chat_id_;
}

QString Chat::getTitle() const
{
    return QString::fromStdString(_chat->title_);
}

SecretChatInfo *Chat::getSecretChatInfo() const
{
    if (_chat->type_->get_id() != td_api::chatTypeSecret::ID || _secretChatsInfo->getSecretChat(getIdFromType()) == nullptr) return nullptr;

    auto chat = _secretChatsInfo->getSecretChat(getIdFromType());
    QQmlEngine::setObjectOwnership(chat, QQmlEngine::CppOwnership);
    return chat;
}

BasicGroupInfo *Chat::getBasicGroupInfo() const
{
    if (_chat->type_->get_id() != td_api::chatTypeBasicGroup::ID || _basicGroupsInfo->getBasicGroup(getIdFromType()) == nullptr) return nullptr;

    auto chat = _basicGroupsInfo->getBasicGroup(getIdFromType());
    QQmlEngine::setObjectOwnership(chat, QQmlEngine::CppOwnership);
    return chat;
}

SupergroupInfo *Chat::getSupergroupInfo() const
{
    if (_chat->type_->get_id() != td_api::chatTypeSupergroup::ID || _supergroupsInfo->getSupergroup(getIdFromType()) == nullptr) return nullptr;

    auto chat = _supergroupsInfo->getSupergroup(getIdFromType());
    QQmlEngine::setObjectOwnership(chat, QQmlEngine::CppOwnership);
    return chat;
}

UserFullInfo* Chat::getUserFullInfo() const
{
    if (_chat->type_->get_id() == td_api::chatTypePrivate::ID) {
        auto fullInfo = _users->getUser(static_cast<td_api::chatTypePrivate*>(_chat->type_.get())->user_id_)->getUserFullInfo();
        QQmlEngine::setObjectOwnership(fullInfo, QQmlEngine::CppOwnership);
        return fullInfo;
    }

    return nullptr;
}

BasicGroupFullInfo *Chat::getBasicGroupFullInfo() const
{
    return _basicGroupFullInfo;
}

SupergroupFullInfo *Chat::getSupergroupFullInfo() const
{
    return _supergroupFullInfo;
}

int64_t Chat::getOrder(td_api::ChatList* list) const
{
    switch (list->get_id()) {
    case td_api::chatListMain::ID:
        if (_mainPosition != nullptr) return _mainPosition->order_;
        break;
    case td_api::chatListArchive::ID:
        if (_archivePosition != nullptr) return _archivePosition->order_;
        break;
    case td_api::chatListFilter::ID:
        auto filterListId = static_cast<td_api::chatListFilter*>(list)->chat_filter_id_;
        if (_filterPositions.contains(filterListId)) {
            return _filterPositions[filterListId]->order_;
        }
    }
    return 0;
}

QString Chat::getChatType() const
{
    switch (_chat->type_->get_id()) {
    case td_api::chatTypeBasicGroup::ID:
        return "group";
    case td_api::chatTypePrivate::ID:
        return "private";
    case td_api::chatTypeSecret::ID:
        return "secret";
    case td_api::chatTypeSupergroup::ID:
        if (static_cast<td_api::chatTypeSupergroup*>(_chat->type_.get())->is_channel_) return "channel";
        return "supergroup";
    default:
        return "";
    }
}

int32_t Chat::getUnreadCount() const
{
    return _unreadCount;
}

void Chat::setUnreadCount(int32_t unreadCount)
{
    _unreadCount = unreadCount;

    emit unreadCountChanged(this->getId(), unreadCount);
}

int32_t Chat::getUnreadMentionCount() const
{
    return _unreadMentionCount;
}

void Chat::setUnreadMentionCount(int32_t unreadMentionCount)
{
    _unreadMentionCount = unreadMentionCount;

    emit unreadMentionCountChanged(unreadMentionCount);
}

int64_t Chat::firstUnreadMention() const
{
    int64_t messageId = 0;
    for (QVector<const int64_t>::reverse_iterator it = _message_ids.rbegin(); it != _message_ids.rend(); ++it) {
        if (_messages.contains(*it) && _messages[*it]->containsUnreadMention()) {
            messageId = *it;
            break;
        }
    }
    return messageId;
}

void Chat::updateMessageMentionRead(int32_t unreadMentionCount, int64_t messageId)
{
    _unreadMentionCount = unreadMentionCount;

    if (_messages.contains(messageId)) _messages[messageId]->setContainsUnreadMention(false);

    emit unreadMentionCountChanged(unreadMentionCount);
}

void Chat::setLastReadInboxMessageId(int64_t messageId)
{
    _lastReadInboxMessageId = messageId;
    emit lastReadInboxMessageIdChanged(getId(), messageId);
}

void Chat::setLastReadOutboxMessageId(int64_t messageId)
{
    _lastReadOutboxMessageId = messageId;
    emit lastReadOutboxMessageIdChanged(getId(), messageId);
}

void Chat::setChatPhoto(td_api::object_ptr<td_api::chatPhotoInfo> chatPhoto)
{
    if (chatPhoto == nullptr) {
        _smallPhotoId = 0;
        _bigPhotoId = 0;
    } else {
        _smallPhotoId = chatPhoto->small_->id_;
        _files->appendFile(std::move(chatPhoto->small_), "avatar");
        _bigPhotoId = chatPhoto->big_->id_;
        _files->appendFile(std::move(chatPhoto->big_), "");
    }
    _chat->photo_ = std::move(chatPhoto);
    emit chatPhotoChanged(getId());
}

bool Chat::isSelf() const
{
    if (_chat->type_->get_id() != td_api::chatTypePrivate::ID) return false;

    int64_t userId = getIdFromType();
    return userId == _manager->getMyId();
}

void Chat::setIsOpen(bool isOpen)
{
    _isOpen = isOpen;
}

bool Chat::isOpen() const
{
    return _isOpen;
}

bool Chat::isPinned(td_api::object_ptr<td_api::ChatList> list) const
{
    switch (list->get_id()) {
        case td_api::chatListMain::ID:
        return _mainPosition->is_pinned_;
    case td_api::chatListArchive::ID:
        return _archivePosition->is_pinned_;
    case td_api::chatListFilter::ID:
        auto filterListId = static_cast<td_api::chatListFilter*>(list.get())->chat_filter_id_;
        if (_filterPositions.contains(filterListId)) {
            return _filterPositions[filterListId]->is_pinned_;
        } else {
            return -1;
        }
    }
    return 0;
}

int32_t Chat::getTtl() const
{
    return _chat->message_ttl_setting_;
}

int32_t Chat::getMuteFor() const
{
    if (_notificationSettings == nullptr) return 0;

    if (_notificationSettings->use_default_mute_for_) return _scopeNotificationSettings->mute_for_;

    return _notificationSettings->mute_for_;
}

void Chat::setMuteFor(int32_t muteFor)
{
    if (_notificationSettings == nullptr) return;

    auto settings = td_api::make_object<td_api::chatNotificationSettings>(
                _notificationSettings->use_default_mute_for_,
                _notificationSettings->mute_for_,
                _notificationSettings->use_default_sound_,
                _notificationSettings->sound_,
                _notificationSettings->use_default_show_preview_,
                _notificationSettings->show_preview_,
                _notificationSettings->use_default_disable_pinned_message_notifications_,
                _notificationSettings->disable_pinned_message_notifications_,
                _notificationSettings->use_default_disable_mention_notifications_,
                _notificationSettings->disable_mention_notifications_
    );
    settings->use_default_mute_for_ = _scopeNotificationSettings->mute_for_ == muteFor;
    settings->mute_for_ = muteFor;
    _manager->sendQuery(new td_api::setChatNotificationSettings(getId(), move(settings)));
}

bool Chat::getDefaultMuteFor() const
{
    if (_notificationSettings == nullptr) return false;

    return _scopeNotificationSettings->mute_for_ == _notificationSettings->mute_for_;
}

bool Chat::getShowPreview() const
{
    if (_notificationSettings == nullptr) return 0;

    if (_notificationSettings->use_default_show_preview_) return _scopeNotificationSettings->show_preview_;

    return _notificationSettings->show_preview_;
}

void Chat::setShowPreview(bool showPreview)
{
    if (_notificationSettings == nullptr) return;

    auto settings = td_api::make_object<td_api::chatNotificationSettings>(
                _notificationSettings->use_default_mute_for_,
                _notificationSettings->mute_for_,
                _notificationSettings->use_default_sound_,
                _notificationSettings->sound_,
                _notificationSettings->use_default_show_preview_,
                _notificationSettings->show_preview_,
                _notificationSettings->use_default_disable_pinned_message_notifications_,
                _notificationSettings->disable_pinned_message_notifications_,
                _notificationSettings->use_default_disable_mention_notifications_,
                _notificationSettings->disable_mention_notifications_
    );
    settings->use_default_show_preview_ = _scopeNotificationSettings->show_preview_ == showPreview;
    settings->show_preview_ = showPreview;
    _manager->sendQuery(new td_api::setChatNotificationSettings(getId(), move(settings)));
}

bool Chat::getDefaultShowPreview() const
{
    if (_notificationSettings == nullptr) return false;

    return _scopeNotificationSettings->show_preview_ == _notificationSettings->show_preview_;
}

bool Chat::getDisablePinnedMessageNotifications() const
{
    if (_notificationSettings == nullptr) return 0;

    if (_notificationSettings->use_default_disable_pinned_message_notifications_) return _scopeNotificationSettings->disable_pinned_message_notifications_;

    return _notificationSettings->disable_pinned_message_notifications_;
}

void Chat::setDisablePinnedMessageNotifications(bool disablePinnedMessageNotifications)
{
    if (_notificationSettings == nullptr) return;

    auto settings = td_api::make_object<td_api::chatNotificationSettings>(
                _notificationSettings->use_default_mute_for_,
                _notificationSettings->mute_for_,
                _notificationSettings->use_default_sound_,
                _notificationSettings->sound_,
                _notificationSettings->use_default_show_preview_,
                _notificationSettings->show_preview_,
                _notificationSettings->use_default_disable_pinned_message_notifications_,
                _notificationSettings->disable_pinned_message_notifications_,
                _notificationSettings->use_default_disable_mention_notifications_,
                _notificationSettings->disable_mention_notifications_
    );
    settings->use_default_disable_pinned_message_notifications_ = _scopeNotificationSettings->disable_pinned_message_notifications_ == disablePinnedMessageNotifications;
    settings->disable_pinned_message_notifications_ = disablePinnedMessageNotifications;
    _manager->sendQuery(new td_api::setChatNotificationSettings(getId(), move(settings)));
}

bool Chat::getDefaultDisablePinnedMessageNotifications() const
{
    if (_notificationSettings == nullptr) return false;

    return _scopeNotificationSettings->disable_pinned_message_notifications_ == _notificationSettings->disable_pinned_message_notifications_;
}

bool Chat::getDisableMentionNotifications() const
{
    if (_notificationSettings == nullptr) return 0;

    if (_notificationSettings->use_default_disable_mention_notifications_) return _scopeNotificationSettings->disable_mention_notifications_;

    return _notificationSettings->disable_mention_notifications_;
}

void Chat::setDisableMentionNotifications(bool disableMentionNotifications)
{
    if (_notificationSettings == nullptr) return;

    auto settings = td_api::make_object<td_api::chatNotificationSettings>(
                _notificationSettings->use_default_mute_for_,
                _notificationSettings->mute_for_,
                _notificationSettings->use_default_sound_,
                _notificationSettings->sound_,
                _notificationSettings->use_default_show_preview_,
                _notificationSettings->show_preview_,
                _notificationSettings->use_default_disable_pinned_message_notifications_,
                _notificationSettings->disable_pinned_message_notifications_,
                _notificationSettings->use_default_disable_mention_notifications_,
                _notificationSettings->disable_mention_notifications_
    );
    settings->use_default_disable_mention_notifications_ = _scopeNotificationSettings->disable_mention_notifications_ == disableMentionNotifications;
    settings->disable_mention_notifications_ = disableMentionNotifications;
    _manager->sendQuery(new td_api::setChatNotificationSettings(getId(), move(settings)));
}

bool Chat::getDefaultDisableMentionNotifications() const
{
    if (_notificationSettings == nullptr) return false;

    return _scopeNotificationSettings->disable_mention_notifications_ == _notificationSettings->disable_mention_notifications_;
}

bool Chat::getCanSendMessages() const
{
    return _chat->permissions_->can_send_messages_;
}

bool Chat::getCanSendMediaMessages() const
{
    return _chat->permissions_->can_send_media_messages_;
}

bool Chat::getCanSendPolls() const
{
    return _chat->permissions_->can_send_polls_;
}

bool Chat::getCanSendOtherMessages() const
{
    return _chat->permissions_->can_send_other_messages_;
}

bool Chat::getCanAddWebPagePreviews() const
{
    return _chat->permissions_->can_add_web_page_previews_;
}

bool Chat::getCanChangeInfo() const
{
    return _chat->permissions_->can_change_info_;
}

bool Chat::getCanInviteUsers() const
{
    return _chat->permissions_->can_invite_users_;
}

bool Chat::getCanPinMessages() const
{
    return _chat->permissions_->can_pin_messages_;
}

QVector<int32_t> Chat::getFoundChatMembers() const
{
    return _foundChatMembers;
}

void Chat::updatePosition(td_api::chatPosition *position)
{
    td_api::chatPosition** listPosition;
    switch (position->list_->get_id()) {
    case td_api::chatListMain::ID:
        listPosition = &_mainPosition;
        break;
    case td_api::chatListArchive::ID:
        listPosition = &_archivePosition;
        break;
    case td_api::chatListFilter::ID:
        listPosition = &_filterPositions[static_cast<td_api::chatListFilter*>(position->list_.get())->chat_filter_id_];
        break;
    }

    if ((*listPosition) != nullptr) delete (*listPosition);
    if (position->order_ == 0) (*listPosition) = nullptr;
    else (*listPosition) = position;
}

QString Chat::draftMessage() const
{
    if (!_chat->draft_message_) return "";

    return QString::fromStdString(static_cast<td_api::inputMessageText*>(_chat->draft_message_->input_message_text_.get())->text_->text_);
}

int64_t Chat::draftMessageReplyId() const
{
    if (!_chat->draft_message_) return 0;

    return _chat->draft_message_->reply_to_message_id_;
}

void Chat::setDraftMessage(QString message, int64_t replyMessageId)
{
    td_api::object_ptr<td_api::draftMessage> newMessage = td_api::make_object<td_api::draftMessage>();
    auto messageContent = td_api::make_object<td_api::inputMessageText>();
    messageContent->text_ = td_api::make_object<td_api::formattedText>();
    messageContent->text_->text_ = message.toStdString();
    messageContent->disable_web_page_preview_ = false;
    newMessage->input_message_text_ = move(messageContent);
    newMessage->reply_to_message_id_ = replyMessageId;

    _manager->sendQuery(new td_api::setChatDraftMessage(getId(), 0, move(newMessage)));
}

void Chat::setDraftMessage(td_api::object_ptr<td_api::draftMessage> draftMessage)
{
    _chat->draft_message_ = move(draftMessage);
    emit draftMessageChanged();
}

QString Chat::actionText()
{
    if (_chatActions.empty()) return "";
    int typingCount = count(_chatActions.begin(), _chatActions.end(), ChatAction::Typing);

    if (typingCount > 2) {
        return tr("Many people are typing...");
    } else if (typingCount > 1) {
        QPair<int64_t, int64_t> users = {0,0};
        for (QHash<int64_t, ChatAction>::iterator it; it != _chatActions.end(); ++it) {
            if (it.value() == ChatAction::Typing)
                if (users.first == 0) users.first = it.key();
                else users.second = it.key();
        }
        return tr("%1 and %2 are typing...").arg(_users->getUser(users.first)->getFirstName()).arg(_users->getUser(users.second)->getFirstName());
    } else if (typingCount == 1) {
        int64_t userId = find(_chatActions.begin(), _chatActions.end(), ChatAction::Typing).key();
        return tr("%1 is typing...").arg(_users->getUser(userId)->getFirstName());
    }

    ChatAction action = _chatActions.begin().value();
    QString userFirstName = _users->getUser(_chatActions.begin().key())->getFirstName();
    switch (action) {
    case ChatAction::RecordingVideo:
        return tr("%1 is recording video").arg(userFirstName);
    case ChatAction::UploadingVideo:
        return tr("%1 is uploading video").arg(userFirstName);
    case ChatAction::RecordingVoiceNote:
        return tr("%1 is recording voicenote").arg(userFirstName);
    case ChatAction::UploadingVoiceNote:
        return tr("%1 is uploading voicenote").arg(userFirstName);
    case ChatAction::UploadingPhoto:
        return tr("%1 is uploading photo").arg(userFirstName);
    case ChatAction::UploadingDocument:
        return tr("%1 is uploading document").arg(userFirstName);
    case ChatAction::ChoosingSticker:
        return tr("%1 is choosing sticker").arg(userFirstName);
    case ChatAction::ChoosingLocation:
        return tr("%1 is choosing location").arg(userFirstName);
    case ChatAction::ChoosingContact:
        return tr("%1 is choosing contact").arg(userFirstName);
    case ChatAction::StartPlayingGame:
        return tr("%1 is playing game").arg(userFirstName);
    case ChatAction::RecordingVideoNote:
        return tr("%1 is recording videonote").arg(userFirstName);
    case ChatAction::UploadingVideoNote:
        return tr("%1 is uploading videonote").arg(userFirstName);
    case ChatAction::WatchingAnimations:
        return tr("%1 is watching animations").arg(userFirstName);
    }
}

void Chat::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateChatReadInbox, this, &Chat::updateChatReadInbox);
    connect(_manager.get(), &TelegramManager::updateChatReadOutbox, this, &Chat::updateChatReadOutbox);
    connect(_manager.get(), &TelegramManager::updateNewMessage, this, &Chat::updateNewMessage);
    connect(_manager.get(), &TelegramManager::updateChatTitle, this, &Chat::updateChatTitle);
    connect(_manager.get(), &TelegramManager::updateDeleteMessages, this, &Chat::updateDeleteMessages);
    connect(_manager.get(), &TelegramManager::updateBasicGroupFullInfo, this, &Chat::updateBasicGroupFullInfo);
    connect(_manager.get(), &TelegramManager::updateSupergroupFullInfo, this, &Chat::updateSupergroupFullInfo);
    connect(_manager.get(), &TelegramManager::myIdChanged, this, &Chat::isSelfChanged);
    connect(_manager.get(), &TelegramManager::updateChatNotificationSettings, this, &Chat::updateChatNotificationSettings);
    connect(_manager.get(), &TelegramManager::updateChatPermissions, this, &Chat::updateChatPermissions);
    connect(_manager.get(), &TelegramManager::gotChatHistory, this, &Chat::onGotChatHistory);
    connect(_manager.get(), &TelegramManager::updateChatMessageTtlSetting, this, &Chat::updateChatMessageTtlSetting);
    connect(_manager.get(), &TelegramManager::updateUserChatAction, this, &Chat::updateUserChatAction);
    connect(_manager.get(), &TelegramManager::gotMessage, this, &Chat::onGotMessage);
}

void Chat::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void Chat::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void Chat::setSecretChatsInfo(shared_ptr<SecretChatsInfo> secretChatsInfo)
{
    _secretChatsInfo = secretChatsInfo;

    connect(_secretChatsInfo.get(), &SecretChatsInfo::secretChatInfoChanged, [this](int32_t chatId){ if (chatId == getIdFromType()) emit secretChatChanged(getId()); });
}

void Chat::setBasicGroupsInfo(shared_ptr<BasicGroupsInfo> basicGroupsInfo)
{
    _basicGroupsInfo = basicGroupsInfo;

    connect(_basicGroupsInfo.get(), &BasicGroupsInfo::basicGroupInfoChanged, [this](int32_t chatId){ if (chatId == getIdFromType()) emit basicGroupChanged(getId()); });
}

void Chat::setSupergroupsInfo(shared_ptr<SupergroupsInfo> supergroupsInfo)
{
    _supergroupsInfo = supergroupsInfo;

    connect(_supergroupsInfo.get(), &SupergroupsInfo::supergroupInfoChanged, [this](int32_t chatId){ if (chatId == getIdFromType()) emit supergroupChanged(getId()); });
}

void Chat::injectDependencies(PinnedMessages *component)
{
    component->setTelegramManager(_manager);
    component->setFiles(_files);
    component->setUsers(_users);
}

int Chat::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _message_ids.size();
}

QVariant Chat::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() < 0 || index.row() >= rowCount()) return QVariant();

    if (!_messages.contains(_message_ids[index.row()])) {
        if (!_getMessageCache.contains(_message_ids[index.row()])) {
            const_cast<Chat*>(this)->_getMessageCache.append(_message_ids[index.row()]);
            fetchMessage(_message_ids[index.row()]);
        }
        return QVariant();
    }

    auto message = _messages[_message_ids[index.row()]];
    switch (role) {
    case MessageRoles::TypeRole:
        return getChatType();
    case MessageRoles::IdRole:
        return QVariant::fromValue(message->getId());
    case MessageRoles::MessageRole:
        if (message->getType() == "messageSupergroupChatCreate" && getChatType() == "channel") {
            return tr("Channel created");
        }
        return message->getText();
    case MessageRoles::MessageTypeRole:
        return message->getType();
    case MessageRoles::ReceivedRole:
        return QVariant::fromValue(message->received());
    case MessageRoles::ReplyMessageIdRole:
        return QVariant::fromValue(message->replyMessageId());
    case MessageRoles::DisplayAvatarRole:
    {
        bool display = true;
        if (_message_ids.contains(index.row()-1)) {
            Message* prev = _messages[_message_ids[index.row()-1]];
            if (message->getSenderUserId() != 0) display = prev->getSenderUserId() != message->getSenderUserId();
            if (message->getSenderChatId() != 0) display = prev->getSenderChatId() != message->getSenderChatId();
        }
        return message->received() && (getChatType() == "group" || getChatType() == "supergroup") && !message->isService() && display;
    }
    case MessageRoles::IsServiceRole:
        return message->isService();
    case MessageRoles::IsForwardedRole:
        return message->getForwardedInfo() != nullptr;
    case MessageRoles::ForwardUserRole:
    {
        if (message->getForwardedInfo() != nullptr && message->getForwardedInfo()->origin_->get_id() == td_api::messageForwardOriginUser::ID) {
            return QVariant::fromValue(static_cast<const td_api::messageForwardOriginUser&>(*message->getForwardedInfo()->origin_).sender_user_id_);
        }
        return 0;
    }
    case MessageRoles::ForwardChatRole:
    {
        if (message->getForwardedInfo() != nullptr && message->getForwardedInfo()->origin_->get_id() == td_api::messageForwardOriginChat::ID) {
            return QVariant::fromValue(static_cast<const td_api::messageForwardOriginChat&>(*message->getForwardedInfo()->origin_).sender_chat_id_);
        }
        return 0;
    }
    case MessageRoles::ForwardUsernameRole:
    {
        if (message->getForwardedInfo() != nullptr && message->getForwardedInfo()->origin_->get_id() == td_api::messageForwardOriginHiddenUser::ID) {
            return QString::fromStdString(static_cast<const td_api::messageForwardOriginHiddenUser&>(*message->getForwardedInfo()->origin_).sender_name_);
        }
        return "";
    }
    case MessageRoles::ForwardChannelRole:
    {
        if (message->getForwardedInfo() != nullptr && message->getForwardedInfo()->origin_->get_id() == td_api::messageForwardOriginChannel::ID) {
            return QVariant::fromValue(static_cast<const td_api::messageForwardOriginChannel&>(*message->getForwardedInfo()->origin_).chat_id_);
        }
        return 0;
    }
    case MessageRoles::ForwardTimeRole:
        return message->getFormattedForwardTimestamp();
    case MessageRoles::FileRole:
    {
        switch (message->getContentType()) {
        case td_api::messagePhoto::ID:
        {
            auto photo = message->getContent<Photo>();
            QQmlEngine::setObjectOwnership(photo, QQmlEngine::CppOwnership);
            return QVariant::fromValue(photo);
        }
        case td_api::messageSticker::ID:
        {
            auto sticker = message->getContent<Sticker>();
            QQmlEngine::setObjectOwnership(sticker, QQmlEngine::CppOwnership);
            return QVariant::fromValue(sticker);
        }
        case td_api::messageVideo::ID:
        {
            auto video = message->getContent<Video>();
            QQmlEngine::setObjectOwnership(video, QQmlEngine::CppOwnership);
            return QVariant::fromValue(video);
        }
        case td_api::messageDocument::ID:
        {
            auto document = message->getContent<Document>();
            QQmlEngine::setObjectOwnership(document, QQmlEngine::CppOwnership);
            return QVariant::fromValue(document);
        }
        case td_api::messageAudio::ID:
        {
            auto audio = message->getContent<Audio>();
            QQmlEngine::setObjectOwnership(audio, QQmlEngine::CppOwnership);
            return QVariant::fromValue(audio);
        }
        case td_api::messageAnimation::ID:
        {
            auto animation = message->getContent<Animation>();
            QQmlEngine::setObjectOwnership(animation, QQmlEngine::CppOwnership);
            return QVariant::fromValue(animation);
        }
        case td_api::messageVoiceNote::ID:
        {
            auto voiceNote = message->getContent<VoiceNote>();
            QQmlEngine::setObjectOwnership(voiceNote, QQmlEngine::CppOwnership);
            return QVariant::fromValue(voiceNote);
        }
        case td_api::messageVideoNote::ID:
        {
            auto videoNote = message->getContent<VideoNote>();
            QQmlEngine::setObjectOwnership(videoNote, QQmlEngine::CppOwnership);
            return QVariant::fromValue(videoNote);
        }
        case td_api::messageAnimatedEmoji::ID:
        {
            auto animatedEmoji = message->getContent<AnimatedEmoji>();
            QQmlEngine::setObjectOwnership(animatedEmoji, QQmlEngine::CppOwnership);
            return QVariant::fromValue(animatedEmoji);
        }
        }
        break;
     }
    case MessageRoles::TimeRole:
        return message->getFormattedTimestamp();
    case MessageRoles::SenderRole:
        return message->getSender();
    case MessageRoles::SenderIdRole:
        if (message->getSenderUserId() != 0)
            return QVariant::fromValue(message->getSenderUserId());
        else
            return QVariant::fromValue(message->getSenderChatId());
    case MessageRoles::EditedRole:
        return message->isEdited();
    case MessageRoles::CanBeEditedRole:
        return message->canBeEdited();
    case MessageRoles::CanBeForwardedRole:
        return message->canBeForwarded();
    case MessageRoles::CanBeDeletedRole:
        return message->canBeForwarded();
    case MessageRoles::IsChannelPostRole:
        return message->isChannelPost();
    case MessageRoles::EditedDateRole:
        return message->getEditedDate();
    case MessageRoles::ViewsRole:
        return message->getViews();
    case MessageRoles::ReadRole:
    {
        if (message->received()) {
            return message->getId() <= _lastReadInboxMessageId;
        }  else {
            return message->getId() <= _lastReadOutboxMessageId;
        }
    }
    case MessageRoles::HasWebPageRole:
        return message->hasWebPage();
    case MessageRoles::WebPageRole:
    {
        auto webPage = message->getWebPage();
        QQmlEngine::setObjectOwnership(webPage, QQmlEngine::CppOwnership);
        return QVariant::fromValue(webPage);
    }
    case MessageRoles::PollRole:
    {
        auto poll = message->getContent<Poll>();
        QQmlEngine::setObjectOwnership(poll, QQmlEngine::CppOwnership);
        return QVariant::fromValue(poll);
    }
    case MessageRoles::ContainsUnreadMentionRole:
        return message->containsUnreadMention();
    case MessageRoles::ContainsUnreadReplyRole:
        return message->received() && message->getId() <= _lastReadInboxMessageId && message->replyMessageId() != 0;
    case MessageRoles::CallRole:
    {
        auto call = message->getContent<Call>();
        QQmlEngine::setObjectOwnership(call, QQmlEngine::CppOwnership);
        return QVariant::fromValue(call);
    }
    case MessageRoles::LocationRole:
    {
        auto location = message->getContent<Location>();
        QQmlEngine::setObjectOwnership(location, QQmlEngine::CppOwnership);
        return QVariant::fromValue(location);
    }
    case MessageRoles::ContactRole:
    {
        auto contact = message->getContent<Contact>();
        QQmlEngine::setObjectOwnership(contact, QQmlEngine::CppOwnership);
        return QVariant::fromValue(contact);
    }
    }

    return QVariant();
}

QHash<int, QByteArray> Chat::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[IdRole]   = "messageId";
    roles[MessageRole] = "messageText";
    roles[MessageTypeRole] = "messageType";
    roles[ReceivedRole] = "received";
    roles[ReplyMessageIdRole] = "replyMessageId";
    roles[DisplayAvatarRole] = "displayAvatar";
    roles[IsServiceRole] = "isService";
    roles[IsForwardedRole] = "isForwarded";
    roles[ForwardUserRole] = "forwardUserId";
    roles[ForwardChatRole] = "forwardChatId";
    roles[ForwardUsernameRole] = "forwardUsername";
    roles[ForwardChannelRole] = "forwardChannelId";
    roles[ForwardTimeRole] = "forwardTimestamp";
    roles[FileRole] = "file";
    roles[TimeRole] = "timestamp";
    roles[SenderRole] = "sender";
    roles[SenderIdRole] = "senderId";
    roles[EditedRole] = "edited";
    roles[CanBeEditedRole] = "canBeEdited";
    roles[CanBeForwardedRole] = "canBeForwarded";
    roles[CanBeDeletedRole] = "canBeDeleted";
    roles[IsChannelPostRole] = "isChannel";
    roles[EditedDateRole] = "editedDate";
    roles[ViewsRole] = "views";
    roles[ReadRole] = "isRead";
    roles[HasWebPageRole] = "hasWebPage";
    roles[WebPageRole] = "webPage";
    roles[PollRole] = "poll";
    roles[ContainsUnreadMentionRole] = "containsUnreadMention";
    roles[ContainsUnreadReplyRole] = "containsUnreadReply";
    roles[CallRole] = "call";
    roles[LocationRole] = "location";
    roles[ContactRole] = "contact";
    return roles;
}

td_api::chat *Chat::getChat()
{
    return _chat;
}

int Chat::getMessageIndex(int64_t messageId)
{
    if(rowCount() <= 0) return -1;

    auto it = std::find(_message_ids.begin(), _message_ids.end(), messageId);

    if (it != _message_ids.end()) {
        return std::distance(_message_ids.begin(), it);
    }

    return -1;
}

int64_t Chat::getLatestMessageId() const
{
    auto max = std::max_element(_message_ids.begin(), _message_ids.end());

    if (_message_ids.end() != max) {
        return *max;
    }

    return 0;
}

td_api::message *Chat::getLastMessage()
{
    return _chat->last_message_.get();
}

shared_ptr<Message> Chat::lastMessage()
{
    return _lastMessage;
}

int64_t Chat::getLastMessageId() const
{
    if (_chat->last_message_ == nullptr) return 0;

    return _chat->last_message_->id_;
}

void Chat::setLastMessage(td_api::object_ptr<td_api::message> lastMessage)
{
    _chat->last_message_ = move(lastMessage);

    shared_ptr<Message> newLastMessage = make_shared<Message>();
    newLastMessage->setTelegramManager(_manager);
    newLastMessage->setUsers(_users);
    newLastMessage->setFiles(_files);
    newLastMessage->setMessage(_chat->last_message_.get());
    newLastMessage->setChatId(getId());
    _lastMessage = newLastMessage;

    emit lastMessageIdChanged(_chat->last_message_->id_);
}

void Chat::newMessage(td_api::object_ptr<td_api::message> message, bool addToList)
{
    if (false == _messages.contains(message->id_)) {
        auto newMessage = new Message();
        newMessage->setTelegramManager(_manager);
        newMessage->setUsers(_users);
        newMessage->setFiles(_files);
        newMessage->setMessage(message.release());
        newMessage->setChatId(this->getId());
        connect(newMessage, SIGNAL(contentChanged(int64_t)), this, SLOT(onMessageContentChanged(int64_t)));
        connect(newMessage, SIGNAL(messageIdChanged(int64_t,int64_t)), this, SLOT(onMessageIdChanged(int64_t,int64_t)));
        _messages[newMessage->getId()] = newMessage;

        if (addToList) {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
            _message_ids.append(newMessage->getId());
            emit lastMessageIdChanged(newMessage->getId());
            endInsertRows();
        }

//        if (newMessage->getId() == _chat->pinned_message_id_) emit pinnedMessageIdChanged();
    } else if (!_message_ids.contains(message->id_) && addToList) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        _message_ids.append(message->id_);
        emit lastMessageIdChanged(message->id_);
        endInsertRows();
    }
}

std::vector<std::int32_t> Chat::splitToIntVector(QString string, QString separator)
{
    QStringList optionList = string.split(separator);
    std::vector<std::int32_t> vector;
    for (auto option: optionList) {
        vector.push_back(option.toInt());
    }
    return vector;
}

std::vector<string> Chat::splitToVector(QString string, QString separator)
{
    QStringList optionList = string.split(separator);
    std::vector<std::string> vector;
    for (auto option: optionList) {
        vector.push_back(option.toStdString());
    }
    return vector;
}

void Chat::sendMessage(QString message, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageText>();
    messageContent->text_ = td_api::make_object<td_api::formattedText>();
    messageContent->text_->text_ = message.toStdString();
    messageContent->disable_web_page_preview_ = false;
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::getMoreChatHistory()
{
    auto min = std::min_element(_message_ids.begin(), _message_ids.end());

    if (_message_ids.end() != min) {

        this->getChatHistory(*min, 30);
    }
}

void Chat::getMoreChatMessages()
{
    auto max = std::max_element(_message_ids.begin(), _message_ids.end());

    if (_message_ids.end() != max) {
        this->getChatHistory(*max, 30, -30);
    }
}

int64_t Chat::getAuthorByIndex(int32_t index)
{
    if (rowCount() <= 0 || index < 0 || index >= rowCount()) return 0;

    return _messages[_message_ids[index]]->getSenderUserId();
}

void Chat::setMessageAsRead(int64_t messageId, int64_t threadId)
{
    _manager->sendQuery(new td_api::viewMessages(this->getId(), threadId, {messageId}, false));
}

void Chat::sendPhoto(QString path, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessagePhoto>();
    auto inputFile = td_api::make_object<td_api::inputFileLocal>();
    inputFile->path_ = path.toStdString();
    messageContent->photo_ = std::move(inputFile);
    messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendPhotos(QStringList paths, int64_t replyToMessageId)
{
    if (paths.empty()) return;

    auto sendMessage = new td_api::sendMessageAlbum();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    std::vector<td_api::object_ptr<td_api::InputMessageContent>> contents;
    for (QString path : paths) {
        auto messageContent = td_api::make_object<td_api::inputMessagePhoto>();
        auto inputFile = td_api::make_object<td_api::inputFileLocal>();
        inputFile->path_ = path.toStdString();
        messageContent->photo_ = std::move(inputFile);
        messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
        contents.emplace_back(std::move(messageContent));
    }
    sendMessage->input_message_contents_ = std::move(contents);

    _manager->sendQuery(sendMessage);
}

void Chat::sendFile(QString path, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageDocument>();
    auto inputFile = td_api::make_object<td_api::inputFileLocal>();
    inputFile->path_ = path.toStdString();
    messageContent->document_ = std::move(inputFile);
    messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendMusic(QString path, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageAudio>();
    auto inputFile = td_api::make_object<td_api::inputFileLocal>();
    inputFile->path_ = path.toStdString();
    messageContent->audio_ = std::move(inputFile);
    messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendMusics(QStringList paths, int64_t replyToMessageId)
{
    if (paths.empty()) return;

    auto sendMessage = new td_api::sendMessageAlbum();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    std::vector<td_api::object_ptr<td_api::InputMessageContent>> contents;
    for (QString path : paths) {
        auto messageContent = td_api::make_object<td_api::inputMessageAudio>();
        auto inputFile = td_api::make_object<td_api::inputFileLocal>();
        inputFile->path_ = path.toStdString();
        messageContent->audio_ = std::move(inputFile);
        messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
        contents.emplace_back(std::move(messageContent));
    }
    sendMessage->input_message_contents_ = std::move(contents);

    _manager->sendQuery(sendMessage);
}

void Chat::sendVideo(QString path, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageVideo>();
    auto inputFile = td_api::make_object<td_api::inputFileLocal>();
    inputFile->path_ = path.toStdString();
    messageContent->video_ = std::move(inputFile);
    messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendVideos(QStringList paths, int64_t replyToMessageId)
{
    if (paths.empty()) return;

    auto sendMessage = new td_api::sendMessageAlbum();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    std::vector<td_api::object_ptr<td_api::InputMessageContent>> contents;
    for (QString path : paths) {
        auto messageContent = td_api::make_object<td_api::inputMessageVideo>();
        auto inputFile = td_api::make_object<td_api::inputFileLocal>();
        inputFile->path_ = path.toStdString();
        messageContent->video_ = std::move(inputFile);
        messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
        contents.emplace_back(std::move(messageContent));
    }
    sendMessage->input_message_contents_ = std::move(contents);

    _manager->sendQuery(sendMessage);
}

void Chat::sendPoll(QString question, QStringList options, bool anonymous, bool multipleAnswers, bool quizMode, int validAnswer = -1)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    auto poll = td_api::make_object<td_api::inputMessagePoll>();
    poll->question_ = question.toStdString();

    std::vector<std::string> vectorOptions;
    for (auto option : options) vectorOptions.push_back(option.toStdString());
    poll->options_ = vectorOptions;
    poll->is_anonymous_ = anonymous;
    if (quizMode) {
        td_api::object_ptr<td_api::formattedText> explanation = td_api::make_object<td_api::formattedText>();
        poll->type_ = td_api::make_object<td_api::pollTypeQuiz>(validAnswer, move(explanation));
    } else {
        poll->type_ = td_api::make_object<td_api::pollTypeRegular>(multipleAnswers);
    }

    sendMessage->input_message_content_ = std::move(poll);
    _manager->sendQuery(sendMessage);
}

void Chat::sendVoiceNote(QString path, QString waveform, int64_t duration, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageVoiceNote>();
    auto inputFile = td_api::make_object<td_api::inputFileLocal>();
    inputFile->path_ = path.toStdString();
    messageContent->voice_note_ = std::move(inputFile);
    messageContent->waveform_ = waveform.toStdString();
    messageContent->duration_ = static_cast<int32_t>(duration/1000);
    messageContent->caption_ =  td_api::make_object<td_api::formattedText>();
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendSticker(int32_t fileId, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageSticker>();
    auto inputFile = td_api::make_object<td_api::inputFileId>();
    inputFile->id_ = fileId;
    messageContent->sticker_ = std::move(inputFile);
    messageContent->width_ = 512;
    messageContent->height_ = 512;
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendLocation(float latitude, float longitude, float horizontalAccuracy, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageLocation>();
    messageContent->location_ = td_api::make_object<td_api::location>(latitude, longitude, horizontalAccuracy);

    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendAnimation(int32_t fileId, int32_t width, int32_t height, int64_t replyToMessageId)
{
    auto sendMessage = new td_api::sendMessage();
    sendMessage->chat_id_ = _chat->id_;

    if (replyToMessageId != 0) {
        sendMessage->reply_to_message_id_ = replyToMessageId;
    }

    auto messageContent = td_api::make_object<td_api::inputMessageAnimation>();
    auto inputFile = td_api::make_object<td_api::inputFileId>();
    inputFile->id_ = fileId;
    messageContent->animation_ = std::move(inputFile);
    messageContent->width_ = width;
    messageContent->height_ = height;
    sendMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(sendMessage);
}

void Chat::sendForwardedMessages(QStringList forwardedMessages, int64_t forwardedFrom)
{
    forwardedMessages.sort();

    for (QString message : forwardedMessages) {
        auto sendMessage = new td_api::sendMessage();
        sendMessage->chat_id_ = _chat->id_;

        auto messageContent = td_api::make_object<td_api::inputMessageForwarded>();
        messageContent->from_chat_id_ = forwardedFrom;
        messageContent->message_id_ = message.toLongLong();
        sendMessage->input_message_content_ = std::move(messageContent);

        _manager->sendQuery(sendMessage);
    }
}

void Chat::open(QString path)
{
    QDesktopServices::openUrl(QUrl("file://" + path));
}

void Chat::deleteMessage(int64_t messageId)
{
    _manager->sendQuery(new td_api::deleteMessages(_chat->id_, std::vector<int64_t>(1, messageId), true));
}

void Chat::editMessageText(int64_t messageId, QString messageText)
{
    auto editMessage = new td_api::editMessageText();
    editMessage->chat_id_ = _chat->id_;
    editMessage->message_id_ = messageId;

    auto messageContent = td_api::make_object<td_api::inputMessageText>();
    messageContent->text_ = td_api::make_object<td_api::formattedText>();
    messageContent->text_->text_ = messageText.toStdString();
    messageContent->disable_web_page_preview_ = false;
    editMessage->input_message_content_ = std::move(messageContent);

    _manager->sendQuery(editMessage);
}

void Chat::chose(int64_t messageId, QString indexes)
{
    auto optionIds = splitToIntVector(indexes, ",");
    auto request = new td_api::setPollAnswer();
    request->chat_id_ = _chat->id_;
    request->message_id_ = messageId;
    request->option_ids_ = optionIds;
    _manager->sendQuery(request);
}

void Chat::openSecretChat()
{
    _manager->sendQuery(new td_api::createNewSecretChat(getIdFromType()));
}

void Chat::closeSecretChat()
{
    _manager->sendQuery(new td_api::closeSecretChat(static_cast<td_api::chatTypeSecret*>(_chat->type_.get())->secret_chat_id_));
}

void Chat::clearHistory(bool deleteChat)
{
    _manager->sendQuery(new td_api::deleteChatHistory(getId(), deleteChat, true));

}

void Chat::saveToGallery(QString filePath)
{
    QFile::copy(filePath, QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + filePath.split('/').last());
}

void Chat::pinMessage(int64_t messageId, bool notify, bool onlyForSelf)
{
    _manager->sendQuery(new td_api::pinChatMessage(getId(), messageId, !notify, onlyForSelf));
}

void Chat::unpinMessage(int64_t messageId)
{
    _manager->sendQuery(new td_api::unpinChatMessage(getId(), messageId));
}

void Chat::clearCachedHistory()
{
    _message_ids.clear();
}

void Chat::sendAction(ChatAction action)
{
    td_api::object_ptr<td_api::ChatAction> tdAction;
    switch (action) {
    case ChatAction::None:
        tdAction = td_api::make_object<td_api::chatActionCancel>();
        break;
    case ChatAction::Typing:
        tdAction = td_api::make_object<td_api::chatActionTyping>();
        break;
    case ChatAction::RecordingVideo:
        tdAction = td_api::make_object<td_api::chatActionRecordingVideo>();
        break;
    case ChatAction::UploadingVideo:
        tdAction = td_api::make_object<td_api::chatActionUploadingVideo>();
        break;
    case ChatAction::RecordingVoiceNote:
        tdAction = td_api::make_object<td_api::chatActionRecordingVoiceNote>();
        break;
    case ChatAction::UploadingVoiceNote:
        tdAction = td_api::make_object<td_api::chatActionUploadingVoiceNote>();
        break;
    case ChatAction::UploadingPhoto:
        tdAction = td_api::make_object<td_api::chatActionUploadingPhoto>();
        break;
    case ChatAction::UploadingDocument:
        tdAction = td_api::make_object<td_api::chatActionUploadingDocument>();
        break;
    case ChatAction::ChoosingSticker:
        tdAction = td_api::make_object<td_api::chatActionChoosingSticker>();
        break;
    case ChatAction::ChoosingLocation:
        tdAction = td_api::make_object<td_api::chatActionChoosingLocation>();
        break;
    case ChatAction::ChoosingContact:
        tdAction = td_api::make_object<td_api::chatActionChoosingContact>();
        break;
    case ChatAction::StartPlayingGame:
        tdAction = td_api::make_object<td_api::chatActionStartPlayingGame>();
        break;
    case ChatAction::RecordingVideoNote:
        tdAction = td_api::make_object<td_api::chatActionRecordingVideoNote>();
        break;
    case ChatAction::UploadingVideoNote:
        tdAction = td_api::make_object<td_api::chatActionUploadingVideoNote>();
        break;
    case ChatAction::WatchingAnimations:
        tdAction = td_api::make_object<td_api::chatActionWatchingAnimations>();
        break;
    }
    _manager->sendQuery(new td_api::sendChatAction(getId(), 0, move(tdAction)));
}

void Chat::setTtl(int32_t ttl)
{
    _manager->sendQuery(new td_api::setChatMessageTtlSetting(getId(), ttl));
}

void Chat::getChatHistory(int64_t from_message, int limit, int offset, bool localOnly)
{
    auto getChatHistoryQuery = new td_api::getChatHistory();
    getChatHistoryQuery->chat_id_ = _chat->id_;
    getChatHistoryQuery->from_message_id_ = from_message;
    getChatHistoryQuery->offset_ = offset;
    getChatHistoryQuery->limit_ = limit;
    getChatHistoryQuery->only_local_ = localOnly;

    _manager->sendQueryWithRespone(getId(), td_api::getChatHistory::ID, 0, getChatHistoryQuery);
}

void Chat::scrollToMessage(int64_t messageId)
{
    beginResetModel();
    _message_ids.clear();
    getChatHistory(messageId, 40, -20);
    endResetModel();
}

void Chat::fetchMessage(int64_t messageId) const
{
    if (messageId == 0) return;
    _manager->sendQueryWithRespone(getId(), td_api::getMessage::ID, 0, new td_api::getMessage(getId(), messageId));
}

QVariant Chat::getMessage(int64_t messageId)
{
    if (!_messages.contains(messageId)) return QVariant();
    auto message = _messages[messageId];
    QQmlEngine::setObjectOwnership(message, QQmlEngine::CppOwnership);
    return QVariant::fromValue(message);
}

bool Chat::hasPhoto()
{
    return _chat->photo_ != nullptr;
}

File* Chat::getSmallPhoto()
{
    auto file = _files->getFile(_smallPhotoId);
    if (file == nullptr) return nullptr;
    return file.get();
}

File* Chat::getBigPhoto()
{
    auto file = _files->getFile(_bigPhotoId);
    if (file == nullptr) return nullptr;
    return file.get();
}

void Chat::updateChatReadInbox(td_api::updateChatReadInbox *updateChatReadInbox)
{
    if (updateChatReadInbox->chat_id_ == this->getId()) {
        setUnreadCount(updateChatReadInbox->unread_count_);
        setLastReadInboxMessageId(updateChatReadInbox->last_read_inbox_message_id_);
        emit dataChanged(createIndex(0, 0), createIndex(rowCount(), 0), {ReadRole});
    }
}

void Chat::updateChatReadOutbox(td_api::updateChatReadOutbox *updateChatReadOutbox)
{
    if (updateChatReadOutbox->chat_id_ == this->getId()) {
        setLastReadOutboxMessageId(updateChatReadOutbox->last_read_outbox_message_id_);
        emit dataChanged(createIndex(0, 0), createIndex(rowCount(), 0), {ReadRole});
    }
}

void Chat::updateNewMessage(td_api::updateNewMessage *updateNewMessage)
{
    if (updateNewMessage->message_ != nullptr && updateNewMessage->message_->chat_id_ == this->getId()) {
        int64_t lastMessageId = getLatestMessageId();

        bool hasAllMessages = (lastMessageId == getLastMessageId());

        this->newMessage(move(updateNewMessage->message_), hasAllMessages);
    }
}

void Chat::updateChatTitle(td_api::updateChatTitle *updateChatTitle)
{
    if (updateChatTitle->chat_id_ == getId()) {
        _chat->title_ = updateChatTitle->title_;
        emit titleChanged();
    }
}

void Chat::updateDeleteMessages(td_api::updateDeleteMessages *updateDeleteMessages)
{
    if (updateDeleteMessages->chat_id_ == this->getId()) {
        for (auto messageId : updateDeleteMessages->message_ids_) {
            if (updateDeleteMessages->from_cache_) {
                delete _messages.take(messageId);
            } else {
                auto index = getMessageIndex(messageId);
                if (-1 != index) {
                    beginRemoveRows(QModelIndex(), index, index);
                    delete _messages.take(messageId);
                    _message_ids.remove(index);
                    endRemoveRows();
                }
            }
        }
    }
}
void Chat::onMessageContentChanged(int64_t messageId)
{
    auto index = getMessageIndex(messageId);

    if (0 != index) {
        emit dataChanged(createIndex(index, 0), createIndex(index, 0), {FileRole, MessageRole, HasWebPageRole, WebPageRole, PollRole});
    }
}

void Chat::onMessageIdChanged(int64_t oldMessageId, int64_t newMessageId)
{
    auto index = getMessageIndex(oldMessageId);
    if (index == -1) return;
    _message_ids[index] = newMessageId;
    _messages[newMessageId] = _messages.take(oldMessageId);
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {IdRole});
}

void Chat::updateBasicGroupFullInfo(td_api::updateBasicGroupFullInfo *updateBasicGroupFullInfo)
{
    if (_chat->type_->get_id() == td_api::chatTypeBasicGroup::ID) {
        auto basicGroupId = static_cast<td_api::chatTypeBasicGroup*>(_chat->type_.get())->basic_group_id_;

        if (updateBasicGroupFullInfo->basic_group_id_ == basicGroupId) {
            _basicGroupFullInfo->setBasicGroupFullInfo(std::move(updateBasicGroupFullInfo->basic_group_full_info_));
            emit basicGroupFullInfoChanged();
        }
    }
}

void Chat::updateSupergroupFullInfo(td_api::updateSupergroupFullInfo *updateSupergroupFullInfo)
{
    if (_chat->type_->get_id() == td_api::chatTypeSupergroup::ID) {
        auto supergroupId = static_cast<td_api::chatTypeSupergroup*>(_chat->type_.get())->supergroup_id_;

        if (updateSupergroupFullInfo->supergroup_id_ == supergroupId) {
            _supergroupFullInfo->setSupergroupFullInfo(std::move(updateSupergroupFullInfo->supergroup_full_info_));
            emit supergroupFullInfoChanged();
        }
    }
}

void Chat::updateChatNotificationSettings(td_api::updateChatNotificationSettings *updateChatNotificationSettings)
{
    if (updateChatNotificationSettings->chat_id_ == getId()) {
        _notificationSettings = std::move(updateChatNotificationSettings->notification_settings_);
        emit chatNotificationSettingsChanged();
    }
}

void Chat::scopeNotificationSettingsChanged(td_api::scopeNotificationSettings *scopeNotificationSettings)
{
    _scopeNotificationSettings = scopeNotificationSettings;
}

void Chat::onGotMessage(td_api::message *message)
{
    if (message->chat_id_ == getId()) {
        int64_t messageId = message->id_;
        newMessage(td_api::object_ptr<td_api::message>(message), false);
        emit gotMessage(messageId);
        if (_getMessageCache.contains(messageId)) {
            _getMessageCache.removeAll(messageId);
            emit dataChanged(createIndex(getMessageIndex(messageId), 0), createIndex(getMessageIndex(messageId), 0), {});
        }
    }
}

void Chat::updateChatPermissions(td_api::updateChatPermissions *updateChatPermissions)
{
    if (updateChatPermissions->chat_id_ != getId()) return;

    _chat->permissions_ = move(updateChatPermissions->permissions_);
    emit permissionsChanged();
}

void Chat::updateChatMessageTtlSetting(td_api::updateChatMessageTtlSetting *updateChatMessageTtlSetting)
{
    if (updateChatMessageTtlSetting->chat_id_ != getId()) return;
    _chat->message_ttl_setting_ = updateChatMessageTtlSetting->message_ttl_setting_;
    emit ttlChanged(_chat->message_ttl_setting_);
}

void Chat::updateUserChatAction(td_api::updateUserChatAction *updateUserChatAction)
{
    if (updateUserChatAction->chat_id_ != getId()) return;

    if (updateUserChatAction->action_->get_id() == td_api::chatActionCancel::ID && _chatActions.contains(updateUserChatAction->user_id_)) {
        _chatActions.remove(updateUserChatAction->user_id_);
    }

    ChatAction action = None;
    downcast_call(
        *updateUserChatAction->action_, overloaded(
            [&](td_api::chatActionTyping&) { action = Typing; },
            [&](td_api::chatActionRecordingVideo&) { action = RecordingVideo; },
            [&](td_api::chatActionUploadingVideo&) { action = UploadingVideo; },
            [&](td_api::chatActionRecordingVoiceNote&) { action = RecordingVoiceNote; },
            [&](td_api::chatActionUploadingVoiceNote&) { action = UploadingVoiceNote; },
            [&](td_api::chatActionUploadingPhoto&) { action = UploadingPhoto; },
            [&](td_api::chatActionUploadingDocument&) { action = UploadingDocument; },
            [&](td_api::chatActionChoosingSticker&) { action = ChoosingSticker; },
            [&](td_api::chatActionChoosingLocation&) { action = ChoosingLocation; },
            [&](td_api::chatActionChoosingContact&) { action = ChoosingContact; },
            [&](td_api::chatActionStartPlayingGame&) { action = StartPlayingGame; },
            [&](td_api::chatActionRecordingVideoNote&) { action = RecordingVideoNote; },
            [&](td_api::chatActionUploadingVideoNote&) { action = UploadingVideoNote; },
            [&](td_api::chatActionWatchingAnimations&) { action = WatchingAnimations; },
            [&](td_api::chatActionCancel&) { action = None; }
        )
    );

    if (action != ChatAction::None) _chatActions[updateUserChatAction->user_id_] = action;
    emit actionTextChanged();
}

void Chat::onGotChatHistory(int64_t chatId, td_api::messages *messages)
{
    if (chatId != getId()) return;

    if (messages->total_count_ == 0 || messages->messages_.size() <= 0 || !messages->messages_[0]) return;

    int addedMessages = 0;
    for (int i = 0; i < messages->total_count_; i++) {
        if (messages->messages_[i]) {
            int64_t id = messages->messages_[i]->id_;
            if (false == _messages.contains(id)) {
                addedMessages++;
            } else if (!_message_ids.contains(id)) {
                addedMessages++;
            }
        }
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount()+addedMessages-1);
    for (int i = 0; i < messages->total_count_; i++) {
        td_api::message* message = messages->messages_[i].release();
        if (message != nullptr) {
            if (false == _messages.contains(message->id_)) {
                auto newMessage = new Message();
                newMessage->setTelegramManager(_manager);
                newMessage->setUsers(_users);
                newMessage->setFiles(_files);
                newMessage->setMessage(message);
                newMessage->setChatId(this->getId());
                connect(newMessage, SIGNAL(contentChanged(int64_t)), this, SLOT(onMessageContentChanged(int64_t)));
                connect(newMessage, SIGNAL(messageIdChanged(int64_t,int64_t)), this, SLOT(onMessageIdChanged(int64_t,int64_t)));
                _messages[newMessage->getId()] = newMessage;

                _message_ids.append(newMessage->getId());
            } else {
                if (!_message_ids.contains(message->id_)) {
                    _message_ids.append(message->id_);
                }
                delete message;
            }
        }
    }
    endInsertRows();

    emit gotChatHistory();
}
