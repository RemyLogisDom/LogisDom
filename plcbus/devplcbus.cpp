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
#include "net1wire.h"
#include "vmc.h"
#include "plcbus.h"
#include "remote.h"
#include "inputdialog.h"
#include "devplcbus.h"



devpclbus::devpclbus(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 2, 1, 1, 1);
	ButtonOn.setText(cstr::toStr(cstr::ON));
	ButtonOff.setText(cstr::toStr(cstr::OFF));
	setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
    htmlBind->addCommand(cstr::toStr(cstr::ON), PCLDEVON);
	htmlBind->addCommand(cstr::toStr(cstr::OFF), PCLDEVOFF);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), PCLDEVON);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), PCLDEVOFF);
	lastsavevalue = logisdom::NA;
	HouseCode = 0;
	ModuleCode = 0;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	ui.labelfamily->setText("PCLBUS");	
	if (master) ui.labelmaster->setText(master->getipaddress());
	ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
	connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(clickOn()));
	connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickOn()));
	connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(clickOff()));
	connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(clickOff()));
	connect(this, SIGNAL(stateChanged()), parent->SwitchArea , SLOT(updateStatus()));
    setname("");
	MainValue = logisdom::NA;
	if (parent->isRemoteMode())
		if (!parent->RemoteConnection->isAdmin())
		{
			ui.pushButtonON->setEnabled(false);
			ui.pushButtonOFF->setEnabled(false);
		}
}





void devpclbus::SetOrder(const QString &)
{
}




QString devpclbus::MainValueToStrLocal(const QString &str)
{
    ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
    QString S;
    if (str.isEmpty())
    {
        if (int(MainValue) == logisdom::NA) S = cstr::toStr(cstr::NA);
        else if (int(MainValue) == 0) S = QString(cstr::toStr(cstr::OFF));
        else if (int(MainValue) == 100) S = QString(cstr::toStr(cstr::ON));
        else S = QString("%1 %").arg(MainValue, 0, 'f', 0);
    }
    ui.labelStatus->setText("Status : " + S);
    return S;
}






void devpclbus::lecture()
{
	if (!master) return;
	switch (master->gettype())
	{
		case Ezl50_X10 : MainValueToStr();
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void devpclbus::lecturerec()
{
	if (!master) return;
	switch (master->gettype())
	{
		case Ezl50_X10 : MainValueToStr();
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}




void devpclbus::clickOn()
{
	On(true);
	emit(stateChanged());
}



void devpclbus::clickOff()
{
	Off(true);
	emit(stateChanged());
}




void devpclbus::On(bool emitPCL)
{
	QString empty = "";
	if (emitPCL)
	{
		// Unit Code
		QString Code = QString("%1").arg((HouseCode * 16 + ModuleCode), 2, 16, QChar('0')).toUpper();
		// Command
		if (ui.checkBoxAck->isChecked())
		{
			 Code += QString("%1").arg(plcbus::PLC_ON + plcbus::ACK_PULSE, 2, 16, QChar('0')).toUpper();
			// Data 0000
			Code += "0000";
			if (master) master->addtofifo(PCLBUSCommand, empty, Code);
		}
		else
		{
			Code += QString("%1").arg(plcbus::PLC_ON, 2, 16, QChar('0')).toUpper();
			// Data 0000
			Code += "0000";
			if (master) master->addtofifo(PCLBUSCommand, empty, Code);
			setMainValue(100, true);
		}
	}
	else
	{
		setMainValue(100, true);
	}
}





void devpclbus::Off(bool emitPCL)
{
	QString empty = "";
	if (emitPCL)
	{
		// Unit Code
		QString Code = QString("%1").arg((HouseCode * 16 + ModuleCode), 2, 16, QChar('0')).toUpper();
		// Command
		if (ui.checkBoxAck->isChecked())
		{
			Code += QString("%1").arg(plcbus::PLC_OFF + plcbus::ACK_PULSE, 2, 16, QChar('0')).toUpper();
			// Data 0000
			Code += "0000";
			if (master) master->addtofifo(PCLBUSCommand, empty, Code);
		}
		else
		{
			Code += QString("%1").arg(plcbus::PLC_OFF, 2, 16, QChar('0')).toUpper();
			// Data 0000
			Code += "0000";
			if (master) master->addtofifo(PCLBUSCommand, empty, Code);
			setMainValue(0, true);
		}
	}
	else
	{
		setMainValue(0, true);
	}
}





bool devpclbus::isSwitchFamily()
{
	return true;
}



bool devpclbus::isDimmmable()
{
	return ui.checkBoxDimmable->isChecked();
}


bool devpclbus::setscratchpad(const QString&, bool)
{
	return false;
}




void devpclbus::setconfig(const QString &strsearch)
{
	QString Name, H, M;
	Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Lamp ")));
	H = logisdom::getvalue("HouseCode", strsearch);
	M = logisdom::getvalue("ModuleCode", strsearch);
	setHouseCode(H, M);
	bool ok;
	int F = logisdom::getvalue("DeviceFeedBack", strsearch).toInt(&ok);
	if (ok && (F != 0)) ui.checkBoxAck->setCheckState(Qt::Checked);
	QString TxtOFF = logisdom::getvalue("TextOFF", strsearch);
        if (!TxtOFF.isEmpty()) ButtonOff.setText(TxtOFF);
	QString TxtON = logisdom::getvalue("TextON", strsearch);
        if (!TxtON.isEmpty()) ButtonOn.setText(TxtON);
}




void devpclbus::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("HouseCode", QString("%1").arg(HouseCode));
	str += logisdom::saveformat("ModuleCode", QString("%1").arg(ModuleCode));
	str += logisdom::saveformat("DeviceFeedBack", QString("%1").arg(ui.checkBoxAck->isChecked()));
	str += logisdom::saveformat("TextOFF", ButtonOff.text());
	str += logisdom::saveformat("TextON", ButtonOn.text());
	str += logisdom::saveformat("ValueAsText", ButtonOn.text());
}





void devpclbus::setHouseCode(QString House, QString Module)
{
	bool ok;
	int H = House.toInt(&ok, 10);
	if (!ok) return;
	int M = Module.toInt(&ok, 10);
	if (!ok) return;
	setHouseCode(H, M);
}




void devpclbus::setHouseCode(int House, int Module)
{
	if ((House < 0) or (House > 15) or (Module < 0) or (Module > 15)) return;
	HouseCode = House;
	ModuleCode = Module;
    ui.labelcode->setText(QChar(HouseCode + 65) + QString("%1").arg(ModuleCode + 1));
}




void devpclbus::setCode()
{
 	bool ok;
	QStringList itemsHouseCode;
	itemsHouseCode << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "I" << "J" << "K" << "L" << "M" << "N" << "O" << "P";
	QString HouseCode = inputDialog::getItemPalette(this, tr("House Code"), tr("House Code : "), itemsHouseCode, 0, false, &ok, Qt::Dialog, parent);
	if (!ok) return;
	if (HouseCode.isEmpty()) return;
	QStringList itemsDevCode;
	itemsDevCode << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15" << "16";
	QString DevCode = inputDialog::getItemPalette(this, tr("Unit Code "), tr("Unit Code : "), itemsDevCode, 0, false, &ok, Qt::Dialog, parent);
	if (!ok) return;
	if (DevCode.isEmpty()) return;
	setHouseCode((int)HouseCode[0].toLatin1() - 65, DevCode.toInt() - 1);
}





