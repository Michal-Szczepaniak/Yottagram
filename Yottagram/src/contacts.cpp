#include "contacts.h"

Contacts::Contacts(QObject *parent) : QAbstractListModel(parent)
{

}

void Contacts::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::gotContacts, this, &Contacts::onGotContacts);
    connect(_manager.get(), &TelegramManager::importedContacts, this, &Contacts::onImportedContacts);
}

void Contacts::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

int Contacts::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _contacts.size();
}

QVariant Contacts::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    std::shared_ptr<User> contact = _contacts[index.row()];

    switch (role) {
    case ContactRoles::IdRole:
        return QVariant::fromValue(contact->getId());
    case ContactRoles::UserRole:
    {
        User* user = contact.get();
        QQmlEngine::setObjectOwnership(user, QQmlEngine::CppOwnership);
        return QVariant::fromValue(user);
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Contacts::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[UserRole] = "user";
    return roles;
}

void Contacts::load()
{
    _manager->sendQueryWithResponse(td_api::getContacts::ID, new td_api::getContacts());
}

void Contacts::importJson(QString json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toLatin1());
    td_api::array<td_api::object_ptr<td_api::contact>> importContactsArray;


    for (auto object : doc.array()) {
        QJsonObject obj = object.toObject();

        td_api::object_ptr<td_api::contact> contact = td_api::make_object<td_api::contact>();
        contact->first_name_ = obj["firstName"].toString().toStdString();
        contact->last_name_ = obj["lastName"].toString().toStdString();
        contact->phone_number_ = obj["phoneNumber"].toString().toStdString();

        importContactsArray.push_back(move(contact));
    }

    td_api::importContacts *importContacts = new td_api::importContacts(move(importContactsArray));
    _manager->sendQueryWithResponse(td_api::importContacts::ID, importContacts);
}

void Contacts::onGotContacts(td_api::users *contacts)
{
    beginResetModel();
    _contacts.clear();

    for (int64_t contactId : contacts->user_ids_) {
        std::shared_ptr<User> user = _users->getUser(contactId);
        _contacts.append(user);
    }

    endResetModel();
}

void Contacts::onImportedContacts(td_api::importedContacts *importedContacts)
{
    int importedCount = 0;

    for (qint64 userId : importedContacts->user_ids_)
        if (userId != 0) importedCount++;

    emit importDone(importedCount);
}

