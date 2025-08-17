#include "supergroupinfo.h"

SupergroupInfo::SupergroupInfo(QObject *parent) : QObject(parent)
{

}

void SupergroupInfo::setSupergroupInfo(td_api::object_ptr<td_api::supergroup> supergroup)
{
    _supergroup = std::move(supergroup);

    emit supergroupInfoChanged();
}

td_api::supergroup *SupergroupInfo::getSupergroup()
{
    return _supergroup.get();
}

QString SupergroupInfo::getMemberType() const
{
    switch (_supergroup->status_->get_id()) {
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

bool SupergroupInfo::getCanSendMessages() const
{
    auto status = _supergroup->status_.get();
    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        if (_supergroup->is_channel_)
            return static_cast<const td_api::chatMemberStatusAdministrator*>(status)->rights_->can_post_messages_;
        else return true;
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

bool SupergroupInfo::getCanSendMediaMessages() const
{
    auto status = _supergroup->status_.get();
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
               permissions->can_send_polls_;
    }
    default:
        return true;
    }
}

bool SupergroupInfo::getCanSendPolls() const
{
    auto status = _supergroup->status_.get();
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

bool SupergroupInfo::getCanSendOtherMessages() const
{
    auto status = _supergroup->status_.get();
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

bool SupergroupInfo::getCanAddWebPagePreviews() const
{
    auto status = _supergroup->status_.get();
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
        return static_cast<const td_api::chatMemberStatusRestricted*>(status)->permissions_->can_add_link_previews_;
    default:
        return true;
    }
}

bool SupergroupInfo::getCanChangeInfo() const
{
    auto status = _supergroup->status_.get();
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

bool SupergroupInfo::getCanInviteUsers() const
{
    auto status = _supergroup->status_.get();
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

bool SupergroupInfo::getCanPinMessages() const
{
    auto status = _supergroup->status_.get();
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

bool SupergroupInfo::getEditMessages() const
{
    if (!_supergroup->is_channel_) return false;

    auto status = _supergroup->status_.get();

    switch (status->get_id()) {
    case td_api::chatMemberStatusAdministrator::ID:
        return static_cast<const td_api::chatMemberStatusAdministrator*>(status)->rights_->can_edit_messages_;
    case td_api::chatMemberStatusCreator::ID:
        return true;
    default:
        return false;
    }
}

bool SupergroupInfo::isForum() const
{
    return _supergroup->is_forum_;
}
