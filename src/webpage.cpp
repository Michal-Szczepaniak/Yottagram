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

WebPage::WebPage(QObject *parent) : QObject(parent)
{

}

void WebPage::setWebpage(td_api::object_ptr<td_api::webPage> webPage)
{
    _webPage = std::move(webPage);

    emit webPageChanged();
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
