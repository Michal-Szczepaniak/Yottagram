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

#ifndef VIDEO_H
#define VIDEO_H

#include <QObject>
#include <QSize>
#include "contentfile.h"

class Video : public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(bool isSecret READ isSecret NOTIFY videoChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY videoChanged)
    Q_PROPERTY(QString name READ getName NOTIFY videoChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY videoChanged)
    Q_PROPERTY(QByteArray thumbnail READ getThumbnail NOTIFY videoChanged)
    Q_PROPERTY(bool hasStickers READ hasStickers NOTIFY videoChanged)
    Q_PROPERTY(bool supportsStreaming READ getSupportsStreaming NOTIFY videoChanged)
    Q_PROPERTY(File* video READ getVideo NOTIFY videoChanged)
public:
    explicit Video(QObject *parent = nullptr);

    void setVideo(td_api::object_ptr<td_api::messageVideo> messageVideo);
    File* getVideo() const;
    void addUpdateFiles();

    bool isSecret() const;
    td_api::formattedText* getCaption();
    QSize getSize() const;
    QString getName() const;
    QString getMimeType() const;
    QByteArray getThumbnail() const;
    bool hasStickers() const;
    bool getSupportsStreaming() const;

signals:
    void videoChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageVideo> _video;
    qint32 _videoFileId;
};

#endif // VIDEO_H
