/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
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
#include "ofonovoicecallhandler.h"
#include "ofonovoicecallprovider.h"

#include <qofonovoicecall.h>
#include <qofonovoicecallmanager.h>

#include <QElapsedTimer>
#include <QTimerEvent>

class OfonoVoiceCallHandlerPrivate
{
    Q_DECLARE_PUBLIC(OfonoVoiceCallHandler)

public:
    OfonoVoiceCallHandlerPrivate(OfonoVoiceCallHandler *q, const QString &pHandlerId, OfonoVoiceCallProvider *pProvider, QOfonoVoiceCallManager *manager)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), ofonoVoiceCallManager(manager), ofonoVoiceCall(NULL)
        , duration(0), durationTimerId(-1), isIncoming(false)
    { /* ... */ }

    OfonoVoiceCallHandler *q_ptr;

    QString handlerId;

    OfonoVoiceCallProvider *provider;

    QOfonoVoiceCallManager *ofonoVoiceCallManager;
    QOfonoVoiceCall *ofonoVoiceCall;

    quint64 duration;
    int durationTimerId;
    QElapsedTimer elapsedTimer;
    bool isIncoming;
};

OfonoVoiceCallHandler::OfonoVoiceCallHandler(const QString &handlerId, const QString &path, OfonoVoiceCallProvider *provider, QOfonoVoiceCallManager *manager)
    : AbstractVoiceCallHandler(provider), d_ptr(new OfonoVoiceCallHandlerPrivate(this, handlerId, provider, manager))
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall = new QOfonoVoiceCall(this);
    d->ofonoVoiceCall->setVoiceCallPath(path);

    QObject::connect(d->ofonoVoiceCall, SIGNAL(lineIdentificationChanged(QString)), SIGNAL(lineIdChanged(QString)));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(emergencyChanged(bool)), SIGNAL(emergencyChanged(bool)));
    QObject::connect(d->ofonoVoiceCall, SIGNAL(multipartyChanged(bool)), SIGNAL(multipartyChanged(bool)));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(stateChanged(QString)), SLOT(onStatusChanged()));

    QObject::connect(d->ofonoVoiceCall, SIGNAL(validChanged(bool)), SLOT(onValidChanged(bool)));
    if(d->ofonoVoiceCall->isValid()) {
        onValidChanged(true);
    }
}

OfonoVoiceCallHandler::~OfonoVoiceCallHandler()
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    delete d;
}

void OfonoVoiceCallHandler::onValidChanged(bool isValid)
{
    Q_D(OfonoVoiceCallHandler);

    if (isValid)
    {
        // Properties are now ready
        d->isIncoming = d->ofonoVoiceCall->state() == QLatin1String("incoming");
    }

    emit validChanged(isValid);
}

QString OfonoVoiceCallHandler::path() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->voiceCallPath();
}

AbstractVoiceCallProvider* OfonoVoiceCallHandler::provider() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->provider;
}

QString OfonoVoiceCallHandler::handlerId() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->handlerId;
}

QString OfonoVoiceCallHandler::lineId() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->lineIdentification();
}

QDateTime OfonoVoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    DEBUG_T("CALL START TIME: %s", qPrintable(d->ofonoVoiceCall->startTime()));
    return QDateTime::fromString(d->ofonoVoiceCall->startTime(), "");
}

int OfonoVoiceCallHandler::duration() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return int(qRound(d->duration/1000.0));
}

bool OfonoVoiceCallHandler::isIncoming() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->isIncoming;
}

bool OfonoVoiceCallHandler::isMultiparty() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->multiparty();
}

bool OfonoVoiceCallHandler::isEmergency() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    return d->ofonoVoiceCall->emergency();
}

//XXX Monitor VoiceCallManager Forwarded signal
bool OfonoVoiceCallHandler::isForwarded() const
{
    TRACE
    return false;
}

bool OfonoVoiceCallHandler::isRemoteHeld() const
{
    TRACE
    return false;
}

AbstractVoiceCallHandler::VoiceCallStatus OfonoVoiceCallHandler::status() const
{
    TRACE
    Q_D(const OfonoVoiceCallHandler);
    QString state = d->ofonoVoiceCall->state();

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

void OfonoVoiceCallHandler::answer()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    if (status() == STATUS_WAITING)
        d->ofonoVoiceCallManager->holdAndAnswer();
    else
        d->ofonoVoiceCall->answer();
}

void OfonoVoiceCallHandler::hangup()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall->hangup();
}

void OfonoVoiceCallHandler::hold(bool on)
{
    Q_UNUSED(on)
    TRACE
    Q_D(OfonoVoiceCallHandler);
    bool isHeld = status() == STATUS_HELD;
    if (isHeld == on)
        return;

    d->ofonoVoiceCallManager->swapCalls();
}

void OfonoVoiceCallHandler::deflect(const QString &target)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCall->deflect(target);
}

void OfonoVoiceCallHandler::sendDtmf(const QString &tones)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);
    d->ofonoVoiceCallManager->sendTones(tones);
}

void OfonoVoiceCallHandler::timerEvent(QTimerEvent *event)
{
    TRACE
    Q_D(OfonoVoiceCallHandler);

    // Whilst call is active, increase duration by a second each second.
    if(isOngoing() && event->timerId() == d->durationTimerId)
    {
        d->duration += d->elapsedTimer.restart();
        emit this->durationChanged(d->duration);
    }
}

void OfonoVoiceCallHandler::onStatusChanged()
{
    TRACE
    Q_D(OfonoVoiceCallHandler);

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
