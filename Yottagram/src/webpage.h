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

#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QObject>
#include <QDebug>
#include "core/telegrammanager.h"
#include "files/files.h"

class WebPage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString url READ getUrl NOTIFY webPageChanged)
    Q_PROPERTY(QString displayUrl READ getDisplayUrl NOTIFY webPageChanged)
    Q_PROPERTY(QString type READ getType NOTIFY webPageChanged)
    Q_PROPERTY(QString name READ getName NOTIFY webPageChanged)
    Q_PROPERTY(QString title READ getTitle NOTIFY webPageChanged)
    Q_PROPERTY(QString description READ getDescription NOTIFY webPageChanged)
    Q_PROPERTY(File* photo READ getPhoto NOTIFY webPageChanged)
public:
    explicit WebPage(QObject *parent = nullptr);
    void setWebpage(td_api::object_ptr<td_api::webPage> webPage);
    void setFiles(shared_ptr<Files> files);

    QString getUrl();
    QString getDisplayUrl();
    QString getType();
    QString getName();
    QString getTitle();
    QString getDescription();
    File* getPhoto();

signals:
    void webPageChanged();

private:
    int32_t _photoId;
    td_api::object_ptr<td_api::webPage> _webPage;
    std::shared_ptr<Files> _files;
};
Q_DECLARE_METATYPE(WebPage*)

#endif // WEBPAGE_H
