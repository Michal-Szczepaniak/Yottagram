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

File::File(td_api::object_ptr<td_api::file> file, std::shared_ptr<TelegramManager> manager, QString type) : _file{std::move(file)}, _manager(manager), _type(type)
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

    emit localPathChanged(localPath(), getId());
}

void File::updateFile(td_api::object_ptr<td_api::file> file)
{
    if (file->remote_->id_ != _file->remote_->id_) {
        _file->remote_ = move(file->remote_);
        emit remoteIdChanged(getRemoteId(), getId());
        emit remoteUniqueIdChanged(getRemoteUniqueId(), getId());
        emit uploadedSizeChanged(getUploadedSize(), getId());
        emit isUploadedChanged(isUploaded(), getId());
        emit isUploadingChanged(isUploading(), getId());
    } else {
        if (file->remote_->unique_id_ != _file->remote_->unique_id_) {
            _file->remote_->unique_id_ = file->remote_->unique_id_;
            emit remoteUniqueIdChanged(getRemoteUniqueId(), getId());
        }

        if (file->remote_->uploaded_size_ != _file->remote_->uploaded_size_) {
            _file->remote_->uploaded_size_ = file->remote_->uploaded_size_;
            emit uploadedSizeChanged(getUploadedSize(), getId());
        }

        if (file->remote_->is_uploading_completed_ != _file->remote_->is_uploading_completed_) {
            _file->remote_->is_uploading_completed_ = file->remote_->is_uploading_completed_;
            emit isUploadedChanged(isUploaded(), getId());
        }

        if (file->remote_->is_uploading_active_ != _file->remote_->is_uploading_active_) {
            _file->remote_->is_uploading_active_ = file->remote_->is_uploading_active_;
            emit isUploadingChanged(isUploading(), getId());
        }
    }

    if (file->local_->path_ != _file->local_->path_) {
        _file->local_->path_ = file->local_->path_;
        emit localPathChanged(localPath(), getId());
    }

    if (file->expected_size_ != _file->expected_size_) {
        _file->expected_size_ = file->expected_size_;
        emit expectedSizeChanged(getExpectedSize(), getId());
    }

    if (file->local_->downloaded_size_ != _file->local_->downloaded_size_) {
        _file->local_->downloaded_size_ = file->local_->downloaded_size_;
        emit downloadedSizeChanged(getDownloadedSize(), getId());
    }

    if (file->local_->is_downloading_completed_ != _file->local_->is_downloading_completed_) {
        _file->local_->is_downloading_completed_ = file->local_->is_downloading_completed_;
        emit isDownloadedChanged(isDownloaded(), getId());
    }

    if (file->local_->is_downloading_active_ != _file->local_->is_downloading_active_) {
        _file->local_->is_downloading_active_ = file->local_->is_downloading_active_;
        emit isDownloadingChanged(isDownloading(), getId());
    }
}

int32_t File::getId()
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
    _manager->sendQuery(new td_api::downloadFile(getId(), 1, 0, 0, false));
}

void File::cancelDownload()
{
    _manager->sendQuery(new td_api::cancelDownloadFile(getId(), false));
}

void File::cancelUpload()
{
    _manager->sendQuery(new td_api::cancelPreliminaryUploadFile(getId()));
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

int32_t File::getExpectedSize()
{
    return _file->expected_size_;
}

int32_t File::getDownloadedSize()
{
    return _file->local_->downloaded_size_;
}

int32_t File::getUploadedSize()
{
    return _file->remote_->uploaded_size_;
}

QString File::getType()
{
    return _type;
}
