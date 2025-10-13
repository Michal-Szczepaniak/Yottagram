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

#include "ffmpegiohandler.h"

// Code taken from microsoft gsl

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

template <class F>
class final_action
{
public:
    explicit final_action(const F& ff) noexcept : f{ff} { }
    explicit final_action(F&& ff) noexcept : f{std::move(ff)} { }

    ~final_action() noexcept { if (invoke) f(); }

    final_action(final_action&& other) noexcept
        : f(std::move(other.f)), invoke(std::exchange(other.invoke, false))
    { }

    final_action(const final_action&)   = delete;
    void operator=(const final_action&) = delete;
    void operator=(final_action&&)      = delete;

private:
    F f;
    bool invoke = true;
};

// finally() - convenience function to generate a final_action
template <class F>
[[nodiscard]] auto finally(F&& f) noexcept
{
    return final_action<std::decay_t<F>>{std::forward<F>(f)};
}

// END OF MICROSOFT LICENSE

/*

Code partially taken from telegram desktop

This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
    https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

const QByteArray ffmpegIOHandler::NAME("webm");
const QByteArray ffmpegIOHandler::GZ_MAGIC("\x1f\x8b");

constexpr auto kAlignImageBy = 64;
constexpr auto kImageFormat = QImage::Format_ARGB32_Premultiplied;
constexpr auto kMaxScaleByAspectRatio = 16;
constexpr auto kAvioBlockSize = 4096;
constexpr auto kMaxArea = 1920 * 1080 * 4;

void AlignedImageBufferCleanupHandler(void* data) {
    const auto buffer = static_cast<uchar*>(data);
    delete[] buffer;
}

void PremultiplyLine(uchar *dst, const uchar *src, int intsCount) {
    const auto udst = reinterpret_cast<uint*>(dst);
    [[maybe_unused]] const auto usrc = reinterpret_cast<const uint*>(src);

#ifndef LIB_FFMPEG_USE_QT_PRIVATE_API
    for (auto i = 0; i != intsCount; ++i) {
        udst[i] = qPremultiply(usrc[i]);
    }
#else // !LIB_FFMPEG_USE_QT_PRIVATE_API
    static const auto layout = &qPixelLayouts[QImage::Format_ARGB32];
    layout->fetchToARGB32PM(udst, src, 0, intsCount, nullptr, nullptr);
#endif // LIB_FFMPEG_USE_QT_PRIVATE_API
}

void PremultiplyInplace(QImage &image) {
    const auto perLine = image.bytesPerLine();
    const auto width = image.width();
    const auto height = image.height();
    auto bytes = image.bits();
    if (perLine != width * 4) {
        for (auto i = 0; i != height; ++i) {
            PremultiplyLine(bytes, bytes, width);
            bytes += perLine;
        }
    } else {
        PremultiplyLine(bytes, bytes, width * height);
    }
}

void SwscaleDeleter::operator()(SwsContext *value) {
    if (value) {
        sws_freeContext(value);
    }
}

SwscalePointer MakeSwscalePointer(
    QSize srcSize,
    int srcFormat,
    QSize dstSize,
    int dstFormat,
    SwscalePointer *existing) {
    // We have to use custom caching for SwsContext, because
    // sws_getCachedContext checks passed flags with existing context flags,
    // and re-creates context if they're different, but in the process of
    // context creation the passed flags are modified before being written
    // to the resulting context, so the caching doesn't work.
    if (existing && (*existing) != nullptr) {
        const auto &deleter = existing->get_deleter();
        if (deleter.srcSize == srcSize
            && deleter.srcFormat == srcFormat
            && deleter.dstSize == dstSize
            && deleter.dstFormat == dstFormat) {
            return std::move(*existing);
        }
    }
    if (srcFormat <= AV_PIX_FMT_NONE || srcFormat >= AV_PIX_FMT_NB) {
        qDebug() << "frame->format";
        return SwscalePointer();
    }

    const auto result = sws_getCachedContext(
        existing ? existing->release() : nullptr,
        srcSize.width(),
        srcSize.height(),
        AVPixelFormat(srcFormat),
        dstSize.width(),
        dstSize.height(),
        AVPixelFormat(dstFormat),
        0,
        nullptr,
        nullptr,
        nullptr);
    if (!result) {
        qDebug() << "sws_getCachedContext";
    }
    return SwscalePointer(
        result,
        { srcSize, srcFormat, dstSize, dstFormat });
}

void IODeleter::operator()(AVIOContext *value) {
    if (value) {
        av_freep(&value->buffer);
        avio_context_free(&value);
    }
}

void CodecDeleter::operator()(AVCodecContext *value) {
    if (value) {
        avcodec_free_context(&value);
    }
}

void FormatDeleter::operator()(AVFormatContext *value) {
    if (value) {
        const auto deleter = IOPointer(value->pb);
        avformat_close_input(&value);
    }
}

void FrameDeleter::operator()(AVFrame *value) {
    av_frame_free(&value);
}

ffmpegIOHandler::ffmpegIOHandler(QIODevice* device, const QByteArray& format) :
    _frameRate(0.),
    _frameCount(0),
    _currentFrame(0),
    _bytes(device->readAll())
{
    QFileDevice* file = qobject_cast<QFileDevice*>(device);
    if (file) {
        _fileName = QFileInfo(file->fileName()).fileName();
    }
    setDevice(device);
    setFormat(format);

    IOPointer context = createAVIOContext();

    _format = createFormatContext(std::move(context));

    if (nullptr == _format) {
        return;
    }

    int error = 0;
    if ((error = avformat_find_stream_info(_format.get(), nullptr)) < 0) {
        return;
    }
    _streamId = av_find_best_stream(_format.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (_streamId < 0) {
        return;
    }

    AVStream* info = _format->streams[_streamId];
    _codec = createCodec(info);
    _frameCount = info->nb_frames;
}

ffmpegIOHandler::~ffmpegIOHandler()
{
}

bool ffmpegIOHandler::load()
{
    if (_current.frame) {
        return true;
    }

    if (!_codec) {
        LOG("load no codec");
        return false;
    } else if (!_current.frame) {
        readNextFrame();
    }

    std::swap(_current, _next);
    if (!_current.frame) {
        LOG("load no current");

        return false;
    }

    return true;
}

bool ffmpegIOHandler::read(QImage* out)
{
    if (!_codec) {
        LOG("read no codec");
        return false;
    } else if (!_current.frame && !_next.frame) {
        readNextFrame();

        std::swap(_current, _next);
    } else if (_next.frame) {
        std::swap(_current, _next);
    }

    if (!_current.frame) {
        LOG("read no current");
        return false;
    }

    readNextFrame();

    return renderCurrent(out);
}

bool ffmpegIOHandler::canRead() const
{
    if (!device()) {
        return false;
    }

    return _codec != nullptr;
}

QByteArray ffmpegIOHandler::name() const
{
    return NAME;
}

QVariant ffmpegIOHandler::option(ImageOption option) const
{
    switch (option) {
    case Size:
        ((ffmpegIOHandler*)this)->load(); // Cast off const
        return _size;
    case Animation:
        return true;
    case ImageFormat:
        return QImage::Format_ARGB32_Premultiplied;
    default:
        break;
    }
    return QVariant();
}

bool ffmpegIOHandler::supportsOption(ImageOption option) const
{
    switch(option) {
    case Size:
    case Animation:
    case ImageFormat:
        return true;
    default:
        break;
    }
    return false;
}

bool ffmpegIOHandler::jumpToNextImage()
{
    LOG("jumpToNextImage");

    if (_frameCount) {

        return true;
    }
    return false;
}

bool ffmpegIOHandler::jumpToImage(int imageNumber)
{
    LOG("jumpToImage");

    if (_frameCount) {
        if (imageNumber != _currentFrame) {
        }
        return true;
    }
    return false;
}

int ffmpegIOHandler::loopCount() const
{
    return -1;
}

int ffmpegIOHandler::imageCount() const
{
    LOG("Current count: ") << _frameCount;
    return _frameCount;
}

int ffmpegIOHandler::currentImageNumber() const
{
    LOG("Current position: ") << _framePosition;
    return _framePosition;
}

QRect ffmpegIOHandler::currentImageRect() const
{
    LOG("Current size: ") << _size;
    return QRect(QPoint(), _size);
}

int ffmpegIOHandler::avReadStatic(void *opaque, uint8_t *buf, int buf_size)
{
    return static_cast<ffmpegIOHandler*>(opaque)->avRead(buf, buf_size);
}

int ffmpegIOHandler::avRead(uint8_t *buf, int buf_size)
{
    const auto available = _bytes.size() - _deviceOffset;
    if (available <= 0) {
        return AVERROR_EOF;
    }
    const auto fill = std::min(int(available), buf_size);
    memcpy(buf, _bytes.data() + _deviceOffset, fill);
    _deviceOffset += fill;
    return fill;
}

int64_t ffmpegIOHandler::avSeekStatic(void *opaque, int64_t offset, int whence)
{
    return static_cast<ffmpegIOHandler*>(opaque)->avSeek(offset, whence);
}

int64_t ffmpegIOHandler::avSeek(int64_t offset, int whence)
{
    if (whence == AVSEEK_SIZE) {
        return _bytes.size();
    }

    const auto now = [&] {
        switch (whence) {
        case SEEK_SET: return offset;
        case SEEK_CUR: return _deviceOffset + offset;
        case SEEK_END: return int64_t(_bytes.size()) + offset;
        }
        return int64_t(-1);
    }();

    if (now < 0 || now > _bytes.size()) {
        return -1;
    }

    _deviceOffset = now;

    return now;
}

IOPointer ffmpegIOHandler::createAVIOContext()
{
    auto buffer = reinterpret_cast<uchar*>(av_malloc(kAvioBlockSize));
    if (!buffer) {
        LOG("av_malloc");
        return nullptr;
    }

    auto result = avio_alloc_context(
        buffer,
        kAvioBlockSize,
        0,
        this,
        avReadStatic,
        nullptr,
        avSeekStatic
    );

    if (!result) {
        av_freep(&buffer);
        LOG("avio_alloc_context");
        return nullptr;
    }

    return IOPointer(result);
}

FormatPointer ffmpegIOHandler::createFormatContext(IOPointer io)
{
    io->seekable = true;

    auto result = avformat_alloc_context();
    if (result == nullptr) {
        LOG("avformat_alloc_context");
        return nullptr;
    }

    result->pb = io.get();
    result->flags |= AVFMT_FLAG_CUSTOM_IO;

    io.release();

    auto options = (AVDictionary*)nullptr;
    av_dict_set(&options, "usetoc", "1", 0);

    const int error = avformat_open_input(&result, nullptr, nullptr, &options);
    if (error < 0) {
        // avformat_open_input freed 'result' in case an error happened.
        char string[AV_ERROR_MAX_STRING_SIZE] = { 0 };

        LOG("avformat_open_input: ") << QString::fromUtf8(av_make_error_string(string, sizeof(string), error));
        return nullptr;
    }
    result->flags |= AVFMT_FLAG_FAST_SEEK;

    av_dict_free(&options);

    return FormatPointer(result);
}

CodecPointer ffmpegIOHandler::createCodec(AVStream *stream)
{
    AVCodecContext* context = avcodec_alloc_context3(nullptr);
    if (context == nullptr) {
        LOG("avcodec_alloc_context3");
        return nullptr;
    }

    int error = avcodec_parameters_to_context(context, stream->codecpar);
    if (error < 0) {
        LOG("avcodec_parameters_to_context");
        return nullptr;
    }

    context->pkt_timebase = stream->time_base;
    av_opt_set(context, "threads", "auto", 0);
    av_opt_set_int(context, "refcounted_frames", 1, 0);

    const AVCodec* codec = (context->codec_id == AV_CODEC_ID_VP9) ? avcodec_find_decoder_by_name("libvpx-vp9") : avcodec_find_decoder(context->codec_id);
    if (codec == nullptr) {
        LOG("avcodec_find_decoder");
        return nullptr;
    }

    if ((error = avcodec_open2(context, codec, nullptr)) < 0) {
        LOG("avcodec_open2");
        return nullptr;
    }

    return CodecPointer(context);
}

void ffmpegIOHandler::readNextFrame()
{
    auto frame = _next.frame ? std::exchange(_next.frame, {}): FramePointer(av_frame_alloc());
    while (true) {
        auto result = avcodec_receive_frame(_codec.get(), frame.get());
        if (result >= 0) {
            if (frame->width * frame->height > kMaxArea) {
                return;
            }
            _next.frame = std::move(frame);
            _size = QSize(_next.frame->width, _next.frame->height);
            resolveNextFrameTiming();
            return;
        }

        if (result == AVERROR_EOF) {
            return;
        } else if (result != AVERROR(EAGAIN)) {
            char string[AV_ERROR_MAX_STRING_SIZE] = { 0 };

            LOG("Webm Error: Unable to avcodec_receive_frame(), ") << QString::fromUtf8(av_make_error_string(string, sizeof(string), result));
            return;
        }

        auto packet = Packet();
        auto finished = false;
        do {
            const auto result = av_read_frame(
                _format.get(),
                &packet.fields());
            if (result == AVERROR_EOF) {
                finished = true;
                break;
            } else if (result < 0) {
                char string[AV_ERROR_MAX_STRING_SIZE] = { 0 };

                LOG("Webm Error: Unable to av_read_frame(), ") << QString::fromUtf8(av_make_error_string(string, sizeof(string), result));
                return;
            }
        } while (packet.fields().stream_index != _streamId);

        if (finished) {
            result = avcodec_send_packet(_codec.get(), nullptr); // Drain.
        } else {
            const auto native = &packet.fields();
            const auto guard = finally([
                &,
                size = native->size,
                data = native->data
            ] {
                native->size = size;
                native->data = data;
                packet = Packet();
            });
            result = avcodec_send_packet(_codec.get(), native);
        }
        if (result < 0) {
            char string[AV_ERROR_MAX_STRING_SIZE] = { 0 };

            LOG("Webm Error: Unable to avcodec_send_packet(), ") << QString::fromUtf8(av_make_error_string(string, sizeof(string), result));
            return;
        }
    }
}

void ffmpegIOHandler::resolveNextFrameTiming()
{
    const auto base = _format->streams[_streamId]->time_base;
#if DA_FFMPEG_HAVE_DURATION
    const auto duration = _next.frame->duration;
#else
    const auto duration = _next.frame->pkt_duration;
#endif
    const auto framePts = _next.frame->pts;
    auto framePosition = (framePts * 1000LL * base.num) / base.den;
    _currentFrameDelay = _nextFrameDelay;
    if (_framePosition + _currentFrameDelay < framePosition) {
        _currentFrameDelay = (int32_t)(framePosition - _framePosition);
    } else if (framePosition < _framePosition + _currentFrameDelay) {
        framePosition = _framePosition + _currentFrameDelay;
    }

    if (duration == AV_NOPTS_VALUE) {
        _nextFrameDelay = 0;
    } else {
        _nextFrameDelay = (duration * 1000LL * base.num) / base.den;
    }
    _framePosition = framePosition;

    _next.position = _framePosition;
    _next.duration = _nextFrameDelay;
}

bool ffmpegIOHandler::renderCurrent(QImage *out)
{
    if (_current.frame == nullptr) {
        LOG("render current false");
        return false;
    }

    const auto frame = _current.frame.get();
    const auto width = frame->width;
    const auto height = frame->height;
    if (!width || !height) {
        LOG("Webm Error: Bad frame size: ") << width << "x" << height;
        return false;
    }

    const auto widthAlign = kAlignImageBy / kPixelBytesSize;
    const auto neededWidth = width + ((width % widthAlign)
                                          ? (widthAlign - (width % widthAlign))
                                          : 0);
    auto perLine = neededWidth * kPixelBytesSize;
    const auto buffer = new uchar[perLine * height + kAlignImageBy];
    const auto cleanupData = static_cast<void *>(buffer);
    const auto address = reinterpret_cast<uintptr_t>(buffer);
    const auto alignedBuffer = buffer + ((address % kAlignImageBy)
                                             ? (kAlignImageBy - (address % kAlignImageBy))
                                             : 0);
    auto storage = QImage(
        alignedBuffer,
        width,
        height,
        perLine,
        kImageFormat,
        AlignedImageBufferCleanupHandler,
        cleanupData);

    const auto srcFormat = (frame->format == AV_PIX_FMT_NONE)
                               ? _codec->pix_fmt
                               : frame->format;
    const auto srcSize = QSize(frame->width, frame->height);
    const auto bgra = (srcFormat == AV_PIX_FMT_BGRA);
    const auto withAlpha = bgra || (srcFormat == AV_PIX_FMT_YUVA420P);
    const auto dstPerLine = storage.bytesPerLine();
    const auto dstFormat = AV_PIX_FMT_BGRA;
    auto dst = storage.bits();

    if (bgra) {
        const auto srcPerLine = frame->linesize[0];
        const auto perLine = std::min(srcPerLine, int(dstPerLine));
        auto src = frame->data[0];
        for (auto y = 0, height = srcSize.height(); y != height; ++y) {
            memcpy(dst, src, perLine);
            src += srcPerLine;
            dst += dstPerLine;
        }
    } else {
        _scale = MakeSwscalePointer(
            srcSize,
            srcFormat,
            srcSize,
            dstFormat,
            &_scale);
        if (_scale == nullptr) {
            return false;
        }

        // AV_NUM_DATA_POINTERS defined in AVFrame struct
        uint8_t *dstData[AV_NUM_DATA_POINTERS] = { dst, nullptr };
        int dstLinesize[AV_NUM_DATA_POINTERS] = { int(dstPerLine), 0 };
        sws_scale(
            _scale.get(),
            frame->data,
            frame->linesize,
            0,
            frame->height,
            dstData,
            dstLinesize);
    }

    if (withAlpha) {
        PremultiplyInplace(storage);
    }

    *out = storage;
    return true;
}

void ffmpegIOHandler::jumpToStart()
{
    if (!_codec) {
        return;
    }
    auto result = 0;
    if ((result = avformat_seek_file(_format.get(), _streamId, std::numeric_limits<int64_t>::min(), 0, std::numeric_limits<int64_t>::max(), 0)) < 0) {
        if ((result = av_seek_frame(_format.get(), _streamId, 0, AVSEEK_FLAG_BYTE)) < 0) {
            if ((result = av_seek_frame(_format.get(), _streamId, 0, AVSEEK_FLAG_FRAME)) < 0) {
                if ((result = av_seek_frame(_format.get(), _streamId, 0, 0)) < 0) {
                    char string[AV_ERROR_MAX_STRING_SIZE] = { 0 };

                    LOG("Webm Error: Unable to av_seek_frame() to the start, ") << QString::fromUtf8(av_make_error_string(string, sizeof(string), result));
                    return;
                }
            }
        }
    }
    avcodec_flush_buffers(_codec.get());
    _current = ReadFrame();
    _next = ReadFrame();
    _currentFrameDelay = _nextFrameDelay = 0;
    _framePosition = 0;
}

int ffmpegIOHandler::nextImageDelay() const
{
    const auto base = _format->streams[_streamId]->time_base;

    if (!_next.frame) {
        ((ffmpegIOHandler*)this)->jumpToStart();

        return 0;
    }

#if DA_FFMPEG_HAVE_DURATION
    const auto duration = _next.frame->duration;
#else
    const auto duration = _next.frame->pkt_duration;
#endif

    int nextFrameDelay = 0;
    if (duration != AV_NOPTS_VALUE) {
        nextFrameDelay = (duration * 1000LL * base.num) / base.den;
    }

    return nextFrameDelay;
}
