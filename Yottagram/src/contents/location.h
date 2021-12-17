#ifndef LOCATION_H
#define LOCATION_H

#include <QObject>
#include "core/telegrammanager.h"
#include "contentinterface.h"

class Location : public QObject, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(double latitude READ latitude NOTIFY locationChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY locationChanged)
public:
    explicit Location(QObject *parent = nullptr);
    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;

    double latitude() const;
    double longitude() const;

signals:
    void locationChanged();

private:
    td_api::object_ptr<td_api::messageLocation> _location;
};
Q_DECLARE_METATYPE(Location*)

#endif // LOCATION_H
