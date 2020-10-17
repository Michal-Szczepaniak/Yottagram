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

#include "voicenote.h"

VoiceNote::VoiceNote(QObject *parent) : ContentFile(parent)
{

}

void VoiceNote::setVoiceNote(td_api::object_ptr<td_api::messageVoiceNote> messageVoiceNote)
{
    _voiceNote = std::move(messageVoiceNote);

    addUpdateFiles();

    emit voiceNoteChanged();
}

File *VoiceNote::getVoiceNote() const
{
    return _files->getFile(_voiceNoteFileId).get();
}

void VoiceNote::addUpdateFiles()
{
    _voiceNoteFileId = _voiceNote->voice_note_->voice_->id_;
    _files->appendFile(std::move(_voiceNote->voice_note_->voice_), "other");
}

td_api::formattedText* VoiceNote::getCaption()
{
    return _voiceNote->caption_.get();
}

qint32 VoiceNote::getDuration() const
{
    return _voiceNote->voice_note_->duration_;
}

QString VoiceNote::getWaveform() const
{
    return QString::fromStdString(_voiceNote->voice_note_->waveform_);
}

QString VoiceNote::getMimeType() const
{
    return QString::fromStdString(_voiceNote->voice_note_->mime_type_);
}

bool VoiceNote::isListened() const
{
    return _voiceNote->is_listened_;
}
