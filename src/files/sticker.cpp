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

#include "sticker.h"
#include <QFile>
#include <QDataStream>
#include <zlib.h>

Sticker::Sticker(QObject *parent) : ContentFile(parent)
{

}

void Sticker::setSticker(td_api::object_ptr<td_api::messageSticker> messageSticker)
{
    _sticker = std::move(messageSticker);

    addUpdateFiles();

    emit stickerChanged();
}

File *Sticker::getSticker() const
{
//                    QString jsonFilename = _files[_file_ids[0]]->localPath().split(".tgs",QString::SkipEmptyParts).at(0) + ".json";
//                    if (false == QFile::exists(jsonFilename)) {
//                        unpackSticker(_files[_file_ids[0]]);
//                    }
//                    return jsonFilename;
    return _files->getFile(_stickerFileId).get();
}

void Sticker::addUpdateFiles()
{
    _stickerFileId = _sticker->sticker_->sticker_->id_;
    _files->appendFile(std::move(_sticker->sticker_->sticker_), "sticker");
}

QSize Sticker::getSize() const
{
    return QSize(_sticker->sticker_->width_, _sticker->sticker_->height_);
}

qint64 Sticker::getSetId() const
{
    return _sticker->sticker_->set_id_;
}

QString Sticker::getEmoji() const
{
    return QString::fromStdString(_sticker->sticker_->emoji_);
}

bool Sticker::isAnimated() const
{
    return _sticker->sticker_->is_animated_;
}

bool Sticker::isMask() const
{
    return _sticker->sticker_->is_mask_;
}

void Sticker::unpackSticker(shared_ptr<File> file)
{
    QFile inFile(file->localPath());
    inFile.open(QIODevice::ReadOnly);
    QByteArray input = inFile.readAll();
    QByteArray output;

    QString outputFilename = file->localPath().split(".tgs",QString::SkipEmptyParts).at(0) + ".json";

    QFile outFile(outputFilename);
    outFile.open(QIODevice::WriteOnly);
    gzipDecompress(input, output);
    outFile.write(output);
}

bool Sticker::gzipDecompress(QByteArray input, QByteArray &output)
{
    // Prepare output
    output.clear();

    if(input.length() > 0)
    {
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;

        int ret = inflateInit2(&strm, GZIP_WINDOWS_BIT);

        if (ret != Z_OK)
            return(false);

        char *input_data = input.data();
        int input_data_left = input.length();

        do {
            int chunk_size = qMin(GZIP_CHUNK_SIZE, input_data_left);

            if(chunk_size <= 0)
                break;

            strm.next_in = (unsigned char*)input_data;
            strm.avail_in = chunk_size;

            input_data += chunk_size;
            input_data_left -= chunk_size;

            do {

                char out[GZIP_CHUNK_SIZE];

                strm.next_out = (unsigned char*)out;
                strm.avail_out = GZIP_CHUNK_SIZE;

                ret = inflate(&strm, Z_NO_FLUSH);

                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                    break;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_STREAM_ERROR:
                    inflateEnd(&strm);

                    return(false);
                }

                int have = (GZIP_CHUNK_SIZE - strm.avail_out);

                if(have > 0)
                    output.append((char*)out, have);

            } while (strm.avail_out == 0);

        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);

        return (ret == Z_STREAM_END);
    }
    else
        return(true);
}
