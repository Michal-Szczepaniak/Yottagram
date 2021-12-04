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

#ifndef STICKER_H
#define STICKER_H

#include <QObject>
#include <QSize>
#include "files/contentfile.h"
#include "contentinterface.h"

#define GZIP_WINDOWS_BIT 15 + 16
#define GZIP_CHUNK_SIZE 32 * 1024

class Sticker : public ContentFile, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(File* sticker READ getSticker NOTIFY stickerChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY stickerChanged)
    Q_PROPERTY(int64_t setId READ getSetId NOTIFY stickerChanged)
    Q_PROPERTY(QString emoji READ getEmoji NOTIFY stickerChanged)
    Q_PROPERTY(bool isAnimated READ isAnimated NOTIFY stickerChanged)
    Q_PROPERTY(bool isMask READ isMask NOTIFY stickerChanged)
public:
    explicit Sticker(QObject *parent = nullptr);

    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;
    File* getSticker() const;
    void addUpdateFiles();
    QSize getSize() const;
    int64_t getSetId() const;
    QString getEmoji() const;
    bool isAnimated() const;
    bool isMask() const;

signals:
    void stickerChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageSticker> _sticker;
    int32_t _stickerFileId;
};

#endif // STICKER_H
