/*
This file is part of kde-thumbnailer-epub
Copyright (C) 2012 Caig <giacomosrv@gmail.com>

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

//#include <QDebug>

extern "C"
{
    KDE_EXPORT ThumbCreator *new_creator()
    {
        return new EPUBCreator();
    }
}

EPUBCreator::EPUBCreator()
{
    mQXml = new QXmlStreamReader;
}

EPUBCreator::~EPUBCreator()
{
    delete mQXml;
    mQXml = NULL;
    
    epub_cleanup();
}

bool EPUBCreator::create( const QString& path, int width, int height, QImage& img )
{
    mEpub = epub_open(qPrintable(path), 0); // 3 for complete informations
    if (!mEpub)
        return false;
    
    QImage image;
    
    if (coverFromGuide())
    {
        image = searchCoverName();
        //qDebug() << "...working...";
        if (!image.isNull())
        {
            img = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            //qDebug() << "...success!";
        }
    }
    else if (coverFromFirstFile())
    {
        image = searchCoverName();
        //qDebug() << "...working...";
        if (!image.isNull())
        {
            img = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            //qDebug() << "...success!";
        }
    }

    epub_close(mEpub);
    
    return !img.isNull();
}

bool EPUBCreator::coverFromGuide()
{
    bool result = false;

    //qDebug() << "Trying coverFromGuide...";

    mTiterator = epub_get_titerator(mEpub, TITERATOR_GUIDE, 0);

    while (epub_tit_curr_valid(mTiterator))
    {
        //qDebug() << "*" << epub_tit_get_curr_label(mTiterator);
        if (QString(epub_tit_get_curr_label(mTiterator)).toLower() == "cover") //use toLower to avoid problems with some epubs
        {
            //qDebug() << "**" << epub_tit_get_curr_link(mTiterator);
            char *data;
            epub_get_data(mEpub, epub_tit_get_curr_link(mTiterator), &data);
            if (data)
            {
                //qDebug() << "Open" << epub_tit_get_curr_link(mTiterator);
                //qDebug() << data;
                mCoverPage = QString(data);
                result = true;
                break; //found the cover guide
            }
        }

        epub_tit_next(mTiterator); //check the next entry
    }

    epub_free_titerator(mTiterator);
    return result;
}

bool EPUBCreator::coverFromFirstFile()
{
    bool result = false;

    //qDebug() << "Trying coverFromFirstFile...";
    
    mEiterator = epub_get_iterator(mEpub, EITERATOR_SPINE, 0);
    if (epub_it_get_curr(mEiterator))
    {
        mCoverPage = QString(epub_it_get_curr(mEiterator));
        //qDebug() << "Open" << epub_it_get_curr_url(mEiterator);
        //qDebug() << epub_it_get_curr(mEiterator);
        result = true;
    }

    epub_free_iterator(mEiterator);
    return result;
}

QImage EPUBCreator::searchCoverName()
{
    mQXml->addData(mCoverPage);
    
    QImage image;
    
    while (!mQXml->atEnd())
    {
        mQXml->readNextStartElement();
        //qDebug() << "mQXml";
        //qDebug() << "*" << mQXml->name();
        if (mQXml->name() == "img")
        {
            char *data;
            mCoverName = mQXml->attributes().value("src").toString();
            checkCoverName();
            int size = epub_get_data(mEpub, qPrintable(mCoverName), &data); //try to load the cover
            //qDebug() << "img";
            if (data)
            {
                //qDebug() << "data1";
                image.loadFromData((unsigned char *)data, size);
                break;
            }
        }
        else if (mQXml->name() == "image")
        {
            char *data;
            mCoverName = mQXml->attributes().value("xlink:href").toString();
            checkCoverName();
            int size = epub_get_data(mEpub, qPrintable(mCoverName), &data); //try to load the cover
            //qDebug() << "image";
            if (data)
            {
                //qDebug() << "data2";
                image.loadFromData((unsigned char *)data, size);
                break;
            }
        }
    }

    mQXml->clear();
    return image;
}

void EPUBCreator::checkCoverName()
{
    if (mCoverName.left(3) == "../")
        mCoverName.remove(0, 3);
}

ThumbCreator::Flags EPUBCreator::flags() const
{
    return None;
}