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



#ifndef TABLEAU_H
#define TABLEAU_H

#include <QtGui>
#include <QtWidgets/QTableWidget>
#include "onewire.h"
#include "interval.h"
#include "ui_tableau.h"


class logisdom;
class QwtIntervalSample;

class tableausetup : public QWidget
{
public:
    Ui::tableau_ui ui;
    interval saveInterval;
    QGridLayout layout_interval;
    tableausetup()
    {
        ui.setupUi(this);
        ui.tabWidget->widget(1)->setLayout(&layout_interval);
        layout_interval.addWidget(&saveInterval);
    }
};






class tableau : public QWidget
{
	Q_OBJECT
#define DateTimeWidth 2
public:	
	tableau(QString *Name, logisdom *Parent);
	~tableau();
    logisdom *parent;
    QGridLayout *layout;
	QTableWidget table;
    tableausetup ui;
    bool isRefilling;
// Palette setup
	QWidget setup;
	QString name;
    QString StartButtonName;
	QGridLayout setupLayout;
	QStringList tableauDeviceList;
	QListWidgetItem ** widgetList;
	QWidget *previousSetup;
	void setPalette(QWidget *graphsetup);
	void getTableauConfig(QString &str);
	void setTableauConfig(QString &str);
	QString getName();
	void setName(QString Name);
	void addDevice(QString romid);
	void save();
	void checkPreload();
    void getData(const QDateTime Origin, QVector <QwtIntervalSample>&);
private:
	bool IsRomIDthere(QString RomID);
	void mousePressEvent(QMouseEvent *event);
    void rebuild();
    void saveAll(QFile &file);
    QString getFileName();
public slots:
	void updateName(onewiredevice *device);
private slots:
	void browse();
	void savenow();
    void refill();
    void addDevice();
	void removeDevice();
    void clearTable();
    void clickEvent();
	void clickEvent(QModelIndex);
signals:
	void setupClick(QWidget*);
    void updateChart();
};

#endif
