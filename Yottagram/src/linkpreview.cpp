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

#include "linkpreview.h"

LinkPreview::LinkPreview(QObject *parent) : QObject(parent), _mediaRatio(0), _photoId(-1), _videoId(-1),
    _audioId(-1), _documentId(-1), _animationId(-1), _thumbnailId(-1), _mediaAuthor(""),
    _mediaTitle(""), _mediaDuration(0)
{

}

void LinkPreview::setLinkPreview(td_api::object_ptr<td_api::linkPreview> linkPreview)
{
    _linkPreview = std::move(linkPreview);

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypePhoto::ID && !static_cast<td_api::linkPreviewTypePhoto*>(_linkPreview->type_.get())->photo_->sizes_.empty()) {
        auto photo = static_cast<td_api::linkPreviewTypePhoto*>(_linkPreview->type_.get())->photo_.get();
        if (photo->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(photo->minithumbnail_->data_);

        auto sizes = &photo->sizes_;
        auto biggestSize = photo->sizes_.back().get();
        _photoId = biggestSize->photo_->id_;
        _mediaRatio = (float)biggestSize->height_/biggestSize->width_;
        _files->appendFile(move(biggestSize->photo_), "photo");
        _files->considerAutoDownloading(_photoId, "photo");

        if (sizes->size() > 1) {
            _thumbnailId = photo->sizes_.front()->photo_->id_;
            _files->appendFile(move(photo->sizes_.front()->photo_), "thumbnail");
        } else {
            _thumbnailId = _photoId;
        }
    }

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeArticle::ID &&
        static_cast<td_api::linkPreviewTypeArticle*>(_linkPreview->type_.get())->photo_ &&
        !static_cast<td_api::linkPreviewTypeArticle*>(_linkPreview->type_.get())->photo_->sizes_.empty()) {
        auto photo = static_cast<td_api::linkPreviewTypePhoto*>(_linkPreview->type_.get())->photo_.get();
        if (photo->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(photo->minithumbnail_->data_);

        auto sizes = &photo->sizes_;
        auto biggestSize = photo->sizes_.back().get();
        _photoId = biggestSize->photo_->id_;
        _mediaRatio = (float)biggestSize->height_/biggestSize->width_;
        _files->appendFile(move(biggestSize->photo_), "photo");
        _files->considerAutoDownloading(_photoId, "photo");

        getPhoto()->download();

        if (sizes->size() > 1) {
            _thumbnailId = photo->sizes_.front()->photo_->id_;
            _files->appendFile(move(photo->sizes_.front()->photo_), "thumbnail");
        } else {
            _thumbnailId = _photoId;
        }
    }

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeVideo::ID) {
        auto video = static_cast<td_api::linkPreviewTypeVideo*>(_linkPreview->type_.get())->video_.get();
        _mediaRatio = (float)video->height_/video->width_;
        if (video->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(video->minithumbnail_->data_);
        _videoId = video->video_->id_;
        _files->appendFile(move(video->video_), "video");
        _files->considerAutoDownloading(_videoId, "video");
        if (video->thumbnail_) {
            _thumbnailId = video->thumbnail_->file_->id_;
            _files->appendFile(move(video->thumbnail_->file_), "thumbnail");
        }
        _mediaDuration = video->duration_;
    }

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeAudio::ID) {
        auto audio = static_cast<td_api::linkPreviewTypeAudio*>(_linkPreview->type_.get())->audio_.get();
        if (audio->album_cover_minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(audio->album_cover_minithumbnail_->data_);
        _audioId = audio->audio_->id_;
        _files->appendFile(move(audio->audio_), "other");
        _files->considerAutoDownloading(_audioId, "other");
        _mediaAuthor = QString::fromStdString(audio->performer_);
        _mediaTitle = QString::fromStdString(audio->title_);
        _mediaDuration = audio->duration_;

        if (audio->album_cover_thumbnail_) {
            _thumbnailId = audio->album_cover_thumbnail_->file_->id_;
            _files->appendFile(move(audio->album_cover_thumbnail_->file_), "thumbnail");
        }
    }

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeDocument::ID) {
        auto document = static_cast<td_api::linkPreviewTypeDocument*>(_linkPreview->type_.get())->document_.get();
        _documentId = document->document_->id_;
        _files->appendFile(move(document->document_), "other");
        _files->considerAutoDownloading(_documentId, "other");
        _mediaTitle = QString::fromStdString(document->file_name_);

        if (document->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(document->minithumbnail_->data_);

        if (document->thumbnail_) {
            _thumbnailId = document->thumbnail_->file_->id_;
            _files->appendFile(move(document->thumbnail_->file_), "thumbnail");
        }
    }

    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeAnimation::ID) {
        auto animation = static_cast<td_api::linkPreviewTypeAnimation*>(_linkPreview->type_.get())->animation_.get();
        _animationId = animation->animation_->id_;
        _files->appendFile(move(animation->animation_), "other");
        _files->considerAutoDownloading(_animationId, "other");
        _mediaTitle = QString::fromStdString(animation->file_name_);
        _mediaDuration = animation->duration_;
        _mediaRatio = (float)animation->height_/animation->width_;

        if (animation->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(animation->minithumbnail_->data_);

        if (animation->thumbnail_) {
            _thumbnailId = animation->thumbnail_->file_->id_;
            _files->appendFile(move(animation->thumbnail_->file_), "thumbnail");
        }
    }

    emit webPageChanged();
}

