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
#include <QTemporaryFile>
#include <QUrl>
#include <stdio.h>
#include <stdlib.h>
#include <vorbis/vorbisfile.h>
//https://github.com/DrKLO/Telegram/blob/e397bd9afdfd9315bf099f78a903f8754d297d7a/TMessagesProj/jni/audio.c#L603
const QString AudioRecorder::defaultStoragePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

AudioRecorder::AudioRecorder(QObject *parent) : QAudioRecorder(parent), m_autoRemove(false), m_recording(false)
{
    codecSettingsMap = {
        { Vorbis, {"audio/vorbis", ".ogg",  "ogg"} },
        { Speex,  {"audio/speex",  ".oga",  "ogg"} },
        { PCM,    {"audio/PCM",    ".wav",  "wav"} },
        { FLAC,   {"audio/FLAC",   ".flac", "raw"} }
    };
    this->setAudioInput("pulseaudio:");
    setCodec(Vorbis);
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
}

QString AudioRecorder::location() const
{
    return m_location;
}


AudioRecorder::AudioCodec AudioRecorder::codec() const
{
    return m_codec;
}

bool AudioRecorder::autoRemove() const
{
    return m_autoRemove;
}

bool AudioRecorder::recording() const
{
    return m_recording;
}

void AudioRecorder::setLocation(const QString &location)
{
    if (m_location == location)
        return;

    m_location = location;
    emit locationChanged(m_location);
}

void AudioRecorder::setCodec(AudioRecorder::AudioCodec codec)
{
    if (m_codec == codec)
        return;

    m_codec = codec;
    emit codecChanged(m_codec);
}

void AudioRecorder::startRecording()
{
    if(this->state() != QMediaRecorder::StoppedState)
    {
        return;
    }
    m_file = new QTemporaryFile(defaultStoragePath + "/XXXXXX.ogg", this);
    m_file->setAutoRemove(false);
    m_file->open();
    setLocation(m_file->fileName());
    m_file->close();
    m_file->deleteLater();

    QAudioEncoderSettings encoderSettings;
    CodecSetting codec = codecSettingsMap[this->codec()];

    encoderSettings.setCodec(codec.codec);
    encoderSettings.setEncodingMode(QMultimedia::ConstantBitRateEncoding);
    encoderSettings.setQuality(QMultimedia::HighQuality);

    int selectedSampleRate = 16000;
    encoderSettings.setSampleRate(selectedSampleRate);


    this->setEncodingSettings(encoderSettings);
    this->setOutputLocation(QUrl(location()));
    this->setContainerFormat(codec.container);

    this->record();
    m_recording = true;
    emit recordingChanged(m_recording);
}

void AudioRecorder::stopRecording()
{
    this->stop();
    m_recording = false;
    emit recordingChanged(m_recording);

}

void AudioRecorder::deleteRecording()
{
    if(QFile::exists(m_location))
        QFile::remove(m_location);
}

void AudioRecorder::setAutoRemove(bool autoRemove)
{
    if (m_autoRemove == autoRemove)
        return;

    m_autoRemove = autoRemove;
    m_file->setAutoRemove(m_autoRemove);
    emit autoRemoveChanged(m_autoRemove);
}

static inline void set_bits(char *bytes, int32_t bitOffset, int32_t value) {
    bytes += bitOffset / 8;
    bitOffset %= 8;
    *((int32_t *) bytes) |= (value << bitOffset);
}

QString AudioRecorder::getWaveform()
{
    //https://xiph.org/vorbis/doc/vorbisfile/example.html
//    https://github.com/DrKLO/Telegram/blob/e397bd9afdfd9315bf099f78a903f8754d297d7a/TMessagesProj/jni/audio.c#L603
    QFile qf(m_location);
     qf.open(QIODevice::ReadOnly);
     int fd = qf.handle();
     FILE* f = fdopen(fd, "rb");
     OggVorbis_File vf;
      int eof=0;
      int current_section;

      int code = ov_open(f,&vf,NULL,0);
      if(code == 0) {
          int bufferSize = 1024 * 128;
          char sampleBuffer[bufferSize];
            int64_t totalSamples = ov_pcm_total(&vf,-1);
            const uint32_t resultSamples = 100;
            int32_t sampleRate = qMax(1, (int32_t) (totalSamples / resultSamples));
            quint16 samples[100];
            for (int32_t i = 0; i < resultSamples; i++)
            samples[i] = 0;
       uint64_t sampleIndex = 0;
       quint16 peakSample = 0;

       uint32_t index = 0;
       while (!eof) {
            long ret = ov_read(&vf, sampleBuffer, bufferSize, 1, 2,
                                      1, &current_section);
            if (ret == 0) {
                 eof=1;
               } else if (ret < 0) {
                   qWarning() << "Can't get samples for waveform. code - " + ret;
               } else {
                   for (int32_t i = 0; i+1 < ret; i = i+2) {
                       qint16 temp = ((qint16)sampleBuffer[i] << 8) | sampleBuffer[i+1];
                       quint16 sample = (quint16) qAbs(temp);
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
               }
         }
       ov_clear(&vf);
       qf.close();

       int64_t sumSamples = 0;
              for (int32_t i = 0; i < resultSamples; i++)
                  sumSamples += samples[i];

              uint32_t bitstreamLength = (resultSamples * 5) / 8 + 1;

      quint16 peak = (quint16) (sumSamples * 1.8f / resultSamples);
       if (peak < 2500)
           peak = 2500;


       for (int32_t i = 0; i < resultSamples; i++) {
           quint16 sample =  samples[i];
           if (sample > peak)
               samples[i] = peak;
        }




        char *bytes = new char[bitstreamLength + 4];
        memset(bytes, 0, bitstreamLength + 4);

        for (int32_t i = 0; i < resultSamples; i++) {
            int32_t value = qMin(31, qAbs((int32_t) samples[i]) * 31 / peak);
            set_bits(bytes, i * 5, value & 31);
        }

        QString resultArray = QString::fromLatin1(bytes,bitstreamLength);

//        std::string resultArray(bytes,bitstreamLength);
        delete[] bytes;

        return resultArray;//QString::fromStdString(resultArray);
      } else
          qDebug() << "Can't get waverofm. code - " << code;
      qf.close();
      return QString();
}



