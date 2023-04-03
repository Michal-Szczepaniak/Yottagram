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
#include <QVector>
#include <QQmlComponent>
#include "components/userfullinfo.h"
#include "components/basicgroupfullinfo.h"
#include "components/supergroupfullinfo.h"
#include "components/secretchatsinfo.h"
#include "components/basicgroupsinfo.h"
#include "components/supergroupsinfo.h"
#include "components/pinnedmessages.h"

class Chat : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int64_t id READ getId NOTIFY idChanged)
    Q_PROPERTY(int64_t idFromType READ getIdFromType NOTIFY idChanged)
    Q_PROPERTY(SecretChatInfo* secretChatInfo READ getSecretChatInfo NOTIFY secretChatChanged)
    Q_PROPERTY(BasicGroupInfo* basicGroupInfo READ getBasicGroupInfo NOTIFY basicGroupChanged)
    Q_PROPERTY(SupergroupInfo* supergroupInfo READ getSupergroupInfo NOTIFY supergroupChanged)
    Q_PROPERTY(UserFullInfo* userFullInfo READ getUserFullInfo NOTIFY idChanged)
    Q_PROPERTY(BasicGroupFullInfo* basicGroupFullInfo READ getBasicGroupFullInfo NOTIFY basicGroupFullInfoChanged)
    Q_PROPERTY(SupergroupFullInfo* supergroupFullInfo READ getSupergroupFullInfo NOTIFY supergroupFullInfoChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY titleChanged)
    Q_PROPERTY(File* smallPhoto READ getSmallPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(File* bigPhoto READ getBigPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(bool hasPhoto READ hasPhoto NOTIFY chatPhotoChanged)
    Q_PROPERTY(int64_t lastReadInboxMessageId READ lastReadInboxMessageId NOTIFY lastReadInboxMessageIdChanged)
    Q_PROPERTY(int64_t lastReadOutboxMessageId READ lastReadOutboxMessageId NOTIFY lastReadOutboxMessageIdChanged)
    Q_PROPERTY(int64_t latestMessageId READ getLatestMessageId NOTIFY latestMessageIdChanged)
    Q_PROPERTY(int64_t lastMessageId READ getLastMessageId NOTIFY lastMessageIdChanged)
    Q_PROPERTY(int32_t unreadCount READ getUnreadCount NOTIFY unreadCountChanged)
    Q_PROPERTY(int32_t unreadMentionCount READ getUnreadMentionCount NOTIFY unreadMentionCountChanged)
    Q_PROPERTY(int64_t firstUnreadMention READ firstUnreadMention NOTIFY firstUnreadMentionChanged)
    Q_PROPERTY(bool isSelf READ isSelf NOTIFY isSelfChanged)
    Q_PROPERTY(int32_t ttl READ getTtl WRITE setTtl NOTIFY ttlChanged)
    Q_PROPERTY(int32_t muteFor READ getMuteFor WRITE setMuteFor NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(int32_t defaultMuteFor READ getDefaultMuteFor NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool showPreview READ getShowPreview WRITE setShowPreview NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultShowPreview READ getDefaultShowPreview NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool disablePinnedMessageNotifications READ getDisablePinnedMessageNotifications WRITE setDisablePinnedMessageNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultDisablePinnedMessageNotifications READ getDefaultDisablePinnedMessageNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool disableMentionNotifications READ getDisableMentionNotifications WRITE setDisableMentionNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool defaultDisableMentionNotifications READ getDefaultDisableMentionNotifications NOTIFY chatNotificationSettingsChanged)
    Q_PROPERTY(bool canSendMessages READ getCanSendMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendMediaMessages READ getCanSendMediaMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendPolls READ getCanSendPolls NOTIFY permissionsChanged)
    Q_PROPERTY(bool canSendOtherMessages READ getCanSendOtherMessages NOTIFY permissionsChanged)
    Q_PROPERTY(bool canAddWebPagePreviews READ getCanAddWebPagePreviews NOTIFY permissionsChanged)
    Q_PROPERTY(bool canChangeInfo READ getCanChangeInfo NOTIFY permissionsChanged)
    Q_PROPERTY(bool canInviteUsers READ getCanInviteUsers NOTIFY permissionsChanged)
    Q_PROPERTY(bool canPinMessages READ getCanPinMessages NOTIFY permissionsChanged)
    Q_PROPERTY(QVector<int32_t> foundChatMembers READ getFoundChatMembers NOTIFY foundChatMembersChanged)
    Q_PROPERTY(QString draftMessage READ draftMessage NOTIFY draftMessageChanged)
    Q_PROPERTY(int64_t draftMessageReplyId READ draftMessageReplyId NOTIFY draftMessageChanged)
    Q_PROPERTY(QString actionText READ actionText NOTIFY actionTextChanged)
public:
    enum MessageRoles {
        TypeRole = Qt::UserRole + 1,
        IdRole,
        MessageRole,
        MessageTypeRole,
        ReceivedRole,
        ReplyMessageIdRole,
        DisplayAvatarRole,
        IsServiceRole,
        IsForwardedRole,
        ForwardUserRole,
        ForwardChatRole,
        ForwardUsernameRole,
        ForwardChannelRole,
        ForwardTimeRole,
        FileRole,
        TimeRole,
        SenderRole,
        SenderIdRole,
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
        ContainsUnreadMentionRole,
        ContainsUnreadReplyRole,
        CallRole,
        LocationRole,
        ContactRole
    };

    enum ChatAction {
        None,
        Typing,
        RecordingVideo,
        UploadingVideo,
        RecordingVoiceNote,
        UploadingVoiceNote,
        UploadingPhoto,
        UploadingDocument,
        ChoosingSticker,
        ChoosingLocation,
        ChoosingContact,
        StartPlayingGame,
        RecordingVideoNote,
        UploadingVideoNote,
        WatchingAnimations,
    };
    Q_ENUMS(ChatAction)

    Chat();
    Chat(td_api::chat* chat, shared_ptr<Files> files);
    ~Chat();

    int64_t getId() const;
    int64_t getIdFromType() const;
    int32_t getSecretChatId() const;
    QString getTitle() const;
    SecretChatInfo* getSecretChatInfo() const;
    BasicGroupInfo* getBasicGroupInfo() const;
    SupergroupInfo* getSupergroupInfo() const;
    UserFullInfo* getUserFullInfo() const;
    BasicGroupFullInfo* getBasicGroupFullInfo() const;
    SupergroupFullInfo* getSupergroupFullInfo() const;
    int64_t getOrder(td_api::ChatList* list) const;
    Q_INVOKABLE QString getChatType() const;
    int32_t getUnreadCount() const;
    void setUnreadCount(int32_t unreadCount);
    int32_t getUnreadMentionCount() const;
    void setUnreadMentionCount(int32_t unreadMentionCount);
    int64_t firstUnreadMention() const;
    void updateMessageMentionRead(int32_t unreadMentionCount, int64_t messageId);
    int64_t lastReadInboxMessageId() const { return _lastReadInboxMessageId; }
    void setLastReadInboxMessageId(int64_t messageId);
    int64_t lastReadOutboxMessageId() const { return _lastReadOutboxMessageId; }
    void setLastReadOutboxMessageId(int64_t messageId);
    void setChatPhoto(td_api::object_ptr<td_api::chatPhotoInfo> chatPhoto);
    bool isSelf() const;
    void setIsOpen(bool isOpen);
    bool isOpen() const;
    bool isPinned(td_api::object_ptr<td_api::ChatList> list) const;
    int32_t getTtl() const;
    int32_t getMuteFor() const;
    void setMuteFor(int32_t muteFor);
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
    bool getCanSendMessages() const;
    bool getCanSendMediaMessages() const;
    bool getCanSendPolls() const;
    bool getCanSendOtherMessages() const;
    bool getCanAddWebPagePreviews() const;
    bool getCanChangeInfo() const;
    bool getCanInviteUsers() const;
    bool getCanPinMessages() const;
    QVector<int32_t> getFoundChatMembers() const;
    void updatePosition(td_api::chatPosition* position);
    QString draftMessage() const;
    int64_t draftMessageReplyId() const;
    Q_INVOKABLE void setDraftMessage(QString message, int64_t replyMessageId);
    void setDraftMessage(td_api::object_ptr<td_api::draftMessage> draftMessage);
    QString actionText();

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);
    void setSecretChatsInfo(shared_ptr<SecretChatsInfo> secretChatsInfo);
    void setBasicGroupsInfo(shared_ptr<BasicGroupsInfo> basicGroupsInfo);
    void setSupergroupsInfo(shared_ptr<SupergroupsInfo> supergroupsInfo);
    Q_INVOKABLE void injectDependencies(PinnedMessages* component);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = TypeRole) const;
    Q_INVOKABLE QHash<int, QByteArray> roleNames() const;

    td_api::chat* getChat();
    Q_INVOKABLE int getMessageIndex(int64_t messageId);
    int64_t getLatestMessageId() const;
    td_api::message* getLastMessage();
    shared_ptr<Message> lastMessage();
    int64_t getLastMessageId() const;
    void setLastMessage(td_api::object_ptr<td_api::message> lastMessage);
    void newMessage(td_api::object_ptr<td_api::message> message, bool addToList = true);
    std::vector<std::int32_t> splitToIntVector(QString string, QString separator);
    std::vector<std::string> splitToVector(QString string, QString separator);

    Q_INVOKABLE void sendMessage(QString message, int64_t replyToMessageId);
    Q_INVOKABLE void getChatHistory(int64_t from_message, int limit = 20, int offset = 0, bool localOnly = false);
    Q_INVOKABLE void scrollToMessage(int64_t messageId);
    Q_INVOKABLE void fetchMessage(int64_t messageId) const;
    Q_INVOKABLE QVariant getMessage(int64_t messageId);
    Q_INVOKABLE void getMoreChatHistory();
    Q_INVOKABLE void getMoreChatMessages();
    Q_INVOKABLE int64_t getAuthorByIndex(int32_t index);
    Q_INVOKABLE void setMessageAsRead(int64_t messageId, int64_t threadId = 0);
    Q_INVOKABLE void sendPhoto(QString path, int64_t replyToMessageId);
    Q_INVOKABLE void sendPhotos(QStringList paths, int64_t replyToMessageId);
    Q_INVOKABLE void sendFile(QString path, int64_t replyToMessageId);
    Q_INVOKABLE void sendMusic(QString path, int64_t replyToMessageId);
    Q_INVOKABLE void sendMusics(QStringList paths, int64_t replyToMessageId);
    Q_INVOKABLE void sendVideo(QString path, int64_t replyToMessageId);
    Q_INVOKABLE void sendVideos(QStringList paths, int64_t replyToMessageId);
    Q_INVOKABLE void sendPoll(QString question, QStringList options, bool anonymous, bool multipleAnswers, bool quizMode, int validAnswer);
    Q_INVOKABLE void sendVoiceNote(QString path, QString waveform, int64_t duration, int64_t replyToMessageId);
    Q_INVOKABLE void sendSticker(int32_t fileId, int64_t replyToMessageId);
    Q_INVOKABLE void sendLocation(float latitude, float longitude, float horizontalAccuracy, int64_t replyToMessageId);
    Q_INVOKABLE void sendAnimation(int32_t fileId, int32_t width, int32_t height, int64_t replyToMessageId);
    Q_INVOKABLE void sendForwardedMessages(QStringList forwardedMessages, int64_t forwardedFrom);
    Q_INVOKABLE void open(QString path);
    Q_INVOKABLE void deleteMessage(int64_t messageId);
    Q_INVOKABLE void editMessageText(int64_t messageId, QString messageText);
    Q_INVOKABLE void chose(int64_t messageId, QString indexes);
    Q_INVOKABLE void openSecretChat();
    Q_INVOKABLE void closeSecretChat();
    Q_INVOKABLE void clearHistory(bool deleteChat = false);
    Q_INVOKABLE void saveToGallery(QString filePath);
    Q_INVOKABLE void pinMessage(int64_t messageId, bool notify = true, bool onlyForSelf = false);
    Q_INVOKABLE void unpinMessage(int64_t messageId);
    Q_INVOKABLE void clearCachedHistory();
    Q_INVOKABLE void sendAction(ChatAction action);
    void setTtl(int32_t ttl);

    bool hasPhoto();
    File* getSmallPhoto();
    File* getBigPhoto();

signals:
    void idChanged();
    void titleChanged();
    void unreadCountChanged(int64_t chatId, int32_t unreadCount);
    void fileUpdated(td_api::updateFile &update_file);
    void chatPhotoChanged(int64_t chatId);
    void lastReadInboxMessageIdChanged(int64_t chatId, int64_t lastReadInboxMessageIdChanged);
    void lastReadOutboxMessageIdChanged(int64_t chatId, int64_t lastReadOutboxMessageIdChanged);
    void basicGroupFullInfoChanged();
    void supergroupFullInfoChanged();
    void isSelfChanged();
    void secretChatChanged(int64_t chatId);
    void basicGroupChanged(int64_t chatId);
    void supergroupChanged(int64_t chatId);
    void ttlChanged(int32_t ttl);
    void chatNotificationSettingsChanged();
    void permissionsChanged();
    void gotMessage(int64_t messageId);
    void lastMessageIdChanged(int64_t lastMessageId);
    void latestMessageIdChanged(int64_t lastMessageId);
    void foundChatMembersChanged();
    void unreadMentionCountChanged(int32_t unreadMentionCount);
    void gotChatHistory();
    void draftMessageChanged();
    void actionTextChanged();
    void firstUnreadMentionChanged();

public slots:
    void updateChatReadInbox(td_api::updateChatReadInbox *updateChatReadInbox);
    void updateChatReadOutbox(td_api::updateChatReadOutbox *updateChatReadOutbox);
    void updateNewMessage(td_api::updateNewMessage *updateNewMessage);
    void updateChatTitle(td_api::updateChatTitle *updateChatTitle);
    void updateDeleteMessages(td_api::updateDeleteMessages *updateDeleteMessages);
    void onMessageContentChanged(int64_t messageId);
    void onMessageIdChanged(int64_t oldMessageId, int64_t newMessageId);
    void updateBasicGroupFullInfo(td_api::updateBasicGroupFullInfo *updateBasicGroupFullInfo);
    void updateSupergroupFullInfo(td_api::updateSupergroupFullInfo *updateSupergroupFullInfo);
    void updateChatNotificationSettings(td_api::updateChatNotificationSettings *updateChatNotificationSettings);
    void scopeNotificationSettingsChanged(td_api::scopeNotificationSettings *scopeNotificationSettings);
    void onGotMessage(td_api::message *message);
    void updateChatPermissions(td_api::updateChatPermissions *updateChatPermissions);
    void updateChatMessageTtl(td_api::updateChatMessageTtl *updateChatMessageTtl);
    void updateChatAction(td_api::updateChatAction *updateChatAction);
    void onGotSearchChatMessagesFilterUnreadMention(int64_t chatId, td_api::messages *messages);

    void onGotChatHistory(int64_t chatId, td_api::messages *messages);

private:
    int32_t _smallPhotoId;
    int32_t _bigPhotoId;
    int32_t _unreadCount;
    int32_t _unreadMentionCount;
    int64_t _lastReadInboxMessageId;
    int64_t _lastReadOutboxMessageId;
    bool _isAuthorized = false;
    bool _isOpen = false;
    td_api::chat *_chat{};
    QVector<int64_t> _message_ids{};
    QHash<int64_t, Message*> _messages{};
    shared_ptr<TelegramManager> _manager{};
    shared_ptr<Users> _users{};
    shared_ptr<Files> _files{};
    shared_ptr<SecretChatsInfo> _secretChatsInfo{};
    shared_ptr<BasicGroupsInfo> _basicGroupsInfo{};
    shared_ptr<SupergroupsInfo> _supergroupsInfo{};
    BasicGroupFullInfo* _basicGroupFullInfo{};
    SupergroupFullInfo* _supergroupFullInfo{};
    td_api::object_ptr<td_api::chatNotificationSettings> _notificationSettings{};
    td_api::scopeNotificationSettings* _scopeNotificationSettings{};
    QVector<int32_t> _foundChatMembers{};
    td_api::chatPosition* _mainPosition{};
    td_api::chatPosition* _archivePosition{};
    QHash<int32_t, td_api::chatPosition*> _filterPositions{};
    QHash<int64_t, ChatAction> _chatActions{};
    QVector<int64_t> _getMessageCache{};
    shared_ptr<Message> _lastMessage{};
    int64_t _firstUnreadMention;
};
Q_DECLARE_METATYPE(Chat*)

#endif // CHAT_H
