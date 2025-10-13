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

#ifndef FILES_H
#define FILES_H

#include <QObject>
#include <QHash>
#include "file.h"
#include "../core/telegrammanager.h"
#include "../components/autodownloadsettings.h"

class Files : public QObject
{
    Q_OBJECT
public:
    explicit Files(QObject *parent = nullptr);

    void setTelegramManager(std::shared_ptr<TelegramManager> manager);
    void setWifiAutoDownloadSettings(AutoDownloadSettings* settings);
    void setMobileAutoDownloadSettings(AutoDownloadSettings* settings);
    void setRoamingAutoDownloadSettings(AutoDownloadSettings* settings);
    void setOtherAutoDownloadSettings(AutoDownloadSettings* settings);

    void appendFile(td_api::object_ptr<td_api::file> file, QString fileType);
    void considerAutoDownloading(int32_t fileId, QString fileType);
    AutoDownloadSettings* getActiveAutoDownloadSetting();
    shared_ptr<File> getFile(int32_t fileId) const;

public slots:
    void onFileUpdated(td_api::updateFile *updateFile);
    void onFileDownloadedChanged(bool downloaded, int32_t fileId);

private:
    shared_ptr<TelegramManager> _manager;
    QHash<int32_t, std::shared_ptr<File>> _files;
    AutoDownloadSettings* _wifiAutoDownloadSettings;
    AutoDownloadSettings* _mobileAutoDownloadSettings;
    AutoDownloadSettings* _roamingAutoDownloadSettings;
    AutoDownloadSettings* _otherAutoDownloadSettings;
};

#endif // FILES_H
