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
#ifndef OFONOVOICECALLPROVIDER_H
#define OFONOVOICECALLPROVIDER_H

#include <abstractvoicecallprovider.h>
#include <voicecallmanagerinterface.h>

#include <qofonomodem.h>

class OfonoVoiceCallProvider : public AbstractVoiceCallProvider
{
    Q_OBJECT

    Q_PROPERTY(QOfonoModem* modem READ modem)

public:
    explicit OfonoVoiceCallProvider(const QString &path, VoiceCallManagerInterface *manager, QObject *parent = 0);
            ~OfonoVoiceCallProvider();

    QString providerId() const;
    QString providerType() const;
    QList<AbstractVoiceCallHandler*> voiceCalls() const;
    QString errorString() const;

    QOfonoModem* modem() const;

public Q_SLOTS:
    bool dial(const QString &msisdn);

protected Q_SLOTS:
    void interfacesChanged(const QStringList &interfaces);
    void onCallAdded(const QString &call);
    void onCallRemoved(const QString &call);

    void onDialComplete(const bool status);

    void onVoiceCallHandlerValidChanged(bool isValid);

private:
    void initialize();
    class OfonoVoiceCallProviderPrivate *d_ptr;

    Q_DECLARE_PRIVATE(OfonoVoiceCallProvider)
};

#endif // OFONOVOICECALLPROVIDER_H
