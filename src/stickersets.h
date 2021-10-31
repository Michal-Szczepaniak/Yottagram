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
        IsAnimatedRole,
        IsMasksRole,
        IsViewedRole,
        StickerSetRole
    };

    explicit StickerSets(QObject *parent = nullptr);
    void setTelegramManager(shared_ptr<TelegramManager> manager);
    void setFiles(shared_ptr<Files> files);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = IdRole) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void getStickerSets();
signals:

public slots:
    void updateInstalledStickerSets(td_api::updateInstalledStickerSets *updateInstalledStickerSets);
    void gotInstalledStickerSets(td_api::stickerSets *stickerSets);
    void gotStickerSet(td_api::stickerSet *stickerSet);
    void onIsAuthorizedChanged(bool isAuthorized);

private:
    std::shared_ptr<TelegramManager> _manager;
    std::shared_ptr<Files> _files;
    QVector<int64_t> _installedStickerSetIds;
    QVector<int64_t> _stickerSetIds;
    QHash<int64_t, StickerSet*> _stickerSets;
};

#endif // STICKERSETS_H
