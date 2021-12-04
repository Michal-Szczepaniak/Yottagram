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

#include "audio.h"

Audio::Audio(QObject *parent) : ContentFile(parent)
{

}

void Audio::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageAudio::ID) {
        _audio = td_api::move_object_as<td_api::messageAudio>(content);

        addUpdateFiles();

        emit audioChanged();
    }
}

File *Audio::getAudio() const
{
    return _files->getFile(_audioFileId).get();
}

void Audio::addUpdateFiles()
{
    _audioFileId = _audio->audio_->audio_->id_;
    _files->appendFile(std::move(_audio->audio_->audio_), "other");
}

td_api::formattedText* Audio::getCaption()
{
    return _audio->caption_.get();
}

int32_t Audio::getDuration() const
{
    return _audio->audio_->duration_;
}

QString Audio::getTitle() const
{
    return QString::fromStdString(_audio->audio_->title_);
}

QString Audio::getPerformer() const
{
    return QString::fromStdString(_audio->audio_->performer_);
}

QString Audio::getName() const
{
    return QString::fromStdString(_audio->audio_->file_name_);
}

QString Audio::getMimeType() const
{
    return QString::fromStdString(_audio->audio_->mime_type_);
}

QByteArray Audio::getThumbnail() const
{
    if (_audio->audio_->album_cover_minithumbnail_ != nullptr) {
        return QByteArray::fromStdString(_audio->audio_->album_cover_minithumbnail_->data_);
    } else {
        return "";
    }
}
