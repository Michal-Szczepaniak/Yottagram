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

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <gst/gst.h>
#include <QTemporaryFile>
#include <audioresource-qt/AudioResourceQt>

class AudioRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString location READ location NOTIFY locationChanged)
    Q_PROPERTY(bool autoRemove READ autoRemove WRITE setAutoRemove NOTIFY autoRemoveChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(qreal volume READ getVolume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder();

    QString location() const;
    bool autoRemove() const;
    bool recording() const;
    qreal getVolume() const;
    void setVolume(qreal volume);
    int state();

public slots:
    void startRecording();
    void stopRecording();
    void deleteRecording();
    void setAutoRemove(bool autoRemove);
    QString getWaveform();

signals:
    void locationChanged(QString location);

    void autoRemoveChanged(bool autoRemove);
    void recordingChanged(bool recording);
    void volumeChanged();

private:
    static const QString defaultStoragePath;
    QString _location;
    void setLocation(const QString &location);

    GstElement *_pipeline = nullptr;
    QTemporaryFile *_file;

    bool _autoRemove;
    bool _recording;
    qreal _volume = 1.0f;
    AudioResourceQt::AudioResource _audioResource;
};

#endif // AUDIORECORDER_H
