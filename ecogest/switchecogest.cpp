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
#include "ecogest.h"
#include "server.h"
#include "switchecogest.h"

switchecogest::switchecogest(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    ButtonOn.setText(cstr::toStr(cstr::ON));
    ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
    StringValue.setText(tr("Show result as string"));
    setupLayout.addWidget(&StringValue, layoutIndex++, 0, 1, 1);
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(set_On()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(set_Off()));
    lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	family4 = romid.right(4);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Switch MultiGest"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	htmlBind->addCommand(cstr::toStr(cstr::ON), SwitchON);
	htmlBind->addCommand(cstr::toStr(cstr::OFF), SwitchOFF);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), SwitchON);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), SwitchOFF);
	setname(romid);
	MainValue = logisdom::NA;
	Unit.setText("");
}





void switchecogest::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction ActionON(cstr::toStr(cstr::ON), this);
	QAction ActionOFF(cstr::toStr(cstr::OFF), this);
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	if (maison1wirewindow->remoteIsAdmin())
	{
		contextualmenu.addAction(&ActionON);
		contextualmenu.addAction(&ActionOFF);
		contextualmenu.addAction(&Lecture);	
	}
	contextualmenu.addAction(&Nom);
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &ActionON) set_On();
	if (selection == &ActionOFF) set_Off();
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
}





void switchecogest::SetOrder(const QString &)
{
}



void switchecogest::remoteCommandExtra(const QString &command)
{
	if (command == SwitchON) set_On();
	if (command == SwitchOFF) set_Off();
}





bool switchecogest::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
	ScratchPad_Show.setText(devicescratchpad);
	bool ok;
	double val = devicescratchpad.toDouble(&ok);
	if (ok)
	{
		MainValue = val;
		htmlBind->setValue(MainValueToStr());
		htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
		if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
		ClearWarning();
		MainValueToStr();
		setValid(dataValid);
		emitDeviceValueChanged();
		return true;
	}
	return false;
}




QString switchecogest::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}




void switchecogest::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
		scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
		if (family4 == "HPMS")
		{
			QString V = logisdom::getvalue("SWA", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "SPMS")
		{
			QString V = logisdom::getvalue("SWB", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "CRMS")
		{
			QString V = logisdom::getvalue("SWC", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "SDMS")
		{
			QString V = logisdom::getvalue("SWD", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "SEMS")
		{
			QString V = logisdom::getvalue("SWE", scratchpad);
			setscratchpad(V, false);
		}
		if (family4 == "SFMS")
		{
			QString V = logisdom::getvalue("SWF", scratchpad);
			setscratchpad(V, false);
		}
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void switchecogest::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "HPMS")
			{
				QString V = logisdom::getvalue("SWA", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "SPMS")
			{
				QString V = logisdom::getvalue("SWB", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "CRMS")
			{
				QString V = logisdom::getvalue("SWC", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "SDMS")
			{
				QString V = logisdom::getvalue("SWD", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "SEMS")
			{
				QString V = logisdom::getvalue("SWE", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "SFMS")
			{
				QString V = logisdom::getvalue("SWF", scratchpad);
				setscratchpad(V, true);
			}
			//master->getMainValue();
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}




void switchecogest::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("TextOFF", ButtonOff.text());
	str += logisdom::saveformat("TextON", ButtonOn.text());
	str += logisdom::saveformat("ValueAsText", ButtonOn.text());

}



void switchecogest::setconfig(const QString &strsearch)
{
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Switch ")));
	x = logisdom::getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = logisdom::getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if ((ok_h != 0) and (h == 0)) show(); else hide();
	setWindowTitle(logisdom::getvalue("Name", strsearch));
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






void switchecogest::On(bool send)
{
	if (!master)  return;
	if (send)
	{
		QString family4 = romid.right(4);
		if (family4 == familyMultiGestSwitchA) master->addtofifo("SWAON");
		else if (family4 == familyMultiGestSwitchB) master->addtofifo("SWBON");
		else if (family4 == familyMultiGestSwitchC) master->addtofifo("SWCON");
		else if (family4 == familyMultiGestSwitchD) master->addtofifo("SWDON");
		else if (family4 == familyMultiGestSwitchE) master->addtofifo("SWEON");
		else if (family4 == familyMultiGestSwitchF) master->addtofifo("SWFON");
        else if (family4 == familyMultiGestSwitchN) master->addtofifo("SNC==1");
    }
}





void switchecogest::Off(bool send)
{
	if (!master)  return;
	if (send)
	{
		QString family4 = romid.right(4);
		if (family4 == familyMultiGestSwitchA) master->addtofifo("SWAOFF");
		else if (family4 == familyMultiGestSwitchB) master->addtofifo("SWBOFF");
		else if (family4 == familyMultiGestSwitchC) master->addtofifo("SWCOFF");
		else if (family4 == familyMultiGestSwitchD) master->addtofifo("SWDOFF");
		else if (family4 == familyMultiGestSwitchE) master->addtofifo("SWEOFF");
		else if (family4 == familyMultiGestSwitchF) master->addtofifo("SWFOFF");
        else if (family4 == familyMultiGestSwitchN) master->addtofifo("SNC==0");
    }
}




bool switchecogest::isSwitchFamily()
{
	return true;
}

