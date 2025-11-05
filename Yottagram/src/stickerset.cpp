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

StickerSet::StickerSet(QObject *parent) : QAbstractListModel(parent), _stickerSet(nullptr), _stickers(nullptr), _thumbnailId(0), _type("")
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

    if (_stickerSet->thumbnail_ && _stickerSet->thumbnail_->format_->get_id() != td_api::thumbnailFormatMpeg4::ID) {
        _thumbnailId = _stickerSet->thumbnail_->file_->id_;
        _files->appendFile(move(_stickerSet->thumbnail_->file_), "sticker");
    }

    for (auto& sticker : _stickerSet->stickers_) {
        _stickerIds.append(sticker->sticker_->id_);
        _thumbnailIds.append(sticker->thumbnail_->file_->id_);
        _files->appendFile(move(sticker->sticker_), "sticker");
        _files->appendFile(move(sticker->thumbnail_->file_), "thumbnail");
    }

    _type = stickerSet->sticker_type_->get_id() == td_api::stickerTypeCustomEmoji::ID ? "customEmoji" : "sticker";
}

void StickerSet::setStickers(td_api::stickers *stickers, QString type)
{
    while (_stickerIds.count())
        _stickerIds.removeLast();

    if (_stickers != nullptr) {
        for (auto& sticker : _stickers->stickers_) {
            sticker.release();
        }

        delete _stickers;
    }

    _stickers = stickers;

    for (auto& sticker : _stickers->stickers_) {
        if (!sticker->sticker_) continue;

        _stickerIds.append(sticker->sticker_->id_);
        _thumbnailIds.append(sticker->thumbnail_->file_->id_);
        _files->appendFile(move(sticker->sticker_), "sticker");
        _files->appendFile(move(sticker->thumbnail_->file_), "thumbnail");
    }

    _type = type;
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
    if (_stickerSet == nullptr) {
        return 0;
    }

    return _stickerSet->id_;
}

shared_ptr<File> StickerSet::getThumbnail()
{
    if (_stickerSet == nullptr) {
        return {};
    }

    if (!_stickerSet->thumbnail_) {
        return _files->getFile(_stickerIds[0]);
    }

    return _files->getFile(_thumbnailId);
}

QString StickerSet::getType()
{
    return _type;
}

int StickerSet::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (_stickerSet == nullptr) {
        return _stickers->stickers_.size();
    }

    return _stickerSet->stickers_.size();
}

QVariant StickerSet::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() < 0 || index.row() >= rowCount()) return QVariant();

    td_api::sticker* sticker = nullptr;
    if (_stickerSet == nullptr) {
        sticker = _stickers->stickers_[index.row()].get();
    } else {
        sticker = _stickerSet->stickers_[index.row()].get();
    }

    switch (role) {
    case StickerRoles::IdRole:
        return QVariant::fromValue(sticker->id_);
    case StickerRoles::EmojiRole:
        return QString::fromStdString(sticker->emoji_);
    case StickerRoles::ReactionIdRole:
        if (_stickerSet == nullptr) {
            return QString::fromStdString(sticker->emoji_);
        } else {
            return QString::number(static_cast<td_api::stickerFullTypeCustomEmoji*>(sticker->full_type_.get())->custom_emoji_id_);
        }
    case StickerRoles::StickerRole:
    {
        if (_stickerIds.empty()) return QVariant();

        auto sticker = _files->getFile(_stickerIds[index.row()]).get();
        if (sticker != nullptr) {
            return QVariant::fromValue(sticker);
        } else {
            return QVariant();
        }
    }
    case StickerRoles::ThumbnailRole:
    {
        if (_thumbnailIds.empty()) return QVariant();

        auto thumbnail = _files->getFile(_thumbnailIds[index.row()]).get();
        if (thumbnail != nullptr) {
            return QVariant::fromValue(thumbnail);
        } else {
            return QVariant();
        }
    }
    case StickerRoles::TypeRole:
        return _type;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StickerSet::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[EmojiRole] = "emoji";
    roles[ReactionIdRole] = "reactionId";
    roles[IsMaskRole] = "isMask";
    roles[IsAnimatedRole] = "isAnimated";
    roles[StickerRole] = "sticker";
    roles[ThumbnailRole] = "thumbnail";
    roles[TypeRole] = "type";
    return roles;
}
