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

#ifndef YOTTAGRAMVOICECALLHANDLER_H
#define YOTTAGRAMVOICECALLHANDLER_H

#include <abstractvoicecallhandler.h>

class YottagramVoiceCallProvider;
class QYottagramVoiceCallManager;

class YottagramVoiceCallHandler : public AbstractVoiceCallHandler
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path)

public:
    explicit YottagramVoiceCallHandler(const QString &handlerId, const QString &path, YottagramVoiceCallProvider *provider, QYottagramVoiceCallManager *manager);
            ~YottagramVoiceCallHandler();

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

    void merge(const QString &) {}
    void split() {}

private:
    class YottagramVoiceCallHandlerPrivate *d_ptr;

    Q_DECLARE_PRIVATE(YottagramVoiceCallHandler)
};

#endif // YOTTAGRAMVOICECALLHANDLER_H
