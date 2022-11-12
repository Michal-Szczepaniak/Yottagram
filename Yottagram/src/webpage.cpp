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

#include "webpage.h"

WebPage::WebPage(QObject *parent) : QObject(parent), _photoId(-1)
{

}

void WebPage::setWebpage(td_api::object_ptr<td_api::webPage> webPage)
{
    _webPage = std::move(webPage);

    if (_webPage->photo_ && !_webPage->photo_->sizes_.empty()) {
        _photoId = _webPage->photo_->sizes_.back()->photo_->id_;
        _files->appendFile(move(_webPage->photo_->sizes_.back()->photo_), "photo");
    }

    emit webPageChanged();
}

void WebPage::setFiles(shared_ptr<Files> files)
{
    _files = files;
}

QString WebPage::getUrl()
{
    return QString::fromStdString(_webPage->url_);
}

QString WebPage::getDisplayUrl()
{
    return QString::fromStdString(_webPage->display_url_);
}

QString WebPage::getType()
{
    return QString::fromStdString(_webPage->type_);
}

QString WebPage::getName()
{
    return QString::fromStdString(_webPage->site_name_);
}

QString WebPage::getTitle()
{
    return QString::fromStdString(_webPage->title_);
}

QString WebPage::getDescription()
{
    return QString::fromStdString(_webPage->description_->text_);
}

File* WebPage::getPhoto()
{
    if (_photoId == -1) return nullptr;

    return _files->getFile(_photoId).get();
}
