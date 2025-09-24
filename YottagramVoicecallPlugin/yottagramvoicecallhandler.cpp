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
    YottagramVoiceCallHandlerPrivate(YottagramVoiceCallHandler *q, const QString &pHandlerId, const QString &callerName, const bool &incoming, YottagramVoiceCallProvider *pProvider, VoiceCallManagerInterface *manager)
        : q_ptr(q), handlerId(pHandlerId), provider(pProvider), manager(manager), callerName(callerName), isIncoming(incoming), status(incoming ? AbstractVoiceCallHandler::STATUS_INCOMING : AbstractVoiceCallHandler::STATUS_DIALING)
    { /* ... */ }

    YottagramVoiceCallHandler *q_ptr;

    QString handlerId;
    QString callerName;
    VoiceCallManagerInterface *manager;

    YottagramVoiceCallProvider *provider;

    bool isIncoming;
    QDateTime startTime;
    QTimer timer;
    AbstractVoiceCallHandler::VoiceCallStatus status;
};

YottagramVoiceCallHandler::YottagramVoiceCallHandler(const QString &handlerId, const QString &callerName, const bool &incoming, YottagramVoiceCallProvider *provider, VoiceCallManagerInterface *manager)
    : AbstractVoiceCallHandler(provider), d_ptr(new YottagramVoiceCallHandlerPrivate(this, handlerId, callerName, incoming, provider, manager))
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->timer.setSingleShot(false);
    d->timer.setInterval(1000);
    connect(&d->timer, &QTimer::timeout, [this](){ emit durationChanged(duration()); });
}

YottagramVoiceCallHandler::~YottagramVoiceCallHandler()
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    delete d;
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
    return d->callerName;
}

QString YottagramVoiceCallHandler::subscriberId() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->callerName;
}

QDateTime YottagramVoiceCallHandler::startedAt() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->startTime;
}

int YottagramVoiceCallHandler::duration() const
{
    TRACE
    Q_D(const YottagramVoiceCallHandler);
    return d->startTime.secsTo(QDateTime::currentDateTime());
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
    return d->status;
}

void YottagramVoiceCallHandler::setStatus(VoiceCallStatus status)
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->status = status;
    if (status == STATUS_ACTIVE) {
        d->startTime = QDateTime::currentDateTime();
        emit startedAtChanged(d->startTime);
        d->timer.start();
    }
    emit statusChanged(d->status);
}

void YottagramVoiceCallHandler::answer()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    emit d->provider->accept();
}

void YottagramVoiceCallHandler::hangup()
{
    TRACE
    Q_D(YottagramVoiceCallHandler);
    d->timer.stop();
    emit d->provider->discard();
    setStatus(STATUS_DISCONNECTED);
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

void YottagramVoiceCallHandler::filter(VoiceCallFilterAction action)
{
    TRACE
}