void LinkPreview::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

QString LinkPreview::getUrl()
{
    return QString::fromStdString(_linkPreview->url_);
}

QString LinkPreview::getDisplayUrl()
{
    return QString::fromStdString(_linkPreview->display_url_);
}

QString LinkPreview::getType()
{
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeAlbum::ID) return "album";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeAnimation::ID) return "animation";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeApp::ID) return "app";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeArticle::ID) return "article";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeAudio::ID) return "audio";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeBackground::ID) return "background";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeChannelBoost::ID) return "channelBoost";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeChat::ID) return "chat";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeDocument::ID) return "document";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeEmbeddedAnimationPlayer::ID) return "embeddedAnimationPlayer";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeEmbeddedAudioPlayer::ID) return "embeddedAudioPlayer";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeEmbeddedVideoPlayer::ID) return "embeddedVideoPlayer";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeExternalAudio::ID) return "externalAudio";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeExternalVideo::ID) return "externalVideo";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeGroupCall::ID) return "groupCall";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeInvoice::ID) return "invoice";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeMessage::ID) return "message";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypePhoto::ID) return "photo";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypePremiumGiftCode::ID) return "premiumGiftCode";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeShareableChatFolder::ID) return "shareableChatFolder";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeSticker::ID) return "sticker";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeStickerSet::ID) return "stickerSet";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeStory::ID) return "story";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeSupergroupBoost::ID) return "supergroupBoost";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeTheme::ID) return "theme";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeUnsupported::ID) return "unsupported";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeUpgradedGift::ID) return "upgradedGift";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeUser::ID) return "user";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeVideo::ID) return "video";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeVideoChat::ID) return "videoChat";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeVideoNote::ID) return "videoNote";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeVoiceNote::ID) return "voiceNote";
    if (_linkPreview->type_->get_id() == td_api::linkPreviewTypeWebApp::ID) return "webApp";
}

QString LinkPreview::getName()
{
    return QString::fromStdString(_linkPreview->site_name_);
}

QString LinkPreview::getTitle()
{
    return QString::fromStdString(_linkPreview->title_);
}

QString LinkPreview::getDescription()
{
    return QString::fromStdString(_linkPreview->description_->text_);
}

QByteArray LinkPreview::getMiniThumbnail()
{
    return _miniThumbnail;
}

float LinkPreview::getMediaRatio()
{
    return _mediaRatio;
}

QString LinkPreview::getMediaAuthor()
{
    return _mediaAuthor;
}

QString LinkPreview::getMediaTitle()
{
    return _mediaTitle;
}

int LinkPreview::getMediaDuration()
{
    return _mediaDuration;
}

File* LinkPreview::getPhoto()
{
    if (_photoId == -1) return nullptr;

    return _files->getFile(_photoId).get();
}

File *LinkPreview::getThumbnail()
{
    if (_thumbnailId == -1) return nullptr;

    return _files->getFile(_thumbnailId).get();
}

File *LinkPreview::getVideo()
{
    if (_videoId == -1) return nullptr;

    return _files->getFile(_videoId).get();
}

File *LinkPreview::getAudio()
{
    if (_audioId == -1) return nullptr;

    return _files->getFile(_audioId).get();
}

File *LinkPreview::getDocument()
{
    if (_documentId == -1) return nullptr;

    return _files->getFile(_documentId).get();
}

File *LinkPreview::getAnimation()
{
    if (_animationId == -1) return nullptr;

    return _files->getFile(_animationId).get();
}
