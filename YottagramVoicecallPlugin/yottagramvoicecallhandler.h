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
#ifndef OFONOVOICECALLHANDLER_H
#define OFONOVOICECALLHANDLER_H

#include <abstractvoicecallhandler.h>

class OfonoVoiceCallProvider;
class QOfonoVoiceCallManager;

class OfonoVoiceCallHandler : public AbstractVoiceCallHandler
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path)

public:
    explicit OfonoVoiceCallHandler(const QString &handlerId, const QString &path, OfonoVoiceCallProvider *provider, QOfonoVoiceCallManager *manager);
            ~OfonoVoiceCallHandler();

    QString path() const;

    AbstractVoiceCallProvider* provider() const;

    QString handlerId() const;
    QString lineId() const;
    QDateTime startedAt() const;
    int duration() const;
    bool isIncoming() const;
    bool isMultiparty() const;
    bool isEmergency() const;
    bool isForwarded() const;
    bool isRemoteHeld() const;

    // TODO: unimplemented - JB#35997
    QString parentHandlerId() const { return QString(); }
    QList<AbstractVoiceCallHandler*> childCalls() const { return QList<AbstractVoiceCallHandler*>(); }

    VoiceCallStatus status() const;

Q_SIGNALS:
    void validChanged(bool valid);

public Q_SLOTS:
    void answer();
    void hangup();
    void hold(bool on = true);
    void deflect(const QString &target);
    void sendDtmf(const QString &tones);

    // TODO: unimplemented - JB#35997
    void merge(const QString &) {}
    void split() {}

protected Q_SLOTS:
    void onStatusChanged();
    void onValidChanged(bool);

protected:
    void timerEvent(QTimerEvent *event);

private:
    class OfonoVoiceCallHandlerPrivate *d_ptr;

    Q_DECLARE_PRIVATE(OfonoVoiceCallHandler)
};

#endif // OFONOVOICECALLHANDLER_H
