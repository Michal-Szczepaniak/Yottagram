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

#ifndef VIDEONOTE_H
#define VIDEONOTE_H

#include <QObject>
#include <QSize>
#include "contentfile.h"

class VideoNote: public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(bool isSecret READ isSecret NOTIFY videoNoteChanged)
    Q_PROPERTY(bool isViewed READ isViewed NOTIFY videoNoteChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY videoNoteChanged)
    Q_PROPERTY(QByteArray thumbnail READ getThumbnail NOTIFY videoNoteChanged)
    Q_PROPERTY(File* videonote READ getVideoNote NOTIFY videoNoteChanged)
public:
    explicit VideoNote(QObject *parent = nullptr);

    void setVideoNote(td_api::object_ptr<td_api::messageVideoNote> messageVideoNote);
    File* getVideoNote() const;
    QByteArray getThumbnail() const;
    void addUpdateFiles();

    bool isSecret() const;
    bool isViewed() const;
    QSize getSize() const;

signals:
    void videoNoteChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageVideoNote> _videoNote;
    qint32 _videoNoteFileId;
};

#endif // VIDEONOTE_H
