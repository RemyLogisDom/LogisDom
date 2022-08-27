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



#ifndef devpclbus_H
#define devpclbus_H
#include "interval.h"
#include "ui_devplcbus.h"
#include "onewire.h"


class devpclbus : public onewiredevice
{
	Q_OBJECT
#define PCLDEVON "PCLDEVON"
#define PCLDEVOFF "PCLDEVOFF"
public:
	devpclbus(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode);
	bool isSwitchFamily();
	bool isDimmmable();
	void setHouseCode(int House, int Module);
	void setHouseCode(QString House, QString Module);
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
    void SetOrder(const QString &order);
// Palette setup
    QPushButton ButtonOn, ButtonOff;
private:
	Ui::devpclbusui ui;
	QString dailyPrg;
	void setCode();
private slots:
	void clickOn();
	void clickOff();
	void On(bool emitPCL);
	void Off(bool emitPCL);
signals:
	void stateChanged();
};


#endif
