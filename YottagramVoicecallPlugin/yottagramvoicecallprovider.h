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

#ifndef YOTTAGRAMVOICECALLPROVIDER_H
#define YOTTAGRAMVOICECALLPROVIDER_H

#include <abstractvoicecallprovider.h>
#include <voicecallmanagerinterface.h>

class YottagramVoiceCallProvider : public AbstractVoiceCallProvider
{
    Q_OBJECT
public:
    explicit YottagramVoiceCallProvider(const QString &path, VoiceCallManagerInterface *manager, QObject *parent = 0);
            ~YottagramVoiceCallProvider();

    QString providerId() const;
    QString providerType() const;
    QList<AbstractVoiceCallHandler*> voiceCalls() const;
    QString errorString() const;

public Q_SLOTS:
    bool dial(const QString &msisdn);

private:
    class YottagramVoiceCallProviderPrivate *d_ptr;

    Q_DECLARE_PRIVATE(YottagramVoiceCallProvider)
};

#endif // YOTTAGRAMVOICECALLPROVIDER_H
