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
#include "configwindow.h"
#include "server.h"
#include "net1wire.h"
#include "ds2413.h"



ds2413::ds2413(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
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
    ui.labelfamily->setText("Dual Switch");
    if (master) ui.labelmaster->setText(master->getipaddress());
    connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(set_On()));
    connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(set_Off()));
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(set_On()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(set_Off()));
    connect(ui.radioInput, SIGNAL(clicked()), this, SLOT(inputClick()));
    connect(ui.radioOutput, SIGNAL(clicked()), this, SLOT(outputClick()));
    ui.radioInput->setChecked(true);
    inputClick();
    if ((family == family2413_A) || (family == family3A2100H_A))
    {
        commonReg = new CommonRegStruct;
        commonReg->Reg = -1;
        ui.labelfamily->setText(tr("DS2413 I/O 1"));
        operateDecimal.setText(tr("Operate as decimal"));
        setupLayout.addWidget(&operateDecimal, layoutIndex++, 0, 1, 1);
        connect(&operateDecimal, SIGNAL(stateChanged(int)), this, SLOT(opDecChanged(int)));
        B0.setText(tr("0"));
        setupLayout.addWidget(&B0, layoutIndex, 0, 1, 1);
        connect(&B0, SIGNAL(clicked()), this, SLOT(B0Click()));
        B1.setText(tr("1"));
        setupLayout.addWidget(&B1, layoutIndex, 1, 1, 1);
        connect(&B1, SIGNAL(clicked()), this, SLOT(B1Click()));
        B2.setText(tr("2"));
        setupLayout.addWidget(&B2, layoutIndex, 2, 1, 1);
        connect(&B2, SIGNAL(clicked()), this, SLOT(B2Click()));
        B3.setText(tr("3"));
        setupLayout.addWidget(&B3, layoutIndex++, 3, 1, 1);
        connect(&B3, SIGNAL(clicked()), this, SLOT(B3Click()));
        opDecChanged(false);
    }
    else
    {
        QString romIDA = romid.left(16) + "_A";
        onewiredevice *device = parent->configwin->DeviceExist(romIDA);
        if (device) commonReg = device->commonReg;
        if ((family == family2413_B) || (family == family3A2100H_B)) ui.labelfamily->setText(tr("DS2413 I/O 2"));
    }
    ClearWarning();
    LogicalStatusTxt = ui.labelLogicalStatus->text();
    OpenDrainTxt = ui.labelOpenDrain->text();
    StatusTxt = ui.labelStatus->text();
}





void ds2413::contextMenuEvent(QContextMenuEvent * event)
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





void ds2413::SetOrder(const QString &)
{
}




QString ds2413::getOffCommand()
{
    if (!commonReg) return "";
    OLSR = commonReg->Reg;
    if (OLSR < 0) return "";
    if ((family == family2413_A) || (family == family3A2100H_A)) OLSR |= 0x02;
    if ((family == family2413_B) || (family == family3A2100H_B)) OLSR |= 0x08;
    int bit2 = (OLSR & 0x02) / 2;
    int bit4 = (OLSR & 0x08) / 4;
    int data = (bit2 + bit4) | 0xFC;
    commonReg->Reg = OLSR;
    QString hex = QString("%1").arg((unsigned char)data, 2, 16, QChar('0'));
    hex += QString("%1").arg((unsigned char)~data, 2, 16, QChar('0'));
    return hex.toUpper();
}





QString ds2413::getOnCommand()
{
    if (!commonReg) return "";
    OLSR = commonReg->Reg;
    if (OLSR < 0) return "";
    if ((family == family2413_A) || (family == family3A2100H_A)) OLSR &= 0xFD;
    if ((family == family2413_B) || (family == family3A2100H_B)) OLSR &= 0xF7;
    int bit2 = (OLSR & 0x02) / 2;
    int bit4 = (OLSR & 0x08) / 4;
    int data = (bit2 + bit4) | 0xFC;
    commonReg->Reg = OLSR;
    QString hex = QString("%1").arg((unsigned char)data, 2, 16, QChar('0'));
    hex += QString("%1").arg((unsigned char)~data, 2, 16, QChar('0'));
    return hex.toUpper();
}




