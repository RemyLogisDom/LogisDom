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
#include "configwindow.h"
#include "server.h"
#include "net1wire.h"
#include "ds2408.h"


ds2408::ds2408(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 8, 1, 1, 1);
	ButtonOn.setText(cstr::toStr(cstr::ON));
	ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOff, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&ButtonOn, layoutIndex++, 1, 1, 1);
    lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	ui.labelfamily->setText("IO Switch");	
	if (master) ui.labelmaster->setText(master->getipaddress());
	connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(set_On()));
	connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(set_Off()));
    ButtonOn.setStatusTip(tr("Right click to change text"));
    ButtonOn.setContextMenuPolicy(Qt::CustomContextMenu);
    ButtonOff.setStatusTip(tr("Right click to change text"));
    ButtonOff.setContextMenuPolicy(Qt::CustomContextMenu);
    //connect(&ButtonOn, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ON_rigthClick(QPoint)));
    //connect(&ButtonOff, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(OFF_rigthClick(QPoint)));
	connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(set_On()));
	connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(set_Off()));
	connect(ui.radioInput, SIGNAL(clicked()), this, SLOT(inputClick()));
	connect(ui.radioOutput, SIGNAL(clicked()), this, SLOT(outputClick()));
    ui.radioInput->setChecked(true);
	inputClick();
	if (family == family2408_A)
	{
		commonReg = new CommonRegStruct;
		commonReg->Reg = -1;
		ui.labelfamily->setText(tr("DS2408 I/O 1"));
        operateDecimal.setText(tr("Operate as decimal"));
        connect(&operateDecimal, SIGNAL(stateChanged(int)), this, SLOT(opDecChanged(int)));
        setupLayout.addWidget(&operateDecimal, layoutIndex, 0, 1, 1);
        writeValue.setRange(0, 255);
        setupLayout.addWidget(&writeValue, layoutIndex, 1, 1, 1);
        ButtonWrite.setText(tr("Write"));
        setupLayout.addWidget(&ButtonWrite, layoutIndex++, 2, 1, 1);
        connect(&ButtonWrite, SIGNAL(clicked()), this, SLOT(writePort()));
    }
	else
	{
		QString romIDA = romid.left(16) + "_A";
		onewiredevice *device = parent->configwin->DeviceExist(romIDA);
        if (device) commonReg = device->commonReg; else commonReg = nullptr;
		if (family == family2408_B) ui.labelfamily->setText(tr("DS2408 I/O 2"));
		if (family == family2408_C) ui.labelfamily->setText(tr("DS2408 I/O 3"));
		if (family == family2408_D) ui.labelfamily->setText(tr("DS2408 I/O 4"));
		if (family == family2408_E) ui.labelfamily->setText(tr("DS2408 I/O 5"));
		if (family == family2408_F) ui.labelfamily->setText(tr("DS2408 I/O 6"));
		if (family == family2408_G) ui.labelfamily->setText(tr("DS2408 I/O 7"));
		if (family == family2408_H) ui.labelfamily->setText(tr("DS2408 I/O 8"));
	}
	ClearWarning();
	LogicalStatusTxt = ui.labelLogicalStatus->text();
	OpenDrainTxt = ui.labelOpenDrain->text();
	StatusTxt = ui.labelStatus->text();

}






ds2408::~ds2408()
{
	if (family == family2408_A) delete commonReg;
}




bool ds2408::isSwitchFamily()
{
	if (ui.radioOutput->isChecked()) return true;
	else return false;
}



bool ds2408::isDimmmable()
{
    return operateDecimal.isChecked();
}




double ds2408::getMaxValue()
{
    if (operateDecimal.isChecked()) return 255;
    return 1;
}




