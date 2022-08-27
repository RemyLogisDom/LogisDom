/****************************************************************************
**
** Copyright (C) 2019 Remy CARISIO.
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



#ifndef PLOT_H
#define PLOT_H

#include <QTime>
#include <curve.h>
#include <histo.h>
#include <tableau.h>
#include "globalvar.h"
#include <qwt_wheel.h>
#include <qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend_label.h>
#include <qwt_legend.h>
#include "qwt_plot_picker.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_canvas.h"


class graph;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtPlotMarker;


class TimeQwtPicker: public QwtPlotPicker
{
    //QDateTime *origin;
    QString format;
public:
//	TimeQwtPicker(int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand, DisplayMode trackerMode, QwtPlotCanvas *canvas, QDateTime *Origin) : QwtPlotPicker(xAxis, yAxis, selectionFlags, rubberBand, trackerMode, canvas)
    TimeQwtPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QWidget *canvas) : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canvas)
    // QT4 TimeQwtPicker(int xAxis, int yAxis, RubberBand rubberBand, DisplayMode trackerMode, QWidget *canvas, QDateTime *Origin) : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, canvas)
    {
        format = "hh:mm dd-MMM-yyyy";
        //origin = Origin;
    }
    QwtText trackerTextF(const QPointF &pos) const
    {
        double x = pos.x() * SecsInDays;
        double y = pos.y();

#if QT_VERSION > 0x050603
        QString txt = QDateTime::fromSecsSinceEpoch(x).toString(format) + QString(" : %1").arg(y, 0, 'f', 2);
#else
        QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
        QString txt = origin.addSecs((int)x).toString(format) + QString(" : %1").arg(y, 0, 'f', 2);
#endif
        return QwtText(txt);
    }
    void setFormat(QString f)
    {
        format = f;
    }
    double getX()
    {
        QPointF P = invTransform(trackerPosition());
        double X = P.x() * SecsInDays;
        return X;
    }
};



class Plot : public QwtPlot 
{
Q_OBJECT
	friend class energiesolaire;
	friend class graph;
public:
    Plot(QWidget * = nullptr);
    ~Plot();
	void addcurve(QString name, QString RomID, QColor color);
	void addcurve(QString &description);
    void addhisto(QString &description);
    void addhisto(tableau *chart, const QColor &color);
    void showCurve(QwtPlotItem *);
	void update(const QDateTime &origin, const QDateTime &begin, const QDateTime &end, bool compress = true);
	void AddData(double v, const QDateTime &time, QString RomID);
	void getCurveConfig(QString &str);
    void getHistoConfig(QString &str);
    bool IsRomIDthere(QString RomID);
	void reload();
	void delCurve(QString RomID);
	QDateTime Origin, Begin, End;
	void setCurveConfig(QString &str);
    void setHistoConfig(QString &str);
    graph *parent;
    int zoomDelay;
    QwtLegend *d_externalLegend;
    LegendItem *d_legendItem;
    bool showTools = false;
private:
	QMutex mutex;
    QwtPlotMarker *d_marker;
    QwtPlotZoomer *d_zoomer[2];
    QwtPlotPanner *d_panner;
    TimeQwtPicker *d_picker;
public slots:
	void addcurvemenu();
    void addhistomenu(tableau *chart);
    void removecurve();
private slots:
    void showCurve(const QVariant &itemInfo, bool on, int index);
	void emitloadingFinished();
    void zoomed();
private:
	QList <GraphCurve*> curve;
    QList <GraphHisto*> histo;
protected:
	void mouseMoveEvent(QMouseEvent *event);
signals:
	void loadingFinished();
    void zoomFinished();
    void ToolsOn();
};

#endif



