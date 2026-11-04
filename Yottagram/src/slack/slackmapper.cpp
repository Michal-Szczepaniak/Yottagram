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

#include "slackmapper.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace SlackMapper {

namespace {

// All ids stay below 2^53, so a double holds them exactly
int64_t toInt64(const QJsonValue &value)
{
    return static_cast<int64_t>(value.toDouble());
}

string toStdString(const QJsonValue &value)
{
    return value.toString().toStdString();
}

td_api::object_ptr<td_api::TextEntityType> makeEntityType(const QJsonObject &entity)
{
    QString type = entity["type"].toString();

    if (type == "bold") return td_api::make_object<td_api::textEntityTypeBold>();
    if (type == "italic") return td_api::make_object<td_api::textEntityTypeItalic>();
    if (type == "strikethrough") return td_api::make_object<td_api::textEntityTypeStrikethrough>();
    if (type == "code") return td_api::make_object<td_api::textEntityTypeCode>();
    if (type == "pre") return td_api::make_object<td_api::textEntityTypePre>();
    if (type == "url") return td_api::make_object<td_api::textEntityTypeUrl>();
    if (type == "text_url") return td_api::make_object<td_api::textEntityTypeTextUrl>(toStdString(entity["url"]));
    if (type == "custom_emoji") return td_api::make_object<td_api::textEntityTypeCustomEmoji>(toInt64(entity["emoji"]));
    return nullptr;
}

td_api::object_ptr<td_api::formattedText> makeText(const QJsonValue &value, const QJsonArray &entities = QJsonArray())
{
    auto text = td_api::make_object<td_api::formattedText>();
    text->text_ = toStdString(value);

    for (const QJsonValue &item : entities) {
        QJsonObject entity = item.toObject();
        auto type = makeEntityType(entity);
        if (type) {
            text->entities_.push_back(td_api::make_object<td_api::textEntity>(entity["offset"].toInt(), entity["length"].toInt(), std::move(type)));
        }
    }

    return text;
}

td_api::object_ptr<td_api::file> makeFile(const QJsonObject &object)
{
    int64_t size = toInt64(object["size"]);
    bool done = object["done"].toBool();

    auto local = td_api::make_object<td_api::localFile>();
    local->path_ = done ? toStdString(object["path"]) : "";
    local->can_be_downloaded_ = true;
    local->is_downloading_active_ = object["active"].toBool();
    local->is_downloading_completed_ = done;
    local->downloaded_size_ = done ? size : toInt64(object["downloaded"]);

    auto remote = td_api::make_object<td_api::remoteFile>();
    remote->id_ = QString::number(object["id"].toInt()).toStdString();
    remote->unique_id_ = remote->id_;
    remote->is_uploading_completed_ = true;
    remote->uploaded_size_ = size;

    auto file = td_api::make_object<td_api::file>();
    file->id_ = object["id"].toInt();
    file->size_ = size;
    file->expected_size_ = size;
    file->local_ = std::move(local);
    file->remote_ = std::move(remote);
    return file;
}

td_api::object_ptr<td_api::photoSize> makePhotoSize(const char *type, const QJsonValue &file, const QJsonValue &width, const QJsonValue &height)
{
    auto size = td_api::make_object<td_api::photoSize>();
    size->type_ = type;
    size->photo_ = makeFile(file.toObject());
    size->width_ = static_cast<int32_t>(toInt64(width));
    size->height_ = static_cast<int32_t>(toInt64(height));
    return size;
}

td_api::object_ptr<td_api::linkPreview> makeLinkPreview(const QJsonObject &object)
{
    auto article = td_api::make_object<td_api::linkPreviewTypeArticle>();
    if (object.contains("image")) {
        article->photo_ = td_api::make_object<td_api::photo>();
        article->photo_->sizes_.push_back(makePhotoSize("x", object["image"], object["width"], object["height"]));
    }

    auto preview = td_api::make_object<td_api::linkPreview>();
    preview->url_ = toStdString(object["url"]);
    preview->display_url_ = preview->url_;
    preview->site_name_ = toStdString(object["site"]);
    preview->title_ = toStdString(object["title"]);
    preview->description_ = makeText(object["text"]);
    preview->type_ = std::move(article);
    return preview;
}

td_api::object_ptr<td_api::MessageContent> makeContent(const QJsonObject &object)
{
    QJsonObject media = object["media"].toObject();
    QString kind = media["kind"].toString();
    auto caption = makeText(object["text"], object["entities"].toArray());

    if (kind == "photo") {
        auto photo = td_api::make_object<td_api::photo>();
        if (media.contains("thumb")) {
            photo->sizes_.push_back(makePhotoSize("m", media["thumb"], media["thumb_width"], media["thumb_height"]));
        }
        photo->sizes_.push_back(makePhotoSize("x", media["file"], media["width"], media["height"]));

        auto content = td_api::make_object<td_api::messagePhoto>();
        content->photo_ = std::move(photo);
        content->caption_ = std::move(caption);
        return std::move(content);
    }

    if (kind == "document") {
        auto document = td_api::make_object<td_api::document>();
        document->file_name_ = toStdString(media["name"]);
        document->mime_type_ = toStdString(media["mime"]);
        document->document_ = makeFile(media["file"].toObject());

        auto content = td_api::make_object<td_api::messageDocument>();
        content->document_ = std::move(document);
        content->caption_ = std::move(caption);
        return std::move(content);
    }

    if (kind == "call") {
        auto content = td_api::make_object<td_api::messageCall>();
        content->discard_reason_ = td_api::make_object<td_api::callDiscardReasonHungUp>();
        content->duration_ = static_cast<int32_t>(toInt64(media["duration"]));
        return std::move(content);
    }

    auto content = td_api::make_object<td_api::messageText>();
    content->text_ = std::move(caption);
    if (object["preview"].isObject()) {
        content->link_preview_ = makeLinkPreview(object["preview"].toObject());
    }
    return std::move(content);
}

td_api::object_ptr<td_api::messageInteractionInfo> makeInteractionInfo(const QJsonArray &reactions)
{
    auto list = td_api::make_object<td_api::messageReactions>();
    for (const QJsonValue &value : reactions) {
        QJsonObject reaction = value.toObject();

        auto item = td_api::make_object<td_api::messageReaction>();
        if (reaction.contains("custom")) {
            item->type_ = td_api::make_object<td_api::reactionTypeCustomEmoji>(toInt64(reaction["custom"]));
        } else {
            item->type_ = td_api::make_object<td_api::reactionTypeEmoji>(toStdString(reaction["emoji"]));
        }
        item->total_count_ = static_cast<int32_t>(toInt64(reaction["count"]));
        item->is_chosen_ = reaction["chosen"].toBool();
        list->reactions_.push_back(std::move(item));
    }

    auto info = td_api::make_object<td_api::messageInteractionInfo>();
    info->reactions_ = std::move(list);
    return info;
}

td_api::object_ptr<td_api::message> makeMessage(const QJsonObject &object)
{
    auto sender = td_api::make_object<td_api::messageSenderUser>();
    sender->user_id_ = toInt64(object["sender"]);

    auto message = td_api::make_object<td_api::message>();
    message->id_ = toInt64(object["id"]);
    message->chat_id_ = toInt64(object["chat"]);
    message->sender_id_ = std::move(sender);
    message->is_outgoing_ = object["outgoing"].toBool();
    message->date_ = static_cast<int32_t>(toInt64(object["date"]));
    message->edit_date_ = static_cast<int32_t>(toInt64(object["edit_date"]));
    message->content_ = makeContent(object);

    QJsonArray reactions = object["reactions"].toArray();
    if (!reactions.isEmpty()) {
        message->interaction_info_ = makeInteractionInfo(reactions);
    }

    return message;
}

td_api::object_ptr<td_api::user> makeUser(const QJsonObject &object)
{
    auto user = td_api::make_object<td_api::user>();
    user->id_ = toInt64(object["id"]);
    user->first_name_ = toStdString(object["first_name"]);
    user->last_name_ = toStdString(object["last_name"]);
    user->status_ = td_api::make_object<td_api::userStatusEmpty>();
    user->type_ = td_api::make_object<td_api::userTypeRegular>();
    user->have_access_ = true;

    if (object.contains("photo")) {
        QJsonObject photo = object["photo"].toObject();
        user->profile_photo_ = td_api::make_object<td_api::profilePhoto>();
        user->profile_photo_->id_ = toInt64(object["id"]);
        user->profile_photo_->small_ = makeFile(photo["small"].toObject());
        user->profile_photo_->big_ = makeFile(photo["big"].toObject());
    }

    QString username = object["username"].toString();
    if (!username.isEmpty()) {
        user->usernames_ = td_api::make_object<td_api::usernames>();
        user->usernames_->active_usernames_.push_back(username.toStdString());
    }

    return user;
}

td_api::object_ptr<td_api::chatPosition> makePosition(int64_t order, int32_t folder = 0)
{
    auto position = td_api::make_object<td_api::chatPosition>();
    if (folder != 0) {
        position->list_ = td_api::make_object<td_api::chatListFolder>(folder);
    } else {
        position->list_ = td_api::make_object<td_api::chatListMain>();
    }
    position->order_ = order;
    return position;
}

td_api::object_ptr<td_api::chatNotificationSettings> makeNotificationSettings()
{
    auto settings = td_api::make_object<td_api::chatNotificationSettings>();
    settings->show_preview_ = true;
    return settings;
}

td_api::object_ptr<td_api::chatPhotoInfo> makeChatPhoto(const QJsonObject &object)
{
    auto photo = td_api::make_object<td_api::chatPhotoInfo>();
    photo->small_ = makeFile(object["small"].toObject());
    photo->big_ = makeFile(object["big"].toObject());
    return photo;
}

td_api::object_ptr<td_api::chat> makeChat(const QJsonObject &object)
{
    auto permissions = td_api::make_object<td_api::chatPermissions>();
    permissions->can_send_basic_messages_ = true;
    permissions->can_send_audios_ = true;
    permissions->can_send_documents_ = true;
    permissions->can_send_photos_ = true;
    permissions->can_send_videos_ = true;
    permissions->can_send_voice_notes_ = true;

    auto chat = td_api::make_object<td_api::chat>();
    chat->id_ = toInt64(object["id"]);
    chat->title_ = toStdString(object["title"]);
    chat->permissions_ = std::move(permissions);
    chat->notification_settings_ = makeNotificationSettings();

    if (object.contains("photo")) {
        chat->photo_ = makeChatPhoto(object["photo"].toObject());
    }

    if (object["type"].toString() == "private") {
        auto type = td_api::make_object<td_api::chatTypePrivate>();
        type->user_id_ = toInt64(object["user_id"]);
        chat->type_ = std::move(type);
    } else {
        auto type = td_api::make_object<td_api::chatTypeSupergroup>();
        type->supergroup_id_ = toInt64(object["supergroup_id"]);
        type->is_channel_ = false;
        chat->type_ = std::move(type);
    }

    return chat;
}

td_api::object_ptr<td_api::supergroup> makeSupergroup(const QJsonObject &object)
{
    auto supergroup = td_api::make_object<td_api::supergroup>();
    supergroup->id_ = toInt64(object["supergroup_id"]);
    supergroup->status_ = td_api::make_object<td_api::chatMemberStatusMember>();
    return supergroup;
}

td_api::Object* makeAuthorizationState(td_api::object_ptr<td_api::AuthorizationState> state)
{
    auto update = new td_api::updateAuthorizationState();
    update->authorization_state_ = std::move(state);
    return update;
}

td_api::Object* makeLastMessage(const QJsonObject &object)
{
    auto update = new td_api::updateChatLastMessage();
    update->chat_id_ = toInt64(object["chat"]);
    update->last_message_ = makeMessage(object["message"].toObject());
    update->positions_.push_back(makePosition(toInt64(object["order"])));
    return update;
}

void mapReply(uint64_t id, const QJsonObject &object, vector<Output> &outputs)
{
    if (object.contains("error")) {
        QJsonObject error = object["error"].toObject();

        auto reply = new td_api::error();
        reply->code_ = static_cast<int32_t>(toInt64(error["code"]));
        reply->message_ = toStdString(error["message"]);
        outputs.push_back({id, reply});
        return;
    }

    QJsonObject result = object["result"].toObject();
    QString type = result["type"].toString();

    if (type == "messages") {
        QJsonArray messages = result["messages"].toArray();

        auto reply = new td_api::messages();
        reply->total_count_ = messages.size();
        for (const QJsonValue &message : messages) {
            reply->messages_.push_back(makeMessage(message.toObject()));
        }
        outputs.push_back({id, reply});
    } else if (type == "user") {
        outputs.push_back({id, makeUser(result["user"].toObject()).release()});
    } else if (type == "available_reactions") {
        auto reply = new td_api::availableReactions();
        for (const QJsonValue &emoji : result["emojis"].toArray()) {
            auto reaction = td_api::make_object<td_api::availableReaction>();
            reaction->type_ = td_api::make_object<td_api::reactionTypeEmoji>(toStdString(emoji));
            reply->top_reactions_.push_back(std::move(reaction));
        }
        outputs.push_back({id, reply});
    } else if (type == "custom_emojis") {
        auto reply = new td_api::stickers();
        for (const QJsonValue &value : result["emojis"].toArray()) {
            QJsonObject emoji = value.toObject();

            auto fullType = td_api::make_object<td_api::stickerFullTypeCustomEmoji>();
            fullType->custom_emoji_id_ = toInt64(emoji["id"]);

            auto thumbnail = td_api::make_object<td_api::thumbnail>();
            thumbnail->file_ = makeFile(emoji["file"].toObject());

            auto sticker = td_api::make_object<td_api::sticker>();
            sticker->emoji_ = toStdString(emoji["name"]);
            sticker->full_type_ = std::move(fullType);
            sticker->thumbnail_ = std::move(thumbnail);
            sticker->sticker_ = makeFile(emoji["file"].toObject());
            reply->stickers_.push_back(std::move(sticker));
        }
        outputs.push_back({id, reply});
    } else if (type == "members") {
        auto reply = new td_api::chatMembers();
        for (const QJsonValue &user : result["users"].toArray()) {
            auto sender = td_api::make_object<td_api::messageSenderUser>();
            sender->user_id_ = toInt64(user);

            auto member = td_api::make_object<td_api::chatMember>();
            member->member_id_ = std::move(sender);
            member->status_ = td_api::make_object<td_api::chatMemberStatusMember>();
            reply->members_.push_back(std::move(member));
        }
        reply->total_count_ = static_cast<int32_t>(reply->members_.size());
        outputs.push_back({id, reply});
    } else if (type == "properties") {
        auto reply = new td_api::messageProperties();
        reply->can_be_edited_ = true;
        reply->can_be_deleted_for_all_users_ = true;
        outputs.push_back({id, reply});
    } else {
        outputs.push_back({id, new td_api::ok()});
    }
}

td_api::updateNotificationGroup* makeNotificationGroup(const QJsonObject &object)
{
    auto update = new td_api::updateNotificationGroup();
    update->notification_group_id_ = static_cast<int32_t>(toInt64(object["group"]));
    update->type_ = td_api::make_object<td_api::notificationGroupTypeMessages>();
    update->chat_id_ = toInt64(object["chat"]);
    update->notification_settings_chat_id_ = update->chat_id_;
    return update;
}

void mapEvent(const QJsonObject &object, vector<Output> &outputs)
{
    QString event = object["event"].toString();

    if (event == "auth_ready") {
        auto myId = td_api::make_object<td_api::optionValueInteger>();
        myId->value_ = toInt64(object["my_id"]);

        auto option = new td_api::updateOption();
        option->name_ = "my_id";
        option->value_ = std::move(myId);

        outputs.push_back({0, makeAuthorizationState(td_api::make_object<td_api::authorizationStateReady>())});
        outputs.push_back({0, option});
    } else if (event == "users") {
        for (const QJsonValue &user : object["users"].toArray()) {
            auto update = new td_api::updateUser();
            update->user_ = makeUser(user.toObject());
            outputs.push_back({0, update});
        }
    } else if (event == "chats") {
        for (const QJsonValue &value : object["chats"].toArray()) {
            QJsonObject chat = value.toObject();

            if (chat["type"].toString() != "private") {
                auto supergroup = new td_api::updateSupergroup();
                supergroup->supergroup_ = makeSupergroup(chat);
                outputs.push_back({0, supergroup});
            }

            auto newChat = new td_api::updateNewChat();
            newChat->chat_ = makeChat(chat);
            outputs.push_back({0, newChat});

            auto position = new td_api::updateChatPosition();
            position->chat_id_ = toInt64(chat["id"]);
            position->position_ = makePosition(toInt64(chat["order"]));
            outputs.push_back({0, position});

            auto folderPosition = new td_api::updateChatPosition();
            folderPosition->chat_id_ = toInt64(chat["id"]);
            folderPosition->position_ = makePosition(toInt64(chat["order"]), chat["folder"].toInt());
            outputs.push_back({0, folderPosition});
        }
    } else if (event == "ready") {
        auto update = new td_api::updateConnectionState();
        update->state_ = td_api::make_object<td_api::connectionStateReady>();
        outputs.push_back({0, update});
    } else if (event == "new_message") {
        auto update = new td_api::updateNewMessage();
        update->message_ = makeMessage(object["message"].toObject());
        outputs.push_back({0, update});
    } else if (event == "last_message") {
        outputs.push_back({0, makeLastMessage(object)});
    } else if (event == "send_succeeded") {
        auto update = new td_api::updateMessageSendSucceeded();
        update->message_ = makeMessage(object["message"].toObject());
        update->old_message_id_ = toInt64(object["old_id"]);
        outputs.push_back({0, update});
    } else if (event == "message_content") {
        auto update = new td_api::updateMessageContent();
        update->chat_id_ = toInt64(object["chat"]);
        update->message_id_ = toInt64(object["message_id"]);
        update->new_content_ = makeContent(object);
        outputs.push_back({0, update});

        int32_t editDate = static_cast<int32_t>(toInt64(object["edit_date"]));
        if (editDate != 0) {
            auto edited = new td_api::updateMessageEdited();
            edited->chat_id_ = toInt64(object["chat"]);
            edited->message_id_ = toInt64(object["message_id"]);
            edited->edit_date_ = editDate;
            outputs.push_back({0, edited});
        }
    } else if (event == "delete_messages") {
        auto update = new td_api::updateDeleteMessages();
        update->chat_id_ = toInt64(object["chat"]);
        update->is_permanent_ = true;
        for (const QJsonValue &id : object["ids"].toArray()) {
            update->message_ids_.push_back(toInt64(id));
        }
        outputs.push_back({0, update});
    } else if (event == "member_count") {
        auto update = new td_api::updateSupergroupFullInfo();
        update->supergroup_id_ = toInt64(object["supergroup_id"]);
        update->supergroup_full_info_ = td_api::make_object<td_api::supergroupFullInfo>();
        update->supergroup_full_info_->member_count_ = object["count"].toInt();
        update->supergroup_full_info_->description_ = toStdString(object["description"]);
        outputs.push_back({0, update});
    } else if (event == "reactions") {
        auto update = new td_api::updateMessageInteractionInfo();
        update->chat_id_ = toInt64(object["chat"]);
        update->message_id_ = toInt64(object["message_id"]);
        update->interaction_info_ = makeInteractionInfo(object["reactions"].toArray());
        outputs.push_back({0, update});
    } else if (event == "read_inbox") {
        auto read = new td_api::updateChatReadInbox();
        read->chat_id_ = toInt64(object["chat"]);
        read->last_read_inbox_message_id_ = toInt64(object["last_read"]);
        read->unread_count_ = object["unread"].toInt();
        outputs.push_back({0, read});

        auto messages = new td_api::updateUnreadMessageCount();
        messages->chat_list_ = td_api::make_object<td_api::chatListMain>();
        messages->unread_count_ = object["unread_messages"].toInt();
        messages->unread_unmuted_count_ = messages->unread_count_;
        outputs.push_back({0, messages});

        auto chats = new td_api::updateUnreadChatCount();
        chats->chat_list_ = td_api::make_object<td_api::chatListMain>();
        chats->total_count_ = object["total_chats"].toInt();
        chats->unread_count_ = object["unread_chats"].toInt();
        chats->unread_unmuted_count_ = chats->unread_count_;
        outputs.push_back({0, chats});
    } else if (event == "notify") {
        auto type = td_api::make_object<td_api::notificationTypeNewMessage>();
        type->message_ = makeMessage(object["message"].toObject());
        type->show_preview_ = true;

        auto notification = td_api::make_object<td_api::notification>();
        notification->id_ = object["id"].toInt();
        notification->date_ = static_cast<int32_t>(toInt64(object["date"]));
        notification->type_ = std::move(type);

        auto update = makeNotificationGroup(object);
        update->total_count_ = 1;
        update->added_notifications_.push_back(std::move(notification));
        outputs.push_back({0, update});
    } else if (event == "notify_clear") {
        auto update = makeNotificationGroup(object);
        for (const QJsonValue &id : object["ids"].toArray()) {
            update->removed_notification_ids_.push_back(id.toInt());
        }
        outputs.push_back({0, update});
    } else if (event == "file") {
        auto update = new td_api::updateFile();
        update->file_ = makeFile(object["file"].toObject());
        outputs.push_back({0, update});
    } else if (event == "folders") {
        auto update = new td_api::updateChatFolders();
        for (const QJsonValue &value : object["folders"].toArray()) {
            QJsonObject folder = value.toObject();

            auto name = td_api::make_object<td_api::chatFolderName>();
            name->text_ = makeText(folder["name"]);

            auto info = td_api::make_object<td_api::chatFolderInfo>();
            info->id_ = folder["id"].toInt();
            info->name_ = std::move(name);
            info->icon_ = td_api::make_object<td_api::chatFolderIcon>(toStdString(folder["icon"]));
            update->chat_folders_.push_back(std::move(info));
        }
        outputs.push_back({0, update});
    } else if (event == "log") {
        qWarning() << "yottaslack:" << object["message"].toString();
    } else {
        qWarning() << "yottaslack: unknown event" << event;
    }
}

}

vector<Output> map(const QByteArray &json)
{
    vector<Output> outputs;

    QJsonObject object = QJsonDocument::fromJson(json).object();
    if (object.isEmpty()) {
        qWarning() << "yottaslack: invalid event";
        return outputs;
    }

    if (object.contains("id")) {
        mapReply(static_cast<uint64_t>(toInt64(object["id"])), object, outputs);
    } else {
        mapEvent(object, outputs);
    }

    return outputs;
}

td_api::Object* authorizationWaitToken()
{
    return makeAuthorizationState(td_api::make_object<td_api::authorizationStateWaitOtherDeviceConfirmation>());
}

}
