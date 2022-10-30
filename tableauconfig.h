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



#ifndef TABLEAUCONFIG_H
#define TABLEAUCONFIG_H

#include <QtGui>
#include "tableau.h"
#include "onewire.h"


class logisdom;



class tableauconfig : public QWidget
{
	Q_OBJECT
public:	
	tableauconfig(logisdom *Parent);
	~tableauconfig();
	logisdom *parent;
    htmlBinder *htmlBind;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton TableauTool;
	QPushButton AddButton;
	QPushButton ShowButton;
	QPushButton RenameButton;
	QPushButton RemoveButton;
	QListWidget tableauList;
	QWidget *previousSetup;
	void setPalette(QWidget *tableausetup);
	void updateList();
	tableau *nouveautableau(QString name);
	QList <tableau*> tableauPtArray;
    void readconfigfile(const QString &configdata);
	void SaveConfigStr(QString &str);
	void saveTableau();
	void savePreload();
    tableau *chooseTableau();
    tableau *getTableau(const QString &name);
private:
public slots:
	void rename();
	void nouveautableau();
	void DeviceConfigChanged(onewiredevice *);
private slots:
	void connectAll();
	void disconnectAll();
	void voir();
	void removetableau();
	void setPalette();
	void CurveRowChanged(int tableau);
	void CurveRowChanged(QWidget *tableau);
signals:
};

#endif
