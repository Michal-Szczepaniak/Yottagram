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

#include <QElapsedTimer>
#include <QTimerEvent>

class YottagramVoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallHandler)

public:
    YottagramVoiceCallHandlerPrivate(YottagramVoiceCallHandler *q, const QString &pHandlerId, YottagramVoiceCallProvider *pProvider, QYottagramVoiceCallManager *manager)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), duration(0), durationTimerId(-1), isIncoming(false)
    { /* ... */ }

    YottagramVoiceCallHandler *q_ptr;

    QString handlerId;

    YottagramVoiceCallProvider *provider;

    quint64 duration;
    int durationTimerId;
    QElapsedTimer elapsedTimer;
    bool isIncoming;
};

YottagramVoiceCallHandler::YottagramVoiceCallHandler(const QString &handlerId, const QString &path, YottagramVoiceCallProvider *provider, QYottagramVoiceCallManager *manager)
    : AbstractVoiceCallHandler(provider), d_ptr(new YottagramVoiceCallHandlerPrivate(this, handlerId, provider, manager))
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
}

YottagramVoiceCallHandler::~YottagramVoiceCallHandler()
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    delete d;
}

QString YottagramVoiceCallHandler::path() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
//    return d->yottagramVoiceCall->voiceCallPath();
    return "";
}

AbstractVoiceCallProvider* YottagramVoiceCallHandler::provider() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->provider;
}

QString YottagramVoiceCallHandler::handlerId() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->handlerId;
}

QString YottagramVoiceCallHandler::lineId() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return "";
}

QDateTime YottagramVoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
//    return QDateTime::fromString(d->yottagramVoiceCall->startTime(), "");
    return QDateTime();
}

int YottagramVoiceCallHandler::duration() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return int(qRound(d->duration/1000.0));
}

bool YottagramVoiceCallHandler::isIncoming() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->isIncoming;
}

bool YottagramVoiceCallHandler::isMultiparty() const
{
    TRACE
    return false;
}

bool YottagramVoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return false;
}

bool YottagramVoiceCallHandler::isForwarded() const
{
    TRACE
    return false;
}

bool YottagramVoiceCallHandler::isRemoteHeld() const
{
    TRACE
    return false;
}

AbstractVoiceCallHandler::VoiceCallStatus YottagramVoiceCallHandler::status() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    QString state = "waiting";

    if(state == "active")
        return STATUS_ACTIVE;
    else if(state == "held")
        return STATUS_HELD;
    else if(state == "dialing")
        return STATUS_DIALING;
    else if(state == "alerting")
        return STATUS_ALERTING;
    else if(state == "incoming")
        return STATUS_INCOMING;
    else if(state == "waiting")
        return STATUS_WAITING;
    else if(state == "disconnected")
        return STATUS_DISCONNECTED;

    return STATUS_NULL;
}

void YottagramVoiceCallHandler::answer()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
}

void YottagramVoiceCallHandler::hangup()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);;
}

void YottagramVoiceCallHandler::hold(bool)
{
    TRACE
}

void YottagramVoiceCallHandler::deflect(const QString &)
{
    TRACE
}

void YottagramVoiceCallHandler::sendDtmf(const QString &)
{
    TRACE
}
