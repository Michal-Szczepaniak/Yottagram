#ifndef CHATLISTTOPICS_H
#define CHATLISTTOPICS_H

#include "message.h"

#include <QAbstractListModel>
#include <QObject>
#include <users.h>

#include <core/telegrammanager.h>

class ChatListTopics : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ChatTopicsRoles {
        IdRole = Qt::UserRole + 1,
        ChatIdRole,
        NameRole,
        OrderRole,
        ColorRole,
        UnreadCountRole,
        UnreadMentionCountRole,
        LastMessageRole,
        LastMessageAuthorRole,
        IsPinnedRole,
        IsReadRole,
        IsLockedRole,
        LastMessageTimestampRole,

    };

    explicit ChatListTopics(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setFiles(shared_ptr<Files> files);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

    void setTopics(td_api::forumTopics *topics);
    td_api::forumTopic* getTopic(int64_t topicId);

signals:

public slots:
    void onUpdateForumTopic(td_api::updateForumTopic *updateForumTopic);
    void onUpdateForumTopicInfo(td_api::updateForumTopicInfo *updateForumTopicInfo);

private:
    QString getLastMessage(td_api::forumTopic *topic) const;
    QString getLastMessageAuthor(td_api::forumTopic *topic) const;
    QString getLastMessageTimestamp(td_api::forumTopic *topic) const;
    int getTopicIndex(int64_t topicId);

private:
    shared_ptr<TelegramManager> _manager{};
    shared_ptr<Users> _users{};
    shared_ptr<Files> _files{};
    QHash<int64_t, td_api::forumTopic*> _topics;
    QVector<int64_t> _topicIds;
    QHash<int64_t, Message*> _messages;
};

#endif // CHATLISTTOPICS_H
