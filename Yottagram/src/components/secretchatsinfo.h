#ifndef SECRETCHATSINFO_H
#define SECRETCHATSINFO_H

#include <QObject>
#include <QHash>
#include "../core/telegrammanager.h"
#include "secretchatinfo.h"

class SecretChatsInfo : public QObject
{
    Q_OBJECT
public:
    explicit SecretChatsInfo(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    SecretChatInfo* getSecretChat(int32_t id);

signals:
    void secretChatInfoChanged(int32_t chatId);

public slots:
    void updateSecretChat(td_api::updateSecretChat *updateSecretChat);

private:
    shared_ptr<TelegramManager> _manager;
    QHash<int32_t, SecretChatInfo*> _secretChats;
};

#endif // SECRETCHATSINFO_H
