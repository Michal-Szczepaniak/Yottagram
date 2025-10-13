#include "chatlistfolders.h"

ChatListFolders::ChatListFolders(QObject *parent) : QAbstractListModel(parent)
{

}

void ChatListFolders::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateChatFolders, this, &ChatListFolders::updateChatFolders);
}

int ChatListFolders::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _folderChatLists.size();
}

QVariant ChatListFolders::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    ChatList list = _folderChatLists[index.row()];

    switch (role) {
    case ChatListRoles::IdRole:
        return list.id;
        break;
    case ChatListRoles::NameRole:
        return list.name.toHtmlEscaped();
        break;
    case ChatListRoles::IconNameRole:
        return list.iconName;
        break;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatListFolders::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconNameRole] = "iconName";
    return roles;
}

void ChatListFolders::updateChatFolders(td_api::updateChatFolders *updateChatFolders)
{
    beginResetModel();
    _folderChatLists.clear();
    for (td_api::object_ptr<td_api::chatFolderInfo> &chatFolderInfo : updateChatFolders->chat_folders_) {
        _folderChatLists.append({chatFolderInfo->id_, QString::fromStdString(chatFolderInfo->name_->text_->text_), QString::fromStdString(chatFolderInfo->icon_->name_)});
    }
    endResetModel();
}
