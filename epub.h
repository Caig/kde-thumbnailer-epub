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

#ifndef EPUB_H
#define EPUB_H

#include <kzip.h>

class epub : public KZip
{
public:
    epub(const QString &path);
    bool open(QIODevice::OpenMode mode);

    QString parseMetadata();
    QString parseManifest(QString &coverId);
    //QString parseGuide();

    QString getFileUrl(const QString &href);
    QIODevice &getCover(const QString &fileName);

private:
    QSharedPointer<QIODevice> mContainer;
    QStringList mItemsList;
    QString mOpfUrl;

    void getItemsList(const KArchiveDirectory *dir, QString path);
    bool getOpfUrl();
    void getFile(const QString &fileName);
};

#endif // EPUB_H
