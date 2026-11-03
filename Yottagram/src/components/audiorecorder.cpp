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

#include "audiorecorder.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QUrl>
#include <stdio.h>
#include <stdlib.h>
#include <opusfile.h>

//https://github.com/DrKLO/Telegram/blob/e397bd9afdfd9315bf099f78a903f8754d297d7a/TMessagesProj/jni/audio.c#L603
const QString AudioRecorder::defaultStoragePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

AudioRecorder::AudioRecorder(QObject *parent) : QObject(parent), _autoRemove(false), _recording(false),
    _audioResource((QObject*)this, AudioResourceQt::AudioResource::MediaType)
{
    setVolume(_volume);

    connect(&_audioResource, &AudioResourceQt::AudioResource::acquiredChanged, [this](){ if (!_audioResource.isAcquired()) stopRecording(); });
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
}

QString AudioRecorder::location() const
{
    return _location;
}

bool AudioRecorder::autoRemove() const
{
    return _autoRemove;
}

bool AudioRecorder::recording() const
{
    return _recording;
}

qreal AudioRecorder::getVolume() const
{
    return _volume;
}

void AudioRecorder::setVolume(qreal volume)
{
    _volume = volume;
}

int AudioRecorder::state()
{
    if (!_pipeline)
        return GST_STATE_NULL;

    GstState current;
    GstState pending;

    GstStateChangeReturn ret = gst_element_get_state(
        _pipeline,
        &current,
        &pending,
        0
    );

    if (ret == GST_STATE_CHANGE_FAILURE)
        return GST_STATE_NULL;

    if (pending == GST_STATE_PLAYING)
        return pending;

    if (pending == GST_STATE_NULL)
        return pending;

    return current;
}

void AudioRecorder::setLocation(const QString &location)
{
    if (_location == location)
        return;

    _location = location;
    emit locationChanged(_location);
}

void AudioRecorder::startRecording()
{
    if (_pipeline)
        return;

    _file = new QTemporaryFile(defaultStoragePath + "/XXXXXX.ogg", this);
    _file->setAutoRemove(false);
    _file->open();
    setLocation(_file->fileName());
    _file->close();
    _file->deleteLater();

    _audioResource.acquire();

    QString pipelineDescription = QString(
        "pulsesrc ! "
        "audioconvert ! "
        "audioresample ! "
        "volume volume=%2 !"
        "opusenc bitrate=48000 ! "
        "oggmux ! "
        "filesink location=\"%1\""
    ).arg(_location).arg(_volume);

    GError *error = nullptr;

    _pipeline = gst_parse_launch(pipelineDescription.toUtf8().constData(), &error);

    if (!_pipeline) {
        qWarning() << "Failed to create pipeline:"
                   << (error ? error->message : "unknown error");

        if (error)
            g_error_free(error);

        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning() << "Failed to start recording";

        gst_object_unref(_pipeline);
        _pipeline = nullptr;

        return;
    }

    _recording = true;
    emit recordingChanged(_recording);
}

void AudioRecorder::stopRecording()
{
    if (!_pipeline)
        return;

    gst_element_send_event(_pipeline, gst_event_new_eos());

    GstBus *bus = gst_element_get_bus(_pipeline);

    gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(
            GST_MESSAGE_EOS |
            GST_MESSAGE_ERROR
        )
    );

    gst_object_unref(bus);

    gst_element_set_state(
        _pipeline,
        GST_STATE_NULL);

    gst_object_unref(_pipeline);
    _pipeline = nullptr;

    _recording = false;
    emit recordingChanged(_recording);
}

void AudioRecorder::deleteRecording()
{
    if(QFile::exists(_location))
        QFile::remove(_location);
}

void AudioRecorder::setAutoRemove(bool autoRemove)
{
    if (_autoRemove == autoRemove)
        return;

    _autoRemove = autoRemove;
    _file->setAutoRemove(_autoRemove);
    emit autoRemoveChanged(_autoRemove);
}

static inline void set_bits(char *bytes, int32_t bitOffset, int32_t value) {
    bytes += bitOffset / 8;
    bitOffset %= 8;
    *((int32_t *) bytes) |= (value << bitOffset);
}

QString AudioRecorder::getWaveform()
{
    int error = OPUS_OK;
    OggOpusFile *opusFile = op_open_file(_location.toStdString().c_str(), &error);
    if(opusFile != NULL && error == OPUS_OK) {
        int bufferSize = 1024 * 128;
        opus_int16 sampleBuffer[bufferSize];

        int64_t totalSamples = op_pcm_total(opusFile, -1);
        const uint32_t resultSamples = 100;
        int32_t sampleRate = qMax(1, (int32_t) (totalSamples / resultSamples));
        uint16_t *samples = (uint16_t*)malloc(100 * 2);
        uint64_t sampleIndex = 0;
        uint16_t peakSample = 0;
        int32_t index = 0;

        while (1) {
            int readSamples = op_read(opusFile, sampleBuffer, bufferSize / 2, NULL);
            for (int32_t i = 0; i < readSamples; i++) {
                uint16_t sample = (uint16_t) abs(sampleBuffer[i]);
                if (sample > peakSample) {
                    peakSample = sample;
                }
                if (sampleIndex++ % sampleRate == 0) {
                    if (index < resultSamples) {
                        samples[index++] = peakSample;
                    }
                    peakSample = 0;
                }
            }
            if (readSamples == 0) {
                break;
            }
        }

        int64_t sumSamples = 0;
        for (int32_t i = 0; i < resultSamples; i++) {
            sumSamples += samples[i];
        }
        uint16_t peak = (uint16_t) (sumSamples * 1.8f / resultSamples);
        if (peak < 2500) {
            peak = 2500;
        }

        for (int32_t i = 0; i < resultSamples; i++) {
            uint16_t sample = (uint16_t) ((int64_t) samples[i]);
            if (sample > peak) {
                samples[i] = peak;
            }
        }

        op_free(opusFile);

        uint32_t bitstreamLength = (resultSamples * 5) / 8 + 1;
        char *bytes = new char[bitstreamLength + 4];
        memset(bytes, 0, bitstreamLength + 4);

        for (int32_t i = 0; i < resultSamples; i++) {
            int32_t value = qMin(31, qAbs((int32_t) samples[i]) * 31 / peak);
            set_bits(bytes, i * 5, value & 31);
        }

        QString resultArray = QString::fromLatin1(bytes,bitstreamLength);

        delete[] bytes;
        free(samples);

        return resultArray;
    } else {
        qDebug() << "Can't get waverofm. code - " << error;
    }

    return QString();
}



