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

#ifndef YOTTAGRAMTRANSFERIFACE_H
#define YOTTAGRAMTRANSFERIFACE_H

#include <TransferEngine-qt5/transferplugininterface.h>
#include <TransferEngine-qt5/transferplugininfo.h>
#include <TransferEngine-qt5/transfermethodinfo.h>
#include <TransferEngine-qt5/mediatransferinterface.h>

class Q_DECL_EXPORT YottagramSharePlugin : public QObject, public TransferPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.verdanditeam.yottagram.share")
    Q_INTERFACES(TransferPluginInterface)
public:
    YottagramSharePlugin();
    ~YottagramSharePlugin();

    MediaTransferInterface *transferObject();
    TransferPluginInfo *infoObject();
    QString pluginId() const;
    bool enabled() const;
};

#endif // YOTTAGRAMTRANSFERIFACE_H
