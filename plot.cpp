/****************************************************************************
**
** Copyright (C) 2022 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** LogisDom is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.

** LogisDom is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License
** along with LogisDom.  If not, see <https://www.gnu.org/licenses/>
**
****************************************************************************/




#include <QtWidgets/QColorDialog>
#include "globalvar.h"
#include <QPainter>
#include <QMenu>
#include "configwindow.h"
#include "remote.h"
#include "onewire.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "plot.h"
#include "devfinder.h"
#include "qwt_plot.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_legenditem.h"
#include "qwt_legend_label.h"
#include "qwt_scale_map.h"
#include "graph.h"
#include "histo.h"
#include "tableauconfig.h"


class Background: public QwtPlotItem
{
public:
	Background()
	{
		setZ(0.0);
	}
	virtual int rtti() const
	{
		return QwtPlotItem::Rtti_PlotUserItem;
	}
    virtual void draw(QPainter *painter, const QwtScaleMap &, const QwtScaleMap &yMap, const QRectF &rect) const
	{
		QColor c(Qt::white);
        QRectF r = rect;
		for ( int i = 100; i > 0; i -= 10 )
		{
			r.setBottom(yMap.transform(i - 10));
			r.setTop(yMap.transform(i));
			painter->fillRect(r, c);
            c = c.darker(110);
		}
	}
};





class Zoomer: public QwtPlotZoomer
{
public:
#if QT_VERSION < 0x050000
    Zoomer(int xAxis, int yAxis, QWidget *canvas): QwtPlotZoomer(xAxis, yAxis, canvas)
#else
    Zoomer(int xAxis, int yAxis, QWidget *canvas): QwtPlotZoomer(xAxis, yAxis, canvas)
#endif
    {
//		setSelectionFlags(QwtPicker::EllipseRubberBand | QwtPicker::CrossRubberBand);
        setTrackerMode(QwtPicker::AlwaysOff);
        setRubberBand(QwtPicker::NoRubberBand);
        // RightButton: zoom out by 1
        // Ctrl+RightButton: zoom out to full size
        #if QT_VERSION < 0x040000
            setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlButton);
        #else
            setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        #endif
        setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    }
};





Plot::Plot(QWidget *parent): QwtPlot(parent)
{
	setAutoReplot(true);
    plotLayout()->setAlignCanvasToScales(true);
    QwtLegend *legend = new QwtLegend();
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    insertLegend(legend, QwtPlot::RightLegend);
    connect(legend, SIGNAL(checked(QVariant, bool, int)), this, SLOT(showCurve(QVariant, bool, int)));
    //connect(this, SIGNAL( legendDataChanged( const QVariant &, const QList<QwtLegendData> & ) ), legend, SLOT( updateLegend( const QVariant &, const QList<QwtLegendData> & ) ) );
    setAxisLabelRotation(QwtPlot::xBottom, -20.0);
	setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
	const int fmh = QFontMetrics(scaleWidget->font()).height();
	scaleWidget->setMinBorderDist(0, fmh / 2);
    //Background *bg = new Background();
    //bg->attach(this);
    setMouseTracking(true);
	parent = nullptr;
    d_zoomer[0] = new Zoomer(QwtPlot::xBottom, QwtPlot::yLeft, canvas());
    d_zoomer[0]->setRubberBand(QwtPicker::RectRubberBand);
    d_zoomer[0]->setRubberBandPen(QColor(Qt::black));
    d_zoomer[0]->setTrackerMode(QwtPicker::AlwaysOff);
    d_zoomer[0]->setTrackerPen(QColor(Qt::black));
    d_zoomer[1] = new Zoomer(QwtPlot::xTop, QwtPlot::yRight, canvas());
    connect(d_zoomer[0], SIGNAL(zoomed(QRectF)), this, SLOT(zoomed()));
    zoomDelay = 0;
    d_panner = new QwtPlotPanner(canvas());
    d_panner->setMouseButton(Qt::MidButton);
    d_picker = new TimeQwtPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas());
    d_picker->setRubberBandPen(QColor(Qt::black));
    d_picker->setRubberBand(QwtPicker::RectRubberBand);
    d_picker->setTrackerPen(QColor(Qt::black));
    d_marker = new QwtPlotMarker();
    d_marker->setLineStyle(QwtPlotMarker::Cross);
    d_marker->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    d_marker->setLinePen(QPen(Qt::black, 0, Qt::SolidLine));
    d_marker->attach(this);
}



Plot::~Plot()
{
}



