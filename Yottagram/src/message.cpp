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

#include "message.h"

#include <contents/animatedemoji.h>
#include <libsailfishsilica/silicatheme.h>
#include <set>

Message::Message(QObject *parent) : QObject(parent), _message(nullptr), _properties(nullptr), _text(nullptr), _messageContent(nullptr), _linkPreview(nullptr)
{
    _reformatTimer.setInterval(500);
    _reformatTimer.setSingleShot(true);
    connect(&_reformatTimer, &QTimer::timeout, this, &Message::onReformatMessage);

    _reactionsUpdateTimer.setInterval(500);
    _reactionsUpdateTimer.setSingleShot(true);
    connect(&_reactionsUpdateTimer, &QTimer::timeout, [this](){
        emit reactionsChanged(getId());
    });
}

Message::~Message()
{
}

td_api::message *Message::message() const
{
    return _message;
}

void Message::setMessage(td_api::message *message, bool handleContent)
{
    _message = message;

    if (handleContent) {
        handleMessageContent(std::move(message->content_));
    }
}

void Message::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
    connect(_manager.get(), &TelegramManager::updateMessageSendSucceeded, this, &Message::updateMessageSendSucceeded);
    connect(_manager.get(), &TelegramManager::updateMessageContent, this, &Message::updateMessageContent);
}

void Message::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void Message::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void Message::setCustomEmojis(shared_ptr<CustomEmojis> customEmojis)
{
    _customEmojis = customEmojis;
}

void Message::setTextEntityProcessor(TextEntityProcessor *processor)
{
    _entityProcessor = processor;
}

int64_t Message::getId()
{
    return _message->id_;
}

