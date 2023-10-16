#ifndef TELEGRAMSTATUS_H
#define TELEGRAMSTATUS_H

#include <QObject>

#include "core/telegrammanager.h"

class TelegramStatus : public QObject
{
    Q_OBJECT
public:
    explicit TelegramStatus(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);

public slots:
    void onBootupComplete();

signals:
    void bootupComplete();

private:
    shared_ptr<TelegramManager> _manager;

};

#endif // TELEGRAMSTATUS_H
