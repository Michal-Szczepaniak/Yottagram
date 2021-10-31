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
#include "files/photo.h"
#include "files/sticker.h"
#include "files/video.h"
#include "files/document.h"
#include "files/audio.h"
#include "files/animation.h"
#include "files/voicenote.h"
#include "files/videonote.h"
#include "users.h"
#include "webpage.h"
#include "poll.h"

class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText NOTIFY messageChanged)
    Q_PROPERTY(QString type READ getType NOTIFY messageChanged)
    Q_PROPERTY(int32_t senderUserId READ getSenderUserId NOTIFY messageChanged)
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
    int32_t getSenderUserId();
    QString getFormattedTimestamp();
    QString getFormattedForwardTimestamp();

    bool hasWebPage() const;
    WebPage* getWebPage() const;
    Poll* getPoll() const;
    Photo* getPhoto() const;
    Sticker* getSticker() const;
    Video* getVideo() const;
    Document* getDocument() const;
    Audio* getAudio() const;
    Animation* getAnimation() const;
    VoiceNote* getVoiceNote() const;
    VideoNote* getVideoNote() const;

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
    WebPage* _webPage;
    Poll* _poll;
    Photo* _photo;
    Sticker* _sticker;
    Video* _video;
    Document* _document;
    Audio* _audio;
    Animation* _animation;
    VoiceNote* _voiceNote;
    VideoNote* _videoNote;
};

#endif // MESSAGE_H
