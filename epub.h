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

#ifndef EPUB_H
#define EPUB_H

#include <kzip.h>
#include <QtXmlPatterns/QXmlQuery>

class QImage;

class Epub
{
public:
    explicit Epub(const QString &path);
    ~Epub();
    bool open();
    bool getCoverImage(QImage &coverImage);

private:
    KZip mZip;
    
    QStringList mFilesList;
    QString mOpfUrl;
    QString mOpf;
    QXmlQuery mOpfQuery;

    void loadFilesList(const KArchiveDirectory *dir, const QString &subPath = "");
    bool loadOpf();
    
    // searching strategies
    QString getRefFromMetadata();
    QString getRefFromGuide();
    QString getRefFromSpine(); // search in the first ~xhtm according to spine, it could contain the cover image
    QString getRefByName() const; // search for cover by filename
    
    QString getRefById(const QString &coverId);
    QString getAttrFromOpf(const QString &query, const QString &namesp = "");
    QString getRefFromXhtm(const QString &xhtmUrl) const; // search for the first image in a xhtm
    
    bool isImage(const QString &ref) const;
    QString getAbsoluteUrl(const QString &url) const;
    QByteArray getFile(const QString &fileName) const;
};

#endif // EPUB_H
