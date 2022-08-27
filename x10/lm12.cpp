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




#include "globalvar.h"
#include "remote.h"
#include "server.h"
#include "lm12.h"


lm12::lm12(net1wire *Master, logisdom *Parent, QString RomID) : devx10(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 2, 2, 1, 1);
	ButtonOn.setText(cstr::toStr(cstr::ON));
	ButtonOff.setText(cstr::toStr(cstr::OFF));
	ButtonDim.setText(cstr::toStr(cstr::Dim));
	ButtonBrigth.setText(cstr::toStr(cstr::Brigth));
	setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
	setupLayout.addWidget(&ButtonDim, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&ButtonBrigth, layoutIndex++, 1, 1, 1);
	htmlBind->addCommand(cstr::toStr(cstr::ON), AM12ON);
	htmlBind->addCommand(cstr::toStr(cstr::OFF), AM12OFF);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), AM12ON);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), AM12OFF);
	lastsavevalue = logisdom::NA;
	HouseCode = 0;
	ModuleCode = 0;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	ui.labelfamily->setText("LM12 X10 Variator");	
	if (master) ui.labelmaster->setText(master->getipaddress());
	ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
	X10SleepCount = X10SleepStart;
	X10SleepTimer.setInterval(30000);
	ui.pushButtonSLEEP->setText(cstr::toStr(cstr::Sleep));
	connect(&X10SleepTimer, SIGNAL(timeout()), this, SLOT(X10SleepStep()));
	connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(clickX10On()));
	connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickX10On()));
	connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(clickX10Off()));
	connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(clickX10Off()));
	connect(ui.pushButtonBRIGTH, SIGNAL(clicked()), this, SLOT(clickX10Brigth()));
	connect(&ButtonBrigth, SIGNAL(clicked()), this, SLOT(clickX10Brigth()));
	connect(ui.pushButtonDIM, SIGNAL(clicked()), this, SLOT(clickX10Dim()));
	connect(&ButtonDim, SIGNAL(clicked()), this, SLOT(clickX10Dim()));
	connect(ui.pushButtonSLEEP, SIGNAL(clicked()), this, SLOT(X10Sleep()));
	setname("");
	MainValue = logisdom::NA;
	if (parent->isRemoteMode())
		if (!parent->RemoteConnection->isAdmin())
		{
			ui.pushButtonON->setEnabled(false);
			ui.pushButtonOFF->setEnabled(false);
			ui.pushButtonBRIGTH->setEnabled(false);
			ui.pushButtonDIM->setEnabled(false);
			ui.pushButtonSLEEP->setEnabled(false);
		}
}





void lm12::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction ActionON(cstr::toStr(cstr::ON), this);
	QAction ActionOFF(cstr::toStr(cstr::OFF), this);
	QAction ActionDIM(cstr::toStr(cstr::Dim), this);
	QAction ActionBRIGTH(cstr::toStr(cstr::Brigth), this);
	QAction Nom(tr("&Name"), this);
	QAction SetMode(tr("&Extended Mode"), this);
	SetMode.setCheckable(true);
	SetMode.setChecked(ExtendedMode);
	QAction SetCode(tr("&Set Code"), this);
	contextualmenu.addAction(&ActionON);
	contextualmenu.addAction(&ActionOFF);
	contextualmenu.addAction(&ActionDIM);
	contextualmenu.addAction(&ActionBRIGTH);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
		contextualmenu.addAction(&SetMode);	
		contextualmenu.addAction(&SetCode);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &ActionON) On(true);
	if (selection == &ActionOFF) Off(true);
	if (selection == &ActionDIM) Dim(true);
	if (selection == &ActionBRIGTH) Bright(true);
	if (selection == &Nom) changename();
	if (selection == &SetCode) setCode();
	ExtendedMode = SetMode.isChecked();
}






void lm12::SetOrder(const QString &order)
{
	if (order == NetRequestMsg[SwitchON])
	{
		On(true);
		emit(stateChanged());
	}
	if (order == NetRequestMsg[SwitchOFF])
	{
		Off(true);
		emit(stateChanged());
	}
}



void lm12::X10Sleep()
{
	X10SleepCount = X10SleepStart;
	if (ui.pushButtonSLEEP->text() == cstr::toStr(cstr::Sleep))
	{
		ui.pushButtonSLEEP->setText(cstr::toStr(cstr::Cancel));
		X10SleepTimer.start();
	}
	else
	{
		X10SleepTimer.stop();
		ui.pushButtonSLEEP->setText(cstr::toStr(cstr::Sleep));
		Bright(22);
	}
}





void lm12::X10SleepStep()
{
	if (X10SleepCount != 0)
	{
		X10SleepCount --;
		Dim(true);
	}
	else
	{
		X10SleepTimer.stop();
		ui.pushButtonSLEEP->setText(cstr::toStr(cstr::Sleep));
		Off(true);
	}
}




void lm12::setHMCaption()
{
	QString str;
	ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
	if (MainValue == logisdom::NA) str = cstr::toStr(cstr::NA);
	else str = QString("%1 %").arg(MainValue, 0, 'f', 0);
	ui.pushButtonPRC->setText(str);
}



void lm12::clickX10On()
{
	On(true);
	emit(stateChanged());
}


void lm12::clickX10Off()
{
	Off(true);
	emit(stateChanged());
}


void lm12::clickX10Dim()
{
	Dim(true);
	emit(stateChanged());
}


void lm12::clickX10Brigth()
{
	Bright(true);
	emit(stateChanged());
}



bool lm12::isDimmmable()
{
	return true;
}
