/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QImage>
#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QPushButton>
#include <QSpinBox>

// KDE include.

#include <khbox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <khuesaturationselect.h>
#include <kcolorvalueselector.h>
#include <khbox.h>
#include <kinputdialog.h>
#include <ktabwidget.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "ddragobjects.h"
#include "imageinfo.h"
#include "haariface.h"
#include "searchxml.h"
#include "searchtextbar.h"
#include "sketchwidget.h"
#include "thumbnailsize.h"
#include "thumbnailloadthread.h"
#include "fuzzysearchfolderview.h"
#include "fuzzysearchview.h"
#include "fuzzysearchview.moc"

namespace Digikam
{

class FuzzySearchViewPriv
{

public:

    enum FuzzySearchTab
    {
        IMAGE=0,
        SKETCH
    };

    FuzzySearchViewPriv()
    {
        sketchWidget          = 0;
        hsSelector            = 0;
        vSelector             = 0;
        penSize               = 0;
        resultsSketch         = 0;
        resultsImage          = 0;
        resetButton           = 0;
        saveBtnSketch         = 0;
        saveBtnImage          = 0;
        nameEditSketch        = 0;
        nameEditImage         = 0;
        searchFuzzyBar        = 0;
        fuzzySearchFolderView = 0;
        tabWidget             = 0;
        imageInfo             = 0;
        thumbLoadThread       = 0;
        imageSAlbum           = 0;
        sketchSAlbum          = 0;
    }

    QPushButton            *resetButton;
    QPushButton            *saveBtnSketch;
    QPushButton            *saveBtnImage;

    QSpinBox               *penSize;
    QSpinBox               *resultsSketch;
    QSpinBox               *resultsImage;

    QLabel                 *imageWidget;

    KLineEdit              *nameEditSketch;
    KLineEdit              *nameEditImage;

    KTabWidget             *tabWidget;

    KHueSaturationSelector *hsSelector;

    KColorValueSelector    *vSelector;

    KSqueezedTextLabel     *labelFile;
    KSqueezedTextLabel     *labelFolder;

    ImageInfo              *imageInfo;

    SearchTextBar          *searchFuzzyBar;

    FuzzySearchFolderView  *fuzzySearchFolderView;

    SketchWidget           *sketchWidget;

    ThumbnailLoadThread    *thumbLoadThread;

    SAlbum                 *imageSAlbum;
    SAlbum                 *sketchSAlbum;
};

FuzzySearchView::FuzzySearchView(QWidget *parent)
               : QWidget(parent)
{
    d = new FuzzySearchViewPriv;
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    QVBoxLayout *vlay    = new QVBoxLayout(this);
    d->tabWidget         = new KTabWidget(this);

    // ---------------------------------------------------------------
    // Image Panel

    QWidget *imagePanel = new QWidget(this);
    QGridLayout *grid   = new QGridLayout(imagePanel);

    QWidget *box2       = new QWidget(imagePanel);
    QVBoxLayout *vlay3  = new QVBoxLayout(box2);
    KHBox *imageBox     = new KHBox(box2);
    d->imageWidget      = new QLabel(imageBox);
    d->imageWidget->setFixedSize(256, 256);
    d->imageWidget->setText(i18n("Drag & drop an image here\nto perform similar\nitems to search"));
    d->imageWidget->setAlignment(Qt::AlignCenter);
    imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    imageBox->setLineWidth(1);

    vlay3->addStretch(10);
    vlay3->addWidget(imageBox, 0, Qt::AlignCenter);
    vlay3->addStretch(10);
    vlay3->setMargin(0);
    vlay3->setSpacing(0);

    // ---------------------------------------------------------------

    QLabel *file   = new QLabel(i18n("<b>File</b>:"), imagePanel);
    d->labelFile   = new KSqueezedTextLabel(0, imagePanel);
    QLabel *folder = new QLabel(i18n("<b>Folder</b>:"), imagePanel);
    d->labelFolder = new KSqueezedTextLabel(0, imagePanel);
    int hgt        = fontMetrics().height()-2;
    file->setMaximumHeight(hgt);
    folder->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);

    // ---------------------------------------------------------------

