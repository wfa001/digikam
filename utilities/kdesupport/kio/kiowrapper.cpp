/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-10
 * Description : A wrapper to isolate KIO Jobs calls
 *
 * Copyright (C) 2015      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2015-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kiowrapper.h"

// Qt includes

#include <QPair>
#include <QPointer>
#include <QMimeDatabase>

// KDE includes

#include <kio/job.h>
#include <kio/copyjob.h>
#include <kio/mkdirjob.h>
#include <kio/deletejob.h>
#include <krun.h>
#include <kio_version.h>
#include <kjobwidgets.h>

namespace Digikam
{

KIOWrapper::KIOWrapper()
{
}

bool KIOWrapper::fileCopy(const QUrl& src, const QUrl& dest, bool withKJobWidget, QWidget* const widget)
{
    KIO::FileCopyJob* const fileCopyJob = KIO::file_copy(src, dest, KIO::Overwrite);

    if (withKJobWidget)
    {
        KJobWidgets::setWindow(fileCopyJob, widget);
    }

    return fileCopyJob->exec();
}

void KIOWrapper::move(const QUrl& src, const QUrl& dest)
{
    KIO::Job* const job = KIO::move(src, dest);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotKioJobResult(KJob*)));
}

void KIOWrapper::del(const QUrl& url)
{
    KIO::Job* const job = KIO::del(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotKioJobResult(KJob*)));
}

void KIOWrapper::trash(const QUrl& url)
{
    KIO::Job* const job = KIO::trash(url);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotKioJobResult(KJob*)) );
}

void KIOWrapper::slotKioJobResult(KJob* job)
{
    if (job->error() != 0)
    {
        emit signalError(job->errorString());
    }
    else
    {
        emit signalError(QString());
    }
}

bool KIOWrapper::run(const KService& service, const QList<QUrl>& urls, QWidget* const window)
{
#if KIO_VERSION < QT_VERSION_CHECK(5,6,0)
    return KRun::run(service, urls, window);
#else
    return KRun::runService(service, urls, window);
#endif
}

bool KIOWrapper::run(const QString& exec, const QList<QUrl>& urls, QWidget* const window)
{
    return KRun::run(exec, urls, window);
}

bool KIOWrapper::run(const QUrl& url, QWidget* const window)
{
    return KRun::runUrl(url, QMimeDatabase().mimeTypeForUrl(url).name(), window);
}

} // namespace Digikam
