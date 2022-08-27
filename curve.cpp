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




#include <QtWidgets/QColorDialog>
#include <QBrush>
#include <QDateTime>
#include <QTextStream>
#include <QCheckBox>
#include <QtGui>
#include <qwt_plot_curve.h>
#include <qwt_weeding_curve_fitter.h>
#include <qwt_plot_item.h>
#include "globalvar.h"
#include "onewire.h"
#include "formula.h"
#include "configwindow.h"
#include "remote.h"
#include "curve.h"



// curve->setItemAttribute(QwtPlotItem::Legend, false);


GraphCurve::GraphCurve(const QString &title)
{
	curve = new myCurve(title);
	beginOffset = 0;
	finishOffset = 0;
	lastBeginOffset = 0;
	lastFinishOffset = 0;
	Origin.setDate(QDate(yearscalebegin, 1, 1));
	Origin.setTime(QTime(0, 0));
    //Origin = QDateTime(QDate(yearscalebegin, 1, 1), QTime(0, 0, 0, 0));
    setup.setLayout(&setupLayout);
	setupLayout.setAlignment(Qt::AlignLeft | Qt::AlignTop);
	CurveColorButton.setText(tr("Curve color"));
	setupLayout.addWidget(&CurveColorButton);
	connect(&CurveColorButton, SIGNAL(clicked()), this, SLOT(colorChange()));
	FillCurveButton.setText(tr("Fill"));
	FillCurveButton. setCheckState(Qt::Checked);
	setupLayout.addWidget(&FillCurveButton);
	connect(&FillCurveButton, SIGNAL(stateChanged(int)), this, SLOT(setFilled(int)));
	AliasedCurveButton.setText(tr("Smooth"));
	setupLayout.addWidget(&AliasedCurveButton);
	connect(&AliasedCurveButton, SIGNAL(stateChanged(int)), this, SLOT(setAliased(int)));
	secondScale = 1;
	secondScaleStr.setText("1");
	connect(&secondScaleStr, SIGNAL(editingFinished()), this, SLOT(secondScaleChanged()));
	SecondScaleButton.setText(tr("Second scale"));
	setupLayout.addWidget(&SecondScaleButton);
	connect(&SecondScaleButton, SIGNAL(stateChanged(int)), this, SLOT(switchSecondScale(int)));
	secondScaleStr.setToolTip(tr("Place second scale here\nYou can use real like 2.5 or fraction like 1/10"));
	setupLayout.addWidget(&secondScaleStr);
	StyleButton.addItem(tr("Lines"));
	StyleButton.addItem(tr("Sticks "));
	StyleButton.addItem(tr("Steps "));
	StyleButton.addItem(tr("Dots "));
	setupLayout.addWidget(&StyleButton);
	connect(&StyleButton, SIGNAL(currentIndexChanged(int)), this, SLOT(setStyle(int)));

    curveFitter = new QwtWeedingCurveFitter(1.0);
    curve->setCurveFitter(curveFitter);
    curve->setCurveAttribute(QwtPlotCurve::Fitted, false);

/*
    curveFitter->setFitMode(curveFitter->ParametricSpline);
    curveFitter->setSplineSize(100);
*/
}



GraphCurve::~GraphCurve()
{
	delete curve;
}


void GraphCurve::updateLegend(qint64 T, int state)
{
	onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
	if (device)
	{
		QString name = device->getname();
		bool deviceLoading;
		double V = device->getMainValue(T, deviceLoading, nullptr);
		if (device->isDataLoading())
		{
			if (name.isEmpty()) curve->setTitle(RomID);
			else curve->setTitle(name + "  " + tr("Loading"));
			return;
		}
		else
		{
            if (logisdom::isNotNA(V))
			{
				if ((state & Qt::ControlModifier) && (state & Qt::ShiftModifier))
				{
					QString ValStr = device->ValueToStr(V);
					curve->setTitle(ValStr);
					return;
				}
				else if (state & Qt::ControlModifier)
				{
					QString ValStr = device->ValueToStr(V);
					if (name.isEmpty()) curve->setTitle(RomID + " = " + ValStr);
					else curve->setTitle(name  + " = " + ValStr);
					return;
				}
			}
		}
        //QwtText title;
        //title.setColor(plot.curve[n]->Color);
        //title.setText(Title);
        //if (state & Qt::ControlModifier)
		//{
		//}
		if (name.isEmpty()) curve->setTitle(RomID);
		else
		{
            if ((logisdom::AreNotSame(secondScale, 1)) && SecondScaleButton.isChecked()) name.append("\nx " + secondScaleStr.text());
			curve->setTitle(name);
		}
	}
}


void GraphCurve::curverezise(int)
{
}




