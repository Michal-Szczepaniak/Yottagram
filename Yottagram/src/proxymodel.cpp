#include "proxymodel.h"

ProxyModel::ProxyModel(QObject *parent) : QAbstractListModel(parent)
{

}

void ProxyModel::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateChatFilters, this, &ChatListFilters::updateChatFilters);
}

int ProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _filterChatLists.size();
}

QVariant ProxyModel::data(const QModelIndex &index, int role) const
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

QHash<int, QByteArray> ProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    return roles;
}
