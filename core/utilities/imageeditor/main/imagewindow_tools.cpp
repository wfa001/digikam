/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - extra tools
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slideShow(SlideShowSettings& settings)
{
    m_cancelSlideShow   = false;
    settings.exifRotate = MetaEngineSettings::instance()->settings().exifRotate;

    if (!d->imageInfoModel->isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        float cnt = (float)d->imageInfoModel->rowCount();
        int     i = 0;

        foreach (const ItemInfo& info, d->imageInfoModel->imageInfos())
        {
            SlidePictureInfo pictInfo;
            pictInfo.comment    = info.comment();
            pictInfo.rating     = info.rating();
            pictInfo.colorLabel = info.colorLabel();
            pictInfo.pickLabel  = info.pickLabel();
            pictInfo.photoInfo  = info.photoInfoContainer();
            settings.pictInfoMap.insert(info.fileUrl(), pictInfo);
            settings.fileList << info.fileUrl();

            m_nameLabel->setProgressValue((int)((i++ / cnt) * 100.0));
            qApp->processEvents();
        }
    }

/*
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        m_nameLabel->setProgressBarMode(StatusProgressBar::CancelProgressBarMode,
                                     i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->urlList.count();
        DMetadata meta;
        settings.fileList   = d->urlList;

        for (QList<QUrl>::Iterator it = d->urlList.begin() ;
             !m_cancelSlideShow && (it != d->urlList.end()) ; ++it)
        {
            SlidePictureInfo pictInfo;
            meta.load((*it).toLocalFile());
            pictInfo.comment   = meta.getItemComments()[QString("x-default")].caption;
            pictInfo.photoInfo = meta.getPhotographInformation();
            settings.pictInfoMap.insert(*it, pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            qApp->processEvents();
        }
    }
*/

    m_nameLabel->setProgressBarMode(StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        QPointer<Digikam::SlideShow> slide = new SlideShow(settings);
        TagsActionMngr::defaultManager()->registerActionsToWidget(slide);

        if (settings.startWithCurrent)
        {
            slide->setCurrentItem(d->currentUrl());
        }

        connect(slide, SIGNAL(signalRatingChanged(QUrl,int)),
                this, SLOT(slotRatingChanged(QUrl,int)));

        connect(slide, SIGNAL(signalColorLabelChanged(QUrl,int)),
                this, SLOT(slotColorLabelChanged(QUrl,int)));

        connect(slide, SIGNAL(signalPickLabelChanged(QUrl,int)),
                this, SLOT(slotPickLabelChanged(QUrl,int)));

        connect(slide, SIGNAL(signalToggleTag(QUrl,int)),
                this, SLOT(slotToggleTag(QUrl,int)));

        slide->show();
    }
}

} // namespace Digikam
