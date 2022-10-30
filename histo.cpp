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
#include <QBrush>
#include <QDateTime>
#include <QTextStream>
#include <QCheckBox>
#include <QtGui>
#include <qwt_plot_curve.h>
#include <qwt_plot_item.h>
#include "globalvar.h"
#include "onewire.h"
#include "formula.h"
#include "configwindow.h"
#include "remote.h"
#include "histo.h"
#include "tableau.h"
#include "messagebox.h"





GraphHisto::GraphHisto(tableau *Chart)
{
    chart = Chart;
    RomID = chart->name;
    histo = new myHisto(chart->name);
    beginOffset = 0;
    finishOffset = 0;
    lastBeginOffset = 0;
    lastFinishOffset = 0;
    Origin.setDate(QDate(yearscalebegin, 1, 1));
    Origin.setTime(QTime(0, 0));
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
    //connect(&secondScaleStr, SIGNAL(editingFinished()), this, SLOT(secondScaleChanged()));
    SecondScaleButton.setText(tr("Second scale"));
    //setupLayout.addWidget(&SecondScaleButton);
    connect(&SecondScaleButton, SIGNAL(stateChanged(int)), this, SLOT(switchSecondScale(int)));
    secondScaleStr.setToolTip(tr("Place second scale here\nYou can use real like 2.5 or fraction like 1/10"));
    //setupLayout.addWidget(&secondScaleStr);
    StyleButton.addItem(tr("Outline"));
    StyleButton.addItem(tr("Columns "));
    StyleButton.addItem(tr("Lines "));
    StyleButton.addItem(tr("UserStyle "));
    setupLayout.addWidget(&StyleButton);
    Origin = QDateTime(QDate(yearscalebegin, 1, 1), QTime(0, 0, 0, 0));
    connect(&StyleButton, SIGNAL(currentIndexChanged(int)), this, SLOT(setStyle(int)));
}





GraphHisto::~GraphHisto()
{
    delete histo;
}




void GraphHisto::updateLegend(qint32 T, int state)
{
    onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
    if (device)
    {
        QString name = device->getname();
        bool deviceLoading;
        double V = device->getMainValue(T, deviceLoading, nullptr);
        if (device->isDataLoading())
        {
            if (name.isEmpty()) histo->setTitle(RomID);
            else histo->setTitle(name + "  " + tr("Loading"));
            return;
        }
        else
        {
            if (logisdom::isNotNA(V))
            {
                if ((state & Qt::ControlModifier) && (state & Qt::ShiftModifier))
                {
                    QString ValStr = device->ValueToStr(V);
                    histo->setTitle(ValStr);
                    return;
                }
                else if (state & Qt::ControlModifier)
                {
                    QString ValStr = device->ValueToStr(V);
                    if (name.isEmpty()) histo->setTitle(RomID + " = " + ValStr);
                    else histo->setTitle(name  + " = " + ValStr);
                    return;
                }
            }
        }
        //QwtText title;
        //title.setColor(plot.histo[n]->Color);
        //title.setText(Title);
        //if (state & Qt::ControlModifier)
        //{
        //}
        if (name.isEmpty()) histo->setTitle(RomID);
        else
        {
            if ((secondScale != 1) && SecondScaleButton.isChecked()) name.append("\nx " + secondScaleStr.text());
            histo->setTitle(name);
        }
    }
}


void GraphHisto::curverezise(int)
{
}




void GraphHisto::colorChange()
{
    QColor currentcolor = QColorDialog::getColor();
    if (!currentcolor.isValid()) return;
    setColor(currentcolor);
}




void GraphHisto::reload()
{
    onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
    if (device) device->clearData();
}





void  GraphHisto::update(const QDateTime &begin, const QDateTime &finish, bool compress)
{
    QMutexLocker locker(&mutex);
    compressed = compress;
    onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
    if (!device) return;
    QString name = device->getname();
    qint32 b = Origin.secsTo(begin);
    qint32 e = Origin.secsTo(finish);
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
            Xraw.append(x);
            Yraw.append(data.data_Y.at(n));
        }
        double s = secondScale;
        if (compress)
        {
            compressdata();
            if (SecondScaleButton.isChecked())
                for (int n=0; n<Ycompressed.count(); n++)
                    Ycompressed[n] = Ycompressed[n] * s;
            //histo->setSamples(Xcompressed, Ycompressed);
        }
        else
        {
            if (SecondScaleButton.isChecked())
                for (int n=0; n<Yraw.count(); n++)
                    Yraw[n] = Yraw[n] * s;
            //histo->setSamples(Xraw, Yraw);
        }
    }
    else
    {
        if (device->isDataLoading())
        {
            if (name.isEmpty()) histo->setTitle(RomID);
            else histo->setTitle(name + "  " + tr("Loading"));
        }
        else
        {
            if (name.isEmpty()) histo->setTitle(RomID);
            else histo->setTitle(name);
        }
    }
}





