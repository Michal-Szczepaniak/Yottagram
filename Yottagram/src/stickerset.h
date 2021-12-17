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

#ifndef STICKERS_H
#define STICKERS_H

#include <QAbstractListModel>
#include "core/telegrammanager.h"
#include "files/files.h"

class StickerSet : public QAbstractListModel
{
    Q_OBJECT
public:
    enum StickerRoles {
        EmojiRole = Qt::UserRole + 1,
        IsAnimatedRole,
        IsMaskRole,
        StickerRole
    };

    explicit StickerSet(QObject *parent = nullptr);
    void setStickerSet(td_api::stickerSet *stickerSet);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

    int64_t getId() const;
    bool getIsAnimated() const;
    shared_ptr<File> getThumbnail();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = EmojiRole) const;
    QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Files> _files;
    td_api::stickerSet *_stickerSet;
    int32_t _thumbnailId;
    QVector<int32_t> _stickerIds;
};
Q_DECLARE_METATYPE(StickerSet*)

#endif // STICKERS_H
