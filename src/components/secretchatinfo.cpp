#include "secretchatinfo.h"

SecretChatInfo::SecretChatInfo(QObject *parent) : QObject(parent)
{

}

void SecretChatInfo::setSecretChatInfo(td_api::object_ptr<td_api::secretChat> secretChat)
{
    _secretChat = std::move(secretChat);

    emit secretChatInfoChanged();
}

td_api::secretChat *SecretChatInfo::getSecretChat()
{
    return _secretChat.get();
}

int64_t SecretChatInfo::getUserId() const
{
    return _secretChat->user_id_;
}

QString SecretChatInfo::getState() const
{
    switch (_secretChat->state_->get_id()) {
    case td_api::secretChatStateReady::ID:
        return "ready";
    case td_api::secretChatStateClosed::ID:
        return "closed";
    case td_api::secretChatStatePending::ID:
        return "pending";
    default:
        return "";
    }
}
