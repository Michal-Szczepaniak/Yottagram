#ifndef DBUSHELPER_H
#define DBUSHELPER_H

#include <QObject>
#include "calls_adaptor.h"
#include "calls_interface.h"

class DBusHelper : public QObject
{
    Q_OBJECT
public:
    explicit DBusHelper(QObject *parent = nullptr);

    void newCall(const QString &callerName, const bool &isIncoming);
    void discardCall();
    void callReady();

signals:
    void accept();
    void discard();
    void muteMicrophone(bool mute);
    void changeSpeakerMode(bool loudspeaker);

private:
    CallsAdaptor *_adaptor;
    com::verdanditeam::yottagram::calls *_iface;
};

#endif // DBUSHELPER_H
