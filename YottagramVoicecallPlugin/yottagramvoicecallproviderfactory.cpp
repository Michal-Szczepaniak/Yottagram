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
#include "yottagramvoicecallproviderfactory.h"
#include "yottagramvoicecallprovider.h"

#include <voicecallmanagerinterface.h>

#include <QtPlugin>

class YottagramVoiceCallProviderFactoryPrivate
{
    Q_DECLARE_PUBLIC(YottagramVoiceCallProviderFactory)

public:
    YottagramVoiceCallProviderFactoryPrivate(YottagramVoiceCallProviderFactory *q)
        : q_ptr(q), isConfigured(false)
    {/* ... */}

    YottagramVoiceCallProviderFactory *q_ptr;

    bool isConfigured;

    VoiceCallManagerInterface *manager;

    QHash<QString, YottagramVoiceCallProvider*> providers;
};

YottagramVoiceCallProviderFactory::YottagramVoiceCallProviderFactory(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d_ptr(new YottagramVoiceCallProviderFactoryPrivate(this))
{
    TRACE
}

YottagramVoiceCallProviderFactory::~YottagramVoiceCallProviderFactory()
{
    TRACE
    Q_D(YottagramVoiceCallProviderFactory);
    delete d;
}

QString YottagramVoiceCallProviderFactory::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

bool YottagramVoiceCallProviderFactory::initialize()
{
    TRACE
    Q_D(YottagramVoiceCallProviderFactory);
    return true;
}

bool YottagramVoiceCallProviderFactory::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    Q_D(YottagramVoiceCallProviderFactory);
    if(d->isConfigured)
    {
        WARNING_T("YottagramVoiceCallProviderFactory is already configured!");
        return false;
    }

    d->manager = manager;

    d->isConfigured = true;

    return true;
}

bool YottagramVoiceCallProviderFactory::start()
{
    TRACE
    return true;
}

bool YottagramVoiceCallProviderFactory::suspend()
{
    TRACE
    return true;
}

bool YottagramVoiceCallProviderFactory::resume()
{
    TRACE
    return true;
}

void YottagramVoiceCallProviderFactory::finalize()
{
    TRACE
}
