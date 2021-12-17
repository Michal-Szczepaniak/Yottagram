/*
    Copyright (C) 2018 Michał Szczepaniak

    This file is part of Yottagram.

    Yottagram is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Yottagram is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yottagram.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include "YottagramMediaTransfer.h"
#include <QStringList>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>

YottagramMediaTransfer::YottagramMediaTransfer(QObject *parent) :
    MediaTransferInterface(parent)
{
}

YottagramMediaTransfer::~YottagramMediaTransfer()
{
}

QString YottagramMediaTransfer::displayName() const
{
    return QString("Yottagram");
}

QUrl YottagramMediaTransfer::serviceIcon() const
{
    return QUrl::fromLocalFile("/usr/share/icons/hicolor/256x256/apps/Yottagram.png");
}

bool YottagramMediaTransfer::cancelEnabled() const
{
    return false;
}

bool YottagramMediaTransfer::restartEnabled() const
{
    return false;
}

void YottagramMediaTransfer::start()
{
}

void YottagramMediaTransfer::uploadMediaStarted() {
}

void YottagramMediaTransfer::cancel() {
}

void YottagramMediaTransfer::uploadMediaProgress(qint64 bytesSent, qint64 bytesTotal) {
}

void YottagramMediaTransfer::uploadMediaFinished(bool ok) {
    if (ok) {
        setStatus(MediaTransferInterface::TransferFinished);
    } else {
        setStatus(MediaTransferInterface::TransferInterrupted);
    }
}
