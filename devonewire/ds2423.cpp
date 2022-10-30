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
#include "ds2423.h"




ds2423::ds2423(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 4, 1, 1, 2);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	ui.labelfamily->setText(tr("DS2423 : Dual Counter + 4K Ram"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	if (family == family2423_A) ui.MainLabel->setText(tr("Counter port A"));
	if (family == family2423_B) ui.MainLabel->setText(tr("Counter port B"));
	MainValue = logisdom::NA;
	LastCounter = logisdom::NA;
	Delta = logisdom::NA;
	counterMode.addItem(tr("Absolute"));
	counterMode.addItem(tr("Relative"));
	counterMode.addItem(tr("Offset"));
	countInit.setName(tr("Offset update"));
	setupLayout.addWidget(&counterMode, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	Coef.setValue(1);
	Coef.setDecimals(10);
	Coef.setPrefix(tr("Coef : "));
	setupLayout.addWidget(&Coef, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	Offset.setRange(-2147483640, +2147483640);
	Offset.setPrefix(tr("Offset : "));
	setupLayout.addWidget(&Offset, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&countInit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	SaveOnUpdate.setText(tr("Save only on update"));	
	setupLayout.addWidget(&SaveOnUpdate,  layoutIndex++, 0, 1, logisdom::PaletteWidth);
	connect(&SaveOnUpdate, SIGNAL(stateChanged(int)), this, SLOT(saveOnUpdateChanged(int)));
	connect(&counterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
}




void ds2423::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("Coef", QString("%1").arg(Coef.value(), 0, 'f', 10));
	str += logisdom::saveformat("counterMode", QString("%1").arg(counterMode.currentIndex()));
	str += logisdom::saveformat("counterOffset", QString("%1").arg(Offset.value()));
	str += logisdom::saveformat("NextCountInit", countInit.getNext());
	str += logisdom::saveformat("CountInitMode", countInit.getMode());
	str += logisdom::saveformat("CountInitEnabled", countInit.getSaveMode());
	str += logisdom::saveformat("SaveOnUpdate", QString("%1").arg(SaveOnUpdate.isChecked()));
}





void ds2423::setconfig(const QString &strsearch)
{
	bool ok;
	QString coefstr = logisdom::getvalue("Coef", strsearch);
	double coef = 1;
	if (!coefstr.isEmpty()) coef = coefstr.toDouble(&ok);
	if (ok) Coef.setValue(coef);
	Offset.setEnabled(false);
	countInit.setEnabled(false);
	SaveOnUpdate.setEnabled(false);
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Counter ")));
	int mode = 0;
	mode = logisdom::getvalue("counterMode", strsearch).toInt(&ok);
	if (ok) counterMode.setCurrentIndex(mode);
		else counterMode.setCurrentIndex(absoluteMode);
	int offset = 0;
	offset = logisdom::getvalue("counterOffset", strsearch).toInt(&ok);
	if (ok) Offset.setValue(offset);
	QString next = logisdom::getvalue("NextCountInit", strsearch);
	if (next.isEmpty()) countInit.setNext(QDateTime::currentDateTime());
		else countInit.setNext(QDateTime::fromString (next, Qt::ISODate));
	int savemode = logisdom::getvalue("CountInitMode", strsearch).toInt(&ok);
	if (ok) countInit.setMode(savemode);
	int saveen = logisdom::getvalue("CountInitEnabled", strsearch).toInt(&ok);
	if (!ok) countInit.setSaveMode(true);
		else if (saveen) countInit.setSaveMode(true);
			else  countInit.setSaveMode(false);
	int saveupdate = logisdom::getvalue("SaveOnUpdate", strsearch).toInt(&ok);
	if (ok)
	{
		if (saveupdate) SaveOnUpdate.setCheckState(Qt::Checked);
		else SaveOnUpdate.setCheckState(Qt::Unchecked);
	}
}





void ds2423::saveOnUpdateChanged(int state)
{
	switch (state)
	{
		case Qt::Unchecked :
			saveInterval.setEnabled(true);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			saveInterval.SaveEnable.setCheckState(Qt::Unchecked);
			saveInterval.setEnabled(false);
			break;
	}
}




void ds2423::modeChanged(int)
{
	QDateTime now = QDateTime::currentDateTime();
	switch (counterMode.currentIndex())
	{
        case absoluteMode :
			Offset.setEnabled(false);
			countInit.setEnabled(false);
			SaveOnUpdate.setEnabled(false);
			break;
        case relativeMode :
            Offset.setEnabled(true);
            Offset.setPrefix("Delta Max : ");
            Offset.setToolTip(tr("If different from 0, when delta (before being multiplied by coef) is higher then this value, relative value is reseted"));
			countInit.setEnabled(false);
			SaveOnUpdate.setEnabled(false);
			break;
        case offsetMode  :
			Offset.setEnabled(true);
            Offset.setPrefix("Offset :");
            Offset.setToolTip("");
            countInit.setEnabled(true);
			countInit.setNext(now);
			SaveOnUpdate.setEnabled(true);
			break;
	}
}




void ds2423::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	contextualmenu.addAction(&Lecture);
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
}





void ds2423::SetOrder(const QString &)
{
}



void ds2423::lecture()
{
	int ID = 0;
	if (family == family2423_B) ID = 1;
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadCounter, romid);
			break;
#else
		case Ha7Net :
			setscratchpad(master->getScratchPad(romid, ID));
			break;
#endif
		case TCP_HA7SType :
			setscratchpad(master->getScratchPad(romid, ID));
			break;
		case MultiGest :
			setscratchpad(master->getScratchPad(romid, ID));
			break;
	case RemoteType : master->getMainValue();
		break;
	}
}






void ds2423::lecturerec()
{
	int ID = 0;
        if (family == family2423_B) ID = 1;
        setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadCounterRec, romid);
			break;
#else
		case Ha7Net :
			setscratchpad(master->getScratchPad(romid, ID), true);
			break;
#endif
		case TCP_HA7SType :
			setscratchpad(master->getScratchPad(romid, ID), true);
			break;
		case MultiGest :
			setscratchpad(master->getScratchPad(romid, ID), true);
			break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





bool ds2423::setscratchpad(const QString &scratchpad, bool enregistremode)
{
	ScratchPad_Show.setText(scratchpad);
	bool ok;
	logMsg(scratchpad);
	if (scratchpad.isEmpty())
    {
		logMsg(romid + "  scratchpad empty !!!");
		return IncWarning();
	}
    if (scratchpad == "5ADF01FFFFFFFFFFFFFFFFFFFFFF")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(54, scratchpad);
		return IncWarning();		// No answer from device not connected
	}
	if (scratchpad == "0000000000000000000000000000")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(56, scratchpad);
		return IncWarning();		// 1 Wire bus shorted
	}
	// CRC Check	  A5FF01000FD2000000000000A0AD
    //int L = (scratchpad.length() / 2) - 2;
    //if (L <= 0)
    //{
    //    if (master && WarnEnabled.isChecked()) master->GenError(26, scratchpad);
    //    return IncWarning();
    //}
    //byteVec_t scratchpadcrc = byteVec_t(L+1);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = scratchpad.mid(n * 2, 2).toInt(&ok, 16);
    //QString CrcStr = scratchpad.right(4);
    //QString Crc = CrcStr.right(2) + CrcStr.left(2);
    //uint16_t crc = Crc.toInt(&ok, 16);
