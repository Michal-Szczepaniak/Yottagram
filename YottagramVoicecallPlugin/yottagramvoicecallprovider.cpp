/*

This file is part of Yottagram.
Copyright 2020, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Yottagram is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Yottagram is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"
#include "yottagramvoicecallhandler.h"
#include "yottagramvoicecallprovider.h"

class YottagramVoiceCallProviderPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallProvider)

public:
    YottagramVoiceCallProviderPrivate(YottagramVoiceCallProvider *q, VoiceCallManagerInterface *pManager)
        : q_ptr(q), manager(pManager), voiceCall(nullptr)
    { }

    YottagramVoiceCallProvider *q_ptr;

    VoiceCallManagerInterface *manager;

    YottagramVoiceCallHandler* voiceCall;

    QString errorString;
    void setError(const QString &errorString)
    {
        this->errorString = errorString;
        debugMessage(errorString);
    }

    void debugMessage(const QString &message)
    {
        DEBUG_T("YottagramVoiceCallProvider");
    }
};

YottagramVoiceCallProvider::YottagramVoiceCallProvider(VoiceCallManagerInterface *manager, QObject *parent)
    : AbstractVoiceCallProvider(parent), d_ptr(new YottagramVoiceCallProviderPrivate(this, manager))
{
    TRACE
    Q_D(YottagramVoiceCallProvider);

    connect(d->manager, &VoiceCallManagerInterface::setMuteMicrophoneRequested, this, &YottagramVoiceCallProvider::muteMicrophone);
    connect(d->manager, &VoiceCallManagerInterface::setAudioModeRequested, [this](const QString &mode) { emit changeSpeakerMode(mode != "earpiece"); });

    new CallsAdaptor(this);
    QDBusConnection::sessionBus().registerService("com.verdanditeam.yottagram.calls");
    QDBusConnection::sessionBus().registerObject("/calls", this);
}

YottagramVoiceCallProvider::~YottagramVoiceCallProvider()
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    delete d;
}

QString YottagramVoiceCallProvider::providerId() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    return QString("yottagram");
}

QString YottagramVoiceCallProvider::providerType() const
{
    TRACE
    return "voip";
}

QList<AbstractVoiceCallHandler*> YottagramVoiceCallProvider::voiceCalls() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    QList<AbstractVoiceCallHandler*> results;
    if (d->voiceCall != nullptr) results.append(d->voiceCall);
    return results;
}

QString YottagramVoiceCallProvider::errorString() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    return d->errorString;
}

bool YottagramVoiceCallProvider::dial(const QString &)
{
    TRACE
    return false;
}

void YottagramVoiceCallProvider::newCall(const QString &callerName, const bool &incoming)
{
    TRACE
    Q_D(YottagramVoiceCallProvider);

    YottagramVoiceCallHandler* handler = new YottagramVoiceCallHandler(d->manager->generateHandlerId(), callerName, incoming, this, d->manager);
    d->voiceCall = handler;
    emit voiceCallAdded(handler);
    emit voiceCallsChanged();
}

void YottagramVoiceCallProvider::callReady()
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    if (d->voiceCall == nullptr) return;
    d->voiceCall->setStatus(AbstractVoiceCallHandler::STATUS_ACTIVE);
}

void YottagramVoiceCallProvider::discardCall()
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    if (d->voiceCall != nullptr) {
        d->voiceCall->setStatus(AbstractVoiceCallHandler::STATUS_DISCONNECTED);
        QString handlerId = d->voiceCall->handlerId();
        d->voiceCall->deleteLater();
        d->voiceCall = nullptr;
        emit voiceCallRemoved(handlerId);
    }
    emit voiceCallsChanged();
}