void ds2408::assignMainValueLocal(double value)
{
    if (!commonReg) return;
    if (parent->isRemoteMode()) return;
    if (family != family2408_A) return;
    if (ui.radioInput->isChecked()) return;
    if (commonReg->Reg < 0) lecture();
    if (!operateDecimal.isChecked()) return;
    int data = int(value);
    if (data < 1) data = 0;
    if (data > 255) data = 255;
    OLSR = ~data;
    commonReg->Reg = OLSR;
    QString hex = QString("%1").arg(uchar(OLSR), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(~OLSR), 2, 16, QChar('0'));
    hex = hex.toUpper();
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(WritePIO, romid, hex);
        break;
#else
    case Ha7Net :
        //if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
        break;
#endif
    case TCP_HA7SType :
        master->addtofifo(WritePIO, romid, hex);
        break;
    case MultiGest :
        master->addtofifo(WritePIO, romid, hex);
        break;
        case RemoteType : master->getMainValue();
        break;
    }
}




void ds2408::inputClick()
{
	ui.pushButtonON->setEnabled(false);
	ui.pushButtonOFF->setEnabled(false);
    writeValue.setEnabled(false);
	ButtonOn.setEnabled(false);
	ButtonOff.setEnabled(false);
}



void ds2408::outputClick()
{
	ui.pushButtonON->setEnabled(true);
	ui.pushButtonOFF->setEnabled(true);
    writeValue.setEnabled(true);
    ButtonOn.setEnabled(true);
	ButtonOff.setEnabled(true);
}


void ds2408::opDecChanged(int state)
{
    ButtonWrite.setEnabled(state);
    writeValue.setEnabled(state);
    ui.Invert->setChecked(false);
}


void ds2408::writePort()
{
    assignMainValue(writeValue.value());
}





void ds2408::On(bool)
{
	QString command;
	if (ui.radioInput->isChecked()) return;
	if (ui.Invert->isChecked())
	{
		if (commonReg->Reg < 0) lecture();
		command = getOffCommand();
                switch (master->gettype())
                {
#ifdef HA7Net_No_Thread
                case Ha7Net :
                        if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, true);
                        break;
#else
                case Ha7Net :
                        if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
                        break;
#endif
                case TCP_HA7SType :
                        if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
                        break;
                case MultiGest :
                        if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
                        break;
                        case RemoteType : master->getMainValue();
                        break;
                }
	}
	else
	{
		if (commonReg->Reg < 0) lecture();
		command = getOnCommand();
		switch (master->gettype())
		{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, true);
			break;
#else
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
#endif
		case TCP_HA7SType :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
		case MultiGest :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
			case RemoteType : master->getMainValue();
			break;
		}
	}
}



void ds2408::Off(bool)
{
	QString command;
	if (ui.radioInput->isChecked()) return;
	if (ui.Invert->isChecked())
	{
		if (commonReg->Reg < 0) lecture();
		command = getOnCommand();
		switch (master->gettype())
		{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, true);
			break;
#else
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
#endif
		case TCP_HA7SType :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
		case MultiGest :
			if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command);
			break;
			case RemoteType : master->getMainValue();
			break;
		}
	}
	else
	{
		if (commonReg->Reg < 0) lecture();
		command = getOffCommand();
		switch (master->gettype())
		{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, true);
			break;
#else
		case Ha7Net :
			if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
			break;
#endif
		case TCP_HA7SType :
			if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
			break;
		case MultiGest :
			if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
			break;
			case RemoteType : master->getMainValue();
			break;
		}
	}
}





QString ds2408::getOffCommand()
{
    if (!commonReg) return "";
    OLSR = commonReg->Reg;
	if (OLSR < 0) return "";
	if (family == family2408_A) OLSR |= 0x01;
	if (family == family2408_B) OLSR |= 0x02;
	if (family == family2408_C) OLSR |= 0x04;
	if (family == family2408_D) OLSR |= 0x08;
	if (family == family2408_E) OLSR |= 0x10;
	if (family == family2408_F) OLSR |= 0x20;
	if (family == family2408_G) OLSR |= 0x40;
	if (family == family2408_H) OLSR |= 0x80;
	commonReg->Reg = OLSR;
    QString hex = QString("%1").arg(uchar(OLSR), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(~OLSR), 2, 16, QChar('0'));
	return hex.toUpper();
}





