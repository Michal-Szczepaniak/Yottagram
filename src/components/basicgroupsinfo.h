#ifndef BASICGROUPSINFO_H
#define BASICGROUPSINFO_H

#include <QObject>
#include <QHash>
#include "../core/telegrammanager.h"
#include "basicgroupinfo.h"

class BasicGroupsInfo : public QObject
{
    Q_OBJECT
public:
    explicit BasicGroupsInfo(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    BasicGroupInfo* getBasicGroup(int32_t id);

signals:
    void basicGroupInfoChanged(int32_t chatId);

public slots:
    void updateBasicGroup(td_api::updateBasicGroup *updateBasicGroup);

private:
    shared_ptr<TelegramManager> _manager;
    QHash<int32_t, BasicGroupInfo*> _basicGroups;
};

#endif // BASICGROUPSINFO_H
