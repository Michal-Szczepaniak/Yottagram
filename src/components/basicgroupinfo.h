#ifndef BASICGROUPINFO_H
#define BASICGROUPINFO_H

#include <QObject>
#include <td/telegram/Client.h>

using namespace td;

class BasicGroupInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString memberType READ getMemberType NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canSendMessages READ getCanSendMessages NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canSendMediaMessages READ getCanSendMediaMessages NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canSendPolls READ getCanSendPolls NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canSendOtherMessages READ getCanSendOtherMessages NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canAddWebPagePreviews READ getCanAddWebPagePreviews NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canChangeInfo READ getCanChangeInfo NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canInviteUsers READ getCanInviteUsers NOTIFY basicGroupInfoChanged)
    Q_PROPERTY(bool canPinMessages READ getCanPinMessages NOTIFY basicGroupInfoChanged)
public:
    explicit BasicGroupInfo(QObject *parent = nullptr);

    void setBasicGroupInfo(td_api::object_ptr<td_api::basicGroup> basicGroup);
    td_api::basicGroup* getBasicGroup();
    QString getMemberType() const;

    bool getCanSendMessages() const;
    bool getCanSendMediaMessages() const;
    bool getCanSendPolls() const;
    bool getCanSendOtherMessages() const;
    bool getCanAddWebPagePreviews() const;
    bool getCanChangeInfo() const;
    bool getCanInviteUsers() const;
    bool getCanPinMessages() const;

signals:
    void basicGroupInfoChanged();

private:
    td_api::object_ptr<td_api::basicGroup> _basicGroup;
};
Q_DECLARE_METATYPE(BasicGroupInfo*)

#endif // BASICGROUPINFO_H
