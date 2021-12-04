#ifndef CONTACT_H
#define CONTACT_H

#include <QObject>
#include "core/telegrammanager.h"
#include "users.h"
#include "contentinterface.h"

class Contact : public QObject, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(int64_t userId READ userId NOTIFY contactChanged)
    Q_PROPERTY(QString name READ name NOTIFY contactChanged)
    Q_PROPERTY(QString phoneNumber READ phoneNumber NOTIFY contactChanged)
public:
    explicit Contact(QObject *parent = nullptr);
    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);

    int64_t userId();
    QString name();
    QString phoneNumber();
signals:
    void contactChanged();

private:
    td_api::object_ptr<td_api::contact> _contact;
    shared_ptr<TelegramManager> _manager;
    shared_ptr<Users> _users;
};
Q_DECLARE_METATYPE(Contact*)

#endif // CONTACT_H