void Plot::mouseMoveEvent(QMouseEvent *)
{
    uint state = QApplication::keyboardModifiers();
	if (!parent) return;
	for (int n=0; n<curve.count(); n++)
	{
        double X = d_picker->getX();
#if QT_VERSION > 0x050603
        qint64 T = Origin.toSecsSinceEpoch() + X;
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    qint64 T = origin.secsTo(Origin) + X;
#endif
        curve.at(n)->updateLegend(T, state);
	}
    if (!zoomDelay) { if (state & Qt::ControlModifier) parent->updateAxis(""); }
    if (state & Qt::AltModifier)
    {
        d_marker->show();
        QPoint pos = d_picker->trackerPosition();
        double xTop = invTransform(QwtPlot::xBottom, pos.x());
        double yLeft = invTransform(QwtPlot::yLeft, pos.y());
        d_marker->setValue(xTop,yLeft);
        //QwtText lable_value = axisScaleDraw(QwtPlot::xBottom)->label(invTransform(QwtPlot::xBottom,pos.x()));
        //d_marker->setLabel(lable_value);
    }
    else
    {
        d_marker->hide();
    }
    if (!showTools)
    {
        showTools = true;
        emit(ToolsOn());
    }
}



void Plot::showCurve(const QVariant &, bool, int)
{
    foreach (QwtPlotItem *plt_item, itemList())
    {
        if (plt_item->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = (QwtPlotCurve*)(plt_item);
            QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
            QList<QWidget *> legendWidgets = lgd->legendWidgets(itemToInfo(plt_item));
            if (legendWidgets.size() == 1)
            {
                QwtLegendLabel *b = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);
                if (b && b->inherits("QwtLegendLabel"))
                {
                    if (((QwtLegendLabel *)b)->isChecked()) c->show(); else c->hide();
                }
            }
        }
    }
}






void Plot::showCurve(QwtPlotItem *item)
{
    //QwtPlotCurve *c = (QwtPlotCurve*)(item);
    QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
    QList<QWidget *> legendWidgets = lgd->legendWidgets(itemToInfo(item));
    if (legendWidgets.size() == 1)
    {
        QwtLegendLabel *b = qobject_cast<QwtLegendLabel *>(legendWidgets[0]);
        if (b && b->inherits("QwtLegendLabel"))
        {
            ((QwtLegendLabel *)b)->setChecked(true);
        }
    }
}






void Plot::update(const QDateTime &origin, const QDateTime &begin, const QDateTime &end, bool compress)
{
	QMutexLocker locker(&mutex);
	Origin = origin;
	Begin = begin;
	End = end;
	for (int n=0; n<curve.count(); n++)
		curve[n]->update(Begin, End, compress);
}





void Plot::removecurve()
{
	bool ok;
	QStringList curveList;
	if (curve.count() == 0) return;
	for (int n=0; n<curve.count(); n++)
	{
		QString name = curve[n]->getRomID();
        curveList << parent->parent->configwin->getDeviceName(name);
	}
    QString curvechoise = inputDialog::getItemPalette(this, tr("Remove curve"), tr("Select curve : "), curveList, 0, false, &ok, parent->parent);
	if (!ok) return;
    onewiredevice * device = parent->parent->configwin->Devicenameexist(curvechoise);
	if (device)
		if (messageBox::questionHide(this, tr("Remove curve"),
        tr("Are you sure you want to remove curve ") + curvechoise + " ?", parent->parent,
		QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			 delCurve(device->getromid());
}




void Plot::delCurve(QString RomID)
{
	for (int n=0; n<curve.count(); n++)
		if (curve[n]->getRomID() == RomID) 
		{
			curve[n]->curve->detach();
			disconnect(curve[n], SIGNAL(updateFinished()), this, SLOT(emitloadingFinished()));
			delete curve[n];
			curve.removeAt(n);
		}
		//replot();
}






void Plot::addcurvemenu()
{
    onewiredevice *device = nullptr;
    devfinder *devFinder;
    devFinder = new devfinder(parent->parent);
    for (int n=0; n<parent->parent->configwin->devicePtArray.count(); n++)
        if (!IsRomIDthere(parent->parent->configwin->devicePtArray[n]->getromid()))
        {
            devFinder->devicesList.append(parent->parent->configwin->devicePtArray[n]);
        }
    if (devFinder->devicesList.count() == 0)
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), tr("No more device could be found"), parent->parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        delete devFinder;
        return;
    }
    devFinder->sort();
    devFinder->exec();
    device = devFinder->choosedDevice;
    delete devFinder;
    if (!device) return;
    QColor currentcolor = QColorDialog::getColor();
    if (!currentcolor.isValid()) return;
    addcurve(device->getname(), device->getromid(), currentcolor);
}





void Plot::addhistomenu(tableau *chart)
{
    QColor color = QColorDialog::getColor();
    if (!color.isValid()) return;
    addhisto(chart, color);
}



bool Plot::IsRomIDthere(QString RomID)
{
	for (int n=0; n<curve.count(); n++)
		if (curve[n]->getRomID() == RomID) return true;
	return false;
}





