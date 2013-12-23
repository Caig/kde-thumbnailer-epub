/*
This file is part of kde-thumbnailer-epub
Copyright (C) 2012-2013 Caig <giacomosrv@gmail.com>

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

bool endsWith (const QString &coverUrl, const QStringList &extensions);

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
    }
    else {
        QString metadataRef = epubFile.parseMetadata();

        QString coverHref = epubFile.parseManifest(metadataRef);

        qDebug() << "[epub thumbnailer]" << "Searching for cover url...";
        QString coverUrl = epubFile.getFileUrl(coverHref);

        if (coverUrl != "") {
            qDebug() << "[epub thumbnailer]" << "Cover url:" << coverUrl;

            if (endsWith(coverUrl, QStringList() << "jpg" << "jpeg" << "png" << "gif" << "bmp")) {
                QImage coverImage;
                if (epubFile.getCoverImage(coverUrl, coverImage)) {
                    img = coverImage.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    qDebug() << "[epub thumbnailer]" << "Done!";
                }
            } else if (endsWith(coverUrl, QStringList() << "xhtml" << "xhtm" << "html" << "htm" << "xml")) {
                qDebug() << "[epub thumbnailer]" << "Not implemented.";
            }
        } else {
            qDebug() << "[epub thumbnailer]" << "Has it a cover?";
        }
    }

    epubFile.close();
    
    return !img.isNull();
}

ThumbCreator::Flags EPUBCreator::flags() const
{
    return None;
}

bool endsWith (const QString &coverUrl, const QStringList &extensions)
{
    bool returnValue = false;

    for (int i = 0; i < extensions.count(); ++i)
    {
        if (coverUrl.endsWith("." + extensions.at(i), Qt::CaseInsensitive)) {
            returnValue = true;
        }
    }

    return returnValue;
}