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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include "contentfile.h"

class Document : public ContentFile
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY documentChanged)
    Q_PROPERTY(QString mimeType READ getMimeType NOTIFY documentChanged)
    Q_PROPERTY(File* document READ getDocument NOTIFY documentChanged)
public:
    explicit Document(QObject *parent = nullptr);

    void setDocument(td_api::object_ptr<td_api::messageDocument> messageDocument);
    File* getDocument() const;
    void addUpdateFiles();

    td_api::formattedText* getCaption();
    QString getName() const;
    QString getMimeType() const;

signals:
    void documentChanged();

public slots:

private:
    td_api::object_ptr<td_api::messageDocument> _document;
    qint32 _documentFileId;
};

#endif // DOCUMENT_H
