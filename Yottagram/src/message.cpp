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

Message::Message(QObject *parent) : QObject(parent), _message(nullptr), _text(nullptr), _messageContent(nullptr), _webPage(nullptr)
{

}

Message::~Message()
{
}

td_api::message *Message::message() const
{
    return _message;
}

void Message::setMessage(td_api::message *message)
{
    _message = message;
    handleMessageContent(std::move(message->content_));
}

void Message::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
    connect(this->_manager.get(), SIGNAL(updateMessageSendSucceeded(td_api::updateMessageSendSucceeded*)), this, SLOT(updateMessageSendSucceeded(td_api::updateMessageSendSucceeded*)));
    connect(this->_manager.get(), SIGNAL(updateMessageContent(td_api::updateMessageContent*)), this, SLOT(updateMessageContent(td_api::updateMessageContent*)));
}

void Message::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void Message::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int64_t Message::getId()
{
    return _message->id_;
}

QString Message::getText()
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
        return "";
    }
    case td_api::messageChatJoinByLink::ID:
        return tr("%1 joined").arg(_users->getUser(getSenderUserId())->getName());
    case td_api::messagePhoto::ID:
        if (static_cast<Photo*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<Photo*>(_messageContent)->getCaption()->text_);
    case td_api::messageVideo::ID:
        if (static_cast<Video*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<Video*>(_messageContent)->getCaption()->text_);
    case td_api::messageDocument::ID:
        if (static_cast<Document*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<Document*>(_messageContent)->getCaption()->text_);
    case td_api::messageAudio::ID:
        if (static_cast<Audio*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<Audio*>(_messageContent)->getCaption()->text_);
    case td_api::messageAnimation::ID:
        if (static_cast<Animation*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<Animation*>(_messageContent)->getCaption()->text_);
    case td_api::messageVoiceNote::ID:
        if (static_cast<VoiceNote*>(_messageContent)->getCaption() == nullptr) return "";
        return QString::fromStdString(static_cast<VoiceNote*>(_messageContent)->getCaption()->text_);
    case td_api::messagePoll::ID:
        return static_cast<Poll*>(_messageContent)->getQuestion();
    case td_api::messageChatSetTtl::ID:
        return tr("Self-destruct timer set to %n second(s)", "", static_cast<td_api::messageChatSetTtl*>(_message->content_.get())->ttl_);
    case td_api::messagePinMessage::ID:
        if (_message->sender_id_->get_id() == td_api::messageSenderUser::ID) {
            return tr("%1 pinned message").arg(_users->getUser(getSenderUserId())->getName());
        } else if (_message->sender_id_->get_id() == td_api::messageSenderChat::ID) {
            return tr("Message was pinned");
        }
    case td_api::messageCall::ID:
        return _message->is_outgoing_ ? tr("Outgoing call") : tr("Incoming call");
        break;
    case td_api::messageBasicGroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageSupergroupChatCreate::ID:
        return tr("Group created");
    case td_api::messageLocation::ID:
    case td_api::messageContact::ID:
    case td_api::messageSticker::ID:
    case td_api::messageVideoNote::ID:
    case td_api::messageAnimatedEmoji::ID:
        return "";
    case td_api::messageContactRegistered::ID:
        return tr("%1 joined telegram").arg(_users->getUser(getSenderUserId())->getName());
    default:
        return "Message unsupported";
    }
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
    case td_api::messageChatSetTtl::ID:
        return "chatSetTtl";
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
        return "Message unsupported";
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
    return _message->can_be_edited_;
}

bool Message::canBeForwarded()
{
    return _message->can_be_forwarded_;
}

bool Message::canBeDeleted()
{
    return _message->can_be_deleted_for_all_users_;
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

QVector<int32_t> Message::getAddMembersIds()
{
    QVector<int32_t> newUsers;

    if (_contentTypeId == td_api::messageChatAddMembers::ID) {
        auto userIds = static_cast<const td_api::messageChatAddMembers &>(*_message->content_).member_user_ids_;

        for(int32_t userId: userIds) {
            newUsers.append(userId);
        }
    }

    return newUsers;
}

int64_t Message::replyMessageId()
{
    return _message->reply_to_message_id_;
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

bool Message::hasWebPage() const
{
    return _webPage != nullptr;
}

WebPage* Message::getWebPage() const
{
    return _webPage;
}

void Message::handleMessageContent(td_api::object_ptr<td_api::MessageContent> messageContent)
{
    if (!messageContent) return;
    _contentTypeId = messageContent->get_id();
    if (_contentTypeId == td_api::messageText::ID) {
        if (static_cast<td_api::messageText*>(messageContent.get())->web_page_) {
            if (_webPage == nullptr) {
                _webPage = new WebPage();
                _webPage->setFiles(_files);
            }

            _webPage->setWebpage(std::move(static_cast<td_api::messageText*>(messageContent.get())->web_page_));
        }

        _text = td_api::move_object_as<td_api::messageText>(messageContent);
    } else {
        if (_messageContent == nullptr) {
            switch (_contentTypeId) {
            case td_api::messagePhoto::ID:
                _messageContent = new Photo();
                break;
            case td_api::messageSticker::ID:
                _messageContent = new Sticker();
                break;
            case td_api::messageVideo::ID:
                _messageContent = new Video();
                break;
            case td_api::messageDocument::ID:
                _messageContent = new Document();
                break;
            case td_api::messageAudio::ID:
                _messageContent = new Audio();
                break;
            case td_api::messageAnimation::ID:
                _messageContent = new Animation();
                break;
            case td_api::messageVoiceNote::ID:
                _messageContent = new VoiceNote();
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
        }
    }
}

void Message::updateMessageSendSucceeded(td_api::updateMessageSendSucceeded *updateMessageSendSucceeded)
{
    if (updateMessageSendSucceeded->message_ && updateMessageSendSucceeded->old_message_id_ == getId()) {
        _message = updateMessageSendSucceeded->message_.release();
        handleMessageContent(std::move(_message->content_));
        emit messageIdChanged(updateMessageSendSucceeded->old_message_id_, getId());
        emit contentChanged(getId());
        emit messageChanged();
    }
}

void Message::updateMessageContent(td_api::updateMessageContent *updateMessageContent)
{
    if (updateMessageContent->message_id_ == this->getId() && updateMessageContent->chat_id_ == getChatId()) {
        _message->content_.release();
        _message->content_ = move(updateMessageContent->new_content_);
        handleMessageContent(std::move(_message->content_));
        emit contentChanged(this->getId());
    }
}

int64_t Message::getChatId() const
{
    return _chatId;
}

void Message::setChatId(const int64_t &chatId)
{
    _chatId = chatId;
}
