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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include "core/telegrammanager.h"
#include "files/files.h"
#include "files/file.h"
#include "contents/photo.h"
#include "contents/sticker.h"
#include "contents/video.h"
#include "contents/document.h"
#include "contents/audio.h"
#include "contents/animation.h"
#include "contents/voicenote.h"
#include "contents/videonote.h"
#include "users.h"
#include "webpage.h"
#include "contents/poll.h"
#include "contents/call.h"
#include "contents/location.h"
#include "contents/contact.h"
#include "contents/contentinterface.h"
#include <variant>

class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText NOTIFY messageChanged)
    Q_PROPERTY(QString type READ getType NOTIFY messageChanged)
    Q_PROPERTY(int64_t senderUserId READ getSenderUserId NOTIFY messageChanged)
public:
    explicit Message(QObject *parent = nullptr);
    ~Message();

    td_api::message *message() const;
    void setMessage(td_api::message *message);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);

    int64_t getId();
    QString getText();
    QString getType();
    int32_t getContentType();
    bool isEdited();
    bool canBeEdited();
    bool canBeForwarded();
    bool canBeDeleted();
    bool isChannelPost();
    int getEditedDate();
    int getViews();
    bool received();
    int32_t getDeleteMemberId();
    QVector<int32_t> getAddMembersIds();
    int64_t replyMessageId();
    td_api::messageForwardInfo* getForwardedInfo();
    int64_t getSenderUserId();
    QString getFormattedTimestamp();
    QString getFormattedForwardTimestamp();
    bool containsUnreadMention() const;
    void setContainsUnreadMention(bool containsUnreadMention);

    bool hasWebPage() const;
    WebPage* getWebPage() const;
    template<class T>
    T* getContent() {
        if (dynamic_cast<T*>(_messageContent))
            return static_cast<T*>(_messageContent);
        else
            return nullptr;
    }

    void handleMessageContent(td_api::object_ptr<td_api::MessageContent> messageContent);

    int64_t getChatId() const;
    void setChatId(const int64_t &chatId);

signals:
    void contentChanged(int64_t messageId);
    void messageIdChanged(int64_t oldMessageId, int64_t newMessageId);
    void messageChanged();

public slots:
    void updateMessageSendSucceeded(td_api::updateMessageSendSucceeded *updateMessageSendSucceeded);
    void updateMessageContent(td_api::updateMessageContent *updateMessageContent);

private:
    int64_t _chatId;
    td_api::message* _message;
    int32_t _contentTypeId;
    QVector<int32_t> _file_ids;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
    shared_ptr<Files> _files;
    td_api::object_ptr<td_api::messageText> _text;
    ContentInterface* _messageContent;
    WebPage* _webPage;
};

#endif // MESSAGE_H