void GraphHisto::setFilled(int)
{
    setColor(Color);
}





void GraphHisto::setAliased(int)
{
#if QT_VERSION >= 0x040000
    if (AliasedCurveButton.isChecked()) histo->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        else  histo->setRenderHint(QwtPlotItem::RenderAntialiased, false);
    setColor(Color);
#endif
}





void GraphHisto::switchSecondScale(int)
{
    reload();
}




void GraphHisto::secondScaleChanged()
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
                if (okA && okB && (B != 0)) secondScale = A/B;
            }
        }
    }
    reload();
}




void GraphHisto::setStyle(int style)
{
    switch (style)
    {
        case 0 : histo->setStyle(QwtPlotHistogram::Outline); break;
        case 1 : histo->setStyle(QwtPlotHistogram::Columns); break;
        case 2 : histo->setStyle(QwtPlotHistogram::Lines); break;
        case 3 : histo->setStyle(QwtPlotHistogram::UserStyle); break;
        default : histo->setStyle(QwtPlotHistogram::Outline); break;
    }
}



void GraphHisto::updateChart()
{
    QVector <QwtIntervalSample> data;
    chart->getData(Origin, data);
    histo->setSamples(data);
}




void GraphHisto::getConfigStr(QString &str)
{
    str += New_Histo_Begin;
    str += "\n";
    str += logisdom::saveformat(RomIDMark, chart->name);
    str += logisdom::saveformat(RedMark, QString("%1").arg(Color.red()));
    str += logisdom::saveformat(GreenMark, QString("%1").arg(Color.green()));
    str += logisdom::saveformat(BlueMark, QString("%1").arg(Color.blue()));
    str += logisdom::saveformat("Filled", QString("%1").arg(FillCurveButton.isChecked()));
    str += logisdom::saveformat("Alliased", QString("%1").arg(AliasedCurveButton.isChecked()));
    str += logisdom::saveformat("Style", QString("%1").arg(StyleButton.currentIndex()));
    str += logisdom::saveformat("SecondScaleSelected", QString("%1").arg(SecondScaleButton.isChecked()));
    str += logisdom::saveformat("SecondScaleValue", secondScaleStr.text());
    str += New_Histo_Finished;
    str += "\n";
}




void GraphHisto::setHistoConfig(QString &description)
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







void GraphHisto::removesidedata()
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






void GraphHisto::scaleData()
{
    double s = secondScale; //secondScale.value();
    for (int n=0; n<Ycompressed.count(); n++)
        Ycompressed[n] = Ycompressed[n] * s;
}







void GraphHisto::compressdata()
{
    QList <qreal> XTemp;
    QList <qreal> YTemp;
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
        if ((YTemp[n] != Ycompressed.last()) && (YTemp[n] != YTemp[n + div]))
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







void GraphHisto::setColor(const QColor &color)
{
#if QT_VERSION >= 0x040000
    QColor c = color;
    histo->setPen(color);
    c.setAlpha(50);
    if (FillCurveButton.isChecked()) histo->setBrush(c);
        else histo->setBrush(Qt::NoBrush);
    // curve->setBaseline (-20);
#else
    curve->setPen(color);
    if (filled) histo->setBrush(QBrush(color, Qt::SolidPattern));
        else histo->setBrush(Qt::NoBrush);
#endif
    Color = color;
}




void GraphHisto::AddData(double v, const QDateTime &time, QString romid)
{
    QMutexLocker locker(&mutex);
    if (RomID == romid)
    {
        qreal x, y;
        y = v;
        x = (qreal) Origin.secsTo(time);
        x /=(qreal) SecsInDays;
//		if ((Yraw.count()-1 == y) and (Yraw.count()-2 == y))
//		{
//			Xraw[Xraw.count()-1] = x;
//		}
//		else
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





void GraphHisto::setromid(QString ROMID)
{
    RomID = ROMID;
}



QString GraphHisto::getRomID()
{
    return RomID;
}