QString Message::getText(bool formatted)
{
    switch (_contentTypeId) {
    case td_api::messageText::ID:
        if (formatted)
            return _formattedMessage;
        else
            return QString::fromStdString(_text->text_->text_);
    case td_api::messageChatDeleteMember::ID:
        return tr("%1 left").arg(_users->getUser(getDeleteMemberId())->getName());
    case td_api::messageChatAddMembers::ID:
    {
        auto newUsers = getAddMembersIds();
        for(auto userId: newUsers) {
            return tr("%1 joined").arg(_users->getUser(userId)->getName());
        }
        return "";
    }
    case td_api::messageChatJoinByLink::ID:
        return tr("%1 joined").arg(_users->getUser(getSenderUserId())->getName());
    case td_api::messagePhoto::ID:
    {
        if (static_cast<Photo*>(_messageContent)->getCaption() == nullptr) return "";

        return formatted ? _formattedMessage : QString::fromStdString(static_cast<Photo*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messageVideo::ID:
    {
        if (static_cast<Video*>(_messageContent)->getCaption() == nullptr) return "";
        return formatted ? _formattedMessage : QString::fromStdString(static_cast<Video*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messageDocument::ID:
    {
        if (static_cast<Document*>(_messageContent)->getCaption() == nullptr) return "";
        return formatted ? _formattedMessage : QString::fromStdString(static_cast<Document*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messageAudio::ID:
    {
        if (static_cast<Audio*>(_messageContent)->getCaption() == nullptr) return "";
        return formatted ? _formattedMessage : QString::fromStdString(static_cast<Audio*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messageAnimation::ID:
    {
        if (static_cast<Animation*>(_messageContent)->getCaption() == nullptr) return "";
        return formatted ? _formattedMessage : QString::fromStdString(static_cast<Animation*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messageVoiceNote::ID:
    {
        if (static_cast<VoiceNote*>(_messageContent)->getCaption() == nullptr) return "";
        return formatted ? _formattedMessage : QString::fromStdString(static_cast<VoiceNote*>(_messageContent)->getCaption()->text_);
    }
    case td_api::messagePoll::ID:
        return static_cast<Poll*>(_messageContent)->getQuestion();
    case td_api::messageChatSetMessageAutoDeleteTime::ID:
        return tr("Self-destruct timer set to %n second(s)", "", static_cast<td_api::messageChatSetMessageAutoDeleteTime*>(_message->content_.get())->message_auto_delete_time_);
    case td_api::messagePinMessage::ID:
        if (_message->sender_id_->get_id() == td_api::messageSenderUser::ID) {
            return tr("%1 pinned message").arg(_users->getUser(getSenderUserId())->getName());
        } else if (_message->sender_id_->get_id() == td_api::messageSenderChat::ID) {
            return tr("Message was pinned");
        } else {
            return "";
        }
    case td_api::messageCall::ID:
        return _message->is_outgoing_ ? tr("Outgoing call") : tr("Incoming call");
    case td_api::messageBasicGroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageSupergroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageLocation::ID:
    case td_api::messageContact::ID:
    case td_api::messageSticker::ID:
    case td_api::messageVideoNote::ID:
        return "";
    case td_api::messageAnimatedEmoji::ID:
    {
        AnimatedEmoji* emoji = static_cast<AnimatedEmoji*>(_messageContent);
        if (emoji->getSticker() == nullptr) {
            return "";
        }

        QString localPath = emoji->getSticker()->localPath();
        if (localPath == "") {
            return emoji->getEmoji();
        }

        auto theme = Silica::Theme::instance();
        return "<img width=\"" + QString::number(theme->fontSizeSmall()) + "\" height=\"" + QString::number(theme->fontSizeSmall()) + "\" align=\"middle\" src=\"" + localPath + "\"/>";
    }
    case td_api::messageContactRegistered::ID:
        return tr("%1 joined telegram").arg(_users->getUser(getSenderUserId())->getName());
    case td_api::messageForumTopicCreated::ID:
        return tr("Topic %1 created").arg(QString::fromStdString(static_cast<td_api::messageForumTopicCreated*>(_message->content_.get())->name_));
    case td_api::messageForumTopicEdited::ID:
        return tr("Topic %1 edited").arg(QString::fromStdString(static_cast<td_api::messageForumTopicEdited*>(_message->content_.get())->name_));
    case td_api::messageForumTopicIsClosedToggled::ID:
        if (static_cast<td_api::messageForumTopicIsClosedToggled*>(_message->content_.get())->is_closed_) {
            return tr("Topic closed");
        } else {
            return tr("Topic opened");
        }
    case td_api::messageForumTopicIsHiddenToggled::ID:
        if (static_cast<td_api::messageForumTopicIsHiddenToggled*>(_message->content_.get())->is_hidden_) {
            return tr("Topic hidden");
        } else {
            return tr("Topic unhidden");
        }
    default:
        return tr("Message unsupported");
    }
}

QString Message::getEditText()
{
    QString formattedText;

    switch (_contentTypeId) {
    case td_api::messagePhoto::ID:
        if (static_cast<Photo*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<Photo*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageVideo::ID:
        if (static_cast<Video*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<Video*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageDocument::ID:
        if (static_cast<Document*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<Document*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageAudio::ID:
        if (static_cast<Audio*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<Audio*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageAnimation::ID:
        if (static_cast<Animation*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<Animation*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageVoiceNote::ID:
        if (static_cast<VoiceNote*>(_messageContent)->getCaption() != nullptr) {
            formattedText = _entityProcessor->formatTextAsHTML(static_cast<VoiceNote*>(_messageContent)->getCaption(), true);
        }
        break;
    case td_api::messageText::ID:
        formattedText = _entityProcessor->formatTextAsHTML(_text->text_.get(), true);
        break;
    }

    return formattedText;
}

QString Message::getType() const
{
    switch (_contentTypeId) {
    case td_api::messageChatDeleteMember::ID:
        return "messageChatDeleteMember";
    case td_api::messageChatAddMembers::ID:
        return "messageChatAddMembers";
    case td_api::messageChatJoinByLink::ID:
        return "messageChatJoinByLink";
    case td_api::messageText::ID:
        return "text";
    case td_api::messagePhoto::ID:
        return "photo";
    case td_api::messageAnimation::ID:
        return "animation";
    case td_api::messageSticker::ID:
        return "sticker";
    case td_api::messageVideo::ID:
        return "video";
    case td_api::messageVideoNote::ID:
        return "videoNote";
    case td_api::messageAudio::ID:
        return "audio";
    case td_api::messageVoiceNote::ID:
        return "voiceNote";
    case td_api::messageDocument::ID:
        return "document";
    case td_api::messagePoll::ID:
        return "poll";
    case td_api::messageChatSetMessageAutoDeleteTime::ID:
        return "chatSetAutoDeleteTime";
    case td_api::messageCustomServiceAction::ID:
        return "messageCustomServiceAction";
    case td_api::messagePinMessage::ID:
        return "pinMessage";
    case td_api::messageCall::ID:
        return "call";
    case td_api::messageLocation::ID:
        return "location";
    case td_api::messageContact::ID:
        return "contact";
    case td_api::messageContactRegistered::ID:
        return "contactRegistered";
    case td_api::messageAnimatedEmoji::ID:
        return "animatedEmoji";
    case td_api::messageUnsupported::ID:
        return "messageUnsupported";
    case td_api::messageBasicGroupChatCreate::ID:
        return "messageBasicGroupChatCreate";
    case td_api::messageSupergroupChatCreate::ID:
        return "messageSupergroupChatCreate";
    default:
        qDebug() << _contentTypeId;
        return "Message unsupported";
    }
}

QString Message::getTypeText()
{
    switch (_contentTypeId) {
    case td_api::messageText::ID:
        return QString::fromStdString(_text->text_->text_);
    case td_api::messageChatDeleteMember::ID:
        return tr("%1 left").arg(_users->getUser(getDeleteMemberId())->getName());
    case td_api::messageChatAddMembers::ID:
    {
        auto newUsers = getAddMembersIds();
        for(auto userId: newUsers) {
            return tr("%1 joined").arg(_users->getUser(userId)->getName());
        }
        return tr("Users joined");
    }
    case td_api::messageChatJoinByLink::ID:
        return tr("%1 joined").arg(_users->getUser(getSenderUserId())->getName());
    case td_api::messagePhoto::ID:
    {
        return tr("Photo");
    }
    case td_api::messageVideo::ID:
    {
        return tr("Video");
    }
    case td_api::messageDocument::ID:
    {
        return tr("Document");
    }
    case td_api::messageAudio::ID:
    {
        return tr("Audio");
    }
    case td_api::messageAnimation::ID:
    {
        return tr("Animation");
    }
    case td_api::messageVoiceNote::ID:
    {
        return tr("Voice note");
    }
    case td_api::messagePoll::ID:
        return tr("Poll");
    case td_api::messageChatSetMessageAutoDeleteTime::ID:
        return tr("Self-destruct timer set to %n second(s)", "", static_cast<td_api::messageChatSetMessageAutoDeleteTime*>(_message->content_.get())->message_auto_delete_time_);
    case td_api::messagePinMessage::ID:
        if (_message->sender_id_->get_id() == td_api::messageSenderUser::ID) {
            return tr("%1 pinned message").arg(_users->getUser(getSenderUserId())->getName());
        } else if (_message->sender_id_->get_id() == td_api::messageSenderChat::ID) {
            return tr("Message was pinned");
        } else {
            return "";
        }
    case td_api::messageCall::ID:
        return _message->is_outgoing_ ? tr("Outgoing call") : tr("Incoming call");
    case td_api::messageBasicGroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageSupergroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageLocation::ID:
        return tr("Location");
    case td_api::messageContact::ID:
        return tr("Contact");
    case td_api::messageSticker::ID:
        return tr("Sticker");
    case td_api::messageVideoNote::ID:
        return tr("Video note");
    case td_api::messageAnimatedEmoji::ID:
        return static_cast<AnimatedEmoji*>(_messageContent)->getEmoji();
    case td_api::messageContactRegistered::ID:
        return tr("%1 joined telegram").arg(_users->getUser(getSenderUserId())->getName());
    case td_api::messageForumTopicCreated::ID:
        return tr("Topic %1 created").arg(QString::fromStdString(static_cast<td_api::messageForumTopicCreated*>(_message->content_.get())->name_));
    case td_api::messageForumTopicEdited::ID:
        return tr("Topic %1 edited").arg(QString::fromStdString(static_cast<td_api::messageForumTopicEdited*>(_message->content_.get())->name_));
    case td_api::messageForumTopicIsClosedToggled::ID:
        if (static_cast<td_api::messageForumTopicIsClosedToggled*>(_message->content_.get())->is_closed_) {
            return tr("Topic closed");
        } else {
            return tr("Topic opened");
        }
    case td_api::messageForumTopicIsHiddenToggled::ID:
        if (static_cast<td_api::messageForumTopicIsHiddenToggled*>(_message->content_.get())->is_hidden_) {
            return tr("Topic hidden");
        } else {
            return tr("Topic unhidden");
        }
    default:
        return getType();
    }
}

int32_t Message::getContentType()
{
    return _contentTypeId;
}

bool Message::isService() const
{
    QString messageType = getType();
    return  messageType == "chatSetTtl" ||
            messageType == "pinMessage" ||
            messageType == "messageChatDeleteMember" ||
            messageType == "messageChatAddMembers" ||
            messageType == "messageChatJoinByLink";
}

bool Message::isEdited()
{
    return _message->edit_date_ != 0;
}

bool Message::canBeEdited()
{
    if (_properties == nullptr) return false;

    static std::set<int64_t> editableContents = {td_api::messagePhoto::ID, td_api::messageVideo::ID, td_api::messageDocument::ID, td_api::messageAudio::ID, td_api::messageAnimation::ID, td_api::messageVoiceNote::ID, td_api::messageText::ID};

    return _properties->can_be_edited_ && editableContents.contains(_contentTypeId);
}

bool Message::canBeForwarded()
{
    if (_properties == nullptr) return false;
    return _properties->can_be_forwarded_;
}

bool Message::canBeDeleted()
{
    if (_properties == nullptr) return false;
    return _properties->can_be_deleted_for_all_users_;
}

bool Message::isChannelPost()
{
    return _message->is_channel_post_;
}

int Message::getEditedDate()
{
    return _message->edit_date_;
}

int Message::getViews()
{
    if (!_message->interaction_info_) return -1;
    return _message->interaction_info_->view_count_;
}

bool Message::received()
{
    return !_message->is_outgoing_;
}

int32_t Message::getDeleteMemberId()
{
    if (_contentTypeId == td_api::messageChatDeleteMember::ID) {
        return static_cast<const td_api::messageChatDeleteMember &>(*_message->content_).user_id_;
    }

    return -1;
}

QVector<int64_t> Message::getAddMembersIds()
{
    QVector<int64_t> newUsers;

    if (_contentTypeId == td_api::messageChatAddMembers::ID) {
        auto userIds = static_cast<const td_api::messageChatAddMembers &>(*_message->content_).member_user_ids_;

        for(int64_t userId: userIds) {
            newUsers.append(userId);
        }
    }

    return newUsers;
}

int64_t Message::replyMessageId()
{
    if (!_message->reply_to_) return 0;
    // TODO: handle messageReplyToStory
    if (_message->reply_to_->get_id() == td_api::messageReplyToMessage::ID) {
        return static_cast<td_api::messageReplyToMessage*>(_message->reply_to_.get())->message_id_;
    }

    return 0;
}

td_api::messageForwardInfo* Message::getForwardedInfo()
{
    return _message->forward_info_.get();
}

int64_t Message::getSenderUserId()
{
    if (_message->sender_id_->get_id() != td_api::messageSenderUser::ID) return 0;
    return static_cast<td_api::messageSenderUser*>(_message->sender_id_.get())->user_id_;
}

int64_t Message::getSenderChatId()
{
    if (_message->sender_id_->get_id() != td_api::messageSenderChat::ID) return 0;
    return static_cast<td_api::messageSenderChat*>(_message->sender_id_.get())->chat_id_;
}

QString Message::getSender()
{
    if (_message->sender_id_->get_id() == td_api::messageSenderChat::ID) return "chat";
    else return "user";
}

int32_t Message::getTimestamp()
{
    return _message->date_;
}

QString Message::getFormattedTimestamp()
{
    QString format;
    QDateTime current(QDateTime::currentDateTime());

    if (current.toTime_t() - _message->date_ >= 604800) {
        format = "yyyy.MM.dd hh:mm";
    } else if (current.toTime_t() - _message->date_ >= 86400) {
        format = "ddd hh:mm";
    } else {
        format = "hh:mm";
    }

    return QDateTime::fromTime_t(static_cast<uint>(_message->date_)).toString(format);
}

QString Message::getFormattedForwardTimestamp()
{
    if (!_message->forward_info_) return "";

    QString format;
    QDateTime current(QDateTime::currentDateTime());

    if (current.toTime_t() - _message->forward_info_->date_ >= 604800) {
        format = "yyyy.MM.dd hh:mm";
    } else if (current.toTime_t() - _message->forward_info_->date_ >= 86400) {
        format = "ddd hh:mm";
    } else {
        format = "hh:mm";
    }

    return QDateTime::fromTime_t(static_cast<uint>(_message->forward_info_->date_)).toString(format);
}

bool Message::containsUnreadMention() const
{
    return _message->contains_unread_mention_;
}

void Message::setContainsUnreadMention(bool containsUnreadMention)
{
    _message->contains_unread_mention_ = containsUnreadMention;
}

void Message::fetchProperties()
{
    _manager->sendQueryWithResponse(getChatId(), td_api::getMessageProperties::ID, getId(), new td_api::getMessageProperties(getChatId(), getId()));
}

td_api::MessageTopic* Message::getTopicId()
{
    return _message->topic_id_.get();
}

void Message::updateInteractionInfo(td_api::object_ptr<td_api::messageInteractionInfo> interactionInfo)
{
    _message->interaction_info_ = move(interactionInfo);
}

void Message::setMessageProperties(td_api::messageProperties *messageProperties)
{
    if (_properties != nullptr) {
        delete _properties;
    }

    _properties = messageProperties;
}

td_api::messageReactions* Message::getReactions()
{
    if (!_message->interaction_info_ || !_message->interaction_info_->reactions_) {
        return nullptr;
    }

    return _message->interaction_info_->reactions_.get();
}

void Message::reformatMessage()
{
    _reformatTimer.start();
}

void Message::setEditDate(int32_t editDate)
{
    _message->edit_date_ = editDate;
}

bool Message::hasWebPage() const
{
    return _linkPreview != nullptr;
}

LinkPreview* Message::getWebPage() const
{
    return _linkPreview;
}

void Message::handleMessageContent(td_api::object_ptr<td_api::MessageContent> messageContent)
{
    if (!messageContent) return;

    _contentTypeId = messageContent->get_id();

    if (_contentTypeId == td_api::messageText::ID) {
        if (static_cast<td_api::messageText*>(messageContent.get())->link_preview_) {
            if (_linkPreview == nullptr) {
                _linkPreview = new LinkPreview();
                _linkPreview->setFiles(_files);
            }

            _linkPreview->setLinkPreview(std::move(static_cast<td_api::messageText*>(messageContent.get())->link_preview_));
        }

        _text = td_api::move_object_as<td_api::messageText>(messageContent);
        _formattedMessage = formatTextAsHTML(_text->text_.get());
    } else {
        if (_messageContent == nullptr) {
            switch (_contentTypeId) {
            case td_api::messagePhoto::ID:
                _messageContent = new Photo();
                if (static_cast<td_api::messagePhoto*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messagePhoto*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageSticker::ID:
                _messageContent = new Sticker();
                break;
            case td_api::messageVideo::ID:
                _messageContent = new Video();
                if (static_cast<td_api::messageVideo*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messageVideo*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageDocument::ID:
                _messageContent = new Document();
                if (static_cast<td_api::messageDocument*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messageDocument*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageAudio::ID:
                _messageContent = new Audio();
                if (static_cast<td_api::messageAudio*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messageAudio*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageAnimation::ID:
                _messageContent = new Animation();
                if (static_cast<td_api::messageAnimation*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messageAnimation*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageVoiceNote::ID:
                _messageContent = new VoiceNote();
                if (static_cast<td_api::messageVoiceNote*>(messageContent.get())->caption_) {
                    _formattedMessage = formatTextAsHTML(static_cast<td_api::messageVoiceNote*>(messageContent.get())->caption_.get());
                }
                break;
            case td_api::messageVideoNote::ID:
                _messageContent = new VideoNote();
                break;
            case td_api::messagePoll::ID:
                _messageContent = new Poll();
                break;
            case td_api::messageCall::ID:
                _messageContent = new Call();
                break;
            case td_api::messageLocation::ID:
                _messageContent = new Location();
                break;
            case td_api::messageContact::ID:
                _messageContent = new Contact();
                static_cast<Contact*>(_messageContent)->setUsers(_users);
                break;
            case td_api::messageAnimatedEmoji::ID:
                _messageContent = new AnimatedEmoji();
                break;
            }

            if (_messageContent != nullptr) {
                if (dynamic_cast<ContentFile*>(_messageContent)) {
                    dynamic_cast<ContentFile*>(_messageContent)->setTelegramManager(_manager);
                    dynamic_cast<ContentFile*>(_messageContent)->setFiles(_files);
                }
                _messageContent->handleContent(move(messageContent));
            } else {
                _message->content_ = move(messageContent);
            }
        } else {
            _messageContent->handleContent(move(messageContent));

            onReformatMessage();
        }
    }
}

void Message::updateMessageSendSucceeded(td_api::updateMessageSendSucceeded *updateMessageSendSucceeded)
{
    if (updateMessageSendSucceeded->message_ && updateMessageSendSucceeded->old_message_id_ == getId()) {
        _message = updateMessageSendSucceeded->message_.release();
        emit messageIdChanged(updateMessageSendSucceeded->old_message_id_, getId());
        emit contentChanged(getId());
        emit messageChanged();
    }
}

void Message::updateMessageContent(td_api::updateMessageContent *updateMessageContent)
{
    if (updateMessageContent->message_id_ == getId() && updateMessageContent->chat_id_ == getChatId() && (_messageContent != nullptr || _contentTypeId == td_api::messageText::ID)) {
        _message->content_ = move(updateMessageContent->new_content_);
        handleMessageContent(std::move(_message->content_));
        emit contentChanged(getId());
    }
}

void Message::onEntityLocalPathChanged(QString localPath, int32_t fileId)
{
    Q_UNUSED(localPath)
    Q_UNUSED(fileId)

    reformatMessage();
}

void Message::onReactionLocalPathChanged(QString localPath, int32_t fileId)
{
    Q_UNUSED(localPath)
    Q_UNUSED(fileId)

    _reactionsUpdateTimer.start();
}

void Message::onReformatMessage()
{
    _formattedMessage = "";

    switch (_contentTypeId) {
    case td_api::messagePhoto::ID:
        if (static_cast<Photo*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<Photo*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageVideo::ID:
        if (static_cast<Video*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<Video*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageDocument::ID:
        if (static_cast<Document*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<Document*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageAudio::ID:
        if (static_cast<Audio*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<Audio*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageAnimation::ID:
        if (static_cast<Animation*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<Animation*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageVoiceNote::ID:
        if (static_cast<VoiceNote*>(_messageContent)->getCaption() != nullptr) {
            _formattedMessage = formatTextAsHTML(static_cast<VoiceNote*>(_messageContent)->getCaption());
        }
        break;
    case td_api::messageText::ID:
        _formattedMessage = formatTextAsHTML(_text->text_.get());
        break;
    }

    emit messageFormatedChanged(getId());
}

QString Message::formatTextAsHTML(td_api::formattedText *formattedText)
{
    std::vector<int64_t> customStickerIds;
    for (td_api::object_ptr<td_api::textEntity> &entity : formattedText->entities_) {
        if (entity->type_->get_id() == td_api::textEntityTypeCustomEmoji::ID) {
            auto emojiId = static_cast<td_api::textEntityTypeCustomEmoji*>(entity->type_.get())->custom_emoji_id_;
            if (_customEmojis->getCustomEmojiSticker(emojiId) == 0) {
                customStickerIds.emplace_back(emojiId);
            } else {
                shared_ptr<File> file = _files->getFile(_customEmojis->getCustomEmojiSticker(emojiId));
                if (file && file->localPath().isEmpty()) {
                    connect(file.get(), &File::localPathChanged, this, &Message::onEntityLocalPathChanged, Qt::UniqueConnection);
                }
            }
        }
    }

    if (!customStickerIds.empty()) {
        _manager->sendQueryWithResponse(getChatId(), td_api::getCustomEmojiStickers::ID, getId(), new td_api::getCustomEmojiStickers(move(customStickerIds)));
    }

    if (_entityProcessor == nullptr) {
        return "";
    }

    return _entityProcessor->formatTextAsHTML(formattedText);
}

int64_t Message::getChatId() const
{
    return _chatId;
}

void Message::setChatId(const int64_t &chatId)
{
    _chatId = chatId;
}

void Message::updateCustomEmojis(QVector<int32_t> fileIds)
{
    QVector<int32_t> entityIds;
    QVector<int32_t> reactionIds;

    td_api::formattedText* formattedText = nullptr;
    switch (_contentTypeId) {
    case td_api::messagePhoto::ID:
        if (static_cast<Photo*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<Photo*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageVideo::ID:
        if (static_cast<Video*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<Video*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageDocument::ID:
        if (static_cast<Document*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<Document*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageAudio::ID:
        if (static_cast<Audio*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<Audio*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageAnimation::ID:
        if (static_cast<Animation*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<Animation*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageVoiceNote::ID:
        if (static_cast<VoiceNote*>(_messageContent)->getCaption() != nullptr) {
            formattedText = static_cast<VoiceNote*>(_messageContent)->getCaption();
        }
        break;
    case td_api::messageText::ID:
        formattedText = _text->text_.get();
        break;
    }

    if (formattedText == nullptr) return;

    for (td_api::object_ptr<td_api::textEntity> &entity : formattedText->entities_) {
        if (entity->type_->get_id() == td_api::textEntityTypeCustomEmoji::ID) {
            auto emojiId = static_cast<td_api::textEntityTypeCustomEmoji*>(entity->type_.get())->custom_emoji_id_;

            entityIds.append(_customEmojis->getCustomEmojiSticker(emojiId));
        }
    }

    if (getReactions() != nullptr) {
        for (td_api::object_ptr<td_api::messageReaction> &reaction : getReactions()->reactions_) {
            if (reaction->type_->get_id() == td_api::reactionTypeCustomEmoji::ID) {
                auto emojiId = static_cast<td_api::reactionTypeCustomEmoji*>(reaction->type_.get())->custom_emoji_id_;

                reactionIds.append(_customEmojis->getCustomEmojiSticker(emojiId));
            }
        }
    }

    bool updateReactions = false;
    bool updateText = false;
    for (int32_t fileId : fileIds) {
        if (entityIds.contains(fileId)) {
            shared_ptr<File> file = _files->getFile(fileId);
            connect(file.get(), &File::localPathChanged, this, &Message::onEntityLocalPathChanged, Qt::UniqueConnection);

            if (!file->localPath().isEmpty()) {
                updateText = true;
            }
        }

        if (reactionIds.contains(fileId)) {
            updateReactions = true;

            connect(_files->getFile(fileId).get(), &File::localPathChanged, this, &Message::onReactionLocalPathChanged, Qt::UniqueConnection);
        }
    }

    if (updateReactions) {
        emit reactionsChanged(getId());
    }

    if (updateText) {
        reformatMessage();

        emit messageFormatedChanged(getId());
    }
}
