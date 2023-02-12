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
#include "inputdialog.h"
#include "net1wire.h"
#include "ds1822.h"




ds1822::ds1822(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	ui.labelromid->setText(romid);
	ui.labelfamily->setText(tr("1822 9-12bit Temp ±2.0°C"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	family = romid.right(2);
	MainValue = logisdom::NA;
	TalarmH = logisdom::NA;
	TalarmB = logisdom::NA;
	resolution = 0;
	Unit.setText("°C");
	textResolution = tr("Resolution");
	ResolutionButton.setText(textResolution);
	setupLayout.addWidget(&ResolutionButton, layoutIndex++, 0, 1, 1);
	connect(&ResolutionButton, SIGNAL(clicked()), this, SLOT(changresoultion()));
	setupLayout.addWidget(&AXplusB, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&skip85, layoutIndex++, 0, 1, logisdom::PaletteWidth);
}





void ds1822::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	QAction Resolution(tr("&Resolution"), this);
	QAction AlHaute(tr("&High Alarm"), this);
	QAction AlBasse(tr("&Low Alarm "), this);
	QAction EcrireEEProm(tr("&Write EEProm"), this);
	QAction RappelEEProm(tr("Re&call EEProm"), this);
	QAction *selection;
	contextualmenu.addAction(&Lecture);	
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);	
		contextualmenu.addAction(&Resolution);	
		contextualmenu.addAction(&AlHaute);	
		contextualmenu.addAction(&AlBasse);	
		contextualmenu.addAction(&EcrireEEProm);	
		contextualmenu.addAction(&RappelEEProm);	
	}
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
	if (selection == &Resolution) changresoultion();
	if (selection == &AlHaute) changealarmehaute();
	if (selection == &AlBasse) changealarmebasse();
	if ((selection == &EcrireEEProm) && (!parent->isRemoteMode()))
	{
		if (master) master->addtofifo(WriteEEprom, romid);
		if (master) master->addtofifo(ReadTemp, romid);
	}
	if ((selection == &RappelEEProm) && (!parent->isRemoteMode()))
	{
		if (master) master->addtofifo(RecallEEprom, romid);
		if (master) master->addtofifo(ReadTemp, romid);
	}
}





bool ds1822::isTempFamily()
{
	return true;
}





void ds1822::SetOrder(const QString &order)
{
	if (order == NetRequestMsg[ReadTemp]) lecture();
}


void ds1822::changealarmebasse()
{
	bool ok;
    double T = inputDialog::getIntegerPalette(this, tr("Temperature"), tr("Low Alarm "), int(TalarmB), -55, 125, 1, &ok, parent);
	if (ok)
	{
		TalarmB = T;
		writescratchpad();
	}		
}






void ds1822::changealarmehaute()
{
	bool ok;
    double T = double(inputDialog::getIntegerPalette(this, tr("Temperature"), tr("High Alarm"), int(TalarmH), -55, 125, 1, &ok, parent));
	if (ok)
	{
		TalarmH = T;
		writescratchpad();
	}		
}






void ds1822::changresoultion()
{
	bool ok;
	QStringList items;
	int R = 12;
	if (resolution != 0) R = resolution - 9;
	for (int n=0; n<maxbits; n++) items << NbitsStr(n);
    QString Resolution = inputDialog::getItemPalette(this, tr("Resolution"), tr("Resolution"), items, R, false, &ok, parent);
	if (ok) R =  items.indexOf(Resolution);
	if (R != -1)
	{
		 resolution = R + 9;
		writescratchpad();
	}
}



QString ds1822::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}





void ds1822::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadTemp, romid);
			break;
#else
		case Ha7Net :
			setscratchpad(master->getScratchPad(romid));
			break;
#endif
		case TCP_HA7SType :
			setscratchpad(master->getScratchPad(romid));
			break;
		case MultiGest :
			setscratchpad(master->getScratchPad(romid));
			break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void ds1822::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadTempRec, romid);
			break;
#else
		case Ha7Net :
			setscratchpad(master->getScratchPad(romid), true);
			break;
