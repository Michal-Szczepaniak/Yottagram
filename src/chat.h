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

#ifndef CHAT_H
#define CHAT_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"
#include "files/files.h"
#include "files/file.h"
#include "users.h"
#include "message.h"
#include <memory>
#include <QUrl>
#include "components/userfullinfo.h"
#include "components/basicgroupfullinfo.h"
#include "components/supergroupfullinfo.h"

class Chat : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(qint64 id READ getId NOTIFY idChanged)
    Q_PROPERTY(qint64 idFromType READ getIdFromType NOTIFY idChanged)
    Q_PROPERTY(UserFullInfo* userFullInfo READ getUserFullInfo NOTIFY idChanged)
    Q_PROPERTY(BasicGroupFullInfo* basicGroupFullInfo READ getBasicGroupFullInfo NOTIFY basicGroupFullInfoChanged)
    Q_PROPERTY(SupergroupFullInfo* supergroupFullInfo READ getSupergroupFullInfo NOTIFY supergroupFullInfoChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(File* smallPhoto READ getSmallPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(File* bigPhoto READ getBigPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(bool hasPhoto READ hasPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(qint64 lastReadInboxMessageId READ lastReadInboxMessageId NOTIFY lastReadInboxMessageIdChanged)
    Q_PROPERTY(qint64 lastReadOutboxMessageId READ lastReadOutboxMessageId NOTIFY lastReadOutboxMessageIdChanged)
    Q_PROPERTY(qint64 latestMessageId READ getLatestMessageId NOTIFY latestMessageIdChanged)
    Q_PROPERTY(qint64 lastMessageId READ getLastMessageId NOTIFY lastMessageIdChanged)
    Q_PROPERTY(qint32 unreadCount READ getUnreadCount NOTIFY unreadCountChanged)
    Q_PROPERTY(bool isSelf READ isSelf NOTIFY isSelfChanged)
    Q_PROPERTY(QString secretChatState READ getSecretChatState NOTIFY secretChatChanged)
    Q_PROPERTY(qint32 ttl READ getTtl WRITE setTtl NOTIFY ttlChanged)
    Q_PROPERTY(qint32 muteFor READ getMuteFor WRITE setMuteFor NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(qint32 defaultMuteFor READ getDefaultMuteFor NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool showPreview READ getShowPreview WRITE setShowPreview NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultShowPreview READ getDefaultShowPreview NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool disablePinnedMessageNotifications READ getDisablePinnedMessageNotifications WRITE setDisablePinnedMessageNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultDisablePinnedMessageNotifications READ getDefaultDisablePinnedMessageNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool disableMentionNotifications READ getDisableMentionNotifications WRITE setDisableMentionNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultDisableMentionNotifications READ getDefaultDisableMentionNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(qint64 pinnedMessageId READ getPinnedMessageId NOTIFY pinnedMessageIdChanged)
    Q_PROPERTY(Message* pinnedMessage READ getPinnedMessage NOTIFY pinnedMessageIdChanged)
    Q_PROPERTY(bool canSendMessages READ getCanSendMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendMediaMessages READ getCanSendMediaMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendPolls READ getCanSendPolls NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendOtherMessages READ getCanSendOtherMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canAddWebPagePreviews READ getCanAddWebPagePreviews NOTIFY permissionsChanged)
    Q_PROPERTY(bool canChangeInfo READ getCanChangeInfo NOTIFY permissionsChanged)
    Q_PROPERTY(bool canInviteUsers READ getCanInviteUsers NOTIFY permissionsChanged)
    Q_PROPERTY(bool canPinMessages READ getCanPinMessages NOTIFY permissionsChanged)
public:
    enum MessageRoles {
        TypeRole = Qt::UserRole + 1,
        IdRole,
        MessageRole,
        MessageTypeRole,
        ReceivedRole,
        ReplyMessageIdRole,
        IsForwardedRole,
        ForwardUserRole,
        ForwardUsernameRole,
        ForwardChannelRole,
        ForwardTimeRole,
        FileRole,
        TimeRole,
        AuthorIdRole,
        EditedRole,
        CanBeEditedRole,
        CanBeForwardedRole,
        CanBeDeletedRole,
        IsChannelPostRole,
        EditedDateRole,
        ViewsRole,
        ReadRole,
        HasWebPageRole,
        WebPageRole,
        PollRole,
    };

    enum ChatList {
        Main,
        Archive
    };

    Chat(td_api::chat* chat, shared_ptr<Files> files);
    ~Chat();

    qint64 getId() const;
    qint32 getIdFromType() const;
    qint32 getSecretChatId() const;
    QString getTitle() const;
    UserFullInfo* getUserFullInfo() const;
    BasicGroupFullInfo* getBasicGroupFullInfo() const;
    SupergroupFullInfo* getSupergroupFullInfo() const;
    qint64 getOrder() const;
    void setOrder(int64_t order);
    Q_INVOKABLE QString getChatType() const;
    int getChatList() const;
    qint32 getUnreadCount() const;
    void setUnreadCount(qint32 unreadCount);
    qint64 lastReadInboxMessageId() const { return _lastReadInboxMessageId; }
    void setLastReadInboxMessageId(qint64 messageId);
    qint64 lastReadOutboxMessageId() const { return _lastReadOutboxMessageId; }
    void setLastReadOutboxMessageId(qint64 messageId);
    void setChatPhoto(td_api::object_ptr<td_api::chatPhoto> chatPhoto);
    bool isSelf() const;
    void setIsOpen(bool isOpen);
    bool isOpen() const;
    void setSecretChat(td_api::secretChat* secretChat);
    QString getSecretChatState() const;
    bool isPinned() const;
    void setIsPinned(bool isPinned);
    qint32 getTtl() const;
    qint32 getMuteFor() const;
    void setMuteFor(qint32 muteFor);
    bool getDefaultMuteFor() const;
    bool getShowPreview() const;
    void setShowPreview(bool showPreview);
    bool getDefaultShowPreview() const;
    bool getDisablePinnedMessageNotifications() const;
    void setDisablePinnedMessageNotifications(bool disablePinnedMessageNotifications);
    bool getDefaultDisablePinnedMessageNotifications() const;
    bool getDisableMentionNotifications() const;
    void setDisableMentionNotifications(bool disableMentionNotifications);
    bool getDefaultDisableMentionNotifications() const;
    qint64 getPinnedMessageId();
    Message* getPinnedMessage();
    bool getCanSendMessages() const;
    bool getCanSendMediaMessages() const;
    bool getCanSendPolls() const;
    bool getCanSendOtherMessages() const;
    bool getCanAddWebPagePreviews() const;
    bool getCanChangeInfo() const;
    bool getCanInviteUsers() const;
    bool getCanPinMessages() const;

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    Q_INVOKABLE QHash<int, QByteArray> roleNames() const;

    td_api::chat* getChat();
    Q_INVOKABLE int getMessageIndex(qint64 messageId);
    qint64 getLatestMessageId() const;
    td_api::message* getLastMessage();
    qint64 getLastMessageId() const;
    void setLastMessage(td_api::object_ptr<td_api::message> lastMessage);
    void newMessage(td_api::object_ptr<td_api::message> message, bool addToList = true);
    std::vector<std::int32_t> splitToIntVector(QString string, QString separator);
    std::vector<std::string> splitToVector(QString string, QString separator);

    Q_INVOKABLE void sendMessage(QString message, qint64 replyToMessageId);
    Q_INVOKABLE void getChatHistory(qint64 from_message, int limit = 20, int offset = 0, bool localOnly = false);
    Q_INVOKABLE void scrollToMessage(qint64 messageId);
    Q_INVOKABLE void fetchMessage(qint64 messageId);
    Q_INVOKABLE QVariant getMessage(qint64 messageId);
    Q_INVOKABLE void getMoreChatHistory();
    Q_INVOKABLE void getMoreChatMessages();
    Q_INVOKABLE qint64 getAuthorByIndex(qint32 index);
    Q_INVOKABLE void setMessageAsRead(qint64 messageId);
    Q_INVOKABLE void sendPhoto(QString path, qint64 replyToMessageId);
    Q_INVOKABLE void sendFile(QString path, qint64 replyToMessageId);
    Q_INVOKABLE void sendMusic(QString path, qint64 replyToMessageId);
    Q_INVOKABLE void sendVideo(QString path, qint64 replyToMessageId);
    Q_INVOKABLE void sendPoll(QString question, QStringList options, bool anonymous, bool multipleAnswers, bool quizMode, int validAnswer);
    Q_INVOKABLE void sendVoiceNote(QString path, QString waveform, qint64 duration, qint64 replyToMessageId);
    Q_INVOKABLE void sendSticker(qint32 fileId, qint64 replyToMessageId);
    Q_INVOKABLE void sendForwardedMessages(QStringList forwardedMessages, qint64 forwardedFrom);
    Q_INVOKABLE void open(QString path);
    Q_INVOKABLE void deleteMessage(qint64 messageId);
    Q_INVOKABLE void editMessageText(qint64 messageId, QString messageText);
    Q_INVOKABLE void chose(qint64 messageId, QString indexes);
    Q_INVOKABLE void openSecretChat();
    Q_INVOKABLE void closeSecretChat();
    Q_INVOKABLE void clearHistory(bool deleteChat = false);
    Q_INVOKABLE void saveToGallery(QString filePath);
    Q_INVOKABLE void pinMessage(qint64 messageId, bool notify);
    void setTtl(qint32 ttl);

    bool hasPhoto();
    File* getSmallPhoto();
    File* getBigPhoto();

signals:
    void idChanged();
    void titleChanged();
    void unreadCountChanged(qint64 chatId, qint32 unreadCount);
    void fileUpdated(td_api::updateFile &update_file);
    void chatPhotoChanged(qint64 chatId);
    void lastReadInboxMessageIdChanged(qint64 lastReadInboxMessageIdChanged);
    void lastReadOutboxMessageIdChanged(qint64 lastReadOutboxMessageIdChanged);
    void basicGroupFullInfoChanged();
    void supergroupFullInfoChanged();
    void isSelfChanged();
    void secretChatChanged(qint64 chatId);
    void ttlChanged(qint32 ttl);
    void chatNotificationSettingsChanged();
    void pinnedMessageIdChanged();
    void permissionsChanged();
    void gotMessage(qint64 messageId);
    void lastMessageIdChanged(qint64 lastMessageId);
    void latestMessageIdChanged(qint64 lastMessageId);

public slots:
    void updateChatReadInbox(td_api::updateChatReadInbox *updateChatReadInbox);
    void updateChatReadOutbox(td_api::updateChatReadOutbox *updateChatReadOutbox);
    void messages(td_api::messages *messages);
    void updateNewMessage(td_api::updateNewMessage *updateNewMessage);
    void updateChatTitle(td_api::updateChatTitle *updateChatTitle);
    void updateDeleteMessages(td_api::updateDeleteMessages *updateDeleteMessages);
    void onMessageContentChanged(qint64 messageId);
    void onMessageIdChanged(qint64 oldMessageId, qint64 newMessageId);
    void updateBasicGroupFullInfo(td_api::updateBasicGroupFullInfo *updateBasicGroupFullInfo);
    void updateSupergroupFullInfo(td_api::updateSupergroupFullInfo *updateSupergroupFullInfo);
    void updateSecretChat(td_api::updateSecretChat *updateSecretChat);
    void updateChatNotificationSettings(td_api::updateChatNotificationSettings *updateChatNotificationSettings);
    void scopeNotificationSettingsChanged(td_api::scopeNotificationSettings *scopeNotificationSettings);
    void updateChatPinnedMessage(td_api::updateChatPinnedMessage *updateChatPinnedMessage);
    void onGotMessage(td_api::message *message);
    void updateChatPermissions(td_api::updateChatPermissions *updateChatPermissions);

private:
    qint32 _smallPhotoId;
    qint32 _bigPhotoId;
    qint32 _unreadCount;
    qint64 _lastReadInboxMessageId;
    qint64 _lastReadOutboxMessageId;
    bool _isAuthorized = false;
    bool _isOpen = false;
    td_api::chat *_chat;
    QVector<qint64> _message_ids;
    QMap<qint64, Message*> _messages;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
    shared_ptr<Files> _files;
    BasicGroupFullInfo* _basicGroupFullInfo;
    SupergroupFullInfo* _supergroupFullInfo;
    td_api::secretChat* _secretChat;
    td_api::object_ptr<td_api::chatNotificationSettings> _notificationSettings;
    td_api::scopeNotificationSettings* _scopeNotificationSettings;
};
Q_DECLARE_METATYPE(Chat*)

#endif // CHAT_H
