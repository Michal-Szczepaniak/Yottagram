#ifndef SUPERGROUPSINFO_H
#define SUPERGROUPSINFO_H

#include <QObject>
#include <QHash>
#include "../core/telegrammanager.h"
#include "supergroupinfo.h"

class SupergroupsInfo : public QObject
{
    Q_OBJECT
public:
    explicit SupergroupsInfo(QObject *parent = nullptr);

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    SupergroupInfo* getSupergroup(qint32 id);

signals:
    void supergroupInfoChanged(qint32 chatId);

public slots:
    void updateSupergroup(td_api::updateSupergroup *updateSupergroup);

private:
    shared_ptr<TelegramManager> _manager;
    QHash<qint32, SupergroupInfo*> _supergroups;
};

#endif // SUPERGROUPSINFO_H
