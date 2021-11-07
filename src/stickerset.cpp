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

#include "stickerset.h"

StickerSet::StickerSet(QObject *parent) : QAbstractListModel(parent), _stickerSet(nullptr), _thumbnailId(0)
{

}

void StickerSet::setStickerSet(td_api::stickerSet *stickerSet)
{
    if (_stickerSet != nullptr) {
        delete _stickerSet;
    }
    _stickerSet = stickerSet;
    while (_stickerIds.count())
        _stickerIds.removeLast();

    if (_stickerSet->thumbnail_ != nullptr && _stickerSet->thumbnail_->format_->get_id() != td_api::thumbnailFormatMpeg4::ID) {
        _thumbnailId = _stickerSet->thumbnail_->file_->id_;
        _files->appendFile(move(_stickerSet->thumbnail_->file_), "sticker");
    }

    for (auto& sticker : _stickerSet->stickers_) {
        _stickerIds.append(sticker->sticker_->id_);
        _files->appendFile(move(sticker->sticker_), "sticker");
    }
}

void StickerSet::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;
}

void StickerSet::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int64_t StickerSet::getId() const
{
    return _stickerSet->id_;
}

bool StickerSet::getIsAnimated() const
{
    return _stickerSet->is_animated_;
}

shared_ptr<File> StickerSet::getThumbnail()
{
    if (_stickerSet->thumbnail_ == nullptr || getIsAnimated()) {
        return _files->getFile(_stickerIds[0]);
    }

    return _files->getFile(_thumbnailId);
}

int StickerSet::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _stickerSet->stickers_.size();
}

QVariant StickerSet::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0) return QVariant();

    auto sticker = _stickerSet->stickers_[index.row()].get();
    switch (role) {
    case StickerRoles::EmojiRole:
        return QString::fromStdString(sticker->emoji_);
    case StickerRoles::StickerRole:
    {
        auto sticker = _files->getFile(_stickerIds[index.row()]).get();
        if (sticker != nullptr) {
            return QVariant::fromValue(sticker);
        } else {
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StickerSet::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[EmojiRole] = "emoji";
    roles[IsMaskRole] = "isMask";
    roles[IsAnimatedRole] = "isAnimated";
    roles[StickerRole] = "sticker";
    return roles;
}
