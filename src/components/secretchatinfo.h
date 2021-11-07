#ifndef SECRETCHATINFO_H
#define SECRETCHATINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class SecretChatInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int32_t userId READ getUserId NOTIFY secretChatInfoChanged)
    Q_PROPERTY(QString state READ getState NOTIFY secretChatInfoChanged)
    Q_PROPERTY(int32_t ttl READ getTtl NOTIFY secretChatInfoChanged)
public:
    explicit SecretChatInfo(QObject *parent = nullptr);

    void setSecretChatInfo(td_api::object_ptr<td_api::secretChat> secretChat);
    td_api::secretChat* getSecretChat();
    int32_t getUserId() const;
    QString getState() const;
    int32_t getTtl() const;

signals:
    void secretChatInfoChanged();

private:
    td_api::object_ptr<td_api::secretChat> _secretChat;
};
Q_DECLARE_METATYPE(SecretChatInfo*)

#endif // SECRETCHATINFO_H
