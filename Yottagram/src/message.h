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
#include <textentityprocessor.h>
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
#include "linkpreview.h"
#include "customemojis.h"
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
    Q_PROPERTY(QString typeText READ getTypeText NOTIFY messageChanged)
    Q_PROPERTY(QString unformattedText READ getTextUnformatted NOTIFY messageChanged)
    Q_PROPERTY(QString type READ getType NOTIFY messageChanged)
    Q_PROPERTY(QString sender READ getSender NOTIFY messageChanged)
    Q_PROPERTY(int64_t senderChatId READ getSenderChatId NOTIFY messageChanged)
    Q_PROPERTY(int64_t senderUserId READ getSenderUserId NOTIFY messageChanged)
public:
    explicit Message(QObject *parent = nullptr);
    ~Message();

    td_api::message *message() const;
    void setMessage(td_api::message *message, bool handleContent = true);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    void setCustomEmojis(shared_ptr<CustomEmojis> customEmojis);
    void setTextEntityProcessor(TextEntityProcessor* processor);

    int64_t getId();
    QString getText(bool formatted = true);
    QString getTextUnformatted() { return getText(false); }
    QString getEditText();
    QString getType() const;
    QString getTypeText();
    int32_t getContentType();
    bool isService() const;
    bool isEdited();
    bool canBeEdited();
    bool canBeForwarded();
    bool canBeDeleted();
    bool isChannelPost();
    int getEditedDate();
    int getViews();
    bool received();
    int32_t getDeleteMemberId();
    QVector<int64_t> getAddMembersIds();
    int64_t replyMessageId();
    td_api::messageForwardInfo* getForwardedInfo();
    int64_t getSenderUserId();
    int64_t getSenderChatId();
    QString getSender();
    int32_t getTimestamp();
    QString getFormattedTimestamp();
    QString getFormattedForwardTimestamp();
    bool containsUnreadMention() const;
    void setContainsUnreadMention(bool containsUnreadMention);
    void fetchProperties();
    td_api::MessageTopic* getTopicId();
    void updateInteractionInfo(td_api::object_ptr<td_api::messageInteractionInfo> interactionInfo);
    void setMessageProperties(td_api::messageProperties *messageProperties);
    td_api::messageReactions* getReactions();
    void reformatMessage();
    void setEditDate(int32_t editDate);

    bool hasWebPage() const;
    LinkPreview* getWebPage() const;
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

    void updateCustomEmojis(QVector<int32_t> fileIds);

signals:
    void contentChanged(int64_t messageId);
    void messageIdChanged(int64_t oldMessageId, int64_t newMessageId);
    void messageChanged();
    void messageFormatedChanged(int64_t messageId);
    void reactionsChanged(int64_t messageId);

public slots:
    void updateMessageSendSucceeded(td_api::updateMessageSendSucceeded *updateMessageSendSucceeded);
    void updateMessageContent(td_api::updateMessageContent *updateMessageContent);
    void onEntityLocalPathChanged(QString localPath, int32_t fileId);
    void onReactionLocalPathChanged(QString localPath, int32_t fileId);
    void onReformatMessage();

private:
    QString formatTextAsHTML(td_api::formattedText* formattedText);

private:
    int64_t _chatId;
    td_api::message* _message;
    td_api::messageProperties* _properties;
    int32_t _contentTypeId;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
    shared_ptr<Files> _files;
    shared_ptr<CustomEmojis> _customEmojis;
    td_api::object_ptr<td_api::messageText> _text;
    ContentInterface* _messageContent;
    LinkPreview* _linkPreview;
    QString _formattedMessage;
    QTimer _reformatTimer;
    QTimer _reactionsUpdateTimer;
    TextEntityProcessor* _entityProcessor = nullptr;
};

#endif // MESSAGE_H
