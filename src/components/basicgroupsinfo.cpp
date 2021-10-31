#include "basicgroupsinfo.h"

BasicGroupsInfo::BasicGroupsInfo(QObject *parent) : QObject(parent)
{

}

void BasicGroupsInfo::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), SIGNAL(updateBasicGroup(td_api::updateBasicGroup*)), this, SLOT(updateBasicGroup(td_api::updateBasicGroup*)));
}

BasicGroupInfo *BasicGroupsInfo::getBasicGroup(int32_t id)
{
    if (!_basicGroups.contains(id)) return nullptr;

    return _basicGroups[id];
}

void BasicGroupsInfo::updateBasicGroup(td_api::updateBasicGroup *updateBasicGroup)
{
    if (updateBasicGroup->basic_group_ == nullptr) return;

    int32_t chatId = updateBasicGroup->basic_group_->id_;
    if (!_basicGroups.contains(chatId)) {
        BasicGroupInfo* info = new BasicGroupInfo();
        info->setBasicGroupInfo(std::move(updateBasicGroup->basic_group_));
        _basicGroups[chatId] = info;
    } else {
        _basicGroups[chatId]->setBasicGroupInfo(std::move(updateBasicGroup->basic_group_));
    }

    emit basicGroupInfoChanged(chatId);
}
