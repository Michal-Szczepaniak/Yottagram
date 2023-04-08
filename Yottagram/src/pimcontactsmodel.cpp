#include "pimcontactsmodel.h"

PIMContactsModel::PIMContactsModel(QObject *parent) : QAbstractListModel(parent)
{
    QMap<QString, QString> parameters;
    parameters.insert(QString::fromLatin1("mergePresenceChanges"), QString::fromLatin1("false"));
    _cm = new QContactManager(QString::fromLatin1("org.nemomobile.contacts.sqlite"), parameters);
}

int PIMContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _contacts.size();
}

QVariant PIMContactsModel::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() >= rowCount()) return QVariant();

    Contact contact = _contacts[index.row()];

    switch (role) {
    case ContactRoles::IdRole:
        return index.row();
    case ContactRoles::NameRole:
        return contact.firstName + " " + contact.lastName;
    case ContactRoles::FirstNameRole:
        return contact.firstName;
    case ContactRoles::LastNameRole:
        return contact.lastName;
    case ContactRoles::PhoneNumberRole:
        return contact.phoneNumber;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PIMContactsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[FirstNameRole] = "firstName";
    roles[LastNameRole] = "lastName";
    roles[PhoneNumberRole] = "phoneNumber";
    return roles;
}

void PIMContactsModel::load()
{
    beginResetModel();

//    for (int i = 0; i < 100; i++) {
//        Contact newContact{
//            .firstName = QString::fromLatin1("Contact "),
//            .lastName = QString::number(i),
//            .phoneNumber = (i%2 ? "#" : "") + QString::number(i)
//        };

//        _contacts.append(newContact);
//    }

    auto collection = _cm->collections().first();

    QContactCollectionFilter filter;
    filter.setCollectionId(collection.id());
    QList<QContact> allContacts = _cm->contacts();

    for (QContact contact : allContacts) {
        Contact newContact{
            .firstName = contact.detail<QContactName>().firstName(),
            .lastName = contact.detail<QContactName>().lastName(),
            .phoneNumber = contact.detail<QContactPhoneNumber>().number()
        };

        if (newContact.phoneNumber != "" && newContact.firstName != "")
            _contacts.append(newContact);
    }

    endResetModel();
}

void PIMContactsModel::selectContact(QString firstName, QString lastName, QString phoneNumber)
{
    Contact selectedContact{
        .firstName = firstName,
        .lastName = lastName,
        .phoneNumber = phoneNumber
    };

    if (!_selectedContacts.contains(selectedContact)) {
        _selectedContacts.append(selectedContact);
    }
}

void PIMContactsModel::unselectContact(QString firstName, QString lastName, QString phoneNumber)
{
    Contact selectedContact{
        .firstName = firstName,
        .lastName = lastName,
        .phoneNumber = phoneNumber
    };

    _selectedContacts.removeOne(selectedContact);
}

QString PIMContactsModel::exportAsJSON()
{
    QJsonArray contactsArray;

    for (Contact contact : _selectedContacts) {
        contactsArray.append(contact.toQJsonObject());
    }

    QJsonDocument doc(contactsArray);
    return doc.toJson(QJsonDocument::Compact);
}
