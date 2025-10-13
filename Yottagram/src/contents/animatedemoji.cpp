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

#include "animatedemoji.h"
#include <QFile>
#include <QDataStream>
#include <zlib.h>

AnimatedEmoji::AnimatedEmoji(QObject *parent) : ContentFile(parent), _thumbnailFileId(0)
{

}

void AnimatedEmoji::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageAnimatedEmoji::ID) {
        _animatedEmoji = move(static_cast<td_api::messageAnimatedEmoji*>(content.get())->animated_emoji_);
        _emoji = QString::fromStdString(static_cast<td_api::messageAnimatedEmoji*>(content.get())->emoji_);

        addUpdateFiles();

        emit stickerChanged();
    }
}

File *AnimatedEmoji::getSticker() const
{
    return _files->getFile(_stickerFileId).get();
}

void AnimatedEmoji::addUpdateFiles()
{
    if (!_animatedEmoji || !_animatedEmoji->sticker_ || !_animatedEmoji->sticker_->sticker_) return;
    _stickerFileId = _animatedEmoji->sticker_->sticker_->id_;
    _files->appendFile(std::move(_animatedEmoji->sticker_->sticker_), "sticker");
    if (_animatedEmoji->sticker_->thumbnail_) {
        _thumbnailFileId = _animatedEmoji->sticker_->thumbnail_->file_->id_;
        _files->appendFile(std::move(_animatedEmoji->sticker_->thumbnail_->file_), "thumbnail");
    }
}

QSize AnimatedEmoji::getSize() const
{
    if (!_animatedEmoji) return QSize();
    return QSize(_animatedEmoji->sticker_->width_, _animatedEmoji->sticker_->height_);
}

int64_t AnimatedEmoji::getSetId() const
{
    if (!_animatedEmoji) return 0;
    return _animatedEmoji->sticker_->set_id_;
}

QString AnimatedEmoji::getEmoji() const
{
    if (!_animatedEmoji) return "";
    return _emoji;
}

QString AnimatedEmoji::getType() const
{
    if (!_animatedEmoji) return "";

    switch (_animatedEmoji->sticker_->format_->get_id()) {
    case td_api::stickerFormatWebp::ID:
        return "webp";
    case td_api::stickerFormatTgs::ID:
        return "tgs";
    case td_api::stickerFormatWebm::ID:
        return "webm";
    }
}

File *AnimatedEmoji::getThumbnail() const
{
    if (_thumbnailFileId != 0) {
        return _files->getFile(_thumbnailFileId).get();
    } else {
        return nullptr;
    }
}
