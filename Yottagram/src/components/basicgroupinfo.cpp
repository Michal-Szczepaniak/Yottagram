#include "basicgroupinfo.h"

BasicGroupInfo::BasicGroupInfo(QObject *parent) : QObject(parent)
{

}

void BasicGroupInfo::setBasicGroupInfo(td_api::object_ptr<td_api::basicGroup> basicGroup)
{
    _basicGroup = std::move(basicGroup);

    emit basicGroupInfoChanged();
}

td_api::basicGroup *BasicGroupInfo::getBasicGroup()
{
    return _basicGroup.get();
}

QString BasicGroupInfo::getMemberType() const
{
    switch (_basicGroup->status_->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return "administrator";
    case td_api::chatMemberStatusBanned::ID:
        return "banned";
    case td_api::chatMemberStatusCreator::ID:
        return "creator";
    case td_api::chatMemberStatusLeft::ID:
        return "left";
    case td_api::chatMemberStatusRestricted::ID:
        return "restricted";
    case td_api::chatMemberStatusMember::ID:
    default:
        return "member";
    }
}

bool BasicGroupInfo::getCanSendMessages() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return true;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_send_basic_messages_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanSendMediaMessages() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return true;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
    {
        auto& permissions = static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_;
        return permissions->can_send_audios_ ||
               permissions->can_send_documents_ ||
               permissions->can_send_photos_ ||
               permissions->can_send_videos_ ||
               permissions->can_send_video_notes_ ||
               permissions->can_send_voice_notes_ ||
               permissions->can_send_polls_;;
    }
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanSendPolls() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return true;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_send_polls_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanSendOtherMessages() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return true;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_send_other_messages_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanAddWebPagePreviews() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return true;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_add_web_page_previews_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanChangeInfo() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return static_cast<const td_api::chatMemberStatusAdministrator*>(status)->rights_->can_change_info_;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_change_info_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanInviteUsers() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return static_cast<const td_api::chatMemberStatusAdministrator*>(status)->rights_->can_invite_users_;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_invite_users_;
    default:
        return true;
    }
}

bool BasicGroupInfo::getCanPinMessages() const
{
    auto status = _basicGroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return static_cast<const td_api::chatMemberStatusAdministrator*>(status)->rights_->can_pin_messages_;
    case td_api::chatMemberStatusBanned::ID:
        return false;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    case td_api::chatMemberStatusLeft::ID:
        return false;
    case td_api::chatMemberStatusMember::ID:
        return true;
    case td_api::chatMemberStatusRestricted::ID:
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_pin_messages_;
    default:
        return true;
    }
}
