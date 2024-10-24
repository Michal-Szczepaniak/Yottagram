#include "calls.h"
#include <QDebug>
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/linux/audio_device_pulse_linux.h"
#include <glib.h>
#include <QSettings>

namespace tgcalls {
class InstanceImpl;
class InstanceV2Impl;
class InstanceV2ReferenceImpl;
}

const auto Register = tgcalls::Register<tgcalls::InstanceImpl>();
const auto RegisterV2 = tgcalls::Register<tgcalls::InstanceV2Impl>();
const auto RegV2Ref = tgcalls::Register<tgcalls::InstanceV2ReferenceImpl>();

Calls::Calls(QObject *parent) : QObject(parent), _state(State::Discarded)
{
}

void Calls::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateCall, this, &Calls::onUpdateCall);
    connect(_manager.get(), &TelegramManager::updateNewCallSignalingData, this, &Calls::onUpdateNewCallSignalingData);
}

void Calls::setUsers(shared_ptr<Users> users)
{
    _users = users;
}

void Calls::setDBusHelper(shared_ptr<DBusHelper> dbusHelper)
{
    _dbusHelper = dbusHelper;
    connect(_dbusHelper.get(), &DBusHelper::discard, [=]() { discard(); });
    connect(_dbusHelper.get(), &DBusHelper::accept, this, &Calls::answer);
    connect(_dbusHelper.get(), &DBusHelper::muteMicrophone, this, &Calls::muteMicrophone);
    connect(_dbusHelper.get(), &DBusHelper::changeSpeakerMode, this, &Calls::changeSpeakerMode);
}

int Calls::state() const
{
    return _state;
}

void Calls::setState(int32_t stateId)
{
    _state = getStateFromTdlib(stateId);
}

Calls::State Calls::getStateFromTdlib(int32_t stateId)
{
    switch (stateId) {
    case td_api::callStatePending::ID:
        return State::Pending;
        break;
    case td_api::callStateExchangingKeys::ID:
        return State::ExchangingKeys;
        break;
    case td_api::callStateReady::ID:
        return State::Ready;
        break;
    case td_api::callStateHangingUp::ID:
        return State::HangingUp;
        break;
    case td_api::callStateDiscarded::ID:
        return State::Discarded;
        break;
    case td_api::callStateError::ID:
        return State::Error;
        break;
    }
}

