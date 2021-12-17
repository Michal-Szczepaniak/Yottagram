/*
 * This file is a part of the Voice Call Manager Yottagram Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "common.h"
#include "yottagramvoicecallhandler.h"
#include "yottagramvoicecallprovider.h"

#include <qyottagramvoicecall.h>
#include <qyottagramvoicecallmanager.h>

#include <QElapsedTimer>
#include <QTimerEvent>

class YottagramVoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallHandler)

public:
    YottagramVoiceCallHandlerPrivate(YottagramVoiceCallHandler *q, const QString &pHandlerId, YottagramVoiceCallProvider *pProvider, QYottagramVoiceCallManager *manager)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), yottagramVoiceCallManager(manager), yottagramVoiceCall(NULL)
        , duration(0), durationTimerId(-1), isIncoming(false)
    { /* ... */ }

    YottagramVoiceCallHandler *q_ptr;

    QString handlerId;

    YottagramVoiceCallProvider *provider;

    QYottagramVoiceCallManager *yottagramVoiceCallManager;
    QYottagramVoiceCall *yottagramVoiceCall;

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
    d->yottagramVoiceCall = new QYottagramVoiceCall(this);
    d->yottagramVoiceCall->setVoiceCallPath(path);

    QObject::connect(d->yottagramVoiceCall, SIGNAL(lineIdentificationChanged(QString)), SIGNAL(lineIdChanged(QString)));
    QObject::connect(d->yottagramVoiceCall, SIGNAL(emergencyChanged(bool)), SIGNAL(emergencyChanged(bool)));
    QObject::connect(d->yottagramVoiceCall, SIGNAL(multipartyChanged(bool)), SIGNAL(multipartyChanged(bool)));

    QObject::connect(d->yottagramVoiceCall, SIGNAL(stateChanged(QString)), SLOT(onStatusChanged()));

    QObject::connect(d->yottagramVoiceCall, SIGNAL(validChanged(bool)), SLOT(onValidChanged(bool)));
    if(d->yottagramVoiceCall->isValid()) {
        onValidChanged(true);
    }
}

YottagramVoiceCallHandler::~YottagramVoiceCallHandler()
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    delete d;
}

void YottagramVoiceCallHandler::onValidChanged(bool isValid)
{
    Q_D(YottagramVoiceCallHandler);

    if (isValid)
    {
        // Properties are now ready
        d->isIncoming = d->yottagramVoiceCall->state() == QLatin1String("incoming");
    }

    emit validChanged(isValid);
}

QString YottagramVoiceCallHandler::path() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->yottagramVoiceCall->voiceCallPath();
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
    return d->yottagramVoiceCall->lineIdentification();
}

QDateTime YottagramVoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    DEBUG_T("CALL START TIME: %s", qPrintable(d->yottagramVoiceCall->startTime()));
    return QDateTime::fromString(d->yottagramVoiceCall->startTime(), "");
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
    Q_D(const YottagramVoiceCallHandler);
    return d->yottagramVoiceCall->multiparty();
}

bool YottagramVoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->yottagramVoiceCall->emergency();
}

//XXX Monitor VoiceCallManager Forwarded signal
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
    QString state = d->yottagramVoiceCall->state();

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
    if (status() == STATUS_WAITING)
        d->yottagramVoiceCallManager->holdAndAnswer();
    else
        d->yottagramVoiceCall->answer();
}

void YottagramVoiceCallHandler::hangup()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->yottagramVoiceCall->hangup();
}

void YottagramVoiceCallHandler::hold(bool on)
{
    Q_UNUSED(on)
    TRACE
    Q_D(YottagramVoiceCallHandler);
    bool isHeld = status() == STATUS_HELD;
    if (isHeld == on)
        return;

    d->yottagramVoiceCallManager->swapCalls();
}

void YottagramVoiceCallHandler::deflect(const QString &target)
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->yottagramVoiceCall->deflect(target);
}

void YottagramVoiceCallHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->yottagramVoiceCallManager->sendTones(tones);
}

void YottagramVoiceCallHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(YottagramVoiceCallHandler);

    // Whilst call is active, increase duration by a second each second.
    if(isOngoing() && event->timerId() == d->durationTimerId)
    {
        d->duration += d->elapsedTimer.restart();
        emit this->durationChanged(d->duration);
    }
}

void YottagramVoiceCallHandler::onStatusChanged()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);

    if (isOngoing())
    {
        if (d->durationTimerId == -1) {
            d->durationTimerId = this->startTimer(1000);
            d->elapsedTimer.start();
        }
    }
    else if (d->durationTimerId != -1)
    {
        this->killTimer(d->durationTimerId);
        d->durationTimerId = -1;
    }

    emit statusChanged(status());
}
