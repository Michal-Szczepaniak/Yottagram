#ifndef CURRENTUSER_H
#define CURRENTUSER_H

#include <QObject>

#include <core/telegrammanager.h>

class CurrentUser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasPremium READ hasPremium NOTIFY userChanged)
public:
    explicit CurrentUser(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);

    bool hasPremium() const;

signals:
    void userChanged();

public slots:
    void onGotMe(td_api::user *me);
    void onIsAuthorizedChanged(bool isAuthorized);

private:
    shared_ptr<TelegramManager> _manager;
    td_api::user *_me = nullptr;
};

#endif // CURRENTUSER_H
