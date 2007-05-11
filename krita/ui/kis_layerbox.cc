/*
 *  kis_layerbox.cc - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_layerbox.h"

#include <QtDebug>
#include <QToolButton>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QToolTip>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPixmap>
#include <QList>
#include <QVector>

#include <kdebug.h>
#include <kglobal.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <khbox.h>
#include <kicon.h>

#include "KoSliderCombo.h"
#include <KoDocumentSectionView.h>
#include "KoColorSpace.h"

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>

#include "kis_cmb_composite.h"
#include "kis_view2.h"
#include "kis_layer_manager.h"
#include "kis_layer_model.h"

KisLayerBox::KisLayerBox()
    : QDockWidget( i18n("Layers" ) )
    , Ui::WdgLayerBox()
    , m_image( 0 )
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    setupUi(mainWidget);

    setMinimumSize(mainWidget->minimumSizeHint());

    listLayers->viewport()->installEventFilter(this);
    connect(listLayers, SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));

    m_viewModeMenu = new KMenu( this );
    QActionGroup *group = new QActionGroup( this );
    QList<QAction*> actions;
    actions << m_viewModeMenu->addAction(KIcon("fileview-text"),
               i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(KIcon("fileview-detailed"),
               i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(KIcon("view_icon"),
               i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));
    for( int i = 0, n = actions.count(); i < n; ++i )
    {
        actions[i]->setCheckable( true );
        actions[i]->setActionGroup( group );
    }
    actions[1]->trigger(); //TODO save/load previous state

    bnViewMode->setMenu(m_viewModeMenu);
    bnViewMode->setPopupMode(QToolButton::InstantPopup);
    bnViewMode->setIcon(KIcon("view_choose"));
    bnViewMode->setText(i18n("View mode"));

    bnAdd->setToolTip(i18n("Create new layer"));

    bnDelete->setToolTip(i18n("Remove current layer"));

    bnRaise->setToolTip(i18n("Raise current layer"));
    bnRaise->setEnabled(false);

    bnLower->setEnabled(false);
    bnLower->setToolTip(i18n("Lower current layer"));

    bnProperties->setToolTip(i18n("Properties for layer"));

    m_newLayerMenu = new KMenu(this);
    bnAdd->setMenu(m_newLayerMenu);
    bnAdd->setPopupMode(QToolButton::InstantPopup);

    m_newLayerMenu->addAction(KIcon("document-new"), i18n("&New Layer..."), this, SLOT(slotNewLayer()));
    m_newLayerMenu->addAction(KIcon("folder"), i18n("New &Group Layer..."), this, SLOT(slotNewGroupLayer()));
    m_newLayerMenu->addAction(KIcon("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));

    connect(bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(doubleOpacity, SIGNAL(valueChanged(double, bool)), SIGNAL(sigOpacityChanged(double, bool)));
    connect(cmbComposite, SIGNAL(activated(const KoCompositeOp*)), SIGNAL(sigItemComposite(const KoCompositeOp*)));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::setImage(KisImageSP img, KisLayerModel * layerModel)
{
    kDebug() << "KisLayerBox::setImage layer model " << layerModel << endl;

    if (m_image == img)
        return;

    m_layerModel = layerModel;

    if (m_image)
        m_image->disconnect(this);

    if (img) {

        connect(img.data(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerAdded(KisLayerSP)), this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerPropertiesChanged(KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(updateUI()));


        listLayers->setModel( layerModel );

        m_image = img;

        updateUI();
    }
    else {
        listLayers->setModel(0);
    }
}

bool KisLayerBox::eventFilter(QObject *o, QEvent *e)
{
    Q_ASSERT(o == listLayers->viewport());

    if (e->type() == QEvent::MouseButtonDblClick) {

        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QModelIndex mi = listLayers->indexAt(me->pos());
        if (mi.isValid())
            slotPropertiesClicked();
        else
            slotNewLayer();
        return true;
    }

    return QDockWidget::eventFilter(o, e);
}

void KisLayerBox::updateUI()
{
    Q_ASSERT(! m_image.isNull());
    kDebug(41007)  << "###### KisLayerBox::updateUI " << m_image->activeLayer() << endl;

    bnDelete->setEnabled(m_image->activeLayer());
    bnRaise->setEnabled(m_image->activeLayer() && (m_image->activeLayer()->prevSibling() || m_image->activeLayer()->parentLayer()));
    bnLower->setEnabled(m_image->activeLayer() && m_image->activeLayer()->nextSibling());
    doubleOpacity->setEnabled(m_image->activeLayer());
    cmbComposite->setEnabled(m_image->activeLayer());
    if (KisLayerSP active = m_image->activeLayer())
    {
        if (m_image->activeDevice())
            slotSetColorSpace(m_image->activeDevice()->colorSpace());
        else
            slotSetColorSpace(m_image->colorSpace());
        slotSetOpacity(active->opacity() * 100.0 / 255);
        slotSetCompositeOp(active->compositeOp());
    }
}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    cmbComposite->blockSignals(true);
    cmbComposite->setCurrent(compositeOp);
    cmbComposite->blockSignals(false);
}

void KisLayerBox::slotSetColorSpace(const KoColorSpace * colorSpace)
{
    cmbComposite->blockSignals(true);
    cmbComposite->setCompositeOpList(colorSpace->userVisiblecompositeOps());
    cmbComposite->blockSignals(false);
}

// range: 0-100
void KisLayerBox::slotSetOpacity(double opacity)
{
    Q_ASSERT( opacity >= 0 && opacity <= 100 );
    doubleOpacity->blockSignals(true);
    doubleOpacity->setValue(opacity);
    doubleOpacity->blockSignals(false);
}

void KisLayerBox::slotContextMenuRequested(const QPoint &pos, const QModelIndex &index)
{
    QMenu menu;
    if (index.isValid())
    {
        listLayers->addPropertyActions(&menu, index);
        menu.addAction(KIcon("document-properties"), i18n("&Properties..."), this, SLOT(slotPropertiesClicked()));
        menu.addSeparator();
        menu.addAction(KIcon("edit-delete"), i18n("&Remove Layer"), this, SLOT(slotRmClicked()));
        QMenu *sub = menu.addMenu(KIcon("document-new"), i18n("&New"));
        sub->addAction(KIcon("file"), i18n("&Layer..."), this, SLOT(slotNewLayer()));
        sub->addAction(KIcon("folder"), i18n("&Group Layer..."), this, SLOT(slotNewGroupLayer()));
        sub->addAction(KIcon("tool_filter"), i18n("&Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
    }
    else
    {
        menu.addAction(KIcon("document-new"), i18n("&New Layer..."), this, SLOT(slotNewLayer()));
        menu.addAction(KIcon("folder"), i18n("New &Group Layer..."), this, SLOT(slotNewGroupLayer()));
        menu.addAction(KIcon("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
    }
    menu.exec(pos);
}

void KisLayerBox::slotMinimalView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::MinimalMode);
}

void KisLayerBox::slotDetailedView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::DetailedMode);
}

void KisLayerBox::slotThumbnailView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::ThumbnailMode);
}

void KisLayerBox::getNewLayerLocation(KisGroupLayerSP &parent, KisLayerSP &above)
{
    KisGroupLayerSP root = m_image->rootLayer();
    if (KisLayerSP active = m_image->activeLayer())
    {
        if (KisGroupLayer* pactive = qobject_cast<KisGroupLayer*>(active.data()))
        {
            parent = pactive;
            above = parent->firstChild();
        }
        else
        {
            parent = root;
            above = active;
            if (active->parentLayer())
                parent = active->parentLayer();
        }
    }
    else
    {
        parent = root;
        above = m_image->rootLayer()->firstChild();
    }
}

void KisLayerBox::slotNewLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestLayer(parent, above);
}

void KisLayerBox::slotNewGroupLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestGroupLayer(parent, above);
}

void KisLayerBox::slotNewAdjustmentLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestAdjustmentLayer(parent, above);
}

void KisLayerBox::slotRmClicked()
{
    QModelIndexList l = selectedLayers();

    for (int i = 0, n = l.count(); i < n; ++i)
        m_image->removeLayer(m_layerModel->layerFromIndex(l.at(i)));
}

void KisLayerBox::slotRaiseClicked()
{
    QModelIndexList l = selectedLayers();

    KisLayerSP layer = m_layerModel->layerFromIndex(l.first());
    if( l.count() == 1 && layer == layer->parentLayer()->firstChild() && layer->parentLayer() != m_image->rootLayer())
    {
        if (KisGroupLayerSP grandparent = layer->parentLayer()->parentLayer())
            m_image->moveLayer(layer, grandparent, KisLayerSP(layer->parentLayer().data()));
    }
    else
    {
        for (int i = 0, n = l.count(); i < n; ++i)
            if (KisLayerSP li = m_layerModel->layerFromIndex(l[i]))
                if (li->prevSibling())
                    m_image->moveLayer(li, li->parentLayer(), li->prevSibling());
    }

    if( !l.isEmpty() )
        listLayers->scrollTo( l.first() );
}

void KisLayerBox::slotLowerClicked()
{
    QModelIndexList l = selectedLayers();

    for (int i = l.count() - 1; i >= 0; --i)
        if (KisLayerSP layer = m_layerModel->layerFromIndex(l[i]))
            if (layer->nextSibling())
            {
                if (layer->nextSibling()->nextSibling())
                    m_image->moveLayer(layer, layer->parentLayer(), layer->nextSibling()->nextSibling());
                else
                    m_image->moveLayer(layer, layer->parentLayer(), KisLayerSP(0));
            }

    if( !l.isEmpty() )
        listLayers->scrollTo( l.last() );
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisLayerSP active = m_image->activeLayer())
        emit sigRequestLayerProperties(active);
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    //doubleOpacity->setUpdatesEnabled(enable);
    cmbComposite->setUpdatesEnabled(enable);

    doubleOpacity->blockSignals(!enable);
    cmbComposite->blockSignals(!enable);
}

QModelIndexList KisLayerBox::selectedLayers() const
{
    QModelIndexList l = listLayers->selectionModel()->selectedIndexes();
    if (l.count() < 2 && m_image->activeLayer() && !l.contains(listLayers->currentIndex()))
    {
        l.clear();
        l.append(listLayers->currentIndex());
    }
    return l;
}

#include "kis_layerbox.moc"
