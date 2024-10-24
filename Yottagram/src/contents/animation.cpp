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

#include "animation.h"

Animation::Animation(QObject *parent) : ContentFile(parent)
{

}

void Animation::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageAnimation::ID) {
        _animation = td_api::move_object_as<td_api::messageAnimation>(content);

        addUpdateFiles();

        emit animationChanged();
    }
}

File *Animation::getAnimation() const
{
    return _files->getFile(_animationFileId).get();
}

void Animation::addUpdateFiles()
{
    _animationFileId = _animation->animation_->animation_->id_;
    _files->appendFile(std::move(_animation->animation_->animation_), "other");
    if (_animation->animation_->thumbnail_) {
        _thumbnailFileId = _animation->animation_->thumbnail_->file_->id_;
        _files->appendFile(std::move(_animation->animation_->thumbnail_->file_), "thumbnail");
    }
}

bool Animation::isSecret() const
{
    return _animation->is_secret_;
}

td_api::formattedText* Animation::getCaption()
{
    return _animation->caption_.get();
}

QSize Animation::getSize() const
{
    return QSize(_animation->animation_->width_, _animation->animation_->height_);
}

QString Animation::getName() const
{
    return QString::fromStdString(_animation->animation_->file_name_);
}

QString Animation::getMimeType() const
{
    return QString::fromStdString(_animation->animation_->mime_type_);
}

File* Animation::getThumbnail() const
{
    if (_animation->animation_->thumbnail_) {
        return _files->getFile(_thumbnailFileId).get();
    } else {
        return nullptr;
    }
}
