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
#include <QJsonDocument>
#include <QQmlEngine>
#include "emoji.h"

StickerSets::StickerSets(QObject *parent) : QAbstractListModel(parent)
{
}

StickerSets::~StickerSets()
{
    for (StickerSet* set : _stickerSets) {
        delete set;
    }

    for (StickerSet* set : _predefinedStickerSets) {
        delete set;
    }
}

void StickerSets::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateInstalledStickerSets, this, &StickerSets::updateInstalledStickerSets);
    connect(_manager.get(), &TelegramManager::updateRecentStickers, this, &StickerSets::updateRecentStickers);
    connect(_manager.get(), &TelegramManager::updateFavouriteStickers, this, &StickerSets::updateFavouriteStickers);
    connect(_manager.get(), &TelegramManager::gotStickerSet, this, &StickerSets::onGotStickerSet);
    connect(_manager.get(), &TelegramManager::gotRecentStickers, this, &StickerSets::onGotRecentStickers);
    connect(_manager.get(), &TelegramManager::gotFavouriteStickers, this, &StickerSets::onGotFavouriteStickers);
}

void StickerSets::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

void StickerSets::setEmojiMode(bool enable)
{
    _emojiMode = enable;
}

int StickerSets::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _installedStickerSetIds.size() + _predefinedStickerSets.size();
}

QVariant StickerSets::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || index.row() < 0 || index.row() >= rowCount()) return QVariant();

    StickerSet* stickerSet = nullptr;

    int row = index.row() - _predefinedStickerSets.size();
    bool useStickerSets = true;
    if (index.row() < _predefinedStickerSets.size()) {
        stickerSet = _predefinedStickerSets[index.row()];
        useStickerSets = false;
    }

    if (useStickerSets) {
        if (!_stickerSets.contains(_installedStickerSetIds[row])) {
            _manager->sendQueryWithResponse(0, td_api::getStickerSet::ID, 0, new td_api::getStickerSet(_installedStickerSetIds[row]));
            return QVariant();
        }

        stickerSet = _stickerSets[_installedStickerSetIds[row]];
    }

    switch (role) {
    case StickerSetRoles::IdRole:
        return QVariant::fromValue(stickerSet->getId());
    case StickerSetRoles::StickerSetRole:
        QQmlEngine::setObjectOwnership(stickerSet, QQmlEngine::CppOwnership);
        return QVariant::fromValue(stickerSet);
    case StickerSetRoles::ThumbnailRole:
        return QVariant::fromValue(stickerSet->getThumbnail().get());
    case StickerSetRoles::TypeRole:
        return stickerSet->getType();
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
    roles[IsMasksRole] = "isMasks";
    roles[IsViewedRole] = "isViewed";
    roles[StickerSetRole] = "stickerSet";
    roles[TypeRole] = "type";
    return roles;
}

void StickerSets::setupStickers()
{
    if (_predefinedStickerSets.empty()) {
        auto favouriteStickrs = new StickerSet();
        favouriteStickrs ->setTelegramManager(_manager);
        favouriteStickrs ->setFiles(_files);
        _predefinedStickerSets.push_back(favouriteStickrs);

        auto recentStickrs = new StickerSet();
        recentStickrs->setTelegramManager(_manager);
        recentStickrs->setFiles(_files);
        _predefinedStickerSets.push_back(recentStickrs);
    }

    _manager->sendQuery(new td_api::getInstalledStickerSets(td_api::make_object<td_api::stickerTypeRegular>()));
    _manager->sendQuery(new td_api::getRecentStickers());
    _manager->sendQuery(new td_api::getFavoriteStickers());
}

void StickerSets::setupEmojis()
{
    if (_predefinedStickerSets.empty()) {
        QJsonDocument doc = QJsonDocument::fromJson(emoji::json);
        QJsonObject root = doc.object();

        auto addCategory = [this](QJsonObject root, QString name){
            QJsonArray category = root[name].toArray();

            td_api::stickers* stickers = new td_api::stickers;
            for (QJsonValue sticker : category) {
                stickers->stickers_.emplace_back(new td_api::sticker(0, 0, 0, 0, sticker.toObject()["char"].toString().toStdString(), nullptr, nullptr, nullptr, nullptr));
            }

            auto stickerSet = new StickerSet();
            stickerSet ->setTelegramManager(_manager);
            stickerSet ->setFiles(_files);
            stickerSet->setStickers(stickers, name);
            _predefinedStickerSets.push_back(stickerSet);
        };

        addCategory(root, "smileys");
        addCategory(root, "nature");
        addCategory(root, "food");
        addCategory(root, "activities");
        addCategory(root, "travel");
        addCategory(root, "objects");
        addCategory(root, "symbols");
    }

    _manager->sendQuery(new td_api::getInstalledStickerSets(td_api::make_object<td_api::stickerTypeCustomEmoji>()));
}

