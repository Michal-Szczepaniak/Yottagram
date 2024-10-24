#ifndef CALLS_H
#define CALLS_H

#include <QObject>
#include "core/telegrammanager.h"
#include "users.h"
#include "dbushelper.h"
#include "pulseaudiohelper.h"
#include <tgcalls/Instance.h>
#include <tgcalls/StaticThreads.h>
#include <audioresource-qt/AudioResourceQt>

class Calls : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int state READ state NOTIFY callChanged)
    Q_PROPERTY(int64_t userId READ userId NOTIFY callChanged)
public:
    explicit Calls(QObject *parent = nullptr);

    enum State {
        Pending,
        ExchangingKeys,
        Ready,
        HangingUp,
        Discarded,
        Error
    };
    Q_ENUMS(State)

    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setUsers(shared_ptr<Users> users);
    void setDBusHelper(shared_ptr<DBusHelper> dbusHelper);
    int state() const;
    void setState(int32_t stateId);
    State getStateFromTdlib(int32_t stateId);
    void updateCall(td_api::object_ptr<td_api::call> call);
    void updateReadyCallData(td_api::object_ptr<td_api::callStateReady> callStateReady);
    int64_t userId() const;
    Q_INVOKABLE void call(int64_t userId);

private:
    void stopInstance();

signals:
    void callChanged();

public slots:
    void onUpdateNewCallSignalingData(td_api::updateNewCallSignalingData *updateNewCallSignalingData);
    void onUpdateCall(td_api::updateCall *updateCall);
    void discard(int32_t id = 0);
    void answer();
    void muteMicrophone(bool mute);
    void changeSpeakerMode(bool loudspeaker);

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Users> _users;
    shared_ptr<DBusHelper> _dbusHelper;
    State _state = State::Discarded;
    int32_t _callId;
    int64_t _callUserId;
    bool _isOutgoing;
    bool _isVideo;
    std::unique_ptr<tgcalls::Instance> _instance;
    PulseaudioHelper _paHelper;
    AudioResourceQt::AudioResource _audioResource;
};

#endif // CALLS_H
