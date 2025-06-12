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

#ifndef YOTTAGRAMPLUGININFO_H
#define YOTTAGRAMPLUGININFO_H

#include <TransferEngine-qt5/sharingplugininfo.h>
#include <TransferEngine-qt5/sharingmethodinfo.h>

class YottagramPluginInfo : public SharingPluginInfo
{
    Q_OBJECT
public:
    YottagramPluginInfo();
    ~YottagramPluginInfo();

    QList<SharingMethodInfo> info() const;
    void query();
    bool ready() const;

private:
    QList<SharingMethodInfo> m_infoList;
    bool m_ready;

};

#endif // YOTTAGRAMPLUGININFO_H
