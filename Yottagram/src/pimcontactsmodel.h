#ifndef PIMCONTACTSMODEL_H
#define PIMCONTACTSMODEL_H

#include <QAbstractListModel>
#include <QContactManager>
#include <QtContacts>
#include <qcontactmanager.h>

using namespace QtContacts;

class PIMContactsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ContactRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        FirstNameRole,
        LastNameRole,
        PhoneNumberRole
    };

    struct Contact {
        QString firstName;
        QString lastName;
        QString phoneNumber;

        bool operator== (const Contact& other) const
        {
            return other.firstName == firstName && other.lastName == lastName && other.phoneNumber == phoneNumber;
        };

        QJsonObject toQJsonObject() {
            QJsonObject contactObject;

            contactObject.insert("firstName", firstName);
            contactObject.insert("lastName", lastName);
            contactObject.insert("phoneNumber", phoneNumber);

            return contactObject;
        }
    };

    explicit PIMContactsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void selectContact(QString name, QString lastName, QString phoneNumber);
    Q_INVOKABLE void unselectContact(QString name, QString lastName, QString phoneNumber);
    Q_INVOKABLE QString exportAsJSON();

private:
    QVector<Contact> _contacts{};
    QVector<Contact> _selectedContacts{};
    QContactManager* _cm;
};

#endif // PIMCONTACTSMODEL_H