void Plot::getCurveConfig(QString &str)
{
	for (int n=0; n<curve.count(); n++)
		curve[n]->getConfigStr(str);
}




void Plot::getHistoConfig(QString &str)
{
    for (int n=0; n<histo.count(); n++)
        histo[n]->getConfigStr(str);
}



void Plot::setCurveConfig(QString &str)
{
	int currentindex, nextindex;
	QString curvedescription;
	currentindex = str.indexOf(New_Curve_Begin, 0);
	nextindex = currentindex;
	do
	{
		if (currentindex != -1)
		{
			nextindex = str.indexOf(New_Curve_Finished, currentindex);
			if (nextindex != -1)
			{
				curvedescription = str.mid(currentindex, nextindex - currentindex);
				addcurve(curvedescription);
			}
		}
		currentindex = str.indexOf(New_Curve_Begin, nextindex);
	}
	while (currentindex != -1);
}







void Plot::setHistoConfig(QString &str)
{
    int currentindex, nextindex;
    QString histodescription;
    currentindex = str.indexOf(New_Histo_Begin, 0);
    nextindex = currentindex;
    do
    {
        if (currentindex != -1)
        {
            nextindex = str.indexOf(New_Histo_Finished, currentindex);
            if (nextindex != -1)
            {
                histodescription = str.mid(currentindex, nextindex - currentindex);
                addhisto(histodescription);
            }
        }
        currentindex = str.indexOf(New_Histo_Begin, nextindex);
    }
    while (currentindex != -1);
}




 void Plot::addcurve(QString name, QString RomID, QColor color)		// New curve
{
	GraphCurve *newcurve = new GraphCurve(name);
	newcurve->setromid(RomID);	
	newcurve->setColor(color);
	newcurve->curve->attach(this);
	showCurve(newcurve->curve);
	curve.append(newcurve);
	QwtText title;
	title.setText(name);
	//title.setColor(color);
	newcurve->curve->setTitle(title);
	connect(newcurve, SIGNAL(loadingFinished()), this, SLOT(emitloadingFinished()));
	newcurve->update(Begin, End);
}




 void Plot::addcurve(QString &description)
{
	QString RomID = logisdom::getvalue(RomIDMark, description);
	if (RomID.isEmpty()) return;
    GraphCurve *newcurve = new GraphCurve(RomID);
	newcurve->setromid(RomID);	
	newcurve->setCurveConfig(description);	
    onewiredevice *device = parent->parent->configwin->DeviceExist(RomID);
	QwtText title;
	if (device) title.setText(device->getname()); else title.setText(RomID);
    newcurve->curve->setTitle(title);
    newcurve->curve->attach(this);
    showCurve(newcurve->curve);
    curve.append(newcurve);
	connect(newcurve, SIGNAL(loadingFinished()), this, SLOT(emitloadingFinished()));
}





 void Plot::addhisto(QString &description)
{
     QString chartname = logisdom::getvalue(RomIDMark, description);
     if (chartname.isEmpty()) return;
     tableau *chart = parent->parent->tableauConfig->getTableau(chartname);
     if (!chart) return;
     GraphHisto *newhisto = new GraphHisto(chart);
     newhisto->setHistoConfig(description);
     QwtText title;
     title.setText(chart->name);
     newhisto->histo->setTitle(title);
     newhisto->histo->attach(this);
     showCurve(newhisto->histo);
     histo.append(newhisto);
     newhisto->updateChart();
     connect(chart, SIGNAL(updateChart()), newhisto, SLOT(updateChart()));
}





 void Plot::addhisto(tableau *chart, const QColor &color)
{
    GraphHisto *newhisto = new GraphHisto(chart);
    newhisto->setColor(color);
    QwtText title;
    title.setText(chart->name);
    newhisto->histo->setTitle(title);
    newhisto->histo->attach(this);
    showCurve(newhisto->histo);
    histo.append(newhisto);
    newhisto->updateChart();
    connect(chart, SIGNAL(updateChart()), newhisto, SLOT(updateChart()));
}




 void Plot::zoomed()
 {
     //qDebug() << QString("Zoom %1").arg(d_zoomer[0]->zoomRectIndex());
     if (d_zoomer[0]->zoomRectIndex()) zoomDelay = 10; else
     {
         zoomDelay = 0;
         emit(zoomFinished());
     }
 }



void Plot::emitloadingFinished()
{
	emit(loadingFinished());
}




void Plot::AddData(double v, const QDateTime &time, QString RomID)
{
    for (int n=0; n<curve.count(); n++)
		curve[n]->AddData(v, time, RomID);
}



void Plot::reload()
{
	for (int n=0; n<curve.count(); n++)
		curve[n]->reload();
}