    KHBox *hbox3          = new KHBox(imagePanel);
    QLabel *resultsLabel2 = new QLabel(i18n("Results:"), hbox3);
    d->resultsImage       = new QSpinBox(hbox3);
    d->resultsImage->setRange(1, 50);
    d->resultsImage->setSingleStep(1);
    d->resultsImage->setValue(10);
    d->resultsImage->setWhatsThis(i18n("<p>Set here the number of similar items to find."));

    hbox3->setStretchFactor(resultsLabel2, 10);
    hbox3->setMargin(0);
    hbox3->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox *hbox4     = new KHBox(imagePanel);
    hbox4->setMargin(0);
    hbox4->setSpacing(KDialog::spacingHint());

    d->nameEditImage = new KLineEdit(hbox4);
    d->nameEditImage->setClearButtonShown(true);
    d->nameEditImage->setWhatsThis(i18n("<p>Enter the name of the current similar image search to save in the "
                                        "\"My Fuzzy Searches\" view"));

    d->saveBtnImage  = new QPushButton(hbox4);
    d->saveBtnImage->setIcon(SmallIcon("document-save"));
    d->saveBtnImage->setEnabled(false);
    d->saveBtnImage->setToolTip(i18n("Save current similar image search to a new virtual Album"));
    d->saveBtnImage->setWhatsThis(i18n("<p>If you press this button, current "
                                       "similar image search will be saved to a new search "
                                       "virtual album using name "
                                       "set on the left side."));

    // ---------------------------------------------------------------

    grid->addWidget(box2,           0, 0, 1, 3);
    grid->addWidget(file,           1, 0, 1, 1);
    grid->addWidget(d->labelFile,   1, 1, 1, 1);
    grid->addWidget(folder,         2, 0, 1, 1);
    grid->addWidget(d->labelFolder, 2, 1, 1, 1);
    grid->addWidget(hbox3,          3, 0, 1, 3);
    grid->addWidget(hbox4,          4, 0, 1, 3);
    grid->setRowStretch(5, 10);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    d->tabWidget->insertTab(FuzzySearchViewPriv::IMAGE, imagePanel, i18n("Image"));

    // ---------------------------------------------------------------
    // Sketch Panel

    QWidget *sketchPanel = new QWidget(this);
    QGridLayout *grid2   = new QGridLayout(sketchPanel);
    QWidget *box         = new QWidget(sketchPanel);
    QVBoxLayout *vlay2   = new QVBoxLayout(box);
    KHBox *drawingBox    = new KHBox(box);
    d->sketchWidget      = new SketchWidget(drawingBox);
    drawingBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    drawingBox->setLineWidth(1);

    vlay2->addStretch(10);
    vlay2->addWidget(drawingBox, 0, Qt::AlignCenter);
    vlay2->addStretch(10);
    vlay2->setMargin(0);
    vlay2->setSpacing(0);

    // ---------------------------------------------------------------

    d->hsSelector = new KHueSaturationSelector(sketchPanel);
    d->vSelector  = new KColorValueSelector(sketchPanel);
    d->hsSelector->setMinimumSize(200, 142);
    d->vSelector->setMinimumSize(26, 142);
    d->hsSelector->setChooserMode(ChooserValue);
    d->vSelector->setChooserMode(ChooserValue);
    d->hsSelector->setColorValue(255);

    QString tips(i18n("<p>Set here the brush color used to draw sketch."));
    d->hsSelector->setWhatsThis(tips);
    d->vSelector->setWhatsThis(tips);

    // ---------------------------------------------------------------

    KHBox *hbox        = new KHBox(sketchPanel);
    QLabel *brushLabel = new QLabel(i18n("Brush size:"), hbox);
    d->penSize         = new QSpinBox(hbox);
    d->penSize->setRange(1, 40);
    d->penSize->setSingleStep(1);
    d->penSize->setValue(10);
    d->penSize->setWhatsThis(i18n("<p>Set here the brush size in pixels used to draw sketch."));

    QLabel *resultsLabel = new QLabel(i18n("Results:"), hbox);
    d->resultsSketch     = new QSpinBox(hbox);
    d->resultsSketch->setRange(1, 50);
    d->resultsSketch->setSingleStep(1);
    d->resultsSketch->setValue(10);
    d->resultsSketch->setWhatsThis(i18n("<p>Set here the number of items to find using sketch."));