QString ds2408::getOnCommand()
{
    if (!commonReg) return "";
    OLSR = commonReg->Reg;
	if (OLSR < 0) return "";
	if (family == family2408_A) OLSR &= 0xFE;
	if (family == family2408_B) OLSR &= 0xFD;
	if (family == family2408_C) OLSR &= 0xFB;
	if (family == family2408_D) OLSR &= 0xF7;
	if (family == family2408_E) OLSR &= 0xEF;
	if (family == family2408_F) OLSR &= 0xDF;
	if (family == family2408_G) OLSR &= 0xBF;
	if (family == family2408_H) OLSR &= 0x7F;
	commonReg->Reg = OLSR;
    QString hex = QString("%1").arg(uchar(OLSR), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(~OLSR), 2, 16, QChar('0'));
	return hex.toUpper();
}





void ds2408::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	QAction Enregistrer(tr("&Save"), this);
	contextualmenu.addAction(&Lecture);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
		contextualmenu.addAction(&Enregistrer);
	}
 	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
}





void ds2408::SetOrder(const QString &)
{
}






QString ds2408::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	return S;
}



void ds2408::lecture()
{
	QString romID_A = romid.left(16) + "_A";
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadPIO, romID_A);
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





void ds2408::lecturerec()
{
	QString romID_A = romid.left(16) + "_A";
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ReadRecPIO, romID_A);
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
		case RemoteType : master->getMainValue();
		break;
	}
}




// F0 00 88 FF FF FF FF FF FF FF FF FF FF
// F0 88 00 00 FF 00 00 00 88 FF FF F4 6B
bool ds2408::setscratchpad(const QString &scratchpad, bool enregistremode)
{
    if (!commonReg) return false;
    int decimalValue = logisdom::NA;
    ScratchPad_Show.setText(scratchpad);
	bool ok;
	QString str;
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name;
	if (scratchpad.isEmpty())
	{
	//	if(commonReg->Reg != -1)
	//	{
	//		if (master && WarnEnabled.isChecked()) master->GenError(27, errorMsg);
	//		return IncWarning();
	//	}
		return true;
	}
	int LogicState = logisdom::NA;
	int OutputLatch = logisdom::NA;
    //QString CrcStr = scratchpad.right(4);
    //QString Crc = CrcStr.right(2) + CrcStr.left(2);
    //uint16_t crc = uint16_t(Crc.toUInt(&ok, 16));
    //uint16_t crccalc = 0;
    //int L = (scratchpad.length() / 2) - 2;
    //bool crcActive = false;
    //if (L > 4) crcActive = true;
    //if (scratchpad == "AA")
    //{
    //    L = 0;
    //}
    //else if (L <= 0)
    //{
    //    if (master && WarnEnabled.isChecked()) master->GenError(26, errorMsg);
    //    return IncWarning();
    //}
    //byteVec_t scratchpadcrc = byteVec_t(L+1);
    if (scratchpad == "AA")		// end of setChannelOn or setChannelOff
    {
        OLSR = commonReg->Reg;
        OutputLatch = OLSR;
        if (family == family2408_A)
        {
            decimalValue = ~OutputLatch;
            OutputLatch &= 0x01;
        }
        if (family == family2408_B) OutputLatch &= 0x02;
        if (family == family2408_C) OutputLatch &= 0x04;
        if (family == family2408_D) OutputLatch &= 0x08;
        if (family == family2408_E) OutputLatch &= 0x10;
        if (family == family2408_F) OutputLatch &= 0x20;
        if (family == family2408_G) OutputLatch &= 0x40;
        if (family == family2408_H) OutputLatch &= 0x80;
        goto setstatus;
    }
    //for (int n=0; n<L; n++) scratchpadcrc[n] = scratchpad.mid(n * 2, 2).toInt(&ok, 16);
    //if (crcActive) crccalc = calcCRC16(&scratchpadcrc[0], L);
	//logMsg(devicescratchpad);
	if (scratchpad == "F08800FFFFFFFFFFFFFFFFFFFF")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
		return IncWarning();		// No answer from device not connected
	}
	if (scratchpad == "00000000000000000000000000")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
		return IncWarning();		// 1 Wire bus shorted
	}
	// CRC Check   F0 88 00 00 FF 00 00 00 88 FF FF F4 6B
