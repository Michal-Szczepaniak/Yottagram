#include "savedanimations.h"

SavedAnimations::SavedAnimations(QObject *parent) : QAbstractListModel(parent)
{

}

void SavedAnimations::setTelegramManager(shared_ptr<TelegramManager> manager)
{
    _manager = manager;

    connect(_manager.get(), &TelegramManager::updateSavedAnimations, this, &SavedAnimations::onUpdateSavedAnimations);
    connect(_manager.get(), &TelegramManager::gotSavedAnimations, this, &SavedAnimations::onGotSavedAnimations);
}

void SavedAnimations::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

int SavedAnimations::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _savedAnimationsId.size();
}

QVariant SavedAnimations::data(const QModelIndex &index, int role) const
{
    if (rowCount() <= 0 || !_savedAnimations.contains(_savedAnimationsId[index.row()])) return QVariant();

    auto animation = _savedAnimations[_savedAnimationsId[index.row()]];
    switch (role) {
    case AnimationRoles::IdRole:
        return QVariant::fromValue(animation.id);
    case AnimationRoles::FileRole:
        return QVariant::fromValue(_files->getFile(animation.id).get());
    case AnimationRoles::ThumbnailRole:
        if (animation.thumbnailId == 0) return QVariant();
        return QVariant::fromValue(_files->getFile(animation.thumbnailId).get());
    case AnimationRoles::VideoThumbnailRole:
        return animation.videoThumbnail;
    case AnimationRoles::WidthRole:
        return animation.width;
    case AnimationRoles::HeightRole:
        return animation.height;
    }
    return QVariant();
}

QHash<int, QByteArray> SavedAnimations::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[FileRole] = "file";
    roles[ThumbnailRole] = "thumbnail";
    roles[VideoThumbnailRole] = "videoThumbnail";
    roles[WidthRole] = "animationWidth";
    roles[HeightRole] = "animationHeight";
    return roles;
}

void SavedAnimations::onUpdateSavedAnimations(td_api::updateSavedAnimations *updateSavedAnimations)
{
    beginResetModel();
    _savedAnimationsId.clear();
    for (int32_t animationId : updateSavedAnimations->animation_ids_)
        _savedAnimationsId.append(animationId);
    endResetModel();

    _manager->sendQueryWithResponse(0, td_api::getSavedAnimations::ID, 0, new td_api::getSavedAnimations());
}

void SavedAnimations::onGotSavedAnimations(td_api::animations *animations)
{
    for (td_api::object_ptr<td_api::animation> &animation : animations->animations_) {
        if (_savedAnimations.contains(animation->animation_->id_)) _savedAnimations.remove(animation->animation_->id_);

        SavedAnimation newAnimation = { animation->animation_->id_, animation->width_, animation->height_, 0, false };
        _files->appendFile(move(animation->animation_), "animation");
        if (animation->thumbnail_) {
            newAnimation.thumbnailId = animation->thumbnail_->file_->id_;
            newAnimation.videoThumbnail = animation->thumbnail_->format_->get_id() == td_api::thumbnailFormatMpeg4::ID;
            _files->appendFile(move(animation->thumbnail_->file_), "animation");
        }
        _savedAnimations[newAnimation.id] = newAnimation;
        emit dataChanged(createIndex(_savedAnimationsId.indexOf(newAnimation.id), 0), createIndex(_savedAnimationsId.indexOf(newAnimation.id), 0));
    }
}

void SavedAnimations::onIsAuthorizedChanged(bool isAuthorized)
{
    if (isAuthorized) _manager->sendQueryWithResponse(0, td_api::getSavedAnimations::ID, 0, new td_api::getSavedAnimations());
}