void GraphCurve::colorChange()
{
	QColor currentcolor = QColorDialog::getColor();
	if (!currentcolor.isValid()) return;
	setColor(currentcolor);
}




void GraphCurve::reload()
{
	onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
	if (device) device->clearData();
}





void  GraphCurve::update(const QDateTime &begin, const QDateTime &finish, bool compress)
{
	QMutexLocker locker(&mutex);
	compressed = compress;
	onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
	if (!device) return;
	QString name = device->getname();
    qint64 b = Origin.secsTo(begin);
    qint64 e = Origin.secsTo(finish);
	bool ok;
	bool deviceLoading = false;
	dataloader::s_Data data;
	ok = device->getValues(b, e, deviceLoading, data, nullptr);
	//maison1wirewindow->GenMsg(QString("getValue Ok ? : %1").arg(ok));
	//maison1wirewindow->GenMsg(QString("Loading Ok ? : %1").arg(deviceLoading));
	if (ok && !deviceLoading)
	{
        Xraw.clear();
        Yraw.clear();
        Xcompressed.clear();
        Ycompressed.clear();
		double x;
        for (int n=0; n<data.data_Y.count(); n++)
		{
			x = data.offset.at(n);
			x /= SecsInDays;
            Yraw.append(data.data_Y.at(n));
            Xraw.append(x);
		}
        qreal s = secondScale;
		if (compress)
		{
            curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
            compressdata();
            if (SecondScaleButton.isChecked())
                for (int n=0; n<Ycompressed.count(); n++)
					Ycompressed[n] = Ycompressed[n] * s;
            curve->setSamples(Xcompressed, Ycompressed);
        }
		else
		{
            curve->setCurveAttribute(QwtPlotCurve::Fitted, false);
            if (SecondScaleButton.isChecked())
            {
                for (int n=0; n<Yraw.count(); n++)
                    Yraw[n] = Yraw[n] * s;
            }
            curve->setSamples(Xraw, Yraw);
		}
	}
	else
	{
		if (device->isDataLoading())
		{
			if (name.isEmpty()) curve->setTitle(RomID);
			else curve->setTitle(name + "  " + tr("Loading"));
		}
		else
		{
			if (name.isEmpty()) curve->setTitle(RomID);
			else curve->setTitle(name);
		}
	}
}



void GraphCurve::setFilled(int)
{
	setColor(Color);
}





void GraphCurve::setAliased(int)
{
#if QT_VERSION >= 0x040000
	if (AliasedCurveButton.isChecked()) curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
		else  curve->setRenderHint(QwtPlotItem::RenderAntialiased, false);
	setColor(Color);
#endif
}





void GraphCurve::switchSecondScale(int)
{
	reload();
}




void GraphCurve::secondScaleChanged()
{
	bool ok;
	QString txt = secondScaleStr.text();
	double v = txt.toDouble(&ok);
	if (ok)	secondScale = v;
	else
	{
		secondScale = 1;
		int pos = txt.indexOf("/");
		if (pos != -1)
		{
			QStringList p = txt.split("/");
			if (p.count() == 2)
			{
				bool okA, okB;
				double A = p.at(0).toDouble(&okA);
				double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) secondScale = A/B;
			}
		}
	}
	reload();
}




void GraphCurve::setStyle(int style)
{
	switch (style)
	{
		case 0 : curve->setStyle(QwtPlotCurve::Lines); break;
		case 1 : curve->setStyle(QwtPlotCurve::Sticks); break;
		case 2 : curve->setStyle(QwtPlotCurve::Steps); break;
		case 3 : curve->setStyle(QwtPlotCurve::Dots); break;
		default : curve->setStyle(QwtPlotCurve::Lines); break;
	}
}




void GraphCurve::getConfigStr(QString &str)
{
	str += New_Curve_Begin;
	str += "\n";
	str += logisdom::saveformat(RomIDMark, RomID);
	str += logisdom::saveformat(RedMark, QString("%1").arg(Color.red()));
	str += logisdom::saveformat(GreenMark, QString("%1").arg(Color.green()));
	str += logisdom::saveformat(BlueMark, QString("%1").arg(Color.blue()));
	str += logisdom::saveformat("Filled", QString("%1").arg(FillCurveButton.isChecked()));
	str += logisdom::saveformat("Alliased", QString("%1").arg(AliasedCurveButton.isChecked()));
	str += logisdom::saveformat("Style", QString("%1").arg(StyleButton.currentIndex()));
	str += logisdom::saveformat("SecondScaleSelected", QString("%1").arg(SecondScaleButton.isChecked()));
	str += logisdom::saveformat("SecondScaleValue", secondScaleStr.text());
	str += New_Curve_Finished;
	str += "\n";
}




