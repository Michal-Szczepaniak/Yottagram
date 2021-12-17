#include "tgsiohandler.h"

const QByteArray TgsIOHandler::NAME("tgs");
const QByteArray TgsIOHandler::GZ_MAGIC("\x1f\x8b");

TgsIOHandler::TgsIOHandler(QIODevice* device, const QByteArray& format) :
    frameRate(0.),
    frameCount(0),
    currentFrame(0)
{
    QFileDevice* file = qobject_cast<QFileDevice*>(device);
    if (file) {
        fileName = QFileInfo(file->fileName()).fileName();
    }
    setDevice(device);
    setFormat(format);
}

TgsIOHandler::~TgsIOHandler()
{
    if (currentRender.valid()) {
        currentRender.get();
    }
    LOG("Done");
}

TgsIOHandler::ByteArray TgsIOHandler::uncompress()
{
    ByteArray unzipped;
    const QByteArray zipped(device()->readAll());
    if (!zipped.isEmpty()) {
        z_stream unzip;
        memset(&unzip, 0, sizeof(unzip));
        unzip.next_in = (Bytef*)zipped.constData();
        // Add 16 for decoding gzip header
        int zerr = inflateInit2(&unzip, MAX_WBITS + 16);
        if (zerr == Z_OK) {
            const uint chunk = 0x1000;
            unzipped.resize(chunk);
            unzip.next_out = (Bytef*)unzipped.data();
            unzip.avail_in = zipped.size();
            unzip.avail_out = chunk;
            LOG("Compressed size" << zipped.size());
            while (unzip.avail_out > 0 && zerr == Z_OK) {
                zerr = inflate(&unzip, Z_NO_FLUSH);
                if (zerr == Z_OK && unzip.avail_out < chunk) {
                    // Data may get relocated, update next_out too
                    unzipped.resize(unzipped.size() + chunk);
                    unzip.next_out = (Bytef*)unzipped.data() + unzip.total_out;
                    unzip.avail_out += chunk;
                }
            }
            if (zerr == Z_STREAM_END) {
                unzipped.resize(unzip.next_out - (Bytef*)unzipped.data());
                LOG("Uncompressed size" << unzipped.size());
            } else {
                unzipped.clear();
            }
            inflateEnd(&unzip);
        }
    }
    return unzipped;
}

bool TgsIOHandler::load()
{
    if (!animation && device()) {
        ByteArray json(uncompress());
        if (json.size() > 0) {
            animation = rlottie::Animation::loadFromData(json, std::string(), std::string(), false);
            if (animation) {
                size_t width, height;
                animation->size(width, height);
                frameRate = animation->frameRate();
                frameCount = (int) animation->totalFrame();
                size = QSize(width, height);
                LOG(size << frameCount << "frames," << frameRate << "fps");
                render(0); // Pre-render first frame
            }
        }
    }
    return animation != Q_NULLPTR;
}

void TgsIOHandler::finishRendering()
{
    if (currentRender.valid()) {
        currentRender.get();
        prevImage = currentImage;
        if (!currentFrame && !firstImage.isNull()) {
            LOG("Rendered first frame");
            firstImage = currentImage;
        }
    } else {
        // Must be the first frame
        prevImage = currentImage;
    }
}

void TgsIOHandler::render(int frameIndex)
{
    currentFrame = frameIndex % frameCount;
    if (!currentFrame && !firstImage.isNull()) {
        // The first frame only gets rendered once
        currentImage = firstImage;
    } else {
        const int width = (int)size.width();
        const int height = (int)size.height();
        currentImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
        currentRender = animation->render(currentFrame,
            rlottie::Surface((uint32_t*)currentImage.bits(),
                width, height, currentImage.bytesPerLine()));
    }
}

bool TgsIOHandler::read(QImage* out)
{
    if (load() && frameCount > 0) {
        // We must have the first frame, will wait if necessary
        if (currentFrame && currentRender.valid()) {
            std::future_status status = currentRender.wait_for(std::chrono::milliseconds(0));
            if (status != std::future_status::ready) {
                LOG("Skipping frame" << currentFrame);
                currentFrame = (currentFrame + 1) % frameCount;
                *out = prevImage;
                return true;
            }
        }
        finishRendering();
        *out = currentImage;
        render(currentFrame + 1);
        return true;
    }
    return false;
}

bool TgsIOHandler::canRead() const
{
    if (!device()) {
        return false;
    } else if (animation) {
        return true;
    } else {
        // Need to support uncompressed data?
        return device()->peek(2) == GZ_MAGIC;
    }
}

QByteArray TgsIOHandler::name() const
{
    return NAME;
}

QVariant TgsIOHandler::option(ImageOption option) const
{
    switch (option) {
    case Size:
        ((TgsIOHandler*)this)->load(); // Cast off const
        return size;
    case Animation:
        return true;
    case ImageFormat:
        return QImage::Format_ARGB32_Premultiplied;
    default:
        break;
    }
    return QVariant();
}

bool TgsIOHandler::supportsOption(ImageOption option) const
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

bool TgsIOHandler::jumpToNextImage()
{
    if (frameCount) {
        finishRendering();
        render(currentFrame + 1);
        return true;
    }
    return false;
}

bool TgsIOHandler::jumpToImage(int imageNumber)
{
    if (frameCount) {
        if (imageNumber != currentFrame) {
            finishRendering();
            render(imageNumber);
        }
        return true;
    }
    return false;
}

int TgsIOHandler::loopCount() const
{
    return -1;
}

int TgsIOHandler::imageCount() const
{
    return frameCount;
}

int TgsIOHandler::currentImageNumber() const
{
    return currentFrame;
}

QRect TgsIOHandler::currentImageRect() const
{
    return QRect(QPoint(), size);
}

int TgsIOHandler::nextImageDelay() const
{
    return frameRate > 0 ? (int)(1000/frameRate) : 33;
}
