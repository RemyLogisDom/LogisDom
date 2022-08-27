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



#ifndef HISTO_H
#define HISTO_H
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
#include "qwt_plot_seriesitem.h"
#include <qwt_legend.h>
#include <qwt_plot_legenditem.h>
#include "qwt_painter.h"
#include "qpainter.h"

class QwtPlotCurve;
class QCheckBox;
class QTextStream;
class QDateTime;
class tableau;

struct l_Data
{
    QVector <QwtIntervalSample> myData;
};


class LegendItem: public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );

        QColor color( Qt::white );

        setTextPen( color );
#if 1
        setBorderPen( color );

        QColor c( Qt::gray );
        c.setAlpha( 200 );

        setBackgroundBrush( c );
#endif
    }
};



class myHisto : public QwtPlotHistogram
{
public:
    QVector <l_Data> datas;
    myHisto(const QString title) : QwtPlotHistogram(title)
    {
    }
    void setSample(int, QVector <QwtIntervalSample> &)
    {
    }
    void updateLegend(QwtLegend *) const
    {
#if QT_VERSION < 0x050000
        //QwtPlotHistogram::updateLegend();
        //QwtLegendItem *lgdItem = dynamic_cast<QwtLegendItem*>(legend->find(this));
        // ici if ( lgdItem ) lgdItem->setIdentifierSize(QSize(20,15));
        //QwtPlotItem::updateLegend(legend);
#endif
    }
    void drawSeries( QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &, int from, int to ) const
    {
        if ( !painter || dataSize() <= 0 ) return;

        if ( to < 0 ) to = dataSize() - 1;

        switch ( style() )
        {
            case Outline:
                drawOutline( painter, xMap, yMap, from, to );
                break;
            case Lines:
                drawLines( painter, xMap, yMap, from, to );
                break;
            case Columns:
                drawColumns( painter, xMap, yMap, from, to );
                break;
            case UserStyle:
                drawUserStyle( painter, xMap, yMap, from, to );
               break;
            default:
                break;
        }
    }
    void drawUserStyle( QPainter *painter, const QwtScaleMap &, const QwtScaleMap &, int from, int to ) const
    {
        painter->setPen(pen());
        painter->setBrush(brush());

        for ( int i = from; i <= to; i++ )
        {
            /* ici const QwtIntervalSample sample = d_series->sample( i );
            if ( !sample.interval.isnullptr() )
            {
                const QwtColumnRect rect = columnRect( sample, xMap, yMap );
                drawColumn( painter, rect, sample );
            }*/
        }
    }
};



class GraphHisto : public QWidget
{
    Q_OBJECT
public:
    GraphHisto(tableau *Chart);
    ~GraphHisto();
    tableau *chart;
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
    myHisto *histo;
    QDateTime Origin;
    int beginOffset, finishOffset, lastBeginOffset, lastFinishOffset;
    void setHistoConfig(QString &description);
private:
    QMutex mutex;
private slots:
    void curverezise(int);
    void colorChange();
    void setAliased(int);
    void switchSecondScale(int);
    void secondScaleChanged();
    void setFilled(int);
    void setStyle(int);
public slots:
    void updateChart();
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
    void updateLegend(qint32 T, int state);
};



#endif


