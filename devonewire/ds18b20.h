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



#ifndef ds18b20_H
#define ds18b20_H
#include "axb.h"
#include "onewire.h"
#include "ui_ds18b20.h"

class ds18b20 : public onewiredevice
{
	Q_OBJECT
public:
	ds18b20(net1wire *Master, logisdom *Parent, QString RomID);
	bool isTempFamily();
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	void writescratchpad();
    double calcultemperature(const QString &THex);
	void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
    void SetOrder(const QString &order);
	QPushButton ResolutionButton;
	QString textResolution;
// Palette setup
    //axb AXplusB;
private:
	Ui::ds18b20ui ui;
	void changealarmebasse();
	void changealarmehaute();
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots :
	 void changresoultion();
};


#endif
