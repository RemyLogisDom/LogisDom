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
#include "server.h"
#include "net1wire.h"
#include "vmc.h"
#include "inputdialog.h"
#include "devx10.h"



devx10::devx10(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ExtendedMode = true;
	Decimal.hide();
    ButtonOn.setText(cstr::toStr(cstr::ON));
    ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
    StringValue.setText(tr("Show result as string"));
    setupLayout.addWidget(&StringValue, layoutIndex++, 0, 1, 1);
    connect(this, SIGNAL(stateChanged()), parent->SwitchArea , SLOT(updateStatus()));
    ButtonOn.setStatusTip(tr("Right click to change text"));
    ButtonOn.setContextMenuPolicy(Qt::CustomContextMenu);
    ButtonOff.setStatusTip(tr("Right click to change text"));
    ButtonOff.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&ButtonOn, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ON_rigthClick(QPoint)));
    connect(&ButtonOff, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(OFF_rigthClick(QPoint)));
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(ON()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(OFF()));
}





void devx10::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction SetMode(tr("&Extended Mode"), this);
	SetMode.setCheckable(true);
	SetMode.setChecked(ExtendedMode);
	contextualmenu.addAction(&SetMode);	
    contextualmenu.exec(event->globalPos());
	ExtendedMode = SetMode.isChecked();
}





void devx10::SetOrder(const QString &)
{
}




QString devx10::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	setHMCaption();
	return S;
}




QString devx10::ValueToStr(double Value, bool noText)
{
    QString str;
    if (StringValue.isChecked() and (!noText))
    {
            if (logisdom::isNA(Value)) str = cstr::toStr(cstr::NA);
            else if (logisdom::isZero(Value)) str = ButtonOff.text();
            else if (logisdom::AreSame(Value, 100)) str = ButtonOn.text();
            else str = ButtonOn.text() + " " + parent->LogisDomLocal.toString(Value, 'f', 0) + Unit.text();
    }
    else
    {
            if (logisdom::isNA(Value)) str = cstr::toStr(cstr::NA);
            else if (master) str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
    }
    return str;
}




void devx10::lecture()
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






void devx10::lecturerec()
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








void devx10::On(bool emitX10)
{
	if (ExtendedMode)
	{
		QString Code = "0" + X10_EXTENDED + X10CodesStr[HouseCode] + X10_ON + "0" +X10CodesStr[ModuleCode] + "0000";
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
	}
	else
	{
		QString Code = "0" + X10_ADDR + X10CodesStr[HouseCode] + X10CodesStr[ModuleCode];
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
		Code = "0" + X10_FUNC + X10CodesStr[HouseCode] + X10_ON;
		if (emitX10 && master) master->addtofifo(X10Function, romid, Code);
	}
	MainValue = 100;
	savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	emitDeviceValueChanged();
}





void devx10::Off(bool emitX10)
{
	if (ExtendedMode)
	{
		QString Code = "0" + X10_EXTENDED + X10CodesStr[HouseCode] + X10_OFF + "0" +X10CodesStr[ModuleCode] + "0000";
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
	}
	else
	{
		QString Code = "0" + X10_ADDR + X10CodesStr[HouseCode] + X10CodesStr[ModuleCode];
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
		Code = "0" + X10_FUNC + X10CodesStr[HouseCode] + X10_OFF;
		if (emitX10 && master) master->addtofifo(X10Function, romid, Code);
	}
	MainValue = 0;
	savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	emitDeviceValueChanged();
}








void devx10::Dim(bool emitX10)
{
    if (logisdom::isZero(MainValue)) return;
	if (family != familyLM12) return;
	bool ok;
	int Power = 0;
	int code = X10_FUNC.toInt(&ok, 16) + 8 * Power;
	if (ExtendedMode)
	{
		QString Code = "0" + X10_EXTENDED + X10CodesStr[HouseCode] + X10_DIM + "0" +X10CodesStr[ModuleCode] + "0000";
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
	}
	else
	{
		QString Code = QString("%1").arg(code, 2, 16, QChar('0')).toUpper() + X10CodesStr[HouseCode] + X10CodesStr[ModuleCode];
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
		Code = "0" + X10_FUNC + X10CodesStr[HouseCode] + X10_DIM;
		if (emitX10 && master) master->addtofifo(X10Function, romid, Code);
	}
	MainValue -= 6;
	if (MainValue < 0)
	{
		MainValue = 0;
		Off(true);
	}
	MainValueToStr();
	savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	emitDeviceValueChanged();
}







void devx10::Bright(bool emitX10)
{
	bool ok;
	int Power = 1;
	if (family != familyLM12) return;
	int code = X10_FUNC.toInt(&ok, 16) + 8 * Power;
	if (ExtendedMode)
	{
		QString Code = "0" + X10_EXTENDED + X10CodesStr[HouseCode] + X10_BRIGHT + "0" +X10CodesStr[ModuleCode] + "0000";
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
	}
	else
	{
		QString Code = QString("%1").arg(code, 2, 16, QChar('0')).toUpper() + X10CodesStr[HouseCode] + X10CodesStr[ModuleCode];
		if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
		Code = "0" + X10_FUNC + X10CodesStr[HouseCode] + X10_BRIGHT;
		if (emitX10 && master) master->addtofifo(X10Function, romid, Code);
	}
	MainValue += 6;
	if (MainValue > 100) MainValue = 100;
	MainValueToStr();
	savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	emitDeviceValueChanged();
}






