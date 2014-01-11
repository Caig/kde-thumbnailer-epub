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

#ifndef EPUB_H
#define EPUB_H

#include <QtGui/QImage>
#include <QtCore/QXmlStreamReader>
#include <kzip.h>

class epub : public KZip
{
public:
    epub(const QString &path);
    bool open(QIODevice::OpenMode mode);

    QString parseMetadata();
    QString parseGuide();
    QString parseSpine();
    QString parseManifest(const QString &coverId);

    QString getCoverUrl(const QString &href);
    bool getCoverImage(const QString &fileName, QImage &coverImage);

private:
    QStringList mItemsList;
    QScopedPointer<QIODevice> mContainer;
    QXmlStreamReader mQXml;
    QString mDeviceUrl;

    QString mOpfUrl;

    void getItemsList(const KArchiveDirectory *dir, QString path);
    bool getOpfUrl();
    QString getFileUrl(const QString &href);
    bool getFile(const QString &fileName);
    void getXml(const QString &fileName);

    QString parseCoverPage(const QString &coverUrl);
};

#endif // EPUB_H
