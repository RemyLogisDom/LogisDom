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
#include "ds2406.h"



ds2406::ds2406(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    dual_Ouput = false;
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
    if (family == family2406_A)
    {
        ui.labelfamily->setText(tr("DS2406 I/O 1"));
/*        operateDecimal.setText(tr("Operate as decimal"));
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
        opDecChanged(false);*/
    }
    else
    {
        if (family == family2406_B) ui.labelfamily->setText(tr("DS2406 I/O 2"));
    }
    ClearWarning();
    LogicalStatusTxt = ui.labelLogicalStatus->text();
    OpenDrainTxt = ui.labelOpenDrain->text();
    StatusTxt = ui.labelStatus->text();
}





void ds2406::contextMenuEvent(QContextMenuEvent * event)
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





void ds2406::SetOrder(const QString &)
{
}




QString ds2406::getOffCommand()
{
    if (family == family2406_A)
    {
        //return "05FFFFFF";
        return "25FFFFFFFF";
    }
    if (family == family2406_B)
    {
        return "09FFFFFF";
    }
    return "";
}



QString ds2406::getOnCommand()
{
    if (family == family2406_A)
    {
        //return "05FFFF00";
        return "25FFFF00FF";
    }
    if (family == family2406_B)
    {
        return "09FFFF00";
    }
    return "";
}




double ds2406::getMaxValue()
{
    if (family == family2406_A)
    {
        if (operateDecimal.isChecked()) return 3;
        return 1;
    }
    if (family == family2406_B)
    {
        return 1;
    }
    return 1;
}



void ds2406::assignMainValueLocal(double value)
{
    if (parent->isRemoteMode()) return;
    if (family != family2406_A) return;
    if (!operateDecimal.isChecked()) return;
    int data = (int)value;
    if (data < 1) data = 0;
    if (data > 3) data = 3;
    QString hex;
    if (data == 0) hex = "0DFFFFFF";
    if (data == 1) hex = "0DFFFFF0";
    if (data == 2) hex = "0DFFFF0F";
    if (data == 3) hex = "0DFFFF00";
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(ChannelAccessWrite, romid, hex);
        break;
#else
    case Ha7Net :
        //if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romid, command);
        break;
#endif
    case TCP_HA7SType :
            master->addtofifo(ChannelAccessWrite, romid, hex);
        break;
    case MultiGest :
            master->addtofifo(ChannelAccessWrite, romid, hex);
        break;
        case RemoteType : master->getMainValue();
        break;
    }
}




void ds2406::B0Click()
{
    assignMainValue(0);
}


void ds2406::B1Click()
{
    assignMainValue(1);
}

void ds2406::B2Click()
{
    assignMainValue(2);
}

void ds2406::B3Click()
{
    assignMainValue(3);
}


void ds2406::opDecChanged(int state)
{
    bool Output_state;
    if (state) Output_state = ui.radioOutput->isChecked();
    else Output_state = false;
    B0.setEnabled(Output_state);
    B1.setEnabled(Output_state);
    B2.setEnabled(Output_state);
    B3.setEnabled(Output_state);
    ui.Invert->setChecked(false);
}


void ds2406::On(bool)
{
    if (!master) return;
    QString romID_A = romid.left(16) + "_A";
    QString command;
    if (ui.radioInput->isChecked()) return;
    if (ui.Invert->isChecked())
    {
        command = getOffCommand();
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#else
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#endif
        case TCP_HA7SType :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
        case MultiGest :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
            case RemoteType : master->getMainValue();
            break;
        }
    }
    else
    {
        command = getOnCommand();
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#else
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#endif
        case TCP_HA7SType :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
        case MultiGest :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
            case RemoteType : master->getMainValue();
            break;
        }
    }
}





void ds2406::Off(bool)
{
    if (!master) return;
    QString romID_A = romid.left(16) + "_A";
    QString command;
    if (ui.radioInput->isChecked()) return;
    if (ui.Invert->isChecked())
    {
        command = getOnCommand();
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#else
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#endif
        case TCP_HA7SType :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
        case MultiGest :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
            case RemoteType : master->getMainValue();
            break;
        }
    }
    else
    {
        command = getOffCommand();
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#else
        case Ha7Net :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
#endif
        case TCP_HA7SType :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
        case MultiGest :
            if (!command.isEmpty()) master->addtofifo(ChannelAccessWrite, romID_A, command);
            break;
            case RemoteType : master->getMainValue();
            break;
        }
    }
}