    hbox->setStretchFactor(brushLabel, 10);
    hbox->setStretchFactor(resultsLabel, 10);
    hbox->setMargin(0);
    hbox->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    KHBox *hbox2      = new KHBox(sketchPanel);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    d->resetButton    = new QPushButton(hbox2);
    d->resetButton->setIcon(SmallIcon("document-revert"));
    d->resetButton->setToolTip(i18n("Clear sketch"));
    d->resetButton->setWhatsThis(i18n("<p>Use this button to clear sketch contents."));

    d->nameEditSketch = new KLineEdit(hbox2);
    d->nameEditSketch->setClearButtonShown(true);
    d->nameEditSketch->setWhatsThis(i18n("<p>Enter the name of the current sketch search to save in the "
                                         "\"My Fuzzy Searches\" view"));

    d->saveBtnSketch  = new QPushButton(hbox2);
    d->saveBtnSketch->setIcon(SmallIcon("document-save"));
    d->saveBtnSketch->setEnabled(false);
    d->saveBtnSketch->setToolTip(i18n("Save current sketch search to a new virtual Album"));
    d->saveBtnSketch->setWhatsThis(i18n("<p>If you press this button, current sketch "
                                        "fuzzy search will be saved to a new search "
                                        "virtual album using name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    grid2->addWidget(box,            0, 0, 1, 3);
    grid2->addWidget(d->hsSelector,  1, 0, 1, 2);
    grid2->addWidget(d->vSelector,   1, 2, 1, 1);
    grid2->addWidget(hbox,           2, 0, 1, 3);
    grid2->addWidget(hbox2,          3, 0, 1, 3);
    grid2->setRowStretch(5, 10);
    grid2->setColumnStretch(1, 10);
    grid2->setMargin(KDialog::spacingHint());
    grid2->setSpacing(KDialog::spacingHint());

    d->tabWidget->insertTab(FuzzySearchViewPriv::SKETCH, sketchPanel, i18n("Sketch"));

    // ---------------------------------------------------------------

    d->fuzzySearchFolderView = new FuzzySearchFolderView(this);
    d->searchFuzzyBar        = new SearchTextBar(this, "FuzzySearchViewSearchFuzzyBar");

    // ---------------------------------------------------------------

    vlay->addWidget(d->tabWidget);
    vlay->addWidget(d->fuzzySearchFolderView);
    vlay->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                  QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlay->addWidget(d->searchFuzzyBar);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    readConfig();

    // ---------------------------------------------------------------

    connect(d->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->fuzzySearchFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchFuzzyBar, SLOT(slotSearchResult(bool)));

    connect(d->searchFuzzyBar, SIGNAL(textChanged(const QString&)),
            d->fuzzySearchFolderView, SLOT(slotTextSearchFilterChanged(const QString&)));

    connect(d->hsSelector, SIGNAL(valueChanged(int, int)),
            this, SLOT(slotHSChanged(int, int)));

    connect(d->vSelector, SIGNAL(valueChanged(int)),
            this, SLOT(slotVChanged()));

    connect(d->penSize, SIGNAL(valueChanged(int)),
            d->sketchWidget, SLOT(setPenWidth(int)));

    connect(d->resultsSketch, SIGNAL(valueChanged(int)),
            this, SLOT(slotDirtySketch()));

    connect(d->resultsImage, SIGNAL(valueChanged(int)),
            this, SLOT(slotResultsImageChanged()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotClearSketch()));

    connect(d->sketchWidget, SIGNAL(signalSketchChanged(const QImage&)),
            this, SLOT(slotDirtySketch()));

    connect(d->saveBtnSketch, SIGNAL(clicked()),
            this, SLOT(slotSaveSketchSAlbum()));

    connect(d->saveBtnImage, SIGNAL(clicked()),
            this, SLOT(slotSaveImageSAlbum()));

    connect(d->nameEditSketch, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditSketchConditions()));

    connect(d->nameEditImage, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckNameEditImageConditions()));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription&, const QPixmap&)));

    // ---------------------------------------------------------------

    slotCheckNameEditSketchConditions();
    slotCheckNameEditImageConditions();
}

FuzzySearchView::~FuzzySearchView()
{
    writeConfig();
    delete d;
}

