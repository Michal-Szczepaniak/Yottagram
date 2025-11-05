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

#ifndef STICKERSETS_H
#define STICKERSETS_H

#include <QAbstractListModel>
#include "stickerset.h"

class StickerSets : public QAbstractListModel
{
    Q_OBJECT
public:
    enum StickerSetRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        NameRole,
        ThumbnailRole,
        IsInstalledRole,
        IsArchivedRole,
        IsOfficialRole,
        IsMasksRole,
        IsViewedRole,
        StickerSetRole,
        TypeRole,
    };

    explicit StickerSets(QObject *parent = nullptr);
    ~StickerSets();
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);
    void setEmojiMode(bool enable);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

private:
    void setupStickers();
    void setupEmojis();

signals:
    void gotStickerSet(int64_t stickerSetId);
    void gotCustomEmoji(int64_t emojiId, int32_t stickerId, int32_t thumbnailId, QString emoji);

public slots:
    void updateInstalledStickerSets(td_api::updateInstalledStickerSets *updateInstalledStickerSets);
    void updateRecentStickers(td_api::updateRecentStickers *updateRecentStickers);
    void updateFavouriteStickers(td_api::updateFavoriteStickers *updateFavouriteStickers);
    void onGotStickerSet(td_api::stickerSet *stickerSet);
    void onGotRecentStickers(td_api::stickers *stickers);
    void onGotFavouriteStickers(td_api::stickers *stickers);
    void onIsAuthorizedChanged(bool isAuthorized);

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Files> _files;
    QVector<int64_t> _installedStickerSetIds;
    QVector<int64_t> _stickerSetIds;
    QHash<int64_t, StickerSet*> _stickerSets;
    QVector<StickerSet*> _predefinedStickerSets;
    bool _emojiMode = false;
};

#endif // STICKERSETS_H
