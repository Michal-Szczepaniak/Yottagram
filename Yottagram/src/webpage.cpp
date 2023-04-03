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

#include "webpage.h"

WebPage::WebPage(QObject *parent) : QObject(parent), _mediaRatio(0), _photoId(-1), _videoId(-1),
    _audioId(-1), _documentId(-1), _animationId(-1), _thumbnailId(-1), _mediaAuthor(""),
    _mediaTitle(""), _mediaDuration(0)
{

}

void WebPage::setWebpage(td_api::object_ptr<td_api::webPage> webPage)
{
    _webPage = std::move(webPage);

    if (_webPage->photo_ && !_webPage->photo_->sizes_.empty()) {
        if (_webPage->photo_->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(_webPage->photo_->minithumbnail_->data_);

        auto sizes = &_webPage->photo_->sizes_;
        auto biggestSize = _webPage->photo_->sizes_.back().get();
        _photoId = biggestSize->photo_->id_;
        _mediaRatio = (float)biggestSize->height_/biggestSize->width_;
        _files->appendFile(move(biggestSize->photo_), "photo");
        _files->considerAutoDownloading(_photoId, "photo");

        if (getType() == "article")
            getPhoto()->download();

        if (sizes->size() > 1) {
            _thumbnailId = _webPage->photo_->sizes_.front()->photo_->id_;
            _files->appendFile(move(_webPage->photo_->sizes_.front()->photo_), "thumbnail");
        } else {
            _thumbnailId = _photoId;
        }
    }


    if (_webPage->video_) {
        _mediaRatio = (float)_webPage->video_->height_/_webPage->video_->width_;
        if (_webPage->video_->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(_webPage->video_->minithumbnail_->data_);
        _videoId = _webPage->video_->video_->id_;
        _files->appendFile(move(_webPage->video_->video_), "video");
        _files->considerAutoDownloading(_videoId, "video");
        if (_webPage->video_->thumbnail_) {
            _thumbnailId = _webPage->video_->thumbnail_->file_->id_;
            _files->appendFile(move(_webPage->video_->thumbnail_->file_), "thumbnail");
        }
        _mediaDuration = _webPage->video_->duration_;
    }

    if (_webPage->audio_) {
        if (_webPage->audio_->album_cover_minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(_webPage->audio_->album_cover_minithumbnail_->data_);
        _audioId = _webPage->audio_->audio_->id_;
        _files->appendFile(move(_webPage->audio_->audio_), "other");
        _files->considerAutoDownloading(_audioId, "other");
        _mediaAuthor = QString::fromStdString(_webPage->audio_->performer_);
        _mediaTitle = QString::fromStdString(_webPage->audio_->title_);
        _mediaDuration = _webPage->audio_->duration_;

        if (_webPage->audio_->album_cover_thumbnail_) {
            _thumbnailId = _webPage->audio_->album_cover_thumbnail_->file_->id_;
            _files->appendFile(move(_webPage->audio_->album_cover_thumbnail_->file_), "thumbnail");
        }
    }

    if (_webPage->document_) {
        _documentId = _webPage->document_->document_->id_;
        _files->appendFile(move(_webPage->document_->document_), "other");
        _files->considerAutoDownloading(_documentId, "other");
        _mediaTitle = QString::fromStdString(_webPage->document_->file_name_);

        if (_webPage->document_->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(_webPage->document_->minithumbnail_->data_);

        if (_webPage->document_->thumbnail_) {
            _thumbnailId = _webPage->document_->thumbnail_->file_->id_;
            _files->appendFile(move(_webPage->document_->thumbnail_->file_), "thumbnail");
        }
    }

    if (_webPage->animation_) {
        _animationId = _webPage->animation_->animation_->id_;
        _files->appendFile(move(_webPage->animation_->animation_), "other");
        _files->considerAutoDownloading(_animationId, "other");
        _mediaTitle = QString::fromStdString(_webPage->animation_->file_name_);
        _mediaDuration = _webPage->animation_->duration_;
        _mediaRatio = (float)_webPage->animation_->height_/_webPage->animation_->width_;

        if (_webPage->animation_->minithumbnail_)
            _miniThumbnail = QByteArray::fromStdString(_webPage->animation_->minithumbnail_->data_);

        if (_webPage->animation_->thumbnail_) {
            _thumbnailId = _webPage->animation_->thumbnail_->file_->id_;
            _files->appendFile(move(_webPage->animation_->thumbnail_->file_), "thumbnail");
        }
    }

    emit webPageChanged();
}

void WebPage::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

QString WebPage::getUrl()
{
    return QString::fromStdString(_webPage->url_);
}

QString WebPage::getDisplayUrl()
{
    return QString::fromStdString(_webPage->display_url_);
}

QString WebPage::getType()
{
    if (_webPage->animation_) return "animation";
    if (_webPage->audio_) return "audio";
    if (_webPage->document_) return "document";
    if (_webPage->sticker_) return "sticker";
    if (_webPage->video_) return "video";
    if (_webPage->video_note_) return "videoNote";
    if (_webPage->voice_note_) return "voiceNote";
    if (_webPage->photo_ && _webPage->type_ != "article") return "photo";
    return "article";
}

QString WebPage::getName()
{
    return QString::fromStdString(_webPage->site_name_);
}

QString WebPage::getTitle()
{
    return QString::fromStdString(_webPage->title_);
}

QString WebPage::getDescription()
{
    return QString::fromStdString(_webPage->description_->text_);
}

QByteArray WebPage::getMiniThumbnail()
{
    return _miniThumbnail;
}

float WebPage::getMediaRatio()
{
    return _mediaRatio;
}

QString WebPage::getMediaAuthor()
{
    return _mediaAuthor;
}

QString WebPage::getMediaTitle()
{
    return _mediaTitle;
}

int WebPage::getMediaDuration()
{
    return _mediaDuration;
}

File* WebPage::getPhoto()
{
    if (_photoId == -1) return nullptr;

    return _files->getFile(_photoId).get();
}

File *WebPage::getThumbnail()
{
    if (_thumbnailId == -1) return nullptr;

    return _files->getFile(_thumbnailId).get();
}

File *WebPage::getVideo()
{
    if (_videoId == -1) return nullptr;

    return _files->getFile(_videoId).get();
}

File *WebPage::getAudio()
{
    if (_audioId == -1) return nullptr;

    return _files->getFile(_audioId).get();
}

File *WebPage::getDocument()
{
    if (_documentId == -1) return nullptr;

    return _files->getFile(_documentId).get();
}

File *WebPage::getAnimation()
{
    if (_animationId == -1) return nullptr;

    return _files->getFile(_animationId).get();
}