void StickerSets::updateInstalledStickerSets(td_api::updateInstalledStickerSets *updateInstalledStickerSets)
{
    if ((_emojiMode && (updateInstalledStickerSets->sticker_type_->get_id() == td_api::stickerTypeRegular::ID)) ||
        (!_emojiMode && (updateInstalledStickerSets->sticker_type_->get_id() != td_api::stickerTypeRegular::ID))) {
        return;
    }

    beginResetModel();
    _installedStickerSetIds.clear();

    for (auto stickerSetId : updateInstalledStickerSets->sticker_set_ids_) {
        _installedStickerSetIds.append(stickerSetId);
        _stickerSetIds.append(stickerSetId);
    }
    endResetModel();
}

void StickerSets::updateRecentStickers(td_api::updateRecentStickers *updateRecentStickers)
{
    Q_UNUSED(updateRecentStickers)

    if (_emojiMode) return;

    if (_predefinedStickerSets.empty()) return;

    _manager->sendQuery(new td_api::getRecentStickers());
}

void StickerSets::updateFavouriteStickers(td_api::updateFavoriteStickers *updateFavouriteStickers)
{
    Q_UNUSED(updateFavouriteStickers)

    if (_emojiMode) return;

    if (_predefinedStickerSets.empty()) return;

    _manager->sendQuery(new td_api::getFavoriteStickers());
}

void StickerSets::onGotStickerSet(td_api::stickerSet *stickerSet)
{
    if ((_emojiMode && (stickerSet->sticker_type_->get_id() == td_api::stickerTypeRegular::ID)) ||
        (!_emojiMode && (stickerSet->sticker_type_->get_id() != td_api::stickerTypeRegular::ID))) {
        return;
    }

    if (!_stickerSetIds.contains(stickerSet->id_)) _stickerSetIds.append(stickerSet->id_);

    if (!_stickerSets.contains(stickerSet->id_)) {
        auto newStickerSet = new StickerSet();
        newStickerSet->setTelegramManager(_manager);
        newStickerSet->setFiles(_files);
        _stickerSets[stickerSet->id_] = newStickerSet;
    }

    if (stickerSet->sticker_type_->get_id() == td_api::stickerTypeCustomEmoji::ID) {
        for (auto& sticker : stickerSet->stickers_) {
            emit gotCustomEmoji(sticker->id_, sticker->sticker_->id_, sticker->thumbnail_->file_->id_, QString::fromStdString(sticker->emoji_));
        }
    }

    _stickerSets[stickerSet->id_]->setStickerSet(stickerSet);

    emit dataChanged(createIndex(_stickerSetIds.indexOf(stickerSet->id_), 0), createIndex(_stickerSetIds.indexOf(stickerSet->id_), 0));
    emit gotStickerSet(stickerSet->id_);
}

void StickerSets::onGotRecentStickers(td_api::stickers *stickers)
{
    if (_emojiMode) return;

    StickerSet* stickerSet = _predefinedStickerSets[1];
    stickerSet->setStickers(stickers, "recent");
    emit dataChanged(createIndex(1, 0), createIndex(1, 0));
}

void StickerSets::onGotFavouriteStickers(td_api::stickers *stickers)
{
    if (_emojiMode) return;

    StickerSet* stickerSet = _predefinedStickerSets[0];
    stickerSet->setStickers(stickers, "favourite");
    emit dataChanged(createIndex(0, 0), createIndex(0, 0));
}

void StickerSets::onIsAuthorizedChanged(bool isAuthorized)
{
    if (!isAuthorized) {
        return;
    }

    if (_emojiMode) {
        setupEmojis();
    } else {
        setupStickers();
    }

}