void FuzzySearchView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));

    d->tabWidget->setCurrentIndex(group.readEntry("FuzzySearch Tab",
                                  (int)FuzzySearchViewPriv::SKETCH));
    d->penSize->setValue(group.readEntry("Pen Sketch Size", 10));
    d->resultsSketch->setValue(group.readEntry("Result Sketch items", 10));
    d->hsSelector->setXValue(group.readEntry("Pen Sketch Hue", 180));
    d->hsSelector->setYValue(group.readEntry("Pen Sketch Saturation", 128));
    d->vSelector->setValue(group.readEntry("Pen Sketch Value", 255));
    d->resultsImage->setValue(group.readEntry("Result Image items", 10));
    d->hsSelector->updateContents();
    slotHSChanged(d->hsSelector->xValue(), d->hsSelector->yValue());
    d->sketchWidget->setPenWidth(d->penSize->value());
}

void FuzzySearchView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("FuzzySearch SideBar"));
    group.writeEntry("FuzzySearch Tab",        d->tabWidget->currentIndex());
    group.writeEntry("Pen Sketch Size",        d->penSize->value());
    group.writeEntry("Result Sketch items",    d->resultsSketch->value());
    group.writeEntry("Pen Sketch Hue",         d->hsSelector->xValue());
    group.writeEntry("Pen Sketch Saturation",  d->hsSelector->yValue());
    group.writeEntry("Pen Sketch Value",       d->vSelector->value());
    group.writeEntry("Result Image items",     d->resultsImage->value());
    group.sync();
}

FuzzySearchFolderView* FuzzySearchView::folderView() const
{
    return d->fuzzySearchFolderView;
}

SearchTextBar* FuzzySearchView::searchBar() const
{
    return d->searchFuzzyBar;
}

void FuzzySearchView::slotTabChanged(int tab)
{
    switch(tab)
    {
        case FuzzySearchViewPriv::IMAGE:
        {
            AlbumManager::instance()->setCurrentAlbum(d->imageSAlbum);
            break;
        }
        default:  // SKETCH
        {
            AlbumManager::instance()->setCurrentAlbum(d->sketchSAlbum);
            break;
        }
    }
}

void FuzzySearchView::slotHSChanged(int h, int s)
{
    d->vSelector->blockSignals(true);
    d->vSelector->setHue(h);
    d->vSelector->setSaturation(s);
    d->vSelector->updateContents();
    d->vSelector->repaint();
    d->vSelector->blockSignals(false);
    slotVChanged();
}

void FuzzySearchView::slotVChanged()
{
    int hue      = d->vSelector->hue();
    int sat      = d->vSelector->saturation();
    int val      = d->vSelector->value();
    QColor color = QColor::fromHsv(hue, sat, val);

    d->sketchWidget->setPenColor(color);
}

void FuzzySearchView::setActive(bool val)
{
    if (d->fuzzySearchFolderView->selectedItem()) 
    {
        d->fuzzySearchFolderView->setActive(val);
    }
    else if (val)
    {
        slotTabChanged(d->tabWidget->currentIndex());
    }
}

void FuzzySearchView::slotSaveSketchSAlbum()
{
    QString name = d->nameEditSketch->text();
    if (!checkName(name))
        return;

    createNewFuzzySearchAlbumFromSketch(name);
}

void FuzzySearchView::slotDirtySketch()
{
    slotCheckNameEditSketchConditions();
    createNewFuzzySearchAlbumFromSketch(FuzzySearchFolderView::currentFuzzySketchSearchName());
}

void FuzzySearchView::createNewFuzzySearchAlbumFromSketch(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (d->sketchWidget->isClear())
        return;

    // We query database here

    HaarIface haarIface;
    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "signature");         // we pass a signature
    writer.writeAttribute("numberofresults", QString::number(d->resultsSketch->value()));
    writer.writeAttribute("sketchtype", "handdrawn");
    writer.writeValue(haarIface.signatureAsText(d->sketchWidget->sketchImage()));
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->sketchSAlbum = salbum;
}

void FuzzySearchView::slotAlbumSelected(SAlbum* salbum)
{
    if (!salbum) 
        return;

    // NOTE: There is nothing to display in sketch widget. Database do not store the sketch image.

    AlbumManager::instance()->setCurrentAlbum(salbum);

    SearchXmlReader reader(salbum->query());
    reader.readToFirstField();
    QStringRef type             = reader.attributes().value("type");
    QStringRef numResultsString = reader.attributes().value("numberofresults");
    QStringRef sketchTypeString = reader.attributes().value("sketchtype");

    if (type == "imageid")
    {
        setImageId(reader.valueToLongLong());
        d->imageSAlbum = salbum;
    }
    else
    {
        d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::SKETCH);
        d->sketchSAlbum = salbum;
    }
}

