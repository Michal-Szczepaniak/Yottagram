#include "chatlisttopics.h"
#include "message.h"

#include <QColor>

ChatListTopics::ChatListTopics(QObject *parent) : QAbstractListModel(parent)
{

}

void ChatListTopics::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateForumTopic, this, &ChatListTopics::onUpdateForumTopic);
    connect(_manager.get(), &TelegramManager::updateForumTopicInfo, this, &ChatListTopics::onUpdateForumTopicInfo);
}

void ChatListTopics::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void ChatListTopics::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void ChatListTopics::setCustomEmojis(shared_ptr<CustomEmojis> customEmojis)
{
    _customEmojis = customEmojis;
}

int ChatListTopics::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _topicIds.size();
}

QVariant ChatListTopics::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    td_api::forumTopic *topic = _topics[_topicIds[index.row()]];

    switch (role) {
    case ChatTopicsRoles::IdRole:
        return QVariant::fromValue(topic->info_->forum_topic_id_);
    case ChatTopicsRoles::ChatIdRole:
        return QVariant::fromValue(topic->info_->chat_id_);
    case ChatTopicsRoles::NameRole:
        return QString::fromStdString(topic->info_->name_);
    case ChatTopicsRoles::OrderRole:
        return QVariant::fromValue(topic->order_);
    case ChatTopicsRoles::ColorRole:
    {
        int color = topic->info_->icon_->color_;

        return QColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }
    case ChatTopicsRoles::UnreadCountRole:
        return topic->unread_count_;
    case ChatTopicsRoles::UnreadMentionCountRole:
        return topic->unread_mention_count_;
    case ChatTopicsRoles::LastMessageRole:
        return getLastMessage(topic).toHtmlEscaped();
    case ChatTopicsRoles::LastMessageAuthorRole:
        return getLastMessageAuthor(topic);
    case ChatTopicsRoles::IsPinnedRole:
        return topic->is_pinned_;
    case ChatTopicsRoles::IsReadRole:
    {
        td_api::message* message = topic->last_message_.get();
        if (message == nullptr) return false;

        if (message->is_outgoing_) {
            return message->id_ <= topic->last_read_outbox_message_id_;
        }  else {
            return message->id_ <= topic->last_read_inbox_message_id_;
        }
    }
    case ChatTopicsRoles::IsLockedRole:
        return topic->info_->is_closed_;
    case ChatTopicsRoles::LastMessageTimestampRole:
    {
        return getLastMessageTimestamp(topic);
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatListTopics::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ChatIdRole] = "chatId";
    roles[NameRole] = "name";
    roles[OrderRole] = "order";
    roles[ColorRole] = "iconColor";
    roles[UnreadCountRole] = "unreadCount";
    roles[UnreadMentionCountRole] = "unreadMentionCount";
    roles[LastMessageAuthorRole] = "lastMessageAuthor";
    roles[LastMessageRole] = "lastMessage";
    roles[IsPinnedRole] = "isPinned";
    roles[IsReadRole] = "isRead";
    roles[IsLockedRole] = "isLocked";
    roles[LastMessageTimestampRole] = "lastMessageTimestamp";
    return roles;
}

void ChatListTopics::setTopics(td_api::forumTopics *topics)
{
    beginResetModel();

    for (td_api::forumTopic* topic : _topics) {
        delete topic;
    }

    for (Message *message : _messages) {
        delete message;
    }

    _topics.clear();
    _topicIds.clear();
    _messages.clear();

    for (td_api::object_ptr<td_api::forumTopic> &topic : topics->topics_) {
        int64_t topicId = topic->info_->forum_topic_id_;
        _topicIds.append(topicId);
        _topics[topicId] = topic.release();

        Message *message = new Message;
        message->setTelegramManager(_manager);
        message->setUsers(_users);
        message->setFiles(_files);
        message->setCustomEmojis(_customEmojis);
        message->setChatId(_topics[topicId]->info_->chat_id_);
        message->setMessage(_topics[topicId]->last_message_.get());

        _messages[topicId] = message;
    }

    endResetModel();
}

