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

#include "sticker.h"
#include <QFile>
#include <QDataStream>
#include <zlib.h>

Sticker::Sticker(QObject *parent) : ContentFile(parent)
{

}

void Sticker::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messageSticker::ID) {
        _sticker = td_api::move_object_as<td_api::messageSticker>(content);

        addUpdateFiles();

        emit stickerChanged();
    }
}

File *Sticker::getSticker() const
{
    return _files->getFile(_stickerFileId).get();
}

void Sticker::addUpdateFiles()
{
    _stickerFileId = _sticker->sticker_->sticker_->id_;
    _files->appendFile(std::move(_sticker->sticker_->sticker_), "sticker");
}

QSize Sticker::getSize() const
{
    return QSize(_sticker->sticker_->width_, _sticker->sticker_->height_);
}

int64_t Sticker::getSetId() const
{
    return _sticker->sticker_->set_id_;
}

QString Sticker::getEmoji() const
{
    return QString::fromStdString(_sticker->sticker_->emoji_);
}

bool Sticker::isAnimated() const
{
    return _sticker->sticker_->format_->get_id() != td_api::stickerFormatWebp::ID;
}

bool Sticker::isMask() const
{
    return _sticker->sticker_->type_->get_id() == td_api::stickerTypeMask::ID;
}
