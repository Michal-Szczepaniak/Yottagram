#include "telegramstatus.h"

TelegramStatus::TelegramStatus(QObject *parent) : QObject(parent)
{

}

void TelegramStatus::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::bootupComplete, this, &TelegramStatus::onBootupComplete);
}

void TelegramStatus::onBootupComplete()
{
    emit bootupComplete();
}
