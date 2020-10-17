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

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QQuickPaintedItem>
#include <QPainter>

class Thumbnail : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QByteArray image READ getImage WRITE setImage NOTIFY imageChanged)
public:
    Thumbnail(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QByteArray getImage() const;
    void setImage(QByteArray image);

signals:
    void imageChanged(QString image);

public slots:

private:
    QImage _thumbnail;
    QByteArray _image;
};

#endif // THUMBNAIL_H
