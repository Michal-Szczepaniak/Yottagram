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

#ifndef YOTTAGRAMMEDIATRANSFER_H
#define YOTTAGRAMMEDIATRANSFER_H

#include <QObject>
#include <TransferEngine-qt5/mediatransferinterface.h>
#include <TransferEngine-qt5/mediaitem.h>

class YottagramMediaTransfer : public MediaTransferInterface
{
    Q_OBJECT
public:
    YottagramMediaTransfer(QObject * parent = 0);
    ~YottagramMediaTransfer();

    QString	displayName() const;
    QUrl	serviceIcon() const;
    bool	cancelEnabled() const;
    bool	restartEnabled() const;

private slots:
    void uploadMediaStarted();
    void uploadMediaProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadMediaFinished(bool ok);

public slots:
    void cancel();
    void start();
};

#endif // YOTTAGRAMMEDIATRANSFER_H
