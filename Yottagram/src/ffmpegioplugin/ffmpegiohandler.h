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

/*

Code partially taken from telegram desktop

This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
    https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

#ifndef FFMPEGIOHANDLER_H
#define FFMPEGIOHANDLER_H

#include "rlottie.h"
#include <QFileDevice>
#include <QFileInfo>
#include <QImageIOHandler>
#include <QSize>
#include <QImage>
#include <QDebug>
#include <zlib.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/version.h>
} // extern "C"

inline constexpr auto kPixelBytesSize = 4;
inline constexpr auto kAVBlockSize = 4096; // 4Kb for ffmpeg blocksize

constexpr auto kUniversalTimeBase = AVRational{ 1, AV_TIME_BASE };
constexpr auto kNormalAspect = AVRational{ 1, 1 };

#define LOG(x) qDebug() << "[ffmpegIOHandler]" << qPrintable(_fileName) << x
#define WARN(x) qWarning() << "[ffmpegIOHandler]" << qPrintable(_fileName) << x

struct SwscaleDeleter {
    QSize srcSize;
    int srcFormat = int(AV_PIX_FMT_NONE);
    QSize dstSize;
    int dstFormat = int(AV_PIX_FMT_NONE);

    void operator()(SwsContext *value);
};

using SwscalePointer = std::unique_ptr<SwsContext, SwscaleDeleter>;

class Packet {
public:
    Packet() = default;
    Packet(Packet &&other) : _data(std::exchange(other._data, {})) {
    }
    Packet &operator=(Packet &&other) {
        if (this != &other) {
            release();
            _data = std::exchange(other._data, {});
        }
        return *this;
    }
    ~Packet() {
        release();
    }

    [[nodiscard]] AVPacket &fields() {
        if (!_data) {
            _data = av_packet_alloc();
        }
        return *_data;
    }
    [[nodiscard]] const AVPacket &fields() const {
        if (!_data) {
            _data = av_packet_alloc();
        }
        return *_data;
    }

    [[nodiscard]] bool empty() const {
        return !_data || !fields().data;
    }
    void release() {
        av_packet_free(&_data);
    }

private:
    mutable AVPacket *_data = nullptr;

};

struct IODeleter {
    void operator()(AVIOContext *value);
};
using IOPointer = std::unique_ptr<AVIOContext, IODeleter>;

struct FormatDeleter {
    void operator()(AVFormatContext *value);
};
using FormatPointer = std::unique_ptr<AVFormatContext, FormatDeleter>;

struct CodecDeleter {
    void operator()(AVCodecContext *value);
};
using CodecPointer = std::unique_ptr<AVCodecContext, CodecDeleter>;

struct FrameDeleter {
    void operator()(AVFrame *value);
};
using FramePointer = std::unique_ptr<AVFrame, FrameDeleter>;

class ffmpegIOHandler : public QImageIOHandler
{
public:
    static const QByteArray NAME;
    static const QByteArray GZ_MAGIC;
    typedef std::string ByteArray;

    ffmpegIOHandler(QIODevice* device, const QByteArray& format);
    ~ffmpegIOHandler();

    bool load();

    // QImageIOHandler
    bool canRead() const Q_DECL_OVERRIDE;
    QByteArray name() const Q_DECL_OVERRIDE;
    bool read(QImage* image) Q_DECL_OVERRIDE;
    QVariant option(ImageOption option) const Q_DECL_OVERRIDE;
    bool supportsOption(ImageOption option) const Q_DECL_OVERRIDE;
    bool jumpToNextImage() Q_DECL_OVERRIDE;
    bool jumpToImage(int imageNumber) Q_DECL_OVERRIDE;
    int loopCount() const Q_DECL_OVERRIDE;
    int imageCount() const Q_DECL_OVERRIDE;
    int nextImageDelay() const Q_DECL_OVERRIDE;
    int currentImageNumber() const Q_DECL_OVERRIDE;
    QRect currentImageRect() const Q_DECL_OVERRIDE;

    static int avReadStatic(void *opaque, uint8_t *buf, int buf_size);
    int avRead(uint8_t *buf, int buf_size);
    static int64_t avSeekStatic(void *opaque, int64_t offset, int whence);
    int64_t avSeek(int64_t offset, int whence);

private:
    IOPointer createAVIOContext();
    FormatPointer createFormatContext(IOPointer io);
    CodecPointer createCodec(AVStream* stream);
    void readNextFrame();
    void resolveNextFrameTiming();
    bool renderCurrent(QImage *storage);
    void jumpToStart();

private:
    struct ReadFrame {
        FramePointer frame;
        int64_t position = 0;
        int64_t duration = 0;
    };

    QString _fileName;
    QSize _size;
    qreal _frameRate;
    int _frameCount;
    int _currentFrame;
    const QByteArray _bytes;
    int _deviceOffset = 0;
    FormatPointer _format{};
    CodecPointer _codec{};
    ReadFrame _current;
    ReadFrame _next;
    SwscalePointer _scale{};

    int _streamId = 0;

    int64_t _framePosition = 0;
    int _nextFrameDelay = 0;
    int _currentFrameDelay = 0;
};

#endif // FFMPEGIOHANDLER_H
