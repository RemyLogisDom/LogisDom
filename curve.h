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



#ifndef CURVE_H
#define CURVE_H
#include <QDateTime>
#include <QtGui>

#include "ui_stdui.h"
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <qwt_legend_label.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_legend.h>
#include <qwt_plot_legenditem.h>
#include <qwt_spline.h>
#include <qwt_curve_fitter.h>

class QwtPlotCurve;
class QCheckBox;
class QTextStream;
class QDateTime;




class myCurve : public QwtPlotCurve
{
public:
	myCurve(const QString title) : QwtPlotCurve(title)
	{
	}
    void updateLegend(QwtLegend *) const
	{
#if QT_VERSION < 0x050000
        //QwtPlotCurve::updateLegend(legend);
        // QwtLegendItem *lgdItem = dynamic_cast<QwtLegendItem*>(legend->find(this));
        // if ( lgdItem ) lgdItem->setIdentifierSize(QSize(20,15));
        // QwtPlotItem::updateLegend(legend);
#endif
//		if (lgdItem) lgdItem->setIdentifierWidth(8);
//		if (lgdItem) lgdItem->setIdentifierMode(7);
	}
};




class GraphCurve : public QWidget
{
	Q_OBJECT
public:
	GraphCurve(const QString &title);
	~GraphCurve();
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton CurveTool;
	QPushButton CurveColorButton;
	QCheckBox FillCurveButton;
	QCheckBox AliasedCurveButton;
	QCheckBox SecondScaleButton;
	QLineEdit secondScaleStr;
	double secondScale;
	QComboBox StyleButton;
	void setromid(QString ROMID);
	void setColor(const QColor &color);
	QString getRomID();
	void clearfilelist();
	void update(const QDateTime &begin, const QDateTime &finish, bool compress = true);
	void reload();
	void AddData(double v, const QDateTime &time, QString romid);
	void getConfigStr(QString &str);
	myCurve *curve;
    QDateTime Origin;
	int beginOffset, finishOffset, lastBeginOffset, lastFinishOffset;
	void setCurveConfig(QString &description);
private:
	QMutex mutex;
    //QwtSplineCurveFitter *curveFitter;
    QwtCurveFitter *curveFitter;
private slots:
	void curverezise(int);
	void colorChange();
	void setAliased(int);
	void switchSecondScale(int);
	void secondScaleChanged();
	void setFilled(int);
	void setStyle(int);
public:
	QString RomID;
	QColor Color;
    QVector <double> Xraw;
    QVector <double> Yraw;
    QVector <double> Xcompressed;
    QVector <double> Ycompressed;
	void removesidedata();
	void scaleData();
	void compressdata();
	int compressratio;
	bool compressed;
	int readmonth, readyear;
	double Xpoint, lastsavevalue, XDebut, XFin;
    void updateLegend(qint64 T, int state);
signals:
	void loadingFinished();
};



#endif