bool ds2406::isSwitchFamily()
{
    if (ui.radioOutput->isChecked()) return true;
    else return false;
}



bool ds2406::isDimmmable()
{
    return operateDecimal.isChecked();
}


void ds2406::inputClick()
{
    ui.pushButtonON->setEnabled(false);
    ui.pushButtonOFF->setEnabled(false);
    B0.setEnabled(false);
    B1.setEnabled(false);
    B2.setEnabled(false);
    B3.setEnabled(false);
    ButtonOn.setEnabled(false);
    ButtonOff.setEnabled(false);
}



void ds2406::outputClick()
{
    ui.pushButtonON->setEnabled(true);
    ui.pushButtonOFF->setEnabled(true);
    bool state = operateDecimal.isChecked();
    B0.setEnabled(state);
    B1.setEnabled(state);
    B2.setEnabled(state);
    B3.setEnabled(state);
    ButtonOn.setEnabled(true);
    ButtonOff.setEnabled(true);
}




QString ds2406::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    return S;
}




void ds2406::lecture()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ChannelAccessRead, romID_A);
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






void ds2406::lecturerec()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ChannelAccessRead, romID_A);
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



//F5 05FF  FF  00  FFFF
//F5 05FF  12  00  4FB6
//F5 25FF  15  00  4646 00FFFFFFBFBF
bool ds2406::setscratchpad(const QString &scratchpad, bool enregistremode)
{
    ScratchPad_Show.setText(scratchpad);
    int decimalValue = logisdom::NA;
    logMsg(romid + " : " + scratchpad);
    QString CHANNEL_INFO_BYTE;
    QString CHANNEL_CONTROL_BYTE;
    QString DATA_WRITE;
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name;
    QString scratchpad_crc = scratchpad;
    if (scratchpad.length() == 14)  // F545FF10005B16
    {
        CHANNEL_INFO_BYTE = scratchpad.right(8).left(2);
        CHANNEL_CONTROL_BYTE = scratchpad.right(12).left(2);
        DATA_WRITE = scratchpad.right(6).left(2);
        if (scratchpad.right(8) == "FFFFFFFF")
        {
            if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
            return IncWarning();		// No answer from device not connected
        }
        if (scratchpad.right(8) == "00000000")
        {
            if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
            return IncWarning();		// 1 Wire bus shorted
        }
    }
    if (scratchpad.length() == 16)  // F525FF1000451600
    {
        CHANNEL_INFO_BYTE = scratchpad.right(8).left(2);
        CHANNEL_CONTROL_BYTE = scratchpad.right(10).left(2);
        DATA_WRITE = scratchpad.right(2);
        if (scratchpad.right(8) == "FFFFFFFF")
        {
            if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
            return IncWarning();		// No answer from device not connected
        }
        if (scratchpad.right(8) == "00000000")
        {
            if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
            return IncWarning();		// 1 Wire bus shorted
        }
        scratchpad_crc.chop(2);
    }
    bool ok;
    QString str;
    if (scratchpad.isEmpty())
    {
        return true;
    }
    int Channel_Info_Byte = CHANNEL_INFO_BYTE.toInt(&ok, 16);
    //int Channel_Control_Byte = CHANNEL_CONTROL_BYTE.toInt(&ok, 16);
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    /*QString CrcStr = scratchpad_crc.right(4);
    QString Crc = CrcStr.right(2) + CrcStr.left(2);
    uint16_t L = uint16_t((scratchpad_crc.length() / 2) - 2);
    byteVec_t scratchpadcrc = byteVec_t(L+1);
    for (uint16_t n=0; n<L; n++) scratchpadcrc[n] = scratchpad_crc.mid(n * 2, 2).toInt(&ok, 16);
    uint16_t crc = Crc.toInt(&ok, 16);
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    uint16_t crccalc = 0;
    crccalc = calcCRC16(&scratchpadcrc[0], L);
    if (crc != crccalc)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();		// CRC Wrong
    }
    else
    logMsg(QString("CRC ok (%1)").arg(crccalc));*/
    if (!checkCRC16(scratchpad)) return IncWarning();
    //int bit7_supply = status_int & 0x80;    // 0 = no supply
    //int bit6_NbOfchanel = status_int & 0x40; // 0 = chanel A Only
    //int bit5_actB = status_int & 0x20; // PIO B activuty latch
    //int bit4_actA = status_int & 0x10; // PIO A activity latch
    //int bit3_PIOB = status_int & 0x08; // PIO B Sense level
    int LogicState = Channel_Info_Byte & 0x04; // PIO A Sense level
    //int bit1_QB = status_int & 0x02; // PIO-B Channel Flip-Flop Q
    int OutputLatch = Channel_Info_Byte & 0x01; // PIO-A Channel Flip-Flop Q

    //int TOG_bit = Channel_Control_Byte & 0x20; // O = Write 1 = Read
    //int IM_bit = Channel_Control_Byte & 0x40; // PIO A Sense level
    /* TOG IM  CHANNELS EFFECT
       0   0   one channel Write all bits to the selected channel
       0   1   one channel Read all bits from the selected channel
       1   0   one channel Write 8 bits, read 8 bits, write, read, etc. to/from the selected channel
       1   1   one channel Read 8 bits, write 8 bits, read, write, etc. from/to the selected channel
       0   0   two channels Repeat: four times (write A, write B)
       0   1   two channels Repeat: four times (read A, read B)
       1   0   two channels Four times: (write A, write B), four times: (read A, read B), write, read, etc.
       1   1   two channels Four times: (read A, read B), four times: (write A, write B), read, write, etc.*/

    if (scratchpad.length() == 16)  // F525FF1000451600
    {
        QString read_port = scratchpad.right(2);
        if (read_port == "00") LogicState = 0;
        else if (read_port == "FF") LogicState = 1;
        else LogicState = logisdom::NA;
    }

//logMsg("CRC ok   " + scratchpad);
    if ((family == family2406_A) || (family == family3A2100H_A))
    {
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
// Logiscal status
    str = LogicalStatusTxt;
    if (LogicState == logisdom::NA) str += cstr::toStr(cstr::NA);
    else if (LogicState == 0) str += tr("0  (Low)");
    else if (LogicState > 0) str += tr("1  (High)");
    ui.labelLogicalStatus->setText(str);
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
    if (family == family2406_A)
    {
        if (operateDecimal.isChecked()) Value = decimalValue;
        //logMsg(QString("Decimal value = %1").arg(Value));
    }
    setMainValue(Value, enregistremode);
    return true;
}


/*
jeudi 16/05/2013 17:48:35:819  :  E7000000730A9D12_A : E7000000730A9D12_A :

jeudi 16/05/2013 17:48:35:819  :  E7000000730A9D12_A : CRC ok (46671)

jeudi 16/05/2013 17:48:39:289  :  E7000000730A9D12_A : E7000000730A9D12_A :
  F505FF12004FB6
  F505FF00FF0FF6
*/
void ds2406::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("Input", QString("%1").arg(ui.radioInput->isChecked()));
    str += logisdom::saveformat("Invert", QString("%1").arg(ui.Invert->isChecked()));
    str += logisdom::saveformat("TextOFF", ButtonOff.text());
    str += logisdom::saveformat("TextON", ButtonOn.text());
    str += logisdom::saveformat("ValueAsText", ButtonOn.text());
    if (family == family2406_A) str += logisdom::saveformat("DecimalOperate", QString("%1").arg(operateDecimal.isChecked()));
}



void ds2406::setconfig(const QString &strsearch)
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
    if (family == family2406_A)
    {
        int decop = logisdom::getvalue("DecimalOperate", strsearch).toInt(&ok);
        if (ok)
        {
            if (decop) operateDecimal.setCheckState(Qt::Checked);
            else operateDecimal.setCheckState(Qt::Unchecked);
        }
    }
}

