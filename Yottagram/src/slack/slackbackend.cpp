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

#include "slackbackend.h"
#include "slackmapper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>

namespace {

const char *TOKEN_KEY = "slackToken";
const char *APP_TOKEN_KEY = "slackAppToken";
const char *APP_TOKEN_OPTION = "slack_app_token";
const int INVALID_TOKEN_CODE = 401;
const unsigned long CLOSE_TIMEOUT = 3000;

}

SlackBackend::SlackBackend()
{
    connect(&_receiver, &SlackReceiver::messageReceived, this, &SlackBackend::onReceived);

    loadLibrary();
}

void SlackBackend::loadLibrary()
{
    _library.setFileName(QCoreApplication::applicationDirPath() + "/../share/yottagram/lib/libyottaslack.so");

    if (!_library.load()) {
        qCritical() << "Failed to load yottaslack:" << _library.errorString();
        return;
    }

    _api.init = reinterpret_cast<decltype(_api.init)>(_library.resolve("ys_init"));
    _api.send = reinterpret_cast<decltype(_api.send)>(_library.resolve("ys_send"));
    _api.receive = reinterpret_cast<decltype(_api.receive)>(_library.resolve("ys_receive"));
    _api.free = reinterpret_cast<decltype(_api.free)>(_library.resolve("ys_free"));
    _api.close = reinterpret_cast<decltype(_api.close)>(_library.resolve("ys_close"));

    if (!_api.isLoaded()) {
        qCritical() << "yottaslack is missing symbols";
        _api = SlackLibrary();
        return;
    }

    QString dataDirectory = QDir::homePath() + "/.local/share/Yottagram/slack";
    QDir().mkpath(dataDirectory);

    QByteArray path = dataDirectory.toUtf8();
    if (_api.init(path.data()) != 0) {
        qCritical() << "yottaslack failed to initialize";
        _api = SlackLibrary();
    }
}

void SlackBackend::start()
{
    if (_api.isLoaded()) {
        _receiver.setLibrary(_api);
        _receiver.start();
    }

    QString token = QSettings().value(TOKEN_KEY).toString();
    if (token.isEmpty() || !_api.isLoaded()) {
        emit messageReceived(0, SlackMapper::authorizationWaitToken());
        return;
    }

    call(STORED_TOKEN_LOGIN_ID, "login", {{"token", token}, {"app_token", QSettings().value(APP_TOKEN_KEY).toString()}, {"retry", true}});
}

void SlackBackend::send(uint64_t id, td_api::object_ptr<td_api::Function> function)
{
    switch (function->get_id()) {
    case td_api::checkAuthenticationBotToken::ID:
        _pendingToken = QString::fromStdString(static_cast<td_api::checkAuthenticationBotToken*>(function.get())->token_).trimmed();
        _loginRequestId = id;
        call(id, "login", {{"token", _pendingToken}, {"app_token", QSettings().value(APP_TOKEN_KEY).toString()}});
        break;
    case td_api::getMe::ID:
        call(id, "me");
        break;
    case td_api::getChatHistory::ID: {
        auto query = static_cast<td_api::getChatHistory*>(function.get());
        call(id, "history", {
            {"chat", static_cast<double>(query->chat_id_)},
            {"from", static_cast<double>(query->from_message_id_)},
            {"offset", query->offset_},
            {"limit", query->limit_}
        });
        break;
    }
    case td_api::sendMessage::ID:
        handleSendMessage(id, static_cast<td_api::sendMessage*>(function.get()));
        break;
    case td_api::editMessageText::ID:
        handleEditMessage(id, static_cast<td_api::editMessageText*>(function.get()));
        break;
    case td_api::deleteMessages::ID:
        handleDeleteMessages(id, static_cast<td_api::deleteMessages*>(function.get()));
        break;
    case td_api::getMessageProperties::ID:
        call(id, "properties", {{"message", static_cast<double>(static_cast<td_api::getMessageProperties*>(function.get())->message_id_)}});
        break;
    case td_api::openChat::ID:
        call(id, "open", {{"chat", static_cast<double>(static_cast<td_api::openChat*>(function.get())->chat_id_)}});
        break;
    case td_api::closeChat::ID:
        call(id, "close", {{"chat", static_cast<double>(static_cast<td_api::closeChat*>(function.get())->chat_id_)}});
        break;
    case td_api::searchChatMessages::ID:
        reply(id, new td_api::foundChatMessages());
        break;
    case td_api::getScopeNotificationSettings::ID: {
        auto settings = new td_api::scopeNotificationSettings();
        settings->show_preview_ = true;
        reply(id, settings);
        break;
    }
    case td_api::setOption::ID: {
        auto query = static_cast<td_api::setOption*>(function.get());
        if (query->name_ == APP_TOKEN_OPTION && query->value_ && query->value_->get_id() == td_api::optionValueString::ID) {
            QString token = QString::fromStdString(static_cast<td_api::optionValueString*>(query->value_.get())->value_).trimmed();
            QSettings().setValue(APP_TOKEN_KEY, token);
            call(id, "realtime", {{"token", token}});
        } else {
            reply(id, new td_api::ok());
        }
        break;
    }
    case td_api::loadChats::ID:
    case td_api::setNetworkType::ID:
    case td_api::setAutoDownloadSettings::ID:
    case td_api::viewMessages::ID:
    case td_api::setChatDraftMessage::ID:
    case td_api::sendChatAction::ID:
        reply(id, new td_api::ok());
        break;
    default:
        replyError(id, 400, "Not supported by the Slack backend");
    }
}

