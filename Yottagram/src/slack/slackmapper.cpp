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

td_api::object_ptr<td_api::formattedText> makeText(const QJsonValue &value)
{
    auto text = td_api::make_object<td_api::formattedText>();
    text->text_ = toStdString(value);
    return text;
}

td_api::object_ptr<td_api::messageText> makeContent(const QJsonValue &value)
{
    auto content = td_api::make_object<td_api::messageText>();
    content->text_ = makeText(value);
    return content;
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
    message->content_ = makeContent(object["text"]);
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

    QString username = object["username"].toString();
    if (!username.isEmpty()) {
        user->usernames_ = td_api::make_object<td_api::usernames>();
        user->usernames_->active_usernames_.push_back(username.toStdString());
    }

    return user;
}

td_api::object_ptr<td_api::chatPosition> makePosition(int64_t order)
{
    auto position = td_api::make_object<td_api::chatPosition>();
    position->list_ = td_api::make_object<td_api::chatListMain>();
    position->order_ = order;
    return position;
}

td_api::object_ptr<td_api::chatNotificationSettings> makeNotificationSettings()
{
    auto settings = td_api::make_object<td_api::chatNotificationSettings>();
    settings->show_preview_ = true;
    return settings;
}

td_api::object_ptr<td_api::chat> makeChat(const QJsonObject &object)
{
    auto permissions = td_api::make_object<td_api::chatPermissions>();
    permissions->can_send_basic_messages_ = true;

    auto chat = td_api::make_object<td_api::chat>();
    chat->id_ = toInt64(object["id"]);
    chat->title_ = toStdString(object["title"]);
    chat->permissions_ = std::move(permissions);
    chat->notification_settings_ = makeNotificationSettings();

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
    } else {
        outputs.push_back({id, new td_api::ok()});
    }
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
        update->new_content_ = makeContent(object["text"]);
        outputs.push_back({0, update});
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
