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

#include "epub.h"

#include <QtCore/QXmlStreamReader>
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
        for (int i = 0; i < mItemsList.count(); i++)
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
        getFile(mOpfUrl); // ready to be parsed
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

    // parse container.xml to get a reference to the opf file
    QString containerXmlUrl = getFileUrl("META-INF/container.xml"); // it must exist according to format specification, but better to be sure...
    if (containerXmlUrl != "") {
        getFile(containerXmlUrl);

        QXmlStreamReader qxml(mContainer.data());

        while(!qxml.atEnd())
        {
            qxml.readNext();

            if (qxml.name() == "rootfile" && qxml.isStartElement()) {
                QXmlStreamAttributes qxmlAttributes = qxml.attributes();

                if (qxmlAttributes.hasAttribute("full-path")) {
                    value = qxmlAttributes.value("full-path").toString();
                    break;
                }
            }
        }
    }

    if (value == "") {
        qDebug() << "[epub thumbnailer]" << "No or wrong container.xml, trying to find opf file manually...";

        for (int i = 0; i < mItemsList.count(); ++i)
        {
            if (mItemsList.at(i).endsWith(".opf", Qt::CaseInsensitive)) {
                value = mItemsList.at(i);
                qDebug() << "[epub thumbnailer]" << "Opf manually found.";
                break;
            }
        }
    }

    mOpfUrl = value;

    return !mOpfUrl.isEmpty();
}

QString epub::parseMetadata()
{
    qDebug() << "[epub thumbnailer]" << "Searching cover reference in metadata...";

    QXmlStreamReader qxml(mContainer.data());

    QString value = "";

    while(!qxml.atEnd())
    {
        qxml.readNext();

        if (qxml.name() == "metadata" && qxml.isEndElement()) {
            break;
        }

        if (qxml.name() == "meta" && qxml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = qxml.attributes();

            if (qxmlAttributes.hasAttribute("name") && qxmlAttributes.hasAttribute("content")) {
                if (qxmlAttributes.value("name") == "cover") {
                    value = qxmlAttributes.value("content").toString();
                    break;
                }
            }
        }
    }

    if (value == "") {
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
    if (tCoverId == "") {
        tCoverId = "cover";
        exactMatch = false;
    }

    mContainer->seek(0); //ensure the file is parsed from beginning
    QXmlStreamReader qxml(mContainer.data());

    QString value = "";

    while(!qxml.atEnd())
    {
        qxml.readNext();

        if (qxml.name() == "manifest" && qxml.isEndElement()) {
            break;
        }

        if (qxml.name() == "item" && qxml.isStartElement()) {
            QXmlStreamAttributes qxmlAttributes = qxml.attributes();

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

    if (value == "") {
        value = "cover"; //last chance, try to pick a file with "cover" in the filename
        qDebug() << "[epub thumbnailer]" << "No cover href found.";
    } else {
        qDebug() << "[epub thumbnailer]" << "Cover href:" << value;
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

    for (int i = 0; i < mItemsList.count(); ++i)
    {
        if (mItemsList.at(i).contains(tHref)) {
            value = mItemsList.at(i);
            break;
        }
    }

    return value;
}

QString epub::getCoverUrl(const QString &href)
{
    QString value = getFileUrl(href);

    if (value != "") {
        if (endsWith(value, QStringList() << "jpg" << "jpeg" << "png" << "gif" << "bmp")) {
            return value;
        } else if (endsWith(value, QStringList() << "xhtml" << "xhtm" << "html" << "htm" << "xml")) {
            QString tCoverUrl = parseCoverPage(value);
            if (tCoverUrl != "") {
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

void epub::getFile(const QString &fileName)
{
    const KArchiveDirectory *dir = this->directory();
    const KZipFileEntry *file = static_cast<const KZipFileEntry*>(dir->entry(fileName));
    mContainer = QSharedPointer<QIODevice>(file->createDevice());
    // delete dir and file...
}

bool epub::getCoverImage(const QString &fileName, QImage &coverImage)
{
    getFile(fileName);

    QImage tCoverImage;
    if (tCoverImage.loadFromData(mContainer.data()->readAll())) {
        coverImage = tCoverImage;
        return true;
    }

    return false;
}

// parse an xhtm(in an ideal world with just validated epub files) to search for the first image reference
QString epub::parseCoverPage(const QString &coverUrl)
{
    getFile(coverUrl);

    QXmlStreamReader qxml(mContainer.data());

    QString tCoverUrl = "";

    while (!qxml.atEnd())
    {
        qxml.readNextStartElement();

        if (qxml.name() == "img") {
            tCoverUrl = qxml.attributes().value("src").toString();
            break;
        } else if (qxml.name() == "image") {
            tCoverUrl = qxml.attributes().value("xlink:href").toString();
            break;
        }
    }

    return tCoverUrl;
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
