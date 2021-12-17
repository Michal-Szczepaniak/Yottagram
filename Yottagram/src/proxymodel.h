#ifndef PROXYMODEL_H
#define PROXYMODEL_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"

class ProxyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ChatListRoles {
        IdRole = Qt::UserRole + 1,
        NameRole
    };

    struct Proxy {
        int32_t id;
        QString server;
        QString port;
    };

    explicit ProxyModel(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void updateChatFilters(td_api::updateChatFilters *updateChatFilters);

private:
    shared_ptr<TelegramManager> _manager;
    QVector<Proxy> _filterChatLists;
};

#endif // PROXYMODEL_H
