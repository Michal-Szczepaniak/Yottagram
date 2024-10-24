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

#ifndef PHOTO_H
#define PHOTO_H

#include <QObject>
#include <QHash>
#include <QSize>
#include <td/telegram/Client.h>
#include "core/telegrammanager.h"
#include "files/file.h"
#include "files/contentfile.h"
#include "contentinterface.h"

class Photo : public ContentFile, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(bool isSecret READ isSecret NOTIFY photoChanged)
    Q_PROPERTY(bool hasStickers READ getHasStickers NOTIFY photoChanged)
    Q_PROPERTY(File* thumbnail READ getThumbnail NOTIFY photoChanged)
    Q_PROPERTY(QList<QChar> photoSizes READ getPhotoSizes NOTIFY photoChanged)
    Q_PROPERTY(File* biggestPhoto READ getBiggestPhoto NOTIFY photoChanged)
    Q_PROPERTY(QSize biggestPhotoSize READ getBiggestPhotoSize NOTIFY photoChanged)
public:
    explicit Photo(QObject *parent = nullptr);

    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;
    void addUpdateFiles();

    bool isSecret() const;
    bool getHasStickers() const;
    File* getThumbnail() const;
    td_api::formattedText* getCaption();
    QList<QChar> getPhotoSizes() const;
    File* getBiggestPhoto() const;
    QChar getBiggestType() const;
    QSize getBiggestPhotoSize() const;

signals:
    void photoChanged();

private:
    td_api::object_ptr<td_api::messagePhoto> _photo;
    QHash<QChar, int32_t> _photoSizeFileIds;
    QHash<QChar, QSize> _photoSizes;
    QList<QChar> _photoSizesTypes;
};
Q_DECLARE_METATYPE(Photo*)

#endif // PHOTO_H