double ds2413::getMaxValue()
{
    if (operateDecimal.isChecked()) return 3;
    return 1;
}



void ds2413::assignMainValueLocal(double value)
{
    if (!commonReg) return;
    if (parent->isRemoteMode()) return;
    if ((family != family2413_A) && (family != family3A2100H_B)) return;
    if (commonReg->Reg < 0) lecture();
    if (!operateDecimal.isChecked()) return;
    int data = (int)value;
    if (data < 1) data = 0;
    if (data > 3) data = 3;
    int bit0 = data & 0x01;
    int bit1 = data & 0x02;
    OLSR = commonReg->Reg;
    if (bit0) OLSR &= 0xFD; else OLSR |= 0x02;
    if (bit1) OLSR &= 0xF7; else OLSR |= 0x08;
    commonReg->Reg = OLSR;
    QString hex = QString("%1").arg((unsigned char)~data, 2, 16, QChar('0'));
    hex += QString("%1").arg((unsigned char)data, 2, 16, QChar('0'));
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




void ds2413::B0Click()
{
    assignMainValue(0);
}


void ds2413::B1Click()
{
    assignMainValue(1);
}

void ds2413::B2Click()
{
    assignMainValue(2);
}

void ds2413::B3Click()
{
    assignMainValue(3);
}


void ds2413::opDecChanged(int state)
{
    B0.setEnabled(state);
    B1.setEnabled(state);
    B2.setEnabled(state);
    B3.setEnabled(state);
    ui.Invert->setChecked(false);
}


void ds2413::On(bool)
{
    if (!master) return;
    if (!commonReg) return;
    //QString romID_A = romid.left(16) + "_A";
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





void ds2413::Off(bool)
{
    if (!master) return;
    if (!commonReg) return;
    //QString romID_A = romid.left(16) + "_A";
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
            if (!command.isEmpty()) master->addtofifo(setChannelOn, romid);
            break;
#else
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command, true);
            break;
#endif
        case TCP_HA7SType :
            if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command, true);
            break;
        case MultiGest :
            if (!command.isEmpty()) master->addtofifo(setChannelOn, romid, command, true);
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




bool ds2413::isSwitchFamily()
{
    if (ui.radioOutput->isChecked()) return true;
    else return false;
}



bool ds2413::isDimmmable()
{
    return operateDecimal.isChecked();
}



void ds2413::inputClick()
{
    ui.pushButtonON->setEnabled(false);
    ui.pushButtonOFF->setEnabled(false);
    ButtonOn.setEnabled(false);
    ButtonOff.setEnabled(false);
}



void ds2413::outputClick()
{
    ui.pushButtonON->setEnabled(true);
    ui.pushButtonOFF->setEnabled(true);
    ButtonOn.setEnabled(true);
    ButtonOff.setEnabled(true);
}




QString ds2413::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    return S;
}




void ds2413::lecture()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ReadDualSwitch, romID_A);
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






void ds2413::lecturerec()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ReadDualSwitchRec, romID_A);
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




