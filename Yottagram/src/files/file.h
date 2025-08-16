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

#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <td/telegram/Client.h>
#include <memory>
#include "../core/telegrammanager.h"

using namespace td;

class File : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int32_t id READ getId NOTIFY idChanged)
    Q_PROPERTY(QString remoteId READ getRemoteId NOTIFY remoteIdChanged)
    Q_PROPERTY(QString remoteUniqueId READ getRemoteUniqueId NOTIFY remoteUniqueIdChanged)
    Q_PROPERTY(QString localPath READ localPath NOTIFY localPathChanged)
    Q_PROPERTY(int32_t expectedSize READ getExpectedSize NOTIFY expectedSizeChanged)
    Q_PROPERTY(int32_t downloadedSize READ getDownloadedSize NOTIFY downloadedSizeChanged)
    Q_PROPERTY(int32_t uploadedSize READ getUploadedSize NOTIFY uploadedSizeChanged)
    Q_PROPERTY(bool isDownloaded READ isDownloaded NOTIFY isDownloadedChanged)
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY isDownloadingChanged)
    Q_PROPERTY(bool isUploaded READ isUploaded NOTIFY isUploadedChanged)
    Q_PROPERTY(bool isUploading READ isUploading NOTIFY isUploadingChanged)
public:
    explicit File(QObject *parent = nullptr);
    File(td_api::object_ptr<td_api::file> file, std::shared_ptr<TelegramManager> manager);
    ~File();

    td_api::file* getFile();
    void setFile(td_api::object_ptr<td_api::file> file);
    void updateFile(td_api::object_ptr<td_api::file> file);

    int32_t getId();
    QString getRemoteId();
    QString getRemoteUniqueId();
    QString localPath();
    Q_INVOKABLE void download();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void cancelUpload();
    bool isDownloaded();
    bool isDownloading();
    bool isUploaded();
    bool isUploading();
    int32_t getExpectedSize();
    int32_t getDownloadedSize();
    int32_t getUploadedSize();

signals:
    void idChanged(int32_t fileId);
    void localPathChanged(QString path);
    void remoteIdChanged(QString remoteId);
    void remoteUniqueIdChanged(QString remoteUniqueId);
    void expectedSizeChanged(int32_t expectedSize);
    void downloadedSizeChanged(int32_t downloadedSize);
    void uploadedSizeChanged(int32_t uploadedSize);
    void isDownloadedChanged(bool isDownloaded);
    void isDownloadingChanged(bool isDownloading);
    void isUploadedChanged(bool isUploaded);
    void isUploadingChanged(bool isUploading);

private:
    td_api::object_ptr<td_api::file> _file;
    shared_ptr<TelegramManager> _manager;
};
Q_DECLARE_METATYPE(File*)

#endif // FILE_H