td_api::forumTopic* ChatListTopics::getTopic(int64_t topicId)
{
    if (!_topics.contains(topicId)) return nullptr;

    return _topics[topicId];
}

void ChatListTopics::onUpdateForumTopic(td_api::updateForumTopic *updateForumTopic)
{
    if (!_topicIds.contains(updateForumTopic->message_thread_id_)) return;

    td_api::forumTopic* topic =_topics[updateForumTopic->message_thread_id_];
    topic->is_pinned_ = updateForumTopic->is_pinned_;
    topic->last_read_inbox_message_id_ = updateForumTopic->last_read_inbox_message_id_;
    topic->last_read_outbox_message_id_ = updateForumTopic->last_read_outbox_message_id_;
    topic->unread_mention_count_ = updateForumTopic->unread_mention_count_;
    topic->unread_reaction_count_ = updateForumTopic->unread_reaction_count_;

    int index = getTopicIndex(updateForumTopic->message_thread_id_);
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {});
}

void ChatListTopics::onUpdateForumTopicInfo(td_api::updateForumTopicInfo *updateForumTopicInfo)
{
    if (!updateForumTopicInfo->info_ || !_topicIds.contains(updateForumTopicInfo->info_->message_thread_id_)) return;

    td_api::forumTopic* topic =_topics[updateForumTopicInfo->info_->message_thread_id_];
    if (topic->info_.get() == updateForumTopicInfo->info_.get()) return;

    topic->info_ = move(updateForumTopicInfo->info_);

    int index = getTopicIndex(topic->info_->message_thread_id_);
    emit dataChanged(createIndex(index, 0), createIndex(index, 0), {});
}

QString ChatListTopics::getLastMessage(td_api::forumTopic *topic) const
{
    return _messages[topic->info_->message_thread_id_]->getTypeText();
}

QString ChatListTopics::getLastMessageAuthor(td_api::forumTopic *topic) const
{
    td_api::message* message = topic->last_message_.get();
    if (message == nullptr) return "";

    QString lastMessageInfo = "";

    if (message->is_outgoing_) {
        lastMessageInfo += "You: ";
    } else {
        QString name;
        if (message->sender_id_->get_id() == td_api::messageSenderUser::ID) {
            shared_ptr<User> user = _users->getUser(static_cast<td_api::messageSenderUser*>(message->sender_id_.get())->user_id_);
            if(user != nullptr) name = user->getName();
        } else if (message->sender_id_->get_id() == td_api::messageSenderChat::ID){
            name = "Chat";
        }

        if(name != "") {
            lastMessageInfo += name.trimmed() + ": ";
        }
    }

    return lastMessageInfo;
}

QString ChatListTopics::getLastMessageTimestamp(td_api::forumTopic *topic) const
{
    td_api::message* message = topic->last_message_.get();
    if (message == nullptr) return "";

    qint64 timestamp = message->date_;
    qint64 current = QDateTime::currentMSecsSinceEpoch()/1000;

    auto diff = current - timestamp;
    if (diff >= 604800) return QDateTime::fromMSecsSinceEpoch(timestamp*1000).toString("yyyy.MM.dd");
    else if (diff >= 86400) return QDateTime::fromMSecsSinceEpoch(timestamp*1000).toString("ddd");
    return QDateTime::fromMSecsSinceEpoch(timestamp*1000).toString("hh:mm");
}

int ChatListTopics::getTopicIndex(int64_t topicId)
{
    if(rowCount() <= 0) return -1;

    auto it = std::find(_topicIds.begin(), _topicIds.end(), topicId);

    if (it != _topicIds.end()) {
        return std::distance(_topicIds.begin(), it);
    }

    return -1;
}
