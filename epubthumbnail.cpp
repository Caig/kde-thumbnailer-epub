/*
This file is part of kde-thumbnailer-epub
Copyright (C) 2012-2017 Giacomo Barazzetti <giacomosrv@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "epubthumbnail.h"
#include "epub.h"

#include <QtGui/QImage>

extern "C"
{
    Q_DECL_EXPORT ThumbCreator *new_creator()
    {
        return new EPUBCreator();
    }
}

bool EPUBCreator::create(const QString &path, int /*width*/, int /*height*/, QImage &img)
{    
    Epub epubFile(path);

    if (epubFile.open()) {
        epubFile.getCoverImage(img);
    }
    
    return !img.isNull();
}

ThumbCreator::Flags EPUBCreator::flags() const
{
    return None;
}
