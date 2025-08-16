#ifndef TELEGRAMSTATUS_H
#define TELEGRAMSTATUS_H

#include <QObject>

#include "core/telegrammanager.h"

class TelegramStatus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool bootupComplete READ getBootupComplete NOTIFY bootupComplete)
public:
    explicit TelegramStatus(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);

    bool getBootupComplete() const;
public slots:
    void onBootupComplete();

signals:
    void bootupComplete();

private:
    shared_ptr<TelegramManager> _manager;
    bool _bootupComplete = false;
};

#endif // TELEGRAMSTATUS_H
