#ifndef SECRETCHATINFO_H
#define SECRETCHATINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class SecretChatInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint32 userId READ getUserId NOTIFY secretChatInfoChanged)
    Q_PROPERTY(QString state READ getState NOTIFY secretChatInfoChanged)
    Q_PROPERTY(qint32 ttl READ getTtl NOTIFY secretChatInfoChanged)
public:
    explicit SecretChatInfo(QObject *parent = nullptr);

    void setSecretChatInfo(td_api::object_ptr<td_api::secretChat> secretChat);
    td_api::secretChat* getSecretChat();
    qint32 getUserId() const;
    QString getState() const;
    qint32 getTtl() const;

signals:
    void secretChatInfoChanged();

private:
    td_api::object_ptr<td_api::secretChat> _secretChat;
};
Q_DECLARE_METATYPE(SecretChatInfo*)

#endif // SECRETCHATINFO_H