void FuzzySearchView::slotClearSketch()
{
    d->sketchWidget->slotClear();
    slotCheckNameEditSketchConditions();
    AlbumManager::instance()->setCurrentAlbum(0);
}

bool FuzzySearchView::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked) 
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool FuzzySearchView::checkAlbum(const QString& name) const
{
    AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::Iterator it = list.begin() ; it != list.end() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void FuzzySearchView::slotCheckNameEditSketchConditions()
{
    if (!d->sketchWidget->isClear())
    {
        d->nameEditSketch->setEnabled(true);

        if (!d->nameEditSketch->text().isEmpty())
            d->saveBtnSketch->setEnabled(true);
    }
    else
    {
        d->nameEditSketch->setEnabled(false);
        d->saveBtnSketch->setEnabled(false);
    }
}

void FuzzySearchView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    if (salbum->title() == FuzzySearchFolderView::currentFuzzySketchSearchName() ||
        salbum->title() == FuzzySearchFolderView::currentFuzzyImageSearchName())
        return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)").arg(oldName),
                                         i18n("Enter new album name:"),
                                         oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

void FuzzySearchView::dragEnterEvent(QDragEnterEvent *e)
{
    if(DItemDrag::canDecode(e->mimeData()))
        e->acceptProposedAction();
}

void FuzzySearchView::dropEvent(QDropEvent *e)
{
    if(DItemDrag::canDecode(e->mimeData()))
    {
        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
            return;

        if (imageIDs.isEmpty())
            return;

        setImageId(imageIDs.first());
        slotCheckNameEditImageConditions();
        createNewFuzzySearchAlbumFromImage(FuzzySearchFolderView::currentFuzzyImageSearchName());

        e->acceptProposedAction();
    }
}

void FuzzySearchView::slotResultsImageChanged()
{
    if (d->imageInfo)
        setImageInfo(*d->imageInfo);
}

void FuzzySearchView::setImageId(qlonglong imageid)
{
    if (d->imageInfo) 
        delete d->imageInfo;

    d->imageInfo = new ImageInfo(imageid);
    d->labelFile->setText(d->imageInfo->name());
    d->labelFolder->setText(d->imageInfo->fileUrl().directory());
    d->thumbLoadThread->find(d->imageInfo->fileUrl().path());
    d->tabWidget->setCurrentIndex((int)FuzzySearchViewPriv::IMAGE);
}

void FuzzySearchView::setImageInfo(const ImageInfo& info)
{
    setImageId(info.id());
    slotCheckNameEditImageConditions();
    createNewFuzzySearchAlbumFromImage(FuzzySearchFolderView::currentFuzzyImageSearchName());
}

void FuzzySearchView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->imageInfo && KUrl(desc.filePath) == d->imageInfo->fileUrl())
        d->imageWidget->setPixmap(pix.scaled(256, 256, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
}

void FuzzySearchView::createNewFuzzySearchAlbumFromImage(const QString& name)
{
    AlbumManager::instance()->setCurrentAlbum(0);

    if (!d->imageInfo)
        return;

    // We query database here

    HaarIface haarIface;
    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "imageid");
    writer.writeAttribute("numberofresults", QString::number(d->resultsImage->value()));
    writer.writeAttribute("sketchtype", "scanned");
    writer.writeValue(d->imageInfo->id());
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(salbum);
    d->imageSAlbum = salbum;
}

void FuzzySearchView::slotCheckNameEditImageConditions()
{
    if (d->imageInfo)
    {
        d->nameEditImage->setEnabled(true);

        if (!d->nameEditImage->text().isEmpty())
            d->saveBtnImage->setEnabled(true);
    }
    else
    {
        d->nameEditImage->setEnabled(false);
        d->saveBtnImage->setEnabled(false);
    }
}

void FuzzySearchView::slotSaveImageSAlbum()
{
    QString name = d->nameEditImage->text();
    if (!checkName(name))
        return;

    createNewFuzzySearchAlbumFromImage(name);
}

}  // NameSpace Digikam