void devx10::Bright(int Power, bool emitX10)
{
	bool ok;
	if (family != familyLM12) return;
	int code = X10_FUNC.toInt(&ok, 16) + 8 * Power;
	QString Code = QString("%1").arg(code, 2, 16, QChar('0')).toUpper() + X10CodesStr[HouseCode] + X10CodesStr[ModuleCode];
	if (emitX10 && master) master->addtofifo(X10Address, romid, Code);
	Code = "0" + X10_FUNC + X10CodesStr[HouseCode] + X10_BRIGHT;
	if (emitX10 && master) master->addtofifo(X10Function, romid, Code);
	MainValue += 6 * Power;
	if (MainValue > 100) MainValue = 100;
	MainValueToStr();
	savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	emitDeviceValueChanged();
}






void devx10::ON_rigthClick(const QPoint &pos)
{
    QMenu contextualmenu;
    QAction setTextAction(tr("Change Text"), this);
    contextualmenu.addAction(&setTextAction);
    QAction *selection;
    selection = contextualmenu.exec(ButtonOn.mapToGlobal(pos));
    if (selection == &setTextAction)
    {
        QString newText = QInputDialog::getText(this, tr("Change Text"), tr("Change Text"), QLineEdit::Normal, ButtonOn.text());
        if (!newText.isEmpty()) ButtonOn.setText(newText);
    }
}




void devx10::OFF_rigthClick(const QPoint &pos)
{
    QMenu contextualmenu;
    QAction setTextAction(tr("Change Text"), this);
    contextualmenu.addAction(&setTextAction);
    QAction *selection;
    selection = contextualmenu.exec(ButtonOff.mapToGlobal(pos));
    if (selection == &setTextAction)
    {
        QString newText = QInputDialog::getText(this, tr("Change Text"), tr("Change Text"), QLineEdit::Normal, ButtonOff.text());
        if (!newText.isEmpty()) ButtonOff.setText(newText);
    }
}





void devx10::setMainValue(double value, bool)
{
// Do not emitX10 (addtofifo) if in remote mode
    if (logisdom::isNA(value)) return;
    if (logisdom::isZero(value)) Off(!parent->isRemoteMode());
	else On(!parent->isRemoteMode());
	htmlBind->setValue(MainValueToStr());
	htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
}




void devx10::Sleep()
{
}




bool devx10::isX10Family()
{
	return true;
}



bool devx10::isSwitchFamily()
{
	return true;
}



bool devx10::setscratchpad(const QString&, bool)
{
	return false;
}






void devx10::setconfig(const QString &strsearch)
{
	QString Name, H, M;
	Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Lamp ")));
	if (logisdom::getvalue("ExtendedMode", strsearch) == "1") ExtendedMode = true;
		else ExtendedMode = false;
	H = logisdom::getvalue("HouseCode", strsearch);
	M = logisdom::getvalue("ModuleCode", strsearch);
	setHouseCode(H, M);
	QString TxtOFF = logisdom::getvalue("TextOFF", strsearch);
        if (!TxtOFF.isEmpty()) ButtonOff.setText(TxtOFF);
	QString TxtON = logisdom::getvalue("TextON", strsearch);
        if (!TxtON.isEmpty()) ButtonOn.setText(TxtON);
        bool ok;
	int textV = logisdom::getvalue("ValueAsText", strsearch).toInt(&ok);
        if (ok)
        {
                if (textV) StringValue.setCheckState(Qt::Checked);
                else StringValue.setCheckState(Qt::Unchecked);
        }
        else StringValue.setCheckState(Qt::Checked);
}





void devx10::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("HouseCode", QString("%1").arg(HouseCode));
	str += logisdom::saveformat("ModuleCode", QString("%1").arg(ModuleCode));
	str += logisdom::saveformat("ExtendedMode", QString("%1").arg(ExtendedMode));
	str += logisdom::saveformat("TextOFF", ButtonOff.text());
	str += logisdom::saveformat("TextON", ButtonOn.text());
	str += logisdom::saveformat("ValueAsText", ButtonOn.text());
}





void devx10::setHouseCode(QString House, QString Module)
{
	bool ok;
	int H = House.toInt(&ok, 10);
	if (!ok) return;
	int M = Module.toInt(&ok, 10);
	if (!ok) return;
	setHouseCode(H, M);
}




void devx10::setHouseCode(int House, int Module)
{
	if ((House < 0) or (House > 15) or (Module < 0) or (Module > 15)) return;
	HouseCode = House;
	ModuleCode = Module;
	setHMCaption();
}





void devx10::setCode()
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
    setHouseCode(int(HouseCode[0].toLatin1()) - 65, DevCode.toInt() - 1);
}




void devx10::setHMCaption()
{	
}