void Calls::updateCall(td_api::object_ptr<td_api::call> call)
{
    int32_t stateId = call->state_->get_id();

    switch (getStateFromTdlib(stateId)) {
    case State::Pending:
        if (call->id_ != _callId) {
            if (_state == State::Ready) {
                discard(call->id_);
                return;
            }
            _paHelper.probeOutputs();
            _dbusHelper->newCall(_users->getUser(call->user_id_)->getName(), !call->is_outgoing_);
            _paHelper.changeLoudspeakerMode(false);
        }
        break;
    case State::Ready:
        updateReadyCallData(move_tl_object_as<td_api::callStateReady>(call->state_));
        _dbusHelper->callReady();
        break;
    case State::Error:
        qWarning() << QString::fromStdString(static_cast<td_api::callStateError*>(call->state_.get())->error_->message_);
        stopInstance();
        _dbusHelper->discardCall();
        _paHelper.changeLoudspeakerMode(true);
        break;
    case State::Discarded:
        stopInstance();
        _dbusHelper->discardCall();
        _paHelper.changeLoudspeakerMode(true);
        break;
    case State::ExchangingKeys:
        break;
    case State::HangingUp:
        break;
    }

    setState(stateId);
    _callId = call->id_;
    _callUserId = call->user_id_;
    _isOutgoing = call->is_outgoing_;
    _isVideo = call->is_video_;

    emit callChanged();
    qDebug() << "update call id: " << call->id_ << " userId: " << call->user_id_ << " isOutgoing: " << call->is_outgoing_ << " state: " << state();
}

    void Calls::updateReadyCallData(td_api::object_ptr<td_api::callStateReady> callStateReady)
{
    td_api::object_ptr<td_api::callProtocol> protocol = move(callStateReady->protocol_);
    td_api::array<td_api::object_ptr<td_api::callServer>> servers = move(callStateReady->servers_);
    string config = callStateReady->config_;
    td_api::bytes encryption_key = callStateReady->encryption_key_;
    td_api::array<string> emojis = callStateReady->emojis_;
    bool allow_p2p = callStateReady->allow_p2p_;
    qDebug() << allow_p2p;

    auto encryptionKeyValue = std::make_shared<std::array<uint8_t, 256>>();
    memcpy(encryptionKeyValue->data(), encryption_key.data(), 256);

        tgcalls::Descriptor descriptor = {
            .config = tgcalls::Config{
                .initializationTimeout = 5.,
                .receiveTimeout = 5.,
                .dataSaving = tgcalls::DataSaving::Never,
                .enableP2P = allow_p2p,
                .enableAEC = false,
                .enableNS = true,
                .enableAGC = true,
                .enableVolumeControl = true,
                .maxApiLayer = protocol->max_layer_,
                },
            .initialNetworkType = tgcalls::NetworkType::WiFi,
            .encryptionKey = tgcalls::EncryptionKey(
                encryptionKeyValue,
                _isOutgoing),
            .mediaDevicesConfig = tgcalls::MediaDevicesConfig{
                .audioInputId = _paHelper.getDefaultSource().toStdString(),
                .audioOutputId = _paHelper.getDefaultSink().toStdString(),
                .inputVolume = 1.f,//settings.callInputVolume() / 100.f,
                .outputVolume = 1.f,//settings.callOutputVolume() / 100.f,
                },
            .stateUpdated = [=](tgcalls::State state) {
                qDebug() << "stateUpdated " << (int)state;
                qDebug() << QString::fromStdString(_instance->getLastError());
                qDebug() << QString::fromStdString(_instance->getDebugInfo());
            },
            .signalBarsUpdated = [=](int count) {
                qDebug() << "signal bars updated " << count;
            },
            .remoteMediaStateUpdated = [=](
                                           tgcalls::AudioState audio,
                                           tgcalls::VideoState video) {
                qDebug() << "remoteMediaStateUpdated";
                qDebug() << "Audio state: " << (int)audio;
                qDebug() << "Video state: " << (int)video;
            },
            .signalingDataEmitted = [=](const std::vector<uint8_t> &data) {
                qDebug() << "signalingDataEmitted";
                _manager->sendQuery(new td_api::sendCallSignalingData(_callId, std::string(data.begin(), data.end())));
            },
            .createAudioDeviceModule = [=](webrtc::TaskQueueFactory* factory) {
                _audioResource.acquire();
                rtc::scoped_refptr<webrtc::AudioDeviceModule> module = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kLinuxPulseAudio, factory);
                return module;
            }
        };

    const auto callLogPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/last_call_log.txt";
    const auto callLogNative = QDir::toNativeSeparators(callLogPath);
    const auto callLogUtf = QFile::encodeName(callLogNative);
//    descriptor.config.logPath.data = callLogPath.toStdString();
    //    descriptor.config.statsLogPath.data = (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/stats.txt").toStdString();

    for (td_api::object_ptr<td_api::callServer> &server : servers) {
        if (static_cast<const td_api::callServerTypeTelegramReflector&>(*server->type_).peer_tag_.size() == 16) qDebug() << "peer tag";
        if (server->type_->get_id() == td_api::callServerTypeTelegramReflector::ID) {
            qDebug() << "callServerTypeTelegramReflector";
            tgcalls::Endpoint endpoint = {
                .endpointId = server->id_,
                .host = tgcalls::EndpointHost{ server->ip_address_, server->ipv6_address_ },
                .port = static_cast<uint16_t>(server->port_),
                .type = static_cast<td_api::callServerTypeTelegramReflector*>(server->type_.get())->is_tcp_ ? tgcalls::EndpointType::TcpRelay : tgcalls::EndpointType::UdpRelay
            };

            const auto tag = static_cast<td_api::callServerTypeTelegramReflector*>(server->type_.get())->peer_tag_;
            if (tag.size() >= 16) {
                memcpy(endpoint.peerTag, tag.data(), 16);
            }
            descriptor.endpoints.push_back(endpoint);
        }


        if (server->type_->get_id() == td_api::callServerTypeWebrtc::ID) {
            td_api::callServerTypeWebrtc* callServerType = static_cast<td_api::callServerTypeWebrtc*>(server->type_.get());
            qDebug() << "Server " << QString::fromStdString(server->ip_address_) << " " << (uint16_t)server->port_;
            if (callServerType->supports_stun_) {
                tgcalls::RtcServer rtcServer = {
                    .host = server->ip_address_,
                    .port = static_cast<uint16_t>(server->port_),
                    .isTurn = false
                };
                descriptor.rtcServers.emplace_back(rtcServer);
                //                rtcServer.host = server->ipv6_address_;
                //                descriptor.rtcServers.emplace_back(rtcServer);
            }
            if (callServerType->supports_turn_) {
                tgcalls::RtcServer rtcServer = {
                    .host = server->ip_address_,
                    .port = static_cast<uint16_t>(server->port_),
                    .login = callServerType->username_,
                    .password = callServerType->password_,
                    .isTurn = true
                };
                descriptor.rtcServers.emplace_back(rtcServer);
                //                rtcServer.host = server->ipv6_address_;
                //                descriptor.rtcServers.emplace_back(rtcServer);
            }
        }
    }
    for (std::string version : protocol->library_versions_) {
        qDebug() << QString::fromStdString(version);
    }

    _instance = tgcalls::Meta::Create(
        protocol->library_versions_.front(),
        std::move(descriptor));

    if (!_instance) {
        qWarning() << "Call Error: Wrong library version: " << QString::fromStdString(protocol->library_versions_.front());
    }
}

