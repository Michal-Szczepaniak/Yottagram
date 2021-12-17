/*
    Copyright (C) 2020 Slava Monich at al, Michał Szczepaniak.
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TGSIOHANDLER_H
#define TGSIOHANDLER_H

#include "rlottie.h"
#include <QFileDevice>
#include <QFileInfo>
#include <QImageIOHandler>
#include <QSize>
#include <QImage>
#include <QDebug>
#include <zlib.h>

#define LOG(x) qDebug() << "[TgsIOHandler]" << qPrintable(fileName) << x
#define WARN(x) qWarning() << "[TgsIOHandler]" << qPrintable(fileName) << x

class TgsIOHandler : public QImageIOHandler
{
public:
    static const QByteArray NAME;
    static const QByteArray GZ_MAGIC;
    typedef std::string ByteArray;

    TgsIOHandler(QIODevice* device, const QByteArray& format);
    ~TgsIOHandler();

    ByteArray uncompress();
    bool load();
    void render(int frameIndex);
    void finishRendering();

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

public:
    QString fileName;
    QSize size;
    qreal frameRate;
    int frameCount;
    int currentFrame;
    QImage firstImage;
    QImage prevImage;
    QImage currentImage;
    std::future<rlottie::Surface> currentRender;
    std::unique_ptr<rlottie::Animation> animation;
};

#endif // TGSIOHANDLER_H
