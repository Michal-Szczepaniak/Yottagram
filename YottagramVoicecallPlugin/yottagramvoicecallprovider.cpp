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

class YottagramVoiceCallProviderPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallProvider)

public:
    YottagramVoiceCallProviderPrivate(YottagramVoiceCallProvider *q, VoiceCallManagerInterface *pManager)
        : q_ptr(q), manager(pManager)
    { }

    YottagramVoiceCallProvider *q_ptr;

    VoiceCallManagerInterface *manager;

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
        DEBUG_T("YottagramVoiceCallProvider");
    }
};

YottagramVoiceCallProvider::YottagramVoiceCallProvider(const QString &path, VoiceCallManagerInterface *manager, QObject *parent)
    : AbstractVoiceCallProvider(parent), d_ptr(new YottagramVoiceCallProviderPrivate(this, manager))
{
    TRACE
    Q_D(YottagramVoiceCallProvider);
    d->modemPath = path;
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
    return false;
}
