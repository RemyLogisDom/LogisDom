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



#ifndef ds2406_H
#define ds2406_H
#include "onewire.h"
#include "ui_ds2408.h"

class ds2406 : public onewiredevice
{
	Q_OBJECT
public:
	ds2406(net1wire *Master, logisdom *Parent, QString RomID);
	enum Registers { LS_Reg, OLSR_Reg };
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
    void SetOrder(const QString &order);
    bool isDimmmable();
    QString getOffCommand();
	QString getOnCommand();
	bool isSwitchFamily();
    void assignMainValueLocal(double value);
    double getMaxValue();
    bool dual_Ouput;
// Palette setup
    QPushButton B0, B1, B2, B3;
    QCheckBox operateDecimal;
    QPushButton ButtonOn, ButtonOff;
private:
	Ui::ds2408ui ui;
	QString family4;
	int OLSR;
	QString LogicalStatusTxt, OpenDrainTxt, StatusTxt;
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
	void On(bool send);
	void Off(bool send);
    void B0Click();
    void B1Click();
    void B2Click();
    void B3Click();
    void opDecChanged(int state);
private slots:
	void inputClick();
	void outputClick();
};


#endif