void GraphCurve::setCurveConfig(QString &description)
{
	int red, green, blue, v;
	bool okr, okg, okb, okv;
	red = logisdom::getvalue(RedMark, description).toInt(&okr);
	green = logisdom::getvalue(GreenMark, description).toInt(&okg);
	blue = logisdom::getvalue(BlueMark, description).toInt(&okb);
	if (okr && okg && okb)
	{
		QColor color(red, green, blue);
		setColor(color);
	}
	v = logisdom::getvalue("Filled", description).toInt(&okv);
	if (okv)
	{
		if (v) FillCurveButton.setCheckState(Qt::Checked);
		else FillCurveButton.setCheckState(Qt::Unchecked);
	}
	else FillCurveButton.setCheckState(Qt::Checked);
	v = logisdom::getvalue("Alliased", description).toInt(&okv);
	if (okv)
	{
		if (v) AliasedCurveButton.setCheckState(Qt::Checked);
		else AliasedCurveButton.setCheckState(Qt::Unchecked);
	}
	else AliasedCurveButton.setCheckState(Qt::Unchecked);
	v = logisdom::getvalue("Style", description).toInt(&okv);
	if (okv) StyleButton.setCurrentIndex(v);
	v = logisdom::getvalue("SecondScaleSelected", description).toInt(&okv);
	if (okv)
	{
		if (v) SecondScaleButton.setCheckState(Qt::Checked);
		else SecondScaleButton.setCheckState(Qt::Unchecked);
	}
	else SecondScaleButton.setCheckState(Qt::Unchecked);
	QString sc = logisdom::getvalue("SecondScaleValue", description);
	if (!sc.isEmpty()) secondScaleStr.setText(sc);
	secondScaleChanged();
}







void GraphCurve::removesidedata()
{
    Xcompressed.clear();
    Ycompressed.clear();
	double dif = (XFin - XDebut) / 10;
	for (int n=0; n<Xraw.count(); n++)
		if ((Xraw[n]>=XDebut - dif) && (Xraw[n]<=XFin + dif))
		{
			Xcompressed.append(Xraw[n]);
			Ycompressed.append(Yraw[n]);
		}
}






void GraphCurve::scaleData()
{
    double s = secondScale; //secondScale.value();
	for (int n=0; n<Ycompressed.count(); n++)
        Ycompressed[n] = Ycompressed[n] * s;
}







void GraphCurve::compressdata()
{
    QVector <double> XTemp;
    QVector <double> YTemp;
    for (int n=0; n<Xraw.count(); n++)
	{
			XTemp.append(Xraw[n]);
			YTemp.append(Yraw[n]);
	}
    Xcompressed.clear();
	Ycompressed.clear();
	int count = XTemp.count();
	int div = 1;
	if (count > dotspercurve) div = count / dotspercurve;
	if ((!XTemp.isEmpty()) and (!YTemp.isEmpty()))
	{
		Xcompressed.append(XTemp[0]);
		Ycompressed.append(YTemp[0]);
	}
	for (int n=1; n<(count - div - 1); n+=div)
        if ((logisdom::AreNotSame(YTemp[n],Ycompressed.last())) && (logisdom::AreNotSame(YTemp[n], YTemp[n + div])))
		{
			Xcompressed.append(XTemp[n]);
			Ycompressed.append(YTemp[n]);
		}
	if ((XTemp.count() > 2) and (YTemp.count() > 2))
	{
		Xcompressed.append(XTemp.last());
		Ycompressed.append(YTemp.last());
    }
}







void GraphCurve::setColor(const QColor &color)
{
#if QT_VERSION >= 0x040000
	QColor c = color;
	curve->setPen(color);
    c.setAlpha(50);
    if (FillCurveButton.isChecked()) curve->setBrush(c);
		else curve->setBrush(Qt::NoBrush);
	// curve->setBaseline (-20);
#else
	curve->setPen(color);
	if (filled) curve->setBrush(QBrush(color, Qt::SolidPattern));
		else curve->setBrush(Qt::NoBrush);
#endif
	Color = color;
}




void GraphCurve::AddData(double v, const QDateTime &time, QString romid)
{
	QMutexLocker locker(&mutex);
	if (RomID == romid)
	{
		qreal x, y;
		y = v;
        x = qreal(Origin.secsTo(time));
        x /=qreal(SecsInDays);
        if (Xraw.isEmpty())
        {
            Xraw.append(x);
            Yraw.append(y);
        }
        else if (Xraw.last() < x)
		{
			Xraw.append(x);
			Yraw.append(y);
        }
	}
}





void GraphCurve::setromid(QString ROMID)
{
	RomID = ROMID;
}



QString GraphCurve::getRomID()
{
	return RomID;
}


