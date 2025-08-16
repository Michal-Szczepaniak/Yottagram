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

#ifndef LINKPREVIEW_H
#define LINKPREVIEW_H

#include <QObject>
#include <QDebug>
#include "core/telegrammanager.h"
#include "files/files.h"

class LinkPreview : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ getUrl NOTIFY webPageChanged)
    Q_PROPERTY(QString displayUrl READ getDisplayUrl NOTIFY webPageChanged)
    Q_PROPERTY(QString type READ getType NOTIFY webPageChanged)
    Q_PROPERTY(QString name READ getName NOTIFY webPageChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY webPageChanged)
    Q_PROPERTY(QString description READ getDescription NOTIFY webPageChanged)
    Q_PROPERTY(QString mediaAuthor READ getMediaAuthor NOTIFY webPageChanged)
    Q_PROPERTY(QString mediaTitle READ getMediaTitle NOTIFY webPageChanged)
    Q_PROPERTY(int mediaDuration READ getMediaDuration NOTIFY webPageChanged)
    Q_PROPERTY(QByteArray miniThumbnail READ getMiniThumbnail NOTIFY webPageChanged)
    Q_PROPERTY(float mediaRatio READ getMediaRatio NOTIFY webPageChanged)
    Q_PROPERTY(File* photo READ getPhoto NOTIFY webPageChanged)
    Q_PROPERTY(File* thumbnail READ getThumbnail NOTIFY webPageChanged)
    Q_PROPERTY(File* video READ getVideo NOTIFY webPageChanged)
    Q_PROPERTY(File* audio READ getAudio NOTIFY webPageChanged)
    Q_PROPERTY(File* document READ getDocument NOTIFY webPageChanged)
    Q_PROPERTY(File* animation READ getAnimation NOTIFY webPageChanged)
public:
    explicit LinkPreview(QObject *parent = nullptr);
    void setLinkPreview(td_api::object_ptr<td_api::linkPreview> linkPreview);
    void setFiles(shared_ptr<Files> files);

    QString getUrl();
    QString getDisplayUrl();
    QString getType();
    QString getName();
    QString getTitle();
    QString getDescription();
    QByteArray getMiniThumbnail();
    float getMediaRatio();
    QString getMediaAuthor();
    QString getMediaTitle();
    int getMediaDuration();
    File* getPhoto();
    File* getThumbnail();
    File* getVideo();
    File* getAudio();
    File* getDocument();
    File* getAnimation();

signals:
    void webPageChanged();

private:
    float _mediaRatio;
    int32_t _photoId;
    int32_t _videoId;
    int32_t _audioId;
    int32_t _documentId;
    int32_t _animationId;
    int32_t _thumbnailId;
    QByteArray _miniThumbnail;
    QString _mediaAuthor;
    QString _mediaTitle;
    int _mediaDuration;
    td_api::object_ptr<td_api::linkPreview> _linkPreview;
    std::shared_ptr<Files> _files;
};
Q_DECLARE_METATYPE(LinkPreview*)

#endif // LINKPREVIEW_H
