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

#include "autodownloadsettings.h"

AutoDownloadSettings::AutoDownloadSettings(QObject *parent) : QObject(parent), _presetLoading("")
{

}

void AutoDownloadSettings::setConnectionType(QString type)
{
    _connectionType = type;
}

void AutoDownloadSettings::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void AutoDownloadSettings::loadSettings()
{
    QSettings qsettings;
    qsettings.beginGroup(_connectionType + "autoDownloadSettings");
    _isAutoDownloadEnabled = qsettings.value("autoDownload", true).toBool();
    _maxPhotoFileSize = qsettings.value("maxPhotoFileSize", 1048576).toInt();
    _maxVideoFileSize = qsettings.value("maxVideoFileSize", 1048576).toInt();
    _maxOtherFileSize = qsettings.value("maxOtherFileSize", 1048576).toInt();
    _videoUploadBitrate = qsettings.value("videoBitrate", 1048576).toInt();
    _preloadLargeVideos = qsettings.value("preloadLargeVideos", false).toBool();
    _preloadNextAudio = qsettings.value("preloadNextAudio", false).toBool();
    _useLessDataForCalls = qsettings.value("useLessDataForCalls", true).toBool();
    qsettings.endGroup();

    auto settings = td_api::make_object<td_api::autoDownloadSettings>(
        _isAutoDownloadEnabled,
        _maxPhotoFileSize,
        _maxVideoFileSize,
        _maxOtherFileSize,
        _videoUploadBitrate,
        _preloadLargeVideos,
        _preloadNextAudio,
        _useLessDataForCalls
    );
    _manager->sendQuery(new td_api::setAutoDownloadSettings(move(settings), getNetworkType()));
}

bool AutoDownloadSettings::getIsAutoDownloadEnabled() const
{
    return _isAutoDownloadEnabled;
}

void AutoDownloadSettings::setIsAutoDownloadEnabled(bool isAutoDownloadEnabled)
{
    _isAutoDownloadEnabled = isAutoDownloadEnabled;
    saveSettings();
    emit isAutoDownloadEnabledChanged();
}

int32_t AutoDownloadSettings::getMaxPhotoFileSize() const
{
    return _maxPhotoFileSize;
}

void AutoDownloadSettings::setMaxPhotoFileSize(int32_t maxPhotoFileSize)
{
    _maxPhotoFileSize = maxPhotoFileSize;
    saveSettings();
    emit maxPhotoFileSizeChanged();
}

int32_t AutoDownloadSettings::getMaxVideoFileSize() const
{
    return _maxVideoFileSize;
}

void AutoDownloadSettings::setMaxVideoFileSize(int32_t maxVideoFileSize)
{
    _maxVideoFileSize = maxVideoFileSize;
    saveSettings();
    emit maxVideoFileSizeChanged();
}

int32_t AutoDownloadSettings::getMaxOtherFileSize() const
{
    return _maxOtherFileSize;
}

void AutoDownloadSettings::setMaxOtherFileSize(int32_t maxOtherFileSize)
{
    _maxOtherFileSize = maxOtherFileSize;
    saveSettings();
    emit maxOtherFileSizeChanged();
}

int32_t AutoDownloadSettings::getVideoUploadBitrate() const
{
    return _videoUploadBitrate;
}

void AutoDownloadSettings::setVideoUploadBitrate(int32_t videoUploadBitrate)
{
    _videoUploadBitrate = videoUploadBitrate;
    saveSettings();
    emit videoUploadBitrateChanged();
}

bool AutoDownloadSettings::getPreloadLargeVideos() const
{
    return _preloadLargeVideos;
}

void AutoDownloadSettings::setPreloadLargeVideos(bool preloadLargeVideos)
{
    _preloadLargeVideos = preloadLargeVideos;
    saveSettings();
    emit preloadLargeVideosChanged();
}

bool AutoDownloadSettings::getPreloadNextAudio() const
{
    return _preloadNextAudio;
}

void AutoDownloadSettings::setPreloadNextAudio(bool preloadNextAudio)
{
    _preloadNextAudio = preloadNextAudio;
    saveSettings();
    emit preloadNextAudioChanged();
}

