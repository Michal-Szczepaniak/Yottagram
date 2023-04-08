#ifndef CONTACTS_H
#define CONTACTS_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"
#include "contents/contact.h"

class Contacts : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ContactRoles {
        IdRole = Qt::UserRole + 1,
        UserRole
    };

    explicit Contacts(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void importJson(QString json);
signals:
    void importDone(qint32 importedCount);

public slots:
    void onGotContacts(td_api::users *contacts);
    void onImportedContacts(td_api::importedContacts *importedContacts);

private:
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
    QVector<std::shared_ptr<User>> _contacts;
};

#endif // CONTACTS_H
