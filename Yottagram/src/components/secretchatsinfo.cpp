#include "secretchatsinfo.h"
#include <QDebug>

SecretChatsInfo::SecretChatsInfo(QObject *parent) : QObject(parent)
{

}

void SecretChatsInfo::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), SIGNAL(updateSecretChat(td_api::updateSecretChat*)), this, SLOT(updateSecretChat(td_api::updateSecretChat*)));
}

SecretChatInfo *SecretChatsInfo::getSecretChat(int32_t id)
{
    if (!_secretChats.contains(id)) return nullptr;

    return _secretChats[id];
}

void SecretChatsInfo::updateSecretChat(td_api::updateSecretChat *updateSecretChat)
{
    if (updateSecretChat->secret_chat_ == nullptr) return;

    int32_t chatId = updateSecretChat->secret_chat_->id_;
    if (!_secretChats.contains(chatId)) {
        SecretChatInfo* info = new SecretChatInfo();
        info->setSecretChatInfo(std::move(updateSecretChat->secret_chat_));
        _secretChats[chatId] = info;
    } else {
        _secretChats[chatId]->setSecretChatInfo(std::move(updateSecretChat->secret_chat_));
    }

    emit secretChatInfoChanged(chatId);
}