bool ds2413::setscratchpad(const QString &scratchpad, bool enregistremode)
{
    if (!commonReg) return false;
    ScratchPad_Show.setText(scratchpad);
    int COMP;
    int decimalValue = logisdom::NA;
    logMsg(romid + " : " + scratchpad);
    bool ok;
    QString str;
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name;
    if (scratchpad.isEmpty())
    {
    //	if (commonReg->Reg != -1)
    //	{
    //		if (master && WarnEnabled.isChecked()) master->GenError(27, errorMsg);
    //		return IncWarning();
    //	}
        return true;
    }
    int LogicState = logisdom::NA;
    int OutputLatch = logisdom::NA;
    if (scratchpad.right(2) == "AA")
    {
        OLSR = commonReg->Reg;
        OutputLatch = OLSR;
        if ((family == family2413_A) || (family == family3A2100H_A))
        {
            int bit0 = OutputLatch & 0x02;
            int bit1 = OutputLatch & 0x08;
            decimalValue = 0;
            if (!bit0) decimalValue += 1;
            if (!bit1) decimalValue += 2;
            OutputLatch &= 0x02;
            QString RomID_B = romid.left(17) + "B";
            onewiredevice *device_B;
            device_B = parent->configwin->DeviceExist(RomID_B);
            if (device_B) device_B->setscratchpad(scratchpad, enregistremode);
        }
        if ((family == family2413_B) || (family == family3A2100H_B)) OutputLatch &= 0x08;
        goto setstatus;
    }
    if (scratchpad == "FF")
    {
        if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
        return IncWarning();		// No answer from device not connected
    }
    if (scratchpad == "00")
    {
        if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
        return IncWarning();		// 1 Wire bus shorted
    }
    LogicState = scratchpad.toInt(&ok, 16) & 0x0F;
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    COMP = (scratchpad.toInt(&ok, 16) & 0xF0) / 0x10;
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    if ((LogicState + COMP) != 0x0F)
    {
        logMsg("CRC Fail");
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    else logMsg("CRC ok   " + scratchpad);
    OLSR = LogicState;
    if ((family == family2413_A) || (family == family3A2100H_A))
    {
        int bit0 = OLSR & 0x02;
        int bit1 = OLSR & 0x08;
        decimalValue = 0;
        if (!bit0) decimalValue += 1;
        if (!bit1) decimalValue += 2;
        LogicState &= 0x01;
        if (LogicState) LogicState = 1; else LogicState = 0;
        if (OLSR & 0x02) OutputLatch = 1; else OutputLatch = 0;
        commonReg->Reg = OLSR;
        QString RomID_B = romid.left(17) + "B";
        onewiredevice *device_B;
        device_B = parent->configwin->DeviceExist(RomID_B);
        if (master)
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (device_B) device_B->setscratchpad(scratchpad, enregistremode);
            break;
#endif
        }
    }
    else if ((family == family2413_B) || (family == family3A2100H_B))
    {
        LogicState &= 0x04;
        if (LogicState) LogicState = 1; else LogicState = 0;
        if (OLSR & 0x08) OutputLatch = 1; else OutputLatch = 0;
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
                str += cstr::toStr(cstr::ON);
                Value = 1;
            }
            if (LogicState > 0)
            {
                str += cstr::toStr(cstr::OFF);
                Value = 0;
            }
        }
        else
        {
            if (LogicState == 0)
            {
                str += cstr::toStr(cstr::OFF);
                Value = 0;
            }
            if (LogicState > 0)
            {
                str += cstr::toStr(cstr::ON);
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
                str += cstr::toStr(cstr::OFF);
                Value = 0;
            }
            if (OutputLatch > 0)
            {
                str += cstr::toStr(cstr::ON);
                Value = 1;
            }
        }
        else
        {
            if (OutputLatch == 0)
            {
                str += cstr::toStr(cstr::ON);
                Value = 1;
            }
            if (OutputLatch > 0)
            {
                str += cstr::toStr(cstr::OFF);
                Value = 0;
            }
        }
    }
    ui.labelStatus->setText(str);
    if ((family == family2413_A) || (family == family3A2100H_A))
    {
        if (operateDecimal.isChecked()) Value = decimalValue;
        //logMsg(QString("Decimal value = %1").arg(Value));
    }
    setMainValue(Value, enregistremode);
    return true;
}




void ds2413::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("Input", QString("%1").arg(ui.radioInput->isChecked()));
    str += logisdom::saveformat("Invert", QString("%1").arg(ui.Invert->isChecked()));
    str += logisdom::saveformat("TextOFF", ButtonOff.text());
    str += logisdom::saveformat("TextON", ButtonOn.text());
    str += logisdom::saveformat("ValueAsText", ButtonOn.text());
    if ((family == family2413_A) || (family == family3A2100H_A)) str += logisdom::saveformat("DecimalOperate", QString("%1").arg(operateDecimal.isChecked()));
}



void ds2413::setconfig(const QString &strsearch)
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
    if ((family == family2413_A) || (family == family3A2100H_A))
    {
        int decop = logisdom::getvalue("DecimalOperate", strsearch).toInt(&ok);
        if (ok)
        {
            if (decop) operateDecimal.setCheckState(Qt::Checked);
            else operateDecimal.setCheckState(Qt::Unchecked);
        }
    }
}
