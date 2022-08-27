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



#ifndef GRAPH_H
#define GRAPH_H
#include "globalvar.h"
#include "plot.h"


class logisdom;
class htmlBinder;



class DeleteHighlightedItemWhenShiftDelPressedEventFilter : public QObject
{
     Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};


class CustomScaleDraw : public QwtScaleDraw
{
public:
QString Format;
    CustomScaleDraw(QString format, int rotation = 0)
	{
		Format = format;
		setLabelAlignment(Qt::AlignRight  | Qt::AlignVCenter);
        setLabelRotation(double(rotation));
	}
	virtual QwtText label(double v) const
	{
		v = v * SecsInDays;
#if QT_VERSION > 0x050603
        QString str = QDateTime::fromSecsSinceEpoch(qint64(v)).toString(Format);
#else
        QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
        QString str = origin.addSecs(qint64(v)).toString(Format);
#endif
        return str;
	}
};



class graph : public QWidget
{
	Q_OBJECT
    friend class Plot;
public:
enum Unit
	{
		Minutes, Heures, Jours, Mois
	};
	graph(logisdom *Parent, QString &Name);
	~graph();
	Plot plot;
    double scaleDiv = 1;
	logisdom *parent;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton CurveTool;
	QPushButton AddCurveButton;
    QPushButton AddHistoButton;
    QPushButton RemoveCurveButton;
    QListWidget curveList;
    QCheckBox ContinousUpdate;
	QCheckBox showLegend;
	QCheckBox manualScale;
	QLabel MaxText, MinText;
	QLineEdit MaxVal, MinVal;
	void setPalette(QWidget *curvesetup);
	QWidget *previousSetup;
	void updateList();
	void updategraph(QDateTime begin, QDateTime end);
	QPushButton ReloadButton;
    QPushButton UpdateButton;
    QPushButton RemoveScaleButton;
	QCheckBox CompressBox;
    void setPeriode(int Periode);
	void setUnit(int Unit);
	QString getName();
	void setName(QString name);
	void getCfgStr(QString &str);
	void setCfgStr(QString &strsearch);
	void setCurveTitle(QString romID, QString Title);
	void delcurve(QString RomID);
	bool IsRomIDthere(QString RomID);
	void AddData(double v, const QDateTime &time, QString RomID);
	void addcurve(QString name, QString RomID, QColor color);
    void addhisto(QString name, QString RomID, QColor color);
    bool isCurveAlone(QString romID);
	void setScale();
	void mousePressEvent(QMouseEvent *event);
    void updategraph(bool manual = false);
private:
//	void contextMenuEvent(QContextMenuEvent *event);
	int layoutIndex;
	void connectAll();
	void disconnectAll();
	QMutex mutex;
	QDateTime Origin, Begin, End;
	QGridLayout *layout;
	QString Name;
	long int periode;
	int unit;
	double offset;
	QComboBox HUnit;
	QSpinBox HScale;
	QDoubleSpinBox HOffset;
	QComboBox tickFormat;
	QSpinBox rotation;
	QLabel busyLabel;
	bool reloading;
	void waitSign(bool wait = false);
	bool busy;
    bool connected;
    QTimer toolTimer;
public slots:
    void zoomUpdate();
	void updateAxis(QString);
	void reload();
private slots:
    void ToolsOn();
    void ToolsOff();
    void updateYaxisScale();
	void toggleManual(int);
	void toggleCompress(int);
    void showLegendChanged(int);
	void removeScaleLegend();
    void CurveRowChanged(int curve);
	void unitchange(int Unit);
	void scalechange(int Scale);
	void newscale(int Scale);
	void newscale(QString Scale);
	void updateAxis(int);
    void offsetchange(double Offset);
	void addcurve();
    void addhisto();
    void removecurve();
	void loadingFinished();
    void UpdateClick();
signals:
    void setupClick(QWidget*);
};

#endif
