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
    Q_PROPERTY(int32_t id READ getId NOTIFY fileChanged)
    Q_PROPERTY(QString remoteId READ getRemoteId NOTIFY fileChanged)
    Q_PROPERTY(QString remoteUniqueId READ getRemoteUniqueId NOTIFY fileChanged)
    Q_PROPERTY(QString localPath READ localPath NOTIFY localPathChanged)
    Q_PROPERTY(int32_t expectedSize READ getExpectedSize NOTIFY fileChanged)
    Q_PROPERTY(int32_t downloadedSize READ getDownloadedSize NOTIFY fileChanged)
    Q_PROPERTY(int32_t uploadedSize READ getUploadedSize NOTIFY fileChanged)
    Q_PROPERTY(bool isDownloaded READ isDownloaded NOTIFY fileChanged)
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY fileChanged)
    Q_PROPERTY(bool isUploaded READ isUploaded NOTIFY fileChanged)
    Q_PROPERTY(bool isUploading READ isUploading NOTIFY fileChanged)
public:
    explicit File(QObject *parent = nullptr);
    File(td_api::object_ptr<td_api::file> file, std::shared_ptr<TelegramManager> manager);
    ~File();

    td_api::file* getFile();
    void setFile(td_api::object_ptr<td_api::file> file);

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
    void fileChanged(int32_t fileId);
    void localPathChanged(QString path);

public slots:
    void fileUpdated(td_api::updateFile *update_file);

private:
    td_api::object_ptr<td_api::file> _file;
    shared_ptr<TelegramManager> _manager;
};
Q_DECLARE_METATYPE(File*)

#endif // FILE_H
