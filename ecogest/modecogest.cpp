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
#include "ecogest.h"
#include "modecogest.h"




modecogest::modecogest(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	QDateTime Now = QDateTime::currentDateTime();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	family4 = romid.right(4);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Mode MultiGest"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	MainValue = logisdom::NA;
}





bool modecogest::setscratchpad(const QString &scratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	ScratchPad_Show.setText(scratchpad);
	bool ok;
	double NewValue = scratchpad.toDouble(&ok);
	if (ok)
	{
		MainValue = NewValue;
		if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
		MainValueToStr();
		setValid(dataValid);
		htmlBind->setValue(MainValueToStr());
		htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
		ClearWarning();
		emitDeviceValueChanged();
		return true;
	}
	return false;
}







void modecogest::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	contextualmenu.addAction(&Lecture);	
	if (!maison1wirewindow->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
}






QString modecogest::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}





QString modecogest::ValueToStr(double Value, bool)
{
	QString str;
	QString family4 = romid.right(4);
	QString type = family4.left(2);
	if (type == familyMultiGestFluidicMode)
	{
		switch ((int)Value)
		{
			case Mode_Solar_Fill :			str = tr("Fill"); break;
			case Mode_Solar_Run :			str = tr("Run"); break;
			case Mode_Solar_Run_Degaz :		str = tr("Degaz"); break;
			case Mode_Solar_KeepFilled :	str = tr("Keep Filled"); break;
			case Mode_Solar_Off : 			str = tr("Off"); break;
		}
	}
	if (type == familyMultiGestSolarMode)
	{
		switch ((int)Value)
		{
			case Mode_ECS :			str = tr("ECS"); break;
			case Mode_MSD :			str = tr("MSD"); break;
			case Mode_NoSun :	str = tr("No Sun"); break;
		}
	}
	if (Value == logisdom::NA) str = cstr::toStr(cstr::NA);
	else if (str.isEmpty())	str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
	return str;
}





void modecogest::SetOrder(const QString &)
{
}



void modecogest::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
		scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
		if (family4 == "FLMM")
		{
			QString V = logisdom::getvalue("FluidicMode", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "SLMM")
		{
			QString V = logisdom::getvalue("SolarMode", scratchpad);
			setscratchpad(V, false);
		}
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void modecogest::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "FLMM")
			{
				QString V = logisdom::getvalue("FluidicMode", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "SLMM")
			{
				QString V = logisdom::getvalue("SolarMode", scratchpad);
				setscratchpad(V, true);
			}
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





void modecogest::GetConfigStr(QString&)
{
//	str += logisdom::saveformat("Coef", QString("%1").arg(Coef.value(), 0, 'f', 10));
}





void modecogest::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Mode ")));
}

