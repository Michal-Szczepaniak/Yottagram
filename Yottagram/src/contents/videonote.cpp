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

#include "videonote.h"

VideoNote::VideoNote(QObject *parent) : ContentFile(parent)
{

}

void VideoNote::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageVideoNote::ID) {
        _videoNote = td_api::move_object_as<td_api::messageVideoNote>(content);

        addUpdateFiles();

        emit videoNoteChanged();
    }
}

File *VideoNote::getVideoNote() const
{
    return _files->getFile(_videoNoteFileId).get();
}

QByteArray VideoNote::getThumbnail() const
{
    if (_videoNote->video_note_->minithumbnail_ != nullptr) {
        return QByteArray::fromStdString(_videoNote->video_note_->minithumbnail_->data_);
    } else {
        return "";
    }
}

void VideoNote::addUpdateFiles()
{
    _videoNoteFileId = _videoNote->video_note_->video_->id_;
    _files->appendFile(std::move(_videoNote->video_note_->video_), "other");
}

bool VideoNote::isSecret() const
{
    return _videoNote->is_secret_;
}

bool VideoNote::isViewed() const
{
    return _videoNote->is_viewed_;
}

QSize VideoNote::getSize() const
{
    return QSize(_videoNote->video_note_->length_, _videoNote->video_note_->length_);
}
