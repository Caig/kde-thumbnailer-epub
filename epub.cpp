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

#include "epub.h"

#include <QtCore/QDebug>

bool endsWith (const QString &coverUrl, const QStringList &extensions);

epub::epub(const QString &path) : KZip(path)
{
    qDebug() << "[epub thumbnailer]" << "Opening" << path;
}

bool epub::open(QIODevice::OpenMode mode)
{
    bool returnValue = KZip::open(mode);

    qDebug() << "[epub thumbnailer]" << "Retrieving files list...";

    getItemsList(this->directory(), "");

    if (mItemsList.count() != 0) {
        /*
        for (int i = 0; i < mItemsList.count(); ++i)
        {
            qDebug() << "[epub thumbnailer]" << mItemsList.at(i);
        }
        */
        qDebug() << "[epub thumbnailer]" << "Found" << mItemsList.count() << "files.";
    } else {
        qDebug() << "[epub thumbnailer]" << "No files found.";
        return false;
    }

    if (getOpfUrl()) {
        qDebug() << "[epub thumbnailer]" << "Opf:" << mOpfUrl;
    } else {
        qDebug() << "[epub thumbnailer]" << "No OPF file found.";
        return false;
    }

    return returnValue;
}

void epub::getItemsList(const KArchiveDirectory *dir, QString path)
{
    QStringList tempList = dir->entries();

    for (int i = 0; i < tempList.count(); ++i)
    {
        const KArchiveEntry *entry = dir->entry(tempList.at(i));

        if (entry->isFile()) {
            mItemsList.append(path + "/" + tempList.at(i));
        } else {
            getItemsList(static_cast<const KArchiveDirectory*>(entry), path + "/" + tempList.at(i));
        }
    }
}

bool epub::getOpfUrl()
{
    qDebug() << "[epub thumbnailer]" << "Searching for opf url...";

    QString value = "";

    QString containerXmlUrl = getFileUrl("META-INF/container.xml"); // it must exist according to format specification, but better to be sure...
    if (!containerXmlUrl.isEmpty()) {
        if (mDeviceUrl != containerXmlUrl) {
            getXml(containerXmlUrl);
        }

        while(!mQXml.atEnd())
        {
            mQXml.readNext();

            if (mQXml.name() == "rootfile" && mQXml.isStartElement()) {
                QXmlStreamAttributes qxmlAttributes = mQXml.attributes();

                if (qxmlAttributes.hasAttribute("full-path")) {
                    value = qxmlAttributes.value("full-path").toString();
                    break;
                }
            }
        }
    }

    if (value.isEmpty()) {
        qDebug() << "[epub thumbnailer]" << "No or wrong container.xml, trying to find opf file manually...";

        int i = 0;
        while (i < mItemsList.count())
        {
            if (mItemsList.at(i).endsWith(".opf", Qt::CaseInsensitive)) {
                value = mItemsList.at(i);
                qDebug() << "[epub thumbnailer]" << "Opf manually found.";
                break;
            }
            ++i;
        }
    }

    mOpfUrl = value;

    return !mOpfUrl.isEmpty();
}

QString epub::parseMetadata()
{
    qDebug() << "[epub thumbnailer]" << "Searching cover reference in metadata...";

    getXml(mOpfUrl);

    QString value = "";

    while(!mQXml.atEnd())
    {
        mQXml.readNext();

        if (mQXml.name() == "metadata" && mQXml.isEndElement()) {
            break;
        }

        if (mQXml.name() == "meta" && mQXml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = mQXml.attributes();

            if (qxmlAttributes.hasAttribute("name") && qxmlAttributes.hasAttribute("content")) {
                if (qxmlAttributes.value("name") == "cover") {
                    value = qxmlAttributes.value("content").toString();
                    break;
                }
            }
        }
    }

    if (value.isEmpty()) {
        qDebug() << "[epub thumbnailer]" << "No cover reference found.";
    } else {
        qDebug() << "[epub thumbnailer]" << "Metadata reference:" << value;
    }

    return value;
}

QString epub::parseManifest(const QString &coverId)
{
    qDebug() << "[epub thumbnailer]" << "Searching for cover href in manifest...";

    bool exactMatch = true;
    QString tCoverId = coverId;
    if (tCoverId.isEmpty()) {
        tCoverId = "cover";
        exactMatch = false;
    }

    getXml(mOpfUrl);

    QString value = "";

    while(!mQXml.atEnd())
    {
        mQXml.readNext();

        if (mQXml.name() == "manifest" && mQXml.isEndElement()) {
            break;
        }

        if (mQXml.name() == "item" && mQXml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = mQXml.attributes();

            if (qxmlAttributes.hasAttribute("id") && qxmlAttributes.hasAttribute("href")) {
                if (exactMatch == true) {
                    if (qxmlAttributes.value("id").toString().toLower() == tCoverId.toLower()) { //toLower as a workaround for some stupid epubs
                        value = qxmlAttributes.value("href").toString();
                        break;
                    }
                } else {
                    if (qxmlAttributes.value("id").contains(tCoverId, Qt::CaseInsensitive) && qxmlAttributes.value("media-type").contains("image")) {
                        value = qxmlAttributes.value("href").toString();
                        break;
                    }
                }
            }
        }
    }

    if (value.isEmpty()) {
        //value = "cover"; //last chance, try to pick a file with "cover" in the filename
        qDebug() << "[epub thumbnailer]" << "No cover href found.";
    } else {
        qDebug() << "[epub thumbnailer]" << "Cover href:" << value;
    }

    return value;
}