bool AutoDownloadSettings::getUseLessDataForCalls() const
{
    return _useLessDataForCalls;
}

void AutoDownloadSettings::setUseLessDataForCalls(bool useLessDataForCalls)
{
    _useLessDataForCalls = useLessDataForCalls;
    saveSettings();
    emit useLessDataForCallsChanged();
}

td_api::object_ptr<td_api::NetworkType> AutoDownloadSettings::getNetworkType() const
{

    if (_connectionType == "none") return td_api::make_object<td_api::networkTypeNone>();
    if (_connectionType == "wifi") return td_api::make_object<td_api::networkTypeWiFi>();
    if (_connectionType == "cellular") return td_api::make_object<td_api::networkTypeMobile>();
    if (_connectionType == "roaming") return td_api::make_object<td_api::networkTypeMobileRoaming>();
    return td_api::make_object<td_api::networkTypeOther>();
}

void AutoDownloadSettings::saveSettings()
{
    auto settings = td_api::make_object<td_api::autoDownloadSettings>(
        _isAutoDownloadEnabled,
        _maxPhotoFileSize,
        _maxVideoFileSize,
        _maxOtherFileSize,
        _videoUploadBitrate,
        _preloadLargeVideos,
        _preloadNextAudio,
        _useLessDataForCalls
    );

    QSettings qsettings;
    qsettings.beginGroup(_connectionType + "autoDownloadSettings");
    qsettings.setValue("autoDownload", _isAutoDownloadEnabled);
    qsettings.setValue("maxPhotoFileSize", _maxPhotoFileSize);
    qsettings.setValue("maxVideoFileSize", _maxVideoFileSize);
    qsettings.setValue("maxOtherFileSize", _maxOtherFileSize);
    qsettings.setValue("videoBitrate", _videoUploadBitrate);
    qsettings.setValue("preloadLargeVideos", _preloadLargeVideos);
    qsettings.setValue("preloadNextAudio", _preloadNextAudio);
    qsettings.setValue("useLessDataForCalls", _useLessDataForCalls);
    qsettings.endGroup();

    emit isAutoDownloadEnabledChanged();
    emit maxPhotoFileSizeChanged();
    emit maxVideoFileSizeChanged();
    emit maxOtherFileSizeChanged();
    emit videoUploadBitrateChanged();
    emit preloadLargeVideosChanged();
    emit preloadNextAudioChanged();
    emit useLessDataForCallsChanged();

    _manager->sendQuery(new td_api::setAutoDownloadSettings(move(settings), getNetworkType()));
}

void AutoDownloadSettings::loadPreset(QString preset)
{
    _presetLoading = preset;

    _manager->sendQuery(new td_api::getAutoDownloadSettingsPresets());

}

void AutoDownloadSettings::applyPreset(td_api::object_ptr<td_api::autoDownloadSettings> settings)
{
    _isAutoDownloadEnabled = settings->is_auto_download_enabled_;
    _maxPhotoFileSize = settings->max_photo_file_size_;
    _maxVideoFileSize = settings->max_video_file_size_;
    _maxOtherFileSize = settings->max_other_file_size_;
    _videoUploadBitrate = settings->video_upload_bitrate_;
    _preloadLargeVideos = settings->preload_large_videos_;
    _preloadNextAudio = settings->preload_next_audio_;
    _useLessDataForCalls = settings->use_less_data_for_calls_;
}

void AutoDownloadSettings::autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets *autoDownloadSettingsPresets)
{
    if (_presetLoading == "" || autoDownloadSettingsPresets->low_ == nullptr || autoDownloadSettingsPresets->medium_ == nullptr || autoDownloadSettingsPresets->high_ == nullptr) return;

    if (_presetLoading == "low") applyPreset(std::move(autoDownloadSettingsPresets->low_));
    if (_presetLoading == "medium") applyPreset(std::move(autoDownloadSettingsPresets->medium_));
    if (_presetLoading == "high") applyPreset(std::move(autoDownloadSettingsPresets->high_));

    saveSettings();

    _presetLoading = "";
}
