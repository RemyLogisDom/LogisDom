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



#ifndef lm12_H
#define lm12_H
#include "devx10.h"
#include "ui_lm12.h"


class lm12 : public devx10
{
	Q_OBJECT
#define AM12ON "AM12ON"
#define AM12OFF "AM12OFF"
public:
	lm12(net1wire *Master, logisdom *Parent, QString RomID);
	void setHMCaption();
    void SetOrder(const QString &order);
	QPushButton ButtonOn, ButtonOff, ButtonDim, ButtonBrigth;
	bool isDimmmable();
private:
#define X10SleepStart 22
	Ui::lm12ui ui;
	QTimer X10SleepTimer;
public slots:
	virtual void contextMenuEvent(QContextMenuEvent * event);
private slots:
	void clickX10On();
	void clickX10Off();
	void clickX10Dim();
	void clickX10Brigth();
	void X10Sleep();
	void X10SleepStep();
};


#endif
