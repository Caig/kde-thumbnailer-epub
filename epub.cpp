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

#include "epub.h"

#include <algorithm>
#include <functional>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtGui/QImage>

const QStringList imageExt({"jpg", "jpeg", "png", "gif", "bmp"});
const QStringList xmlExt({"xhtml", "xhtm", "html", "htm", "xml"});
bool endsWith (const QString &coverUrl, const QStringList &extensions);

Epub::Epub(const QString &path)
    : mZip(path)
{
    qDebug() << "[epub thumb] > Opening" << path;
}

Epub::~Epub()
{
    mZip.close();
}

bool Epub::open()
{    
    if (mZip.open(QIODevice::ReadOnly)) {
        qDebug() << "[epub thumb] Retrieving files list...";
        loadFilesList(mZip.directory());
        mFilesList.sort(Qt::CaseSensitive);

        if (mFilesList.isEmpty()) {
            qDebug() << "[epub thumb] > No files found.";
            return false;
        }

        if (!loadOpf()) {
            qDebug() << "[epub thumb] > No OPF file found.";
            return false;
        }
        
        return true;
    }
    
    qDebug() << "[epub thumb] > Couldn't open";
    return false;
}

bool Epub::getCoverImage(QImage &coverImage)
{
    std::function<QString()> strategies[4] = {
        std::bind(&Epub::getRefFromMetadata, this),
        std::bind(&Epub::getRefFromGuide, this),
        std::bind(&Epub::getRefFromSpine, this),
        std::bind(&Epub::getRefByName, this)
    };
    
    for (auto strategy : strategies) {
        QString ref = strategy();
        
        if (!ref.isEmpty()) {
            bool coverFound = false;
            
            if (isImage(ref)) {
                coverFound = true;
            } else {
                QString tRef = getRefFromXhtm(ref);
                if (!tRef.isEmpty()) {
                    ref = tRef;
                    coverFound = true;
                }
            }
            
            if (coverFound) {
                qDebug() << "[epub thumb] Cover url:" << ref;
                
                if (coverImage.loadFromData(getFile(ref))) {
                    qDebug() << "[epub thumb] > Generated.";
                    return true;
                }
            }
        }
    }

    qDebug() << "[epub thumb] > No cover found.";
    return false;
}

//-------------------------------------

void Epub::loadFilesList(const KArchiveDirectory *dir, const QString &subPath)
{
    for (const QString &fileName : dir->entries()) {
         const KArchiveEntry *entry = dir->entry(fileName);
         const QString tUrl = QDir(subPath).filePath(fileName);
         
         if (entry->isFile()) {
             if (fileName.endsWith("opf")) {
                 mOpfUrl = tUrl;
             } else if (endsWith(fileName, imageExt) || endsWith(fileName, xmlExt)) {
                 mFilesList.append(tUrl);
             }
         } else {
             loadFilesList(static_cast<const KArchiveDirectory*>(entry), tUrl);
         }
    }
}

bool Epub::loadOpf()
{
    if (!mOpfUrl.isEmpty()) {    
        mOpf = QString(getFile(mOpfUrl));
        mOpfQuery.setFocus(mOpf);
    }
    
    return !mOpf.isEmpty();
}

QString Epub::getRefFromMetadata()
{
    qDebug() << "[epub thumb] Searching for id in metadata...";

    const QString id = getAttrFromOpf("/package/metadata/meta[@name='cover']/@content/string()", "http://www.idpf.org/2007/opf");

    if (!id.isEmpty()) {
        qDebug() << "[epub thumb] Id:" << id;
        return getAbsoluteUrl(getRefById(id));
    }
    
    qDebug() << "[epub thumb] Id not found/invalid.";
    return QString();
}

QString Epub::getRefFromGuide()
{
    qDebug() << "[epub thumb] Searching for ref in guide...";
    
    const QString ref = getAttrFromOpf("/package/guide/reference[@type='cover' or @title='cover']/@href/string()", "http://www.idpf.org/2007/opf");

    if (!ref.isEmpty()) {
        qDebug() << "[epub thumb] Ref:" << ref;
        return getAbsoluteUrl(ref);
    }
    
    qDebug() << "[epub thumb] Ref not found/invalid.";
    return QString();
}

QString Epub::getRefFromSpine()
{
    qDebug() << "[epub thumb] Searching for first xhtm in spine...";

    const QString id = getAttrFromOpf("/package/spine/itemref[1]/@idref/string()", "http://www.idpf.org/2007/opf");
    
    if (!id.isEmpty()) {
        qDebug() << "[epub thumb] Id:" << id;
        return getAbsoluteUrl(getRefById(id));
    }
    
    qDebug() << "[epub thumb] Id not found/invalid.";
    return QString();
}

QString Epub::getRefByName() const
{
    qDebug() << "[epub thumb] Searching for cover by generic filename...";
    return getAbsoluteUrl("cover");
}

QString Epub::getRefById(const QString &coverId)
{
    qDebug() << "[epub thumb] Searching for ref in manifest...";
    
    //CHECK: query.bindVariable("myId", QXmlItem(QVariant(coverId)));
    const QString query = "/package/manifest/item[@id='" + coverId + "']/@href/string()";
    const QString ref = getAttrFromOpf(query, "http://www.idpf.org/2007/opf");

    if (ref.isEmpty()) {
        qDebug() << "[epub thumb] No ref found.";
    } else {
        qDebug() << "[epub thumb] Ref:" << ref;
    }

    return ref;
}

QString Epub::getAbsoluteUrl(const QString &url) const
{
    QString fileUrl;
    
    for (const QString &fileName : mFilesList) {
        if (fileName.contains(url, Qt::CaseInsensitive)) {
            fileUrl = fileName;
            break;
        }
    }
    
    return fileUrl;
}

bool Epub::isImage(const QString &ref) const
{
    if (endsWith(ref, imageExt)) {
        return true;
    }
    
    return false;
}

QString Epub::getRefFromXhtm(const QString &xhtmUrl) const
{
    QXmlQuery xHtmlQuery;
    xHtmlQuery.setFocus(getFile(xhtmUrl));
    xHtmlQuery.setQuery("(//*:image[1]/@*:href | //*:img[1]/@src)/string()");
    
    QString ref;
    xHtmlQuery.evaluateTo(&ref);
    ref = ref.remove('\n');
    
    // fixes relative path
    if (ref.startsWith("../")) {
        ref = ref.remove(0, 3);
    }

    return getAbsoluteUrl(ref);
}

QString Epub::getAttrFromOpf(const QString &query, const QString &namesp)
{
    const QString tNamesp("declare default element namespace \"" + namesp + "\";");
    mOpfQuery.setQuery(tNamesp + query);
    
    QString attribute;
    mOpfQuery.evaluateTo(&attribute);
    attribute = attribute.remove('\n');
    
    return attribute;
}

QByteArray Epub::getFile(const QString &fileName) const
{
    const KArchiveDirectory *dir = mZip.directory();
    const KZipFileEntry *file = static_cast<const KZipFileEntry*>(dir->entry(fileName));

    return file->data();
}

//--------------------

bool endsWith (const QString &coverUrl, const QStringList &extensions)
{
    return std::any_of(extensions.begin(), extensions.end(), [&coverUrl](const QString& ext) -> bool {
        return coverUrl.endsWith("." + ext, Qt::CaseInsensitive);
    });
}