void SlackBackend::handleSendMessage(uint64_t id, td_api::sendMessage *query)
{
    if (!query->input_message_content_ || query->input_message_content_->get_id() != td_api::inputMessageText::ID) {
        replyError(id, 400, "Only text messages are supported");
        return;
    }

    auto content = static_cast<td_api::inputMessageText*>(query->input_message_content_.get());
    if (!content->text_ || content->text_->text_.empty()) {
        replyError(id, 400, "Message is empty");
        return;
    }

    call(id, "send", {
        {"chat", static_cast<double>(query->chat_id_)},
        {"text", QString::fromStdString(content->text_->text_)}
    });
}

void SlackBackend::handleEditMessage(uint64_t id, td_api::editMessageText *query)
{
    if (!query->input_message_content_ || query->input_message_content_->get_id() != td_api::inputMessageText::ID) {
        replyError(id, 400, "Only text messages are supported");
        return;
    }

    auto content = static_cast<td_api::inputMessageText*>(query->input_message_content_.get());
    if (!content->text_) {
        replyError(id, 400, "Message is empty");
        return;
    }

    call(id, "edit", {
        {"chat", static_cast<double>(query->chat_id_)},
        {"message", static_cast<double>(query->message_id_)},
        {"text", QString::fromStdString(content->text_->text_)}
    });
}

void SlackBackend::handleDeleteMessages(uint64_t id, td_api::deleteMessages *query)
{
    QJsonArray messages;
    for (int64_t messageId : query->message_ids_) {
        messages.append(static_cast<double>(messageId));
    }

    call(id, "delete", {
        {"chat", static_cast<double>(query->chat_id_)},
        {"messages", messages}
    });
}

void SlackBackend::close()
{
    if (_closed || !_api.isLoaded()) return;

    // The library stays loaded, the Go runtime can't be shut down
    _closed = true;
    _receiver.requestInterruption();
    _api.close();
    _receiver.wait(CLOSE_TIMEOUT);
}

void SlackBackend::onReceived(uint64_t id, td_api::Object *object)
{
    bool isError = object->get_id() == td_api::error::ID;

    if (id == STORED_TOKEN_LOGIN_ID) {
        if (isError) {
            auto error = static_cast<td_api::error*>(object);
            qWarning() << "Slack login failed:" << QString::fromStdString(error->message_);

            if (error->code_ == INVALID_TOKEN_CODE) {
                QSettings().remove(TOKEN_KEY);
                emit messageReceived(0, SlackMapper::authorizationWaitToken());
            }
        }

        delete object;
        return;
    }

    if (id != 0 && id == _loginRequestId) {
        if (!isError) QSettings().setValue(TOKEN_KEY, _pendingToken);

        _loginRequestId = 0;
        _pendingToken.clear();
    }

    emit messageReceived(id, object);
}

void SlackBackend::call(uint64_t id, const QString &method, const QJsonObject &params)
{
    if (!_api.isLoaded()) {
        replyError(id, 500, "Slack library is not available");
        return;
    }

    QJsonObject request{
        {"id", static_cast<double>(id)},
        {"method", method},
        {"params", params}
    };

    QByteArray json = QJsonDocument(request).toJson(QJsonDocument::Compact);
    _api.send(json.data());
}

void SlackBackend::reply(uint64_t id, td_api::Object *object)
{
    emit messageReceived(id, object);
}

void SlackBackend::replyError(uint64_t id, int32_t code, const string &message)
{
    auto error = new td_api::error();
    error->code_ = code;
    error->message_ = message;
    reply(id, error);
}