// seems useful on very rare cases
QString epub::parseGuide()
{
    qDebug() << "[epub thumbnailer]" << "Searching cover reference in guide...";

    getXml(mOpfUrl);

    QString value = "";

    while(!mQXml.atEnd())
    {
        mQXml.readNext();

        if (mQXml.name() == "guide" && mQXml.isEndElement()) {
            break;
        }

        if (mQXml.name() == "reference" && mQXml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = mQXml.attributes();

            if (qxmlAttributes.hasAttribute("type") && qxmlAttributes.hasAttribute("title") && qxmlAttributes.hasAttribute("href")) {
                if (qxmlAttributes.value("type") == "cover" || qxmlAttributes.value("title") == "cover") {
                    value = qxmlAttributes.value("href").toString();
                    break;
                }
            }
        }
    }

    if (value.isEmpty()) {
        qDebug() << "[epub thumbnailer]" << "No cover reference found.";
    } else {
        qDebug() << "[epub thumbnailer]" << "Guide reference:" << value;
    }

    return value;
}

// pick the first xhtm file according to the spine section, it could contain the cover image
// seems useful on very rare cases
QString epub::parseSpine()
{
    qDebug() << "[epub thumbnailer]" << "Searching first file in spine...";

    getXml(mOpfUrl);

    QString value = "";

    while(!mQXml.atEnd())
    {
        mQXml.readNext();

        if (mQXml.name() == "spine" && mQXml.isEndElement()) {
            break;
        }

        if (mQXml.name() == "itemref" && mQXml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = mQXml.attributes();

            if (qxmlAttributes.hasAttribute("idref")) {
                value = qxmlAttributes.value("idref").toString();
                break;
            }
        }
    }

    if (value.isEmpty()) {
        qDebug() << "[epub thumbnailer]" << "No first file found.";
    } else {
        qDebug() << "[epub thumbnailer]" << "First file:" << value;
    }

    return value;
}

//to ensure the ref is an existing file path
QString epub::getFileUrl(const QString &href)
{
    QString value = "";
    QString tHref = href;

    //sometimes parseCoverPage finds relative path, fixes it before to use it
    if (href.startsWith("../")) {
        tHref = href.mid(3);
    }

    int i = 0;
    while (i < mItemsList.count())
    {
        if (mItemsList.at(i).contains(tHref, Qt::CaseInsensitive)) {
            value = mItemsList.at(i);
            break;
        }
        ++i;
    }

    return value;
}

QString epub::getCoverUrl(const QString &href)
{
    QString value = getFileUrl(href);

    if (!value.isEmpty()) {
        if (endsWith(value, QStringList() << "jpg" << "jpeg" << "png" << "gif" << "bmp")) {
            return value;
        } else if (endsWith(value, QStringList() << "xhtml" << "xhtm" << "html" << "htm" << "xml")) {
            QString tCoverUrl = parseCoverPage(value);
            if (!tCoverUrl.isEmpty()) {
                value = getFileUrl(tCoverUrl);
            } else {
                qDebug() << "[epub thumbnailer]" << "No image found in the cover page.";
            }
        }
    } else {
        qDebug() << "[epub thumbnailer]" << "Has it a cover?";
    }

    return value;
}

bool epub::getFile(const QString &fileName)
{
    if (mDeviceUrl != fileName) {
        const KArchiveDirectory *dir = this->directory();
        const KZipFileEntry *file = static_cast<const KZipFileEntry*>(dir->entry(fileName));
        mContainer.reset(file->createDevice());
        mDeviceUrl = fileName;

        return true;
    }

    return false;
}

void epub::getXml(const QString &fileName)
{
    if (!getFile(fileName)) {
        mContainer.data()->reset(); // to ensure the file is parsed from beginning
    }

    mQXml.setDevice(mContainer.data()); // it's a bit strange to re-set the device in case the QIODevice isn't changed, but seems needed with some (a bit malformed) opf files

    /*
    if (getFile(fileName)) {
        mQXml.setDevice(mContainer.data());
    } else {
        mContainer.data()->reset(); // ensure the file is parsed from beginning
    }
    */
}

bool epub::getCoverImage(const QString &fileName, QImage &coverImage)
{
    if (getFile(fileName)) {
        QImage tCoverImage;
        if (tCoverImage.loadFromData(mContainer.data()->readAll())) {
            coverImage = tCoverImage;

            return true;
        }
    }

    return false;
}

// parse an xhtm(in an ideal world with just validated epub files) to search for the first image reference
QString epub::parseCoverPage(const QString &coverUrl)
{
    getXml(coverUrl);

    QString tCoverUrl = "";

    while (!mQXml.atEnd())
    {
        mQXml.readNextStartElement();

        if (mQXml.name().toString().toLower() == "img") { //toLower as a workaround for some xhtm
            tCoverUrl = mQXml.attributes().value("src").toString();
            break;
        } else if (mQXml.name().toString().toLower() == "image") {
            tCoverUrl = mQXml.attributes().value("xlink:href").toString();
            break;
        }
    }

    return tCoverUrl;
}

bool endsWith (const QString &coverUrl, const QStringList &extensions)
{
    bool returnValue = false;

    int i = 0;
    while (i < extensions.count())
    {
        if (coverUrl.endsWith("." + extensions.at(i), Qt::CaseInsensitive)) {
            returnValue = true;
            break;
        }
        ++i;
    }

    return returnValue;
}
