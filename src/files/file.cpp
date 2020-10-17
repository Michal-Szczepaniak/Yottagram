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

#include "file.h"
#include <QDebug>

File::File(QObject *parent) : QObject(parent)
{
    _file = nullptr;
}

File::File(td_api::object_ptr<td_api::file> file, std::shared_ptr<TelegramManager> manager) : _file{std::move(file)}, _manager(manager)
{

}

File::~File()
{
}

td_api::file *File::getFile()
{
    return _file.get();
}

void File::setFile(td_api::object_ptr<td_api::file> file)
{
    _file = std::move(file);

    emit localPathChanged(localPath());
}

qint32 File::getId()
{
    return _file->id_;
}

QString File::getRemoteId()
{
    return QString::fromStdString(_file->remote_->id_);
}

QString File::getRemoteUniqueId()
{
    return QString::fromStdString(_file->remote_->unique_id_);
}

QString File::localPath()
{
    return QString::fromStdString(_file->local_->path_);
}

void File::download()
{
    _manager->sendQuery(new td_api::downloadFile(getId(), 32, 0, 0, false));
}

bool File::isDownloaded()
{
    return _file->local_->is_downloading_completed_;
}

bool File::isDownloading()
{
    return _file->local_->is_downloading_active_;
}

bool File::isUploaded()
{
    return _file->remote_->is_uploading_completed_;
}

bool File::isUploading()
{
    return _file->remote_->is_uploading_active_;
}

qint32 File::getExpectedSize()
{
    return _file->expected_size_;
}

qint32 File::getDownloadedSize()
{
    return _file->local_->downloaded_size_;
}

qint32 File::getUploadedSize()
{
    return _file->remote_->uploaded_size_;
}

void File::fileUpdated(td_api::updateFile *update_file)
{
    if (update_file->file_ != nullptr && update_file->file_ != nullptr && update_file->file_->id_ == getId()) {
        setFile(std::move(update_file->file_));

        emit fileChanged(getId());
    }
}
