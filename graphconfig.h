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



#ifndef GRAPHCONFIG_H
#define GRAPHCONFIG_H

#include <QtGui>
#include "graph.h"
#include "histo.h"
#include "onewire.h"
#include "qwt_plot_curve.h"


class logisdom;
class graphconfig;
class QwtPlotPicker;



class graphconfig : public QWidget
{
	Q_OBJECT
public:	
	graphconfig(logisdom *Parent);
	~graphconfig();
	logisdom *parent;
	htmlBinder *htmlBind;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton GraphicTool;
	QPushButton AddButton;
	QPushButton ShowButton;
	QPushButton RenameButton;
	QPushButton RemoveButton;
	QListWidget graphList;
	QWidget *previousSetup;
	void setPalette(QWidget *graphsetup);
	void updateList();
	graph * nouveaugraph(QString name);
	QColor currentcolor;
	void updateGraphs();
	int update_counter;
	void raz_counter();
	QList <graph*> graphPtArray;
    void AddData(double v, const QDateTime &time, QString RomID);
    void readconfigfile(const QString &configdata);
	void SaveConfigStr(QString &str);
	void createGraph(onewiredevice *device);
private:
	QMutex mutexAddData;
public slots:
	void ReloadGraphs();
	void addcurve(graph *Selection, QString romid, QColor color);
	void rename();
	void nouveaugraph();
    void DeviceConfigChanged(onewiredevice*);
private slots:
	void connectAll();
	void disconnectAll();
	void voir();
	void removegraph();
	void setPalette();
	void setGraphPalette();
	void CurveRowChanged(int graph);
signals:
};

#endif
