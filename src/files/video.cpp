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

#include "video.h"

Video::Video(QObject *parent) : ContentFile(parent)
{

}

void Video::setVideo(td_api::object_ptr<td_api::messageVideo> messageVideo)
{
    _video = std::move(messageVideo);

    addUpdateFiles();

    emit videoChanged();
}

File *Video::getVideo() const
{
    return _files->getFile(_videoFileId).get();
}

void Video::addUpdateFiles()
{
    _videoFileId = _video->video_->video_->id_;
    _files->appendFile(std::move(_video->video_->video_), "video");
}

bool Video::isSecret() const
{
    return _video->is_secret_;
}

td_api::formattedText* Video::getCaption()
{
    return _video->caption_.get();
}

QSize Video::getSize() const
{
    return QSize(_video->video_->width_, _video->video_->height_);
}

QString Video::getName() const
{
    return QString::fromStdString(_video->video_->file_name_);
}

QString Video::getMimeType() const
{
    return QString::fromStdString(_video->video_->mime_type_);
}

QByteArray Video::getThumbnail() const
{
    if (_video->video_->minithumbnail_ != nullptr) {
        return QByteArray::fromStdString(_video->video_->minithumbnail_->data_);
    } else {
        return "";
    }
}

bool Video::hasStickers() const
{
    return _video->video_->has_stickers_;
}

bool Video::getSupportsStreaming() const
{
    return _video->video_->supports_streaming_;
}
