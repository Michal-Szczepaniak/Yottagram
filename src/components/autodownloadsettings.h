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

#ifndef AUTODOWNLOADSETTINGS_H
#define AUTODOWNLOADSETTINGS_H

#include <QObject>
#include "../core/telegrammanager.h"

class AutoDownloadSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAutoDownloadEnabled READ getIsAutoDownloadEnabled WRITE setIsAutoDownloadEnabled NOTIFY isAutoDownloadEnabledChanged)
    Q_PROPERTY(qint32 maxPhotoFileSize READ getMaxPhotoFileSize WRITE setMaxPhotoFileSize NOTIFY maxPhotoFileSizeChanged)
    Q_PROPERTY(qint32 maxVideoFileSize READ getMaxVideoFileSize WRITE setMaxVideoFileSize NOTIFY maxVideoFileSizeChanged)
    Q_PROPERTY(qint32 maxOtherFileSize READ getMaxOtherFileSize WRITE setMaxOtherFileSize NOTIFY maxOtherFileSizeChanged)
    Q_PROPERTY(qint32 videoUploadBitrate READ getVideoUploadBitrate WRITE setVideoUploadBitrate NOTIFY videoUploadBitrateChanged)
    Q_PROPERTY(bool preloadLargeVideos READ getPreloadLargeVideos WRITE setPreloadLargeVideos NOTIFY preloadLargeVideosChanged)
    Q_PROPERTY(bool preloadNextAudio READ getPreloadNextAudio WRITE setPreloadNextAudio NOTIFY preloadNextAudioChanged)
    Q_PROPERTY(bool useLessDataForCalls READ getUseLessDataForCalls WRITE setUseLessDataForCalls NOTIFY useLessDataForCallsChanged)
public:
    explicit AutoDownloadSettings(QObject *parent = nullptr);
    void setConnectionType(QString type);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void loadSettings();

    bool getIsAutoDownloadEnabled() const;
    void setIsAutoDownloadEnabled(bool isAutoDownloadEnabled);
    qint32 getMaxPhotoFileSize() const;
    void setMaxPhotoFileSize(qint32 maxPhotoFileSize);
    qint32 getMaxVideoFileSize() const;
    void setMaxVideoFileSize(qint32 maxVideoFileSize);
    qint32 getMaxOtherFileSize() const;
    void setMaxOtherFileSize(qint32 maxVideoFileSize);
    qint32 getVideoUploadBitrate() const;
    void setVideoUploadBitrate(qint32 videoUploadBitrate);
    bool getPreloadLargeVideos() const;
    void setPreloadLargeVideos(bool preloadLargeVideos);
    bool getPreloadNextAudio() const;
    void setPreloadNextAudio(bool preloadNextAudio);
    bool getUseLessDataForCalls() const;
    void setUseLessDataForCalls(bool useLessDataForCalls);

    td_api::object_ptr<td_api::NetworkType> getNetworkType() const;
    void saveSettings();
    Q_INVOKABLE void loadPreset(QString preset);
    void applyPreset(td_api::object_ptr<td_api::autoDownloadSettings> settings);

signals:
    void isAutoDownloadEnabledChanged();
    void maxPhotoFileSizeChanged();
    void maxVideoFileSizeChanged();
    void maxOtherFileSizeChanged();
    void videoUploadBitrateChanged();
    void preloadLargeVideosChanged();
    void preloadNextAudioChanged();
    void useLessDataForCallsChanged();
    void resetPendingChanged();

public slots:
    void autoDownloadSettingsPresets(td_api::autoDownloadSettingsPresets* autoDownloadSettingsPresets);

private:
    std::shared_ptr<TelegramManager> _manager;
    QString _connectionType;
    bool 	_isAutoDownloadEnabled;
    qint32 	_maxPhotoFileSize;
    qint32 	_maxVideoFileSize;
    qint32 	_maxOtherFileSize;
    qint32 	_videoUploadBitrate;
    bool 	_preloadLargeVideos;
    bool 	_preloadNextAudio;
    bool 	_useLessDataForCalls;
    QString _presetLoading;
};

#endif // AUTODOWNLOADSETTINGS_H
