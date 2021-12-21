#ifndef PROXYMODEL_H
#define PROXYMODEL_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"

class ProxyModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TelegramManager::ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
public:
    enum ProxyRoles {
        IdRole = Qt::UserRole + 1,
        ServerRole,
        PortRole,
        IsEnabledRole,
        UsernameRole,
        PasswordRole,
        TypeRole
    };

    enum ProxyType {
        Socks5,
        HTTP,
        MTProto
    };
    Q_ENUMS(ProxyType)

    struct Proxy {
        int32_t id;
        QString server;
        int32_t port;
        bool enabled;
        QString username;
        QString password;
        ProxyType type;
    };

    explicit ProxyModel(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void getProxies();
    Q_INVOKABLE void enable(int index);
    Q_INVOKABLE void disable();
    Q_INVOKABLE void add();
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void update(int index);
    Q_INVOKABLE void test(int index);
    TelegramManager::ConnectionState connectionState() const;

public slots:
    void gotProxies(td_api::proxies *proxies);
    void onError(int64_t chatId, int32_t type, int32_t subType, int32_t code, QString message);

signals:
    void editError(QString message);
    void addError(QString message);
    void testError(QString message);
    void testSuccessful();
    void connectionStateChanged();

private:
    shared_ptr<TelegramManager> _manager;
    QVector<Proxy> _proxyList;
    td_api::object_ptr<td_api::ProxyType> getProxyType(ProxyType type, QString username, QString password) const;
};

#endif // PROXYMODEL_H
