/*
This file is part of kde-thumbnailer-epub
Copyright (C) 2012-2013-2014 Caig <giacomosrv@gmail.com>

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
#include <QtCore/QDebug>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new EPUBCreator();
    }
}

EPUBCreator::EPUBCreator()
{

}

EPUBCreator::~EPUBCreator()
{

}

bool EPUBCreator::create(const QString &path, int width, int height, QImage &img)
{    
    epub epubFile(path);

    if (!epubFile.open(QIODevice::ReadOnly)) {
        qDebug() << "[epub thumbnailer]" << "Couldn't open or parse" << path;
    } else {
        QString metadataRef = epubFile.parseMetadata();
        QString coverHref = epubFile.parseManifest(metadataRef);

        if (coverHref.isEmpty()) {
            coverHref = epubFile.parseGuide();

            if (coverHref.isEmpty()) {
                QString idRef = epubFile.parseSpine();
                if (idRef.isEmpty()) {
                    coverHref = "cover"; // last chance, will try to pick a file with "cover" in the url
                } else {
                    coverHref = epubFile.parseManifest(idRef);
                    if (coverHref.isEmpty()) {
                        coverHref = "cover"; // last chance
                    }
                }
            }
        }
        
        qDebug() << "[epub thumbnailer]" << "Searching for cover url...";
        QString coverUrl = epubFile.getCoverUrl(coverHref);

        if (!coverUrl.isEmpty()) {
            qDebug() << "[epub thumbnailer]" << "Cover url:" << coverUrl;

            QImage coverImage;
            if (epubFile.getCoverImage(coverUrl, coverImage)) {
                img = coverImage.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                qDebug() << "[epub thumbnailer]" << "Done!";
            }
        }
    }

    epubFile.close();
    
    return !img.isNull();
}

ThumbCreator::Flags EPUBCreator::flags() const
{
    return None;
}