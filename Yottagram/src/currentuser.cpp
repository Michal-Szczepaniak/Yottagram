#include "currentuser.h"

CurrentUser::CurrentUser(QObject *parent) : QObject(parent)
{

}

void CurrentUser::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::gotMe, this, &CurrentUser::onGotMe);
}

bool CurrentUser::hasPremium() const
{
    if (_me == nullptr) return false;

    return _me->is_premium_;
}

void CurrentUser::onGotMe(td_api::user *me)
{
    if (_me != nullptr) {
        delete _me;
    }

    _me = me;

    emit userChanged();
}

void CurrentUser::onIsAuthorizedChanged(bool isAuthorized)
{
    if (!isAuthorized) return;

    _manager->sendQueryWithResponse(td_api::getMe::ID, new td_api::getMe());
}