int64_t Calls::userId() const
{
    return _callUserId;
}

void Calls::call(int64_t userId)
{
    td_api::createCall *createCall = new td_api::createCall();

    td_api::object_ptr<td_api::callProtocol> callProtocol = td_api::make_object<td_api::callProtocol>();
    std::vector<std::string> versions = tgcalls::Meta::Versions();
    std::reverse(versions.begin(), versions.end());
    callProtocol->udp_p2p_ = true;
    callProtocol->udp_reflector_ = true;
    callProtocol->library_versions_ = versions;
    callProtocol->min_layer_ = 65;
    callProtocol->max_layer_ = tgcalls::Meta::MaxLayer();
    createCall->protocol_ = move(callProtocol);
    createCall->is_video_ = false;
    createCall->user_id_ = userId;

    _manager->sendQuery(createCall);
}

void Calls::stopInstance()
{
    if (_instance) {
        _instance->stop([](tgcalls::FinalState) {
        });
        _instance.reset();
    }
}

void Calls::onUpdateNewCallSignalingData(td_api::updateNewCallSignalingData *updateNewCallSignalingData)
{
    qDebug() << "update callSignalingData id: " << updateNewCallSignalingData->call_id_;
    if (_instance) {
        std::vector<uint8_t> prepared;
        for (char c : updateNewCallSignalingData->data_) {
            prepared.emplace_back(static_cast<uint8_t>(c));
        }
        _instance->receiveSignalingData(prepared);
    }
}

void Calls::onUpdateCall(td_api::updateCall *updateCall)
{
    this->updateCall(move(updateCall->call_));
}

void Calls::discard(int32_t id)
{
    td_api::discardCall *discardCall = new td_api::discardCall();
    discardCall->call_id_ = id == 0 ? _callId : id;
    discardCall->is_disconnected_ = false;
    discardCall->is_video_ = false;
    discardCall->connection_id_ = _instance ? _instance->getPreferredRelayId() : 0;
    _manager->sendQuery(discardCall);
    _dbusHelper->discardCall();
}

void Calls::answer()
{
    td_api::acceptCall *acceptCall = new td_api::acceptCall;
    acceptCall->call_id_ = _callId;

    td_api::object_ptr<td_api::callProtocol> callProtocol = td_api::make_object<td_api::callProtocol>();
    std::vector<std::string> versions = tgcalls::Meta::Versions();
    std::reverse(versions.begin(), versions.end());
    for (std::string version : versions) {
        qDebug() << QString::fromStdString(version);
    }
    callProtocol->udp_p2p_ = true;
    callProtocol->udp_reflector_ = true;
    callProtocol->library_versions_ = versions;
    callProtocol->min_layer_ = 65;
    callProtocol->max_layer_ = tgcalls::Meta::MaxLayer();

    acceptCall->protocol_ = move(callProtocol);

    _manager->sendQuery(acceptCall);
}

void Calls::muteMicrophone(bool mute)
{
    if (_instance) {
        _instance->setMuteMicrophone(mute);
    }
}

void Calls::changeSpeakerMode(bool loudspeaker)
{
    if (_state == State::Ready) {
        _paHelper.changeLoudspeakerMode(loudspeaker);
    }
}
