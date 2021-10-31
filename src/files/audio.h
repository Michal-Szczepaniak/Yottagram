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

#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include "contentfile.h"

class Audio : public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(int32_t duration READ getDuration NOTIFY audioChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY audioChanged)
    Q_PROPERTY(QString performer READ getPerformer NOTIFY audioChanged)
    Q_PROPERTY(QString name READ getName NOTIFY audioChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY audioChanged)
    Q_PROPERTY(QByteArray thumbnail READ getThumbnail NOTIFY audioChanged)
    Q_PROPERTY(File* audio READ getAudio NOTIFY audioChanged)
public:
    explicit Audio(QObject *parent = nullptr);

    void setAudio(td_api::object_ptr<td_api::messageAudio> messageAudio);
    File* getAudio() const;
    void addUpdateFiles();

    td_api::formattedText* getCaption();
    int32_t getDuration() const;
    QString getTitle() const;
    QString getPerformer() const;
    QString getName() const;
    QString getMimeType() const;
    QByteArray getThumbnail() const;

signals:
    void audioChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageAudio> _audio;
    int32_t _audioFileId;
};

#endif // AUDIO_H
