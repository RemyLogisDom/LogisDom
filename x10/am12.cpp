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






#include "globalvar.h"
#include "server.h"
#include "remote.h"
#include "connection.h"
#include "am12.h"

am12::am12(net1wire *Master, logisdom *Parent, QString RomID) : devx10(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 2, 1, 1, 1);
	ButtonOn.setText(cstr::toStr(cstr::ON));
	ButtonOff.setText(cstr::toStr(cstr::OFF));
	setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
	htmlBind->addCommand(cstr::toStr(cstr::ON), LM12ON);
	htmlBind->addCommand(cstr::toStr(cstr::OFF), LM12OFF);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), LM12ON);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), LM12OFF);
	lastsavevalue = logisdom::NA;
	HouseCode = 1;
	ModuleCode = 1;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	ui.labelfamily->setText("AM12 X10 Switch");	
	if (master) ui.labelmaster->setText(master->getipaddress());
	ui.labelcode->setText(QChar(HouseCode + 64) + QString("%1").arg(ModuleCode));
	connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(clickX10On()));
	connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickX10On()));
	connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(clickX10Off()));
	connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(clickX10Off()));
	setname("");
	MainValue = logisdom::NA;
	if (parent->isRemoteMode())
		if (!parent->RemoteConnection->isAdmin())
		{
			ui.pushButtonON->setEnabled(false);
			ui.pushButtonOFF->setEnabled(false);
		}
}





void am12::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction ActionON(cstr::toStr(cstr::ON), this);
	QAction ActionOFF(cstr::toStr(cstr::OFF), this);
	QAction Nom(tr("&Name"), this);
	QAction SetMode(tr("&Extended Mode"), this);
	SetMode.setCheckable(true);
	SetMode.setChecked(ExtendedMode);
	QAction SetCode(tr("&Set Code"), this);
	contextualmenu.addAction(&ActionON);
	contextualmenu.addAction(&ActionOFF);
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
	if (selection == &Nom) changename();
	if (selection == &SetCode) setCode();
	ExtendedMode = SetMode.isChecked();
}





void am12::SetOrder(const QString &order)
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





void am12::remoteCommandExtra(const QString &command)
{
	if (command == LM12ON) clickX10On();
	if (command == LM12OFF) clickX10Off();
}



void am12::setHMCaption()
{
	QString str = tr("Status : ");
	ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
	if (MainValue == logisdom::NA) str += cstr::toStr(cstr::NA);
	else if (MainValue == 0) str += QString(cstr::toStr(cstr::OFF));
	else if (MainValue == 100) str += QString(cstr::toStr(cstr::ON));
	else str += QString("%1 %").arg(MainValue, 0, 'f', 0);
	ui.labelStatus->setText(str);
}



void am12::clickX10On()
{
	On(true);
	emit(stateChanged());
}



void am12::clickX10Off()
{
	Off(true);
	emit(stateChanged());
}



