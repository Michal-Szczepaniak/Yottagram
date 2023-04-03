#include "supergroupsinfo.h"

SupergroupsInfo::SupergroupsInfo(QObject *parent) : QObject(parent)
{

}

void SupergroupsInfo::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), SIGNAL(updateSupergroup(td_api::updateSupergroup*)), this, SLOT(updateSupergroup(td_api::updateSupergroup*)));
}

SupergroupInfo *SupergroupsInfo::getSupergroup(int32_t id)
{
    if (!_supergroups.contains(id)) return nullptr;

    return _supergroups[id];
}

void SupergroupsInfo::updateSupergroup(td_api::updateSupergroup *updateSupergroup)
{
    if (!updateSupergroup->supergroup_) return;

    int32_t chatId = updateSupergroup->supergroup_->id_;
    if (!_supergroups.contains(chatId)) {
        SupergroupInfo* info = new SupergroupInfo();
        info->setSupergroupInfo(std::move(updateSupergroup->supergroup_));
        _supergroups[chatId] = info;
    } else {
        _supergroups[chatId]->setSupergroupInfo(std::move(updateSupergroup->supergroup_));
    }

    emit supergroupInfoChanged(chatId);
}