//	logMsg(QString("CRC received (%1)").arg(crc));
    //if (!ok)
    //{
    //	if (master && WarnEnabled.isChecked()) master->GenError(37, scratchpad);
    //	return IncWarning();
    //}
    //uint16_t crccalc = calcCRC16(&scratchpadcrc[0], L);
//	logMsg(QString("CRC calc (%1)").arg(crccalc));
    //if (crc != crccalc)
    //{
    //	if (master && WarnEnabled.isChecked()) master->GenError(53, scratchpad);
    //	return IncWarning();		// CRC Wrong
    //}
    //logMsg(QString("CRC ok (%1)").arg(crccalc));
    if (!checkCRC16(scratchpad)) return IncWarning();
	QString countLSB = scratchpad.mid(10,2) + scratchpad.mid(8,2);
	QString countMSB = scratchpad.mid(14,2) + scratchpad.mid(12,2);
	long int CLSB = countLSB.toInt(&ok,16);
	if (!ok) return IncWarning();
	long int CMSB = countMSB.toInt(&ok,16);
	if (!ok) return IncWarning();
	long int counter = (0x10000 * CMSB) + CLSB;
	setResult(counter, enregistremode);
	setMainValue(MainValue, false);
	ui.MainText->setText(QString("%1 pulses, Delta = %2").arg(counter).arg(Delta));
	ui.valueui->setText(MainValueToStr());
	bool offsetUpdate = countInit.isitnow();
	if (enregistremode)
	{
		if ((SaveOnUpdate.isChecked()) && (counterMode.currentIndex() == offsetMode))
		{
			if (offsetUpdate) savevalue(QDateTime::currentDateTime(), MainValue, false);
		}
		else savevalue(QDateTime::currentDateTime(), MainValue);
	}
	if ((counterMode.currentIndex() == offsetMode) && (offsetUpdate)) Offset.setValue((int)counter);
	return true;
}







void ds2423::setResult(long int NewValue, bool enregistremode)
{
    if (LastCounter == logisdom::NA) LastCounter = NewValue;
    if (NewValue < LastCounter)
    {
        LastCounter = NewValue;
        // if device is unplugged it strart from 0 and count again, we must restart lastCounter
    }
    if (enregistremode)
    {
        Delta = NewValue - LastCounter;
        LastCounter = NewValue;
        // we update lastCounter only if record is requested, this is specific for counters
    }
    switch (counterMode.currentIndex())
	{
		case absoluteMode :
			MainValue = NewValue * Coef.value();
		break;
		case relativeMode :
            if (enregistremode)
            {
                if (Offset.value() != 0)
                    if (Delta > Offset.value()) Delta = 0;
                MainValue = Delta * Coef.value();
            }
            // only if lastCounter was updated otherwise display zero for nothing
		break;
		case offsetMode :
			MainValue = (NewValue - Offset.value()) * Coef.value();
		break;
	}
}



