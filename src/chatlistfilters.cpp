#include "chatlistfilters.h"

ChatListFilters::ChatListFilters(QObject *parent) : QAbstractListModel(parent)
{

}

void ChatListFilters::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateChatFilters, this, &ChatListFilters::updateChatFilters);
}

int ChatListFilters::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _filterChatLists.size();
}

QVariant ChatListFilters::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    ChatList list = _filterChatLists[index.row()];

    switch (role) {
    case ChatListRoles::IdRole:
        return list.id;
        break;
    case ChatListRoles::NameRole:
        return list.name;
        break;
    case ChatListRoles::IconNameRole:
        return list.iconName;
        break;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatListFilters::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconNameRole] = "iconName";
    return roles;
}

void ChatListFilters::updateChatFilters(td_api::updateChatFilters *updateChatFilters)
{
    beginResetModel();
    _filterChatLists.clear();
    for (td_api::object_ptr<td_api::chatFilterInfo> &chatFilterInfo : updateChatFilters->chat_filters_) {
        _filterChatLists.append({chatFilterInfo->id_, QString::fromStdString(chatFilterInfo->title_), QString::fromStdString(chatFilterInfo->icon_name_)});
    }
    endResetModel();
}
