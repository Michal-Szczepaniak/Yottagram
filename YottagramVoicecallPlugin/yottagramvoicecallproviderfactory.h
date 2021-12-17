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
#ifndef YOTTAGRAMVOICECALLPROVIDERFACTORY_H
#define YOTTAGRAMVOICECALLPROVIDERFACTORY_H

#include <abstractvoicecallmanagerplugin.h>

class YottagramVoiceCallProviderFactory : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)
    Q_PLUGIN_METADATA(IID "com.verdanditeam.yottagram.voicecall")
public:
    explicit YottagramVoiceCallProviderFactory(QObject *parent = 0);
            ~YottagramVoiceCallProviderFactory();

    QString pluginId() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

private:
    class YottagramVoiceCallProviderFactoryPrivate *d_ptr;

    Q_DECLARE_PRIVATE(YottagramVoiceCallProviderFactory)
};

#endif // YOTTAGRAMVOICECALLPROVIDERFACTORY_H