//	logMsg(QString("CRC received (%1)   scratchpad : ").arg(crc) + scratchpad);
    //if (!ok) return IncWarning();
//	uint16_t crccalc = calcCRC16(&scratchpadcrc[0], L);
//	logMsg(QString("CRC calc (%1)").arg(crccalc));
    /*if ((crc != crccalc) && crcActive)
	{
		if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
		return IncWarning();		// CRC Wrong
	}
    else logMsg(QString("CRC ok (%1)").arg(crccalc));*/
    if (!checkCRC16(scratchpad)) return IncWarning();
    LogicState = scratchpad.mid(6,2).toInt(&ok, 16);
	OLSR = scratchpad.mid(8,2).toInt(&ok, 16);
	if (family == family2408_A)
	{
        decimalValue = ~OLSR;
        LogicState &= 0x01;
		if (ok && LogicState) LogicState = 1; else LogicState = 0;
		if (ok) commonReg->Reg = OLSR; else commonReg->Reg = -1;
		OLSR &= 0x01;
		if (ok && OLSR) OutputLatch = 1; else OutputLatch = 0;
		QString RomID_B = romid.left(17) + "B";
		QString RomID_C = romid.left(17) + "C";
		QString RomID_D = romid.left(17) + "D";
		QString RomID_E = romid.left(17) + "E";
		QString RomID_F = romid.left(17) + "F";
		QString RomID_G = romid.left(17) + "G";
		QString RomID_H = romid.left(17) + "H";
		onewiredevice *device_B;
		onewiredevice *device_C;
		onewiredevice *device_D;
		onewiredevice *device_E;
		onewiredevice *device_F;
		onewiredevice *device_G;
		onewiredevice *device_H;
		device_B = parent->configwin->DeviceExist(RomID_B);
		device_C = parent->configwin->DeviceExist(RomID_C);
		device_D = parent->configwin->DeviceExist(RomID_D);
		device_E = parent->configwin->DeviceExist(RomID_E);
		device_F = parent->configwin->DeviceExist(RomID_F);
		device_G = parent->configwin->DeviceExist(RomID_G);
		device_H = parent->configwin->DeviceExist(RomID_H);
		if (master)
		switch (master->gettype())
		{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			if (device_B) device_B->setscratchpad(scratchpad, enregistremode);
			if (device_C) device_C->setscratchpad(scratchpad, enregistremode);
			if (device_D) device_D->setscratchpad(scratchpad, enregistremode);
			if (device_E) device_E->setscratchpad(scratchpad, enregistremode);
			if (device_F) device_F->setscratchpad(scratchpad, enregistremode);
			if (device_G) device_G->setscratchpad(scratchpad, enregistremode);
			if (device_H) device_H->setscratchpad(scratchpad, enregistremode);
			break;
#endif
		}
	}
	else if (family == family2408_B)
	{
		LogicState &= 0x02;
		if (ok && (OLSR & 0x02)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_C)
	{
		LogicState &= 0x04;
		if (ok && (OLSR & 0x04)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_D)
	{
		LogicState &= 0x08;
		if (ok && (OLSR & 0x08)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_E)
	{
		LogicState &= 0x10;
		if (ok && (OLSR & 0x10)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_F)
	{
		LogicState &= 0x20;
		if (ok && (OLSR & 0x20)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_G)
	{
		LogicState &= 0x40;
		if (ok && (OLSR & 0x40)) OutputLatch = 1; else OutputLatch = 0;
	}
	else if (family == family2408_H)
	{
		LogicState &= 0x80;
		if (ok && (OLSR & 0x80)) OutputLatch = 1; else OutputLatch = 0;
	}
// Logiscal status
	str = LogicalStatusTxt;
	if (LogicState == logisdom::NA) str += cstr::toStr(cstr::NA);
	else if (LogicState == 0) str += tr("0  (Low)");
	else if (LogicState > 0) str += tr("1  (High)");
	ui.labelLogicalStatus->setText(str);
setstatus:
// Open Drain Status
	str = OpenDrainTxt;
	if (OutputLatch == logisdom::NA) str += cstr::toStr(cstr::NA);
	else if (OutputLatch == 0) str += tr("0  (Closed)");
	else if (OutputLatch > 0) str += tr("1  (Opened)");
	ui.labelOpenDrain->setText(str);
	str = StatusTxt;
	int Value = logisdom::NA;
	if (ui.radioInput->isChecked())
	{
		if (LogicState == logisdom::NA) str += cstr::toStr(cstr::NA);
		else if (ui.Invert->isChecked())
		{
			if (LogicState == 0)
			{
                                str += ButtonOn.text();
				Value = 1;
			}
			if (LogicState > 0)
			{
                                str += ButtonOff.text();
				Value = 0;
			}
		}
		else
		{
			if (LogicState == 0)
			{
                                str += ButtonOff.text();
				Value = 0;
			}
			if (LogicState > 0)
			{
                                str += ButtonOn.text();
				Value = 1;
			}
		}
	}
	else
	{
		if (OutputLatch == logisdom::NA) str += cstr::toStr(cstr::NA);
		else if (ui.Invert->isChecked())
		{
			if (OutputLatch == 0)
			{
                                str += ButtonOff.text();
				Value = 0;
			}
			if (OutputLatch > 0)
			{
                                str += ButtonOn.text();
				Value = 1;
			}
		}
		else
		{
			if (OutputLatch == 0)
			{
                                str += ButtonOn.text();
				Value = 1;
			}
			if (OutputLatch > 0)
			{
                                str += ButtonOff.text();
				Value = 0;
			}
		}
	}
	ui.labelStatus->setText(str);
    if (family == family2408_A)
    {
        if (operateDecimal.isChecked()) Value = decimalValue & 0xFF;
        //logMsg(QString("Decimal value = %1").arg(Value));
    }
    setMainValue(Value, enregistremode);
	return true;
}



void ds2408::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("Input", QString("%1").arg(ui.radioInput->isChecked()));
	str += logisdom::saveformat("Invert", QString("%1").arg(ui.Invert->isChecked()));
	str += logisdom::saveformat("TextOFF", ButtonOff.text());
	str += logisdom::saveformat("TextON", ButtonOn.text());
	str += logisdom::saveformat("ValueAsText", ButtonOn.text());
    if (family == family2408_A) str += logisdom::saveformat("DecimalOperate", QString("%1").arg(operateDecimal.isChecked()));
}



void ds2408::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("IO Switch ")));
	bool ok;
	int input = logisdom::getvalue("Input", strsearch).toInt(&ok);
	if (ok)
	{
		if (input)
		{
			inputClick();
			ui.radioInput->setChecked(true);
		}
		else
		{
			outputClick();
			ui.radioOutput->setChecked(true);
		}
	}
	int invert = logisdom::getvalue("Invert", strsearch).toInt(&ok);
	if (ok)
	{
		if (invert) ui.Invert->setCheckState(Qt::Checked);
		else ui.Invert->setCheckState(Qt::Unchecked);
	}
    if (family == family2408_A)
    {
        int decop = logisdom::getvalue("DecimalOperate", strsearch).toInt(&ok);
        if (ok)
        {
            if (decop) operateDecimal.setCheckState(Qt::Checked);
            else operateDecimal.setCheckState(Qt::Unchecked);
        }
    }
}

