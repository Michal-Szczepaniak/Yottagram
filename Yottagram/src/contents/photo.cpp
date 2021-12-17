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

#include "photo.h"
#include <QDebug>
#include <fstream>

Photo::Photo(QObject *parent) : ContentFile(parent)
{

}

void Photo::handleContent(td_api::object_ptr<td_api::MessageContent> content)
{
    if (content->get_id() == td_api::messagePhoto::ID) {
        _photo = td_api::move_object_as<td_api::messagePhoto>(content);

        addUpdateFiles();

        emit photoChanged();
    }
}

void Photo::addUpdateFiles()
{
    for (auto& photoSize : _photo->photo_->sizes_) {
        _photoSizesTypes.append(QChar::fromLatin1(photoSize->type_[0]));
        _photoSizes.insert(QChar::fromLatin1(photoSize->type_[0]), QSize(photoSize->width_, photoSize->height_));
        _photoSizeFileIds.insert(QChar::fromLatin1(photoSize->type_[0]), photoSize->photo_->id_);
        _files->appendFile(std::move(photoSize->photo_), "photo");
    }
}

bool Photo::isSecret() const
{
    return _photo->is_secret_;
}

bool Photo::getHasStickers() const
{
    return _photo->photo_->has_stickers_;
}

QByteArray Photo::getThumbnail() const
{
    if (_photo->photo_->minithumbnail_ != nullptr) {
        return QByteArray::fromStdString(_photo->photo_->minithumbnail_->data_);
    } else {
        return "";
    }
}

td_api::formattedText* Photo::getCaption()
{
    return _photo->caption_.get();
}

QList<QChar> Photo::getPhotoSizes() const
{
    return _photoSizesTypes;
}

File* Photo::getBiggestPhoto() const
{
    if (_photoSizesTypes.empty()) return nullptr;
    return _files->getFile(_photoSizeFileIds[getBiggestType()]).get();
}

QChar Photo::getBiggestType() const
{
    if (_photoSizesTypes.last() == 'i') {
        if (_photoSizesTypes.length() == 1) {
            return _photoSizesTypes.last();
        } else {
            return _photoSizesTypes[_photoSizesTypes.length()-2];
        }
    }

    return _photoSizesTypes.last();
}

QSize Photo::getBiggestPhotoSize() const
{
    if (_photoSizesTypes.empty()) return QSize(0, 0);
    return _photoSizes[getBiggestType()];
}
