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

#include "stickersets.h"
#include <QQmlEngine>

StickerSets::StickerSets(QObject *parent) : QAbstractListModel(parent)
{

}

void StickerSets::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateInstalledStickerSets, this, &StickerSets::updateInstalledStickerSets);
    connect(_manager.get(), &TelegramManager::gotStickerSet, this, &StickerSets::onGotStickerSet);
}

void StickerSets::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int StickerSets::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _installedStickerSetIds.size();
}

QVariant StickerSets::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0) return QVariant();

    if (!_stickerSets.contains(_installedStickerSetIds[index.row()])) {
        _manager->sendQueryWithRespone(0, td_api::getStickerSet::ID, 0, new td_api::getStickerSet(_installedStickerSetIds[index.row()]));
        return QVariant();
    }

    auto stickerSet = _stickerSets[_installedStickerSetIds[index.row()]];
    switch (role) {
    case StickerSetRoles::IdRole:
        return QVariant::fromValue(stickerSet->getId());
    case StickerSetRoles::IsAnimatedRole:
        return stickerSet->getIsAnimated();
    case StickerSetRoles::StickerSetRole:
        QQmlEngine::setObjectOwnership(stickerSet, QQmlEngine::CppOwnership);
        return QVariant::fromValue(stickerSet);
    case StickerSetRoles::ThumbnailRole:
        return QVariant::fromValue(stickerSet->getThumbnail().get());
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> StickerSets::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[NameRole] = "name";
    roles[ThumbnailRole] = "thumbnail";
    roles[IsInstalledRole] = "isInstalled";
    roles[IsArchivedRole] = "isArchived";
    roles[IsOfficialRole] = "isOfficial";
    roles[IsAnimatedRole] = "isAnimated";
    roles[IsMasksRole] = "isMasks";
    roles[IsViewedRole] = "isViewed";
    roles[StickerSetRole] = "stickerSet";
    return roles;
}

void StickerSets::getStickerSets()
{
    _manager->sendQuery(new td_api::getInstalledStickerSets(false));
}

void StickerSets::updateInstalledStickerSets(td_api::updateInstalledStickerSets *updateInstalledStickerSets)
{
    qDebug() << __PRETTY_FUNCTION__;
    beginResetModel();
    _installedStickerSetIds.clear();

    for (auto stickerSetId : updateInstalledStickerSets->sticker_set_ids_) {
        _installedStickerSetIds.append(stickerSetId);
        _stickerSetIds.append(stickerSetId);
    }
    endResetModel();
}

void StickerSets::onGotStickerSet(td_api::stickerSet *stickerSet)
{
    if (!_stickerSetIds.contains(stickerSet->id_)) _stickerSetIds.append(stickerSet->id_);

    if (!_stickerSets.contains(stickerSet->id_)) {
        auto newStickerSet = new StickerSet();
        newStickerSet->setTelegramManager(_manager);
        newStickerSet->setFiles(_files);
        _stickerSets[stickerSet->id_] = newStickerSet;
    }

    _stickerSets[stickerSet->id_]->setStickerSet(stickerSet);

    emit dataChanged(createIndex(_stickerSetIds.indexOf(stickerSet->id_), 0), createIndex(_stickerSetIds.indexOf(stickerSet->id_), 0));
    emit gotStickerSet(stickerSet->id_);
}

void StickerSets::onIsAuthorizedChanged(bool isAuthorized)
{
    if (isAuthorized) {
        _manager->sendQuery(new td_api::getInstalledStickerSets(false));
    }
}
