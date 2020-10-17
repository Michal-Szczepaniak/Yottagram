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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QSize>
#include "contentfile.h"

class Animation : public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(bool isSecret READ isSecret NOTIFY animationChanged)
    Q_PROPERTY(QSize size READ getSize NOTIFY animationChanged)
    Q_PROPERTY(QString name READ getName NOTIFY animationChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY animationChanged)
    Q_PROPERTY(QByteArray thumbnail READ getThumbnail NOTIFY animationChanged)
    Q_PROPERTY(File* animation READ getAnimation NOTIFY animationChanged)
public:
    explicit Animation(QObject *parent = nullptr);

    void setAnimation(td_api::object_ptr<td_api::messageAnimation> messageAnimation);
    File* getAnimation() const;
    void addUpdateFiles();

    bool isSecret() const;
    td_api::formattedText* getCaption();
    QSize getSize() const;
    QString getName() const;
    QString getMimeType() const;
    QByteArray getThumbnail() const;

signals:
    void animationChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageAnimation> _animation;
    qint32 _animationFileId;
};

#endif // ANIMATION_H
