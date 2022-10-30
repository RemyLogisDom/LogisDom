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



#ifndef switchecogest_H
#define switchecogest_H
#include "onewire.h"
#include "ui_switchecogest.h"

class switchecogest : public onewiredevice
{
	Q_OBJECT
#define SwitchON "SwitchON"
#define SwitchOFF "SwitchOFF"
public:
	switchecogest(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
	QString family4;
	void lecture();
	void lecturerec();
	void setconfig(const QString &strsearch);
    void GetConfigStr(QString &str);
    void SetOrder(const QString &order);
	void remoteCommandExtra(const QString &command);
	void On(bool send);
	void Off(bool send);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
// Palette setup
    QPushButton ButtonOn, ButtonOff;
    QCheckBox StringValue;
private:
	Ui::switchecogest ui;
	bool isSwitchFamily();
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
};


#endif
