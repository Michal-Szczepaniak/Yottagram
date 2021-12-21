#include "proxymodel.h"
#include "overloaded.h"

ProxyModel::ProxyModel(QObject *parent) : QAbstractListModel(parent)
{

}

void ProxyModel::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::gotProxies, this, &ProxyModel::gotProxies);
    connect(_manager.get(), &TelegramManager::error, this, &ProxyModel::onError);
    connect(_manager.get(), &TelegramManager::proxyTestSuccessful, this, &ProxyModel::testSuccessful);
    connect(_manager.get(), &TelegramManager::connectionStateChanged, this, &ProxyModel::connectionStateChanged);
}

int ProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _proxyList.size();
}

QVariant ProxyModel::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    Proxy proxy = _proxyList[index.row()];

    switch (role) {
    case ProxyRoles::IdRole:
        return proxy.id;
    case ProxyRoles::ServerRole:
        return proxy.server;
    case ProxyRoles::PortRole:
        return proxy.port;
    case ProxyRoles::IsEnabledRole:
        return proxy.enabled;
    case ProxyRoles::UsernameRole:
        return proxy.username;
    case ProxyRoles::PasswordRole:
        return proxy.password;
    case ProxyRoles::TypeRole:
        return proxy.type;
    default:
        return QVariant();
    }
}

bool ProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return false;

    Proxy &proxy = _proxyList[index.row()];

    switch (role) {
    case ProxyRoles::ServerRole:
        proxy.server = value.toString();
        break;
    case ProxyRoles::PortRole:
        proxy.port = value.toInt();
        break;
    case ProxyRoles::UsernameRole:
        proxy.username = value.toString();
        break;
    case ProxyRoles::PasswordRole:
        proxy.password = value.toString();
        break;
    case ProxyRoles::TypeRole:
        proxy.type = (ProxyType)value.toInt();
        break;
    default:
        return false;
    }
    return true;
}

QHash<int, QByteArray> ProxyModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ServerRole] = "server";
    roles[PortRole] = "port";
    roles[IsEnabledRole] = "isEnabled";
    roles[UsernameRole] = "username";
    roles[PasswordRole] = "password";
    roles[TypeRole] = "type";
    return roles;
}

void ProxyModel::getProxies()
{
    _manager->sendQueryWithRespone(0, td_api::getProxies::ID, 0, new td_api::getProxies());
}

void ProxyModel::enable(int index)
{
    if (rowCount() <= 0 || index >= rowCount()) return;

    Proxy proxy = _proxyList[index];

    _manager->sendQuery(new td_api::enableProxy(proxy.id));
    getProxies();
}

void ProxyModel::disable()
{
    _manager->sendQuery(new td_api::disableProxy());
    getProxies();
}

void ProxyModel::test(int index)
{
    if (rowCount() <= 0 || index >= rowCount()) return;

    Proxy proxy = _proxyList[index];

    _manager->sendQueryWithRespone(0, td_api::testProxy::ID, 0, new td_api::testProxy(proxy.server.toStdString(), proxy.port, getProxyType(proxy.type, proxy.username, proxy.password), -1, 5000));
}

TelegramManager::ConnectionState ProxyModel::connectionState() const
{
    return _manager->getConnectionState();
}

void ProxyModel::add()
{
    _manager->sendQuery(new td_api::addProxy("new proxy", 1234, false, td_api::make_object<td_api::proxyTypeSocks5>()));

    getProxies();
}

void ProxyModel::remove(int index)
{
    if (rowCount() <= 0 || index >= rowCount()) return;

    Proxy proxy = _proxyList[index];

    _manager->sendQuery(new td_api::removeProxy(proxy.id));

    getProxies();
}

void ProxyModel::update(int index)
{
    if (rowCount() <= 0 || index >= rowCount()) return;

    Proxy proxy = _proxyList[index];
    td_api::editProxy *req = new td_api::editProxy();
    req->proxy_id_ = proxy.id;
    req->server_ = proxy.server.toStdString();
    req->port_ = proxy.port;
    req->type_ = getProxyType(proxy.type, proxy.username, proxy.password);
    _manager->sendQuery(req);
    getProxies();
}

void ProxyModel::gotProxies(td_api::proxies *proxies)
{
    beginResetModel();
    _proxyList.clear();
    for (td_api::object_ptr<td_api::proxy> &proxy : proxies->proxies_) {
        Proxy newProxy{};
        newProxy.id = proxy->id_;
        newProxy.server = QString::fromStdString(proxy->server_);
        newProxy.port = proxy->port_;
        newProxy.enabled = proxy->is_enabled_;

        downcast_call(
            *proxy->type_, overloaded(
               [&](td_api::proxyTypeSocks5 &type) {
                   newProxy.type = ProxyType::Socks5;
                   newProxy.username = QString::fromStdString(type.username_);
                   newProxy.password = QString::fromStdString(type.password_);
               },
               [&](td_api::proxyTypeHttp &type) {
                   newProxy.type = ProxyType::HTTP;
                   newProxy.username = QString::fromStdString(type.username_);
                   newProxy.password = QString::fromStdString(type.password_);
               },
               [&](td_api::proxyTypeMtproto &type) {
                   newProxy.type = ProxyType::MTProto;
                   newProxy.username = QString::fromStdString(type.secret_);
               }
            )
        );

        _proxyList.append(newProxy);
    }
    endResetModel();
}

void ProxyModel::onError(int64_t chatId, int32_t type, int32_t subType, int32_t code, QString message)
{
    switch (type) {
    case td_api::editProxy::ID:
        emit editError(message);
        break;
    case td_api::addProxy::ID:
        emit addError(message);
        break;
    case td_api::testProxy::ID:
        emit testError(message);
        break;
    }
}

td_api::object_ptr<td_api::ProxyType> ProxyModel::getProxyType(ProxyType type, QString username, QString password) const
{
    td_api::object_ptr<td_api::ProxyType> newType;
    switch (type) {
    case ProxyType::Socks5:
    {
        td_api::object_ptr<td_api::proxyTypeSocks5> type = td_api::make_object<td_api::proxyTypeSocks5>();
        type->username_ = username.toStdString();
        type->password_ = password.toStdString();
        newType = move(type);
    }
    break;
    case ProxyType::HTTP:
    {
        td_api::object_ptr<td_api::proxyTypeHttp> type = td_api::make_object<td_api::proxyTypeHttp>();
        type->username_ = username.toStdString();
        type->password_ = password.toStdString();
        newType = move(type);
    }
    break;
    case ProxyType::MTProto:
    {
        td_api::object_ptr<td_api::proxyTypeMtproto> type = td_api::make_object<td_api::proxyTypeMtproto>();
        type->secret_ = username.toStdString();
        newType = move(type);
    }
    break;
    }

    return newType;
}
