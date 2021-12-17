#ifndef CALL_H
#define CALL_H

#include <QObject>
#include "core/telegrammanager.h"
#include "contentinterface.h"

class Call : public QObject, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration NOTIFY callChanged)
public:
    explicit Call(QObject *parent = nullptr);
    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;

    bool duration() const;

signals:
    void callChanged();

private:
    td_api::object_ptr<td_api::messageCall> _call;
};
Q_DECLARE_METATYPE(Call*)

#endif // CALL_H
