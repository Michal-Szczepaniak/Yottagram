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

#include "files.h"

Files::Files(QObject *parent) : QObject(parent)
{

}

void Files::setTelegramManager(std::shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void Files::setWifiAutoDownloadSettings(AutoDownloadSettings *settings)
{
    _wifiAutoDownloadSettings = settings;
}

void Files::setMobileAutoDownloadSettings(AutoDownloadSettings *settings)
{
    _mobileAutoDownloadSettings = settings;
}

void Files::setRoamingAutoDownloadSettings(AutoDownloadSettings *settings)
{
    _roamingAutoDownloadSettings = settings;
}

void Files::setOtherAutoDownloadSettings(AutoDownloadSettings *settings)
{
    _otherAutoDownloadSettings = settings;
}

void Files::appendFile(td_api::object_ptr<td_api::file> file, QString fileType)
{
    int32_t fileId = file->id_;

    if (_files.contains(file->id_)) {
        _files[file->id_]->setFile(std::move(file));
    } else {
        auto filePointer = std::make_shared<File>(std::move(file), _manager);
        connect(_manager.get(), SIGNAL(updateFile(td_api::updateFile*)), filePointer.get(), SLOT(fileUpdated(td_api::updateFile*)));
        _files.insert(filePointer->getId(), filePointer);
    }

    if (fileType == "avatar" || fileType == "sticker" || fileType == "animation" || fileType == "thumbnail") considerAutoDownloading(fileId, fileType);
}

void Files::considerAutoDownloading(int32_t fileId, QString fileType)
{
    shared_ptr<File> file = getFile(fileId);
    if (file == nullptr) return;

    if (file->isDownloading() || file->isDownloaded() || (!getActiveAutoDownloadSetting()->getIsAutoDownloadEnabled() && fileType != "avatar" && fileType != "sticker" && fileType != "thumbnail")) return;

    if (fileType == "photo" && file->getExpectedSize() <= getActiveAutoDownloadSetting()->getMaxPhotoFileSize()) file->download();
    if (fileType == "video" && file->getExpectedSize() <= getActiveAutoDownloadSetting()->getMaxVideoFileSize()) file->download();
    if (fileType == "other" && file->getExpectedSize() <= getActiveAutoDownloadSetting()->getMaxOtherFileSize()) file->download();
    if (fileType == "avatar" || fileType == "sticker" || fileType == "animation" || fileType == "thumbnail") file->download();
}

AutoDownloadSettings *Files::getActiveAutoDownloadSetting()
{
    QString networkType = _manager->getNetworkType();

    if (networkType == "wifi") return _wifiAutoDownloadSettings;
    if (networkType == "mobile") return _mobileAutoDownloadSettings;
    if (networkType == "roaming") return _roamingAutoDownloadSettings;
    return _otherAutoDownloadSettings;
}

shared_ptr<File> Files::getFile(int32_t fileId) const
{
    if (_files.contains(fileId)) {
        return _files[fileId];
    }

    return nullptr;
}