#endif
		case TCP_HA7SType :
			setscratchpad(master->getScratchPad(romid), true);
			break;
		case MultiGest :
			setscratchpad(master->getScratchPad(romid), true);
			break;
		case RemoteType : master->saveMainValue();
		break;
	}
}








bool ds1822::setscratchpad(const QString &scratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	ScratchPad_Show.setText(scratchpad);
	logMsg(scratchpad);
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
	if (scratchpad.isEmpty())
	{
		logMsg(romid + "  scratchpad empty !!!");
		return IncWarning();
	}
	if (scratchpad == "FFFFFFFFFFFFFFFFFF")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
		return IncWarning();		// No answer from device not connected
	}
	if (scratchpad == "000000000000000000")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
		return IncWarning();		// 1 Wire bus shorted
	}
    if (!checkCRC8(scratchpad)) return IncWarning();
    QString TStr = scratchpad.mid(2,2) + scratchpad.mid(0,2);
	double T = calcultemperature(TStr);
    if (int(T) == logisdom::NA)
	{
		if (master && WarnEnabled.isChecked()) master->GenError(42, errorMsg + "  T = " + QString("%1").arg(T, 0, 'f', 3));
		return IncWarning();
	}
	if (check85(T))
	{
		if (master && WarnEnabled.isChecked()) master->GenError(36, errorMsg);
		return IncWarning();                // convertion error
	}
	if ((T < -55) or (T > 125))
	{
		if (master && WarnEnabled.isChecked()) master->GenError(44, errorMsg + "  T = " + QString("%1").arg(T, 0, 'f', 3));
		return IncWarning();		// launch reset in case of problem
	}
	TalarmH = calcultemperaturealarmeH(scratchpad);
	TalarmB = calcultemperaturealarmeB(scratchpad);
	resolution = getresolution(scratchpad);
	double V = AXplusB.result(T);
	setMainValue(V, enregistremode);
	return true;
}






void ds1822::writescratchpad()
{
    QString scratchpad;
    short Th = 0, Tb = 0, Cfg = 127;

    Th = short(TalarmH);
    Tb = short(TalarmB);
    if (Th < 0) Th = 128 - Th;
    if (Tb < 0) Tb = 128 - Tb;
    if (resolution == 12) Cfg = 127;
    if (resolution == 11) Cfg = 95;
    if (resolution == 10) Cfg = 63;
    if (resolution == 9) Cfg = 31;
    scratchpad = QString("%1%2%3").arg(Th, 2, 16, QChar('0')).arg(Tb, 2, 16, QChar('0')).arg(Cfg, 2, 16, QChar('0')).toUpper();
    if (!parent->isRemoteMode()) if (master) master->addtofifo(WriteScratchpad, romid, scratchpad);
    if (!parent->isRemoteMode()) if (master) master->addtofifo(ReadTemp, romid);
}




double ds1822::calcultemperature(const QString  &THex)
{
	bool ok;
	bool s = false;
    int T = THex.toInt(&ok, 16);
	if (T & 0x8000)
	{
		T = 0xFFFF - T + 1;
		s = true;
	}
    if (s) return double(-T) / 16.0;
        else return double(T) / 16.0;
}





void ds1822::GetConfigStr(QString &str)
{
	AXplusB.GetConfigStr(str);
	str += logisdom::saveformat("Skip85", QString("%1").arg(skip85.isChecked()));
}


  


void ds1822::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Detector T° ")));
	AXplusB.setconfig(strsearch);
	bool ok;
	int sk85 = logisdom::getvalue("Skip85", strsearch).toInt(&ok);
	if (!ok)
	{
		skip85.setCheckState(Qt::Checked);
		dataLoader->check85 = true;
	}
	else if (ok && sk85)
	{
		skip85.setCheckState(Qt::Checked);
		dataLoader->check85 = true;
	}
	else
	{
		skip85.setCheckState(Qt::Unchecked);
		dataLoader->check85 = false;
	}
}



