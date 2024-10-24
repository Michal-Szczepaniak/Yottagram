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

#ifndef ANIMATEDEMOJI_H
#define ANIMATEDEMOJI_H

#include <QObject>
#include <QSize>
#include "files/contentfile.h"
#include "contentinterface.h"

class AnimatedEmoji : public ContentFile, public ContentInterface
{
    Q_OBJECT
    Q_PROPERTY(File* sticker READ getSticker NOTIFY stickerChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY stickerChanged)
    Q_PROPERTY(int64_t setId READ getSetId NOTIFY stickerChanged)
    Q_PROPERTY(QString emoji READ getEmoji NOTIFY stickerChanged)
public:
    explicit AnimatedEmoji(QObject *parent = nullptr);

    void handleContent(td_api::object_ptr<td_api::MessageContent> content) override;
    File* getSticker() const;
    void addUpdateFiles();
    QSize getSize() const;
    int64_t getSetId() const;
    QString getEmoji() const;

signals:
    void stickerChanged();

public slots:

private:
    td_api::object_ptr<td_api::animatedEmoji> _animatedEmoji;
    int32_t _stickerFileId;
    QString _emoji = "";
};

#endif // ANIMATEDEMOJI_H
