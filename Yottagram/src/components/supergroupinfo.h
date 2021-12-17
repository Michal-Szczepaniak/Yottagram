#ifndef SUPERGROUPINFO_H
#define SUPERGROUPINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class SupergroupInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString memberType READ getMemberType NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canSendMessages READ getCanSendMessages NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canSendMediaMessages READ getCanSendMediaMessages NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canSendPolls READ getCanSendPolls NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canSendOtherMessages READ getCanSendOtherMessages NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canAddWebPagePreviews READ getCanAddWebPagePreviews NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canChangeInfo READ getCanChangeInfo NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canInviteUsers READ getCanInviteUsers NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canPinMessages READ getCanPinMessages NOTIFY supergroupInfoChanged)
    Q_PROPERTY(bool canEditMessages READ getEditMessages NOTIFY supergroupInfoChanged)
public:
    explicit SupergroupInfo(QObject *parent = nullptr);

    void setSupergroupInfo(td_api::object_ptr<td_api::supergroup> supergroup);
    td_api::supergroup* getSupergroup();
    QString getMemberType() const;

    bool getCanSendMessages() const;
    bool getCanSendMediaMessages() const;
    bool getCanSendPolls() const;
    bool getCanSendOtherMessages() const;
    bool getCanAddWebPagePreviews() const;
    bool getCanChangeInfo() const;
    bool getCanInviteUsers() const;
    bool getCanPinMessages() const;
    bool getEditMessages() const;

signals:
    void supergroupInfoChanged();

private:
    td_api::object_ptr<td_api::supergroup> _supergroup;
};
Q_DECLARE_METATYPE(SupergroupInfo*)

#endif // SUPERGROUPINFO_H
