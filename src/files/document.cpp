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

#include "document.h"

Document::Document(QObject *parent) : ContentFile(parent)
{

}

void Document::setDocument(td_api::object_ptr<td_api::messageDocument> messageDocument)
{
    _document = std::move(messageDocument);

    addUpdateFiles();

    emit documentChanged();
}

File *Document::getDocument() const
{
    return _files->getFile(_documentFileId).get();
}

void Document::addUpdateFiles()
{
    _documentFileId = _document->document_->document_->id_;
    _files->appendFile(std::move(_document->document_->document_), "other");
}

td_api::formattedText* Document::getCaption()
{
    return _document->caption_.get();
}

QString Document::getName() const
{
    return QString::fromStdString(_document->document_->file_name_);
}

QString Document::getMimeType() const
{
    return QString::fromStdString(_document->document_->mime_type_);
}
