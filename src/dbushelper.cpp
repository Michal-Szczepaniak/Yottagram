#include "dbushelper.h"

DBusHelper::DBusHelper(QObject *parent) : QObject(parent)
{
    _adaptor = new CallsAdaptor(this);

    QDBusConnection::sessionBus().registerObject("/calls", this);
    _iface = new com::verdanditeam::yottagram::calls("com.verdanditeam.yottagram.calls", "/calls", QDBusConnection::sessionBus(), this);
    connect(_iface, &com::verdanditeam::yottagram::calls::accept, this, &DBusHelper::accept);
    connect(_iface, &com::verdanditeam::yottagram::calls::discard, this, &DBusHelper::discard);
    connect(_iface, &com::verdanditeam::yottagram::calls::muteMicrophone, this, &DBusHelper::muteMicrophone);
    connect(_iface, &com::verdanditeam::yottagram::calls::changeSpeakerMode, this, &DBusHelper::changeSpeakerMode);
}

void DBusHelper::newCall(const QString &callerName, const bool &isIncoming)
{
    _iface->newCall(callerName, isIncoming);
}

void DBusHelper::discardCall()
{
    _iface->discardCall();
}

void DBusHelper::callReady()
{
    _iface->callReady();
}
