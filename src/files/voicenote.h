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

#ifndef VOICENOTE_H
#define VOICENOTE_H

#include <QObject>
#include "contentfile.h"

class VoiceNote : public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(int32_t duration READ getDuration NOTIFY voiceNoteChanged)
    Q_PROPERTY(QString waveform READ getWaveform NOTIFY voiceNoteChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY voiceNoteChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY voiceNoteChanged)
    Q_PROPERTY(bool isListened READ isListened NOTIFY voiceNoteChanged)
    Q_PROPERTY(File* voicenote READ getVoiceNote NOTIFY voiceNoteChanged)
public:
    explicit VoiceNote(QObject *parent = nullptr);

    void setVoiceNote(td_api::object_ptr<td_api::messageVoiceNote> messageVoiceNote);
    File* getVoiceNote() const;
    void addUpdateFiles();

    td_api::formattedText* getCaption();
    int32_t getDuration() const;
    QString getWaveform() const;
    QString getMimeType() const;
    bool isListened() const;

signals:
    void voiceNoteChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageVoiceNote> _voiceNote;
    int32_t _voiceNoteFileId;
};

#endif // VOICENOTE_H
