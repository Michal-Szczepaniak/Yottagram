/*
    Copyright (C) 2020 Slava Monich at al, Michał Szczepaniak.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "tgsioplugin.h"
#include "rlottie.h"

QImageIOPlugin::Capabilities TgsIOPlugin::capabilities(QIODevice*, const QByteArray& format) const
{
    return Capabilities((format == TgsIOHandler::NAME) ? CanRead : 0);
}

QImageIOHandler* TgsIOPlugin::create(QIODevice* device, const QByteArray& format) const
{
    return new TgsIOHandler(device, format);
}
