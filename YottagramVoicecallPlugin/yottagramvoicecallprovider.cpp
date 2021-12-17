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

#include <qyottagrammodem.h>
#include <qyottagramvoicecallmanager.h>

class YottagramVoiceCallProviderPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallProvider)

public:
    YottagramVoiceCallProviderPrivate(YottagramVoiceCallProvider *q, VoiceCallManagerInterface *pManager)
        : q_ptr(q), manager(pManager), yottagramManager(NULL), yottagramModem(NULL)
    { /* ... */ }

    YottagramVoiceCallProvider *q_ptr;

    VoiceCallManagerInterface *manager;

    QYottagramVoiceCallManager   *yottagramManager;
    QYottagramModem              *yottagramModem;
    QString modemPath;

    QHash<QString,YottagramVoiceCallHandler*> voiceCalls;
    QHash<QString,YottagramVoiceCallHandler*> invalidVoiceCalls;

    QString errorString;
    void setError(const QString &errorString)
    {
        this->errorString = errorString;
        debugMessage(errorString);
    }

    void debugMessage(const QString &message)
    {
        DEBUG_T("YottagramVoiceCallProvider(%s): %s", qPrintable(yottagramModem->modemPath()), qPrintable(message));
    }
};

YottagramVoiceCallProvider::YottagramVoiceCallProvider(const QString &path, VoiceCallManagerInterface *manager, QObject *parent)
    : AbstractVoiceCallProvider(parent), d_ptr(new YottagramVoiceCallProviderPrivate(this, manager))
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    d->modemPath = path;
    d->yottagramModem = new QYottagramModem(this);
    d->yottagramModem->setModemPath(path);
    connect(d->yottagramModem, SIGNAL(interfacesChanged(QStringList)), this, SLOT(interfacesChanged(QStringList)));

    if (d->yottagramModem->interfaces().contains(QLatin1String("org.yottagram.VoiceCallManager")))
        initialize();
}

void YottagramVoiceCallProvider::initialize()
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    d->yottagramManager = new QYottagramVoiceCallManager(this);
    d->yottagramManager->setModemPath(d->modemPath);

    QObject::connect(d->yottagramManager, SIGNAL(callAdded(QString)), SLOT(onCallAdded(QString)));
    QObject::connect(d->yottagramManager, SIGNAL(callRemoved(QString)), SLOT(onCallRemoved(QString)));

    foreach (const QString &call, d->yottagramManager->getCalls())
        onCallAdded(call);
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
    return QString("yottagram-") + d->modemPath;
}

QString YottagramVoiceCallProvider::providerType() const
{
    TRACE
    return "cellular";
}

QList<AbstractVoiceCallHandler*> YottagramVoiceCallProvider::voiceCalls() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    QList<AbstractVoiceCallHandler*> results;

    foreach(AbstractVoiceCallHandler* handler, d->voiceCalls.values())
    {
        results.append(handler);
    }

    return results;
}

QString YottagramVoiceCallProvider::errorString() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    return d->errorString;
}

bool YottagramVoiceCallProvider::dial(const QString &msisdn)
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    if(!d->yottagramManager || !d->yottagramManager->isValid())
    {
        d->setError("yottagram connection is not valid");
        return false;
    }

    d->yottagramManager->dial(msisdn, "default");
    return true;
}

QYottagramModem* YottagramVoiceCallProvider::modem() const
{
    TRACE
    Q_D(const YottagramVoiceCallProvider);
    return d->yottagramModem;
}
