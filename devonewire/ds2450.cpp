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
#include "configwindow.h"
#include "net1wire.h"
#include "ds2450.h"




ds2450::ds2450(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	QDateTime Now = QDateTime::currentDateTime();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(4);
    ui.labelromid->setText(romid);
	ui.labelfamily->setText(tr("DS2450 : Quad ADC"));
	if (master) ui.labelmaster->setText(master->getipaddress());
    if (family == family2450_A)
    {
        commonReg = new CommonRegStruct;
        commonReg->Reg = -1;
        ui.MainLabel->setText(tr("ADC port A"));
    }
    else
    {
        QString romIDA = romid.left(16) + "_A";
        onewiredevice *device = parent->configwin->DeviceExist(romIDA);
        if (device) commonReg = device->commonReg; else commonReg = nullptr;
        if (family == family2450_B) ui.MainLabel->setText(tr("ADC port B"));
        if (family == family2450_C) ui.MainLabel->setText(tr("ADC port C"));
        if (family == family2450_D) ui.MainLabel->setText(tr("ADC port D"));
    }
	MainValue = logisdom::NA;
    IR_bit.setText(tr("IR bit"));
    IR_bit.setToolTip(tr("Uncheck 2.55v / Checked 5.10v"));
    OE_bit.setText(tr("OE bit"));
    OC_bit.setText(tr("OC bit"));
    resolution.setRange(1, 16);
    resolution.setPrefix(tr("Resolution") + " ");
    resolution.setSuffix(" " + tr("bits"));
    resolution.setValue(8);
    setupLayout.addWidget(&IR_bit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&resolution, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&OE_bit, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&OC_bit, layoutIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&AXplusB, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    //connect(&IR_bit, SIGNAL(clicked(bool)), this, SLOT(configchanged()));
    //connect(&OC_bit, SIGNAL(clicked(bool)), this, SLOT(configchanged()));
    //connect(&OE_bit, SIGNAL(clicked(bool)), this, SLOT(configchanged()));
}



void ds2450::configchanged()
{
    lecture();
}



void ds2450::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
    //QAction SetConfig(tr("&Write Config"), this);
	QAction Nom(tr("&Name"), this);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Lecture);
        //contextualmenu.addAction(&SetConfig);
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
    //if (selection == &SetConfig) writeMemory();
	if (selection == &Nom) changename();
}






QString ds2450::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.MainText->setText(S);
	return S;
}




void ds2450::SetOrder(const QString &)
{
}



void ds2450::lecture()
{
	QString RomIDLeft = romid.left(17) + "A";
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
            master->addtofifo(ReadADCPage01h, RomIDLeft);
            master->addtofifo(ConvertADC, RomIDLeft);
			master->addtofifo(ReadADC, RomIDLeft);
			break;
#else
		case Ha7Net :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid));
			break;
#endif
		case TCP_HA7SType :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid));
			break;
		case MultiGest :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid));
			break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void ds2450::lecturerec()
{
	QString RomIDLeft = romid.left(17) + "A";
    setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
            master->addtofifo(ReadADCPage01h, RomIDLeft);
            master->addtofifo(ConvertADC, RomIDLeft);
            master->addtofifo(ReadADCRec, RomIDLeft);
			break;
#else
		case Ha7Net :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid), true);
			break;
#endif
		case TCP_HA7SType :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid), true);
			break;
		case MultiGest :
            setscratchpad(master->getScratchPad(romid, 1), true);
            setscratchpad(master->getScratchPad(romid), true);
			break;
		case RemoteType : master->saveMainValue();
		break;
	}
}




bool ds2450::setscratchpad(const QString &myscratchpad, bool enregistremode)
{
    QString scratchpad = myscratchpad;
    int page = 0;
    onewiredevice *deviceB;
    onewiredevice *deviceC;
    onewiredevice *deviceD;
    QString RomIDLeftB = romid.left(17) + "B";
    QString RomIDLeftC = romid.left(17) + "C";
    QString RomIDLeftD = romid.left(17) + "D";
    QString str = logisdom::getvalue("page01h", scratchpad);
    if (str.isEmpty())
    {
        ScratchPad_Show.setText(scratchpad + (" page 0"));
    }
    else
    {
        ScratchPad_Show.setText(scratchpad + (" page 1"));
        scratchpad = str;
        page = 0x01;
    }
	bool ok;
	logMsg(scratchpad);
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name;
	if (scratchpad.isEmpty())
	{
		logMsg(romid + "  scratchpad empty !!!");
		return IncWarning();
	}
	if (scratchpad == "FFFFFFFFFFFFFFFFFFFFFFFFFF")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
		return IncWarning();		// No answer from device not connected
	}
	if (scratchpad == "00000000000000000000000000")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
		return IncWarning();		// 1 Wire bus shorted
	}
	// CRC Check   AA000000030003000300031B24
    //int L = (scratchpad.length() / 2) - 2;
    //if (L <= 0)
    //{
    //    if (master && WarnEnabled.isChecked()) master->GenError(26, errorMsg);
    //    return IncWarning();
    //}
    //byteVec_t scratchpadcrc = byteVec_t(L+1);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = scratchpad.mid(n * 2, 2).toInt(&ok, 16);
    //QString CrcStr = scratchpad.right(4);
    //QString Crc = CrcStr.right(2) + CrcStr.left(2);
    //uint16_t crc = Crc.toInt(&ok, 16);
//	logMsg(QString("CRC received (%1)   scratchpad : ").arg(crc) + scratchpad);
    //if (!ok) return IncWarning();
    //uint16_t crccalc = calcCRC16(&scratchpadcrc[0], L);
//	logMsg(QString("CRC calc (%1)").arg(crccalc));
    //if (crc != crccalc)
    //{
    //	if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
     //   logMsg("Bad CRC");
    //	return IncWarning();		// CRC Wrong
    //}
    //else logMsg(QString("CRC ok (%1)").arg(crccalc));
    if (!checkCRC16(scratchpad)) return IncWarning();
	QString Value;
    if (page == 0x00)
    {
        if (family == family2450_A)
        {
            Value = scratchpad.mid(8,2) + scratchpad.mid(6,2);
            if (master)
            switch (master->gettype())
            {
    #ifdef HA7Net_No_Thread
            case Ha7Net :
                deviceB = parent->configwin->DeviceExist(RomIDLeftB);
                if (deviceB) deviceB->setscratchpad(scratchpad, enregistremode);
                deviceC = parent->configwin->DeviceExist(RomIDLeftC);
                if (deviceC) deviceC->setscratchpad(scratchpad, enregistremode);
                deviceD = parent->configwin->DeviceExist(RomIDLeftD);
                if (deviceD) deviceD->setscratchpad(scratchpad, enregistremode);
                break;
    #endif
            }
        }
        else if (family == family2450_B) Value = scratchpad.mid(12,2) + scratchpad.mid(10,2);
        else if (family == family2450_C) Value = scratchpad.mid(16,2) + scratchpad.mid(14,2);
        else if (family == family2450_D) Value = scratchpad.mid(20,2) + scratchpad.mid(18,2);
        double V = Value.toInt(&ok,16);
        if (!ok) return IncWarning();
        V = AXplusB.result(V);
        ui.MainText->setText(QString("%1").arg(MainValue));
        setMainValue(V, enregistremode);
    }
    if (page == 0x01)
    {
        logMsg("page 1 scratchpad " + scratchpad);
        int IRbit, res;
        int index = 0;
        int adr = 0;
        if (family == family2450_A) { adr = 8; index = 6; }
        if (family == family2450_B) { adr = 10; index = 10; }
        if (family == family2450_C) { adr = 12; index = 14; }
        if (family == family2450_D) { adr = 14; index = 18; }
        res = scratchpad.mid(index,2).toInt(&ok, 16);
        if (ok)
        {
            int config = resolution.value() + (OC_bit.isChecked() * 0x40) + (OE_bit.isChecked() * 0x80);
            if ((res & 0x0F) != config)
            {
                QString newcfg = QString("%1").arg(config, 2, 16, QChar('0')).toUpper();
                QString adrstr = QString("%1").arg(adr, 2, 16, QChar('0')).toUpper() + "00";
                QString str = adrstr + newcfg + "FFFFFF";
                master->addtofifo(WriteMemory, romid, str);
                logMsg("Write to memory " + str + " to address " + adrstr);
            }
        }
        else logMsg("error converting " + scratchpad.mid(index,2));
        IRbit = scratchpad.mid(index + 2,2).toInt(&ok, 16);
        if (ok)
        {
            if ((IRbit & 0x01) != IR_bit.isChecked())
            {
                QString adrstr = QString("%1").arg(adr+1, 2, 16, QChar('0')).toUpper() + "00";
                QString str = adrstr + "00" "FFFFFF";
                if (IR_bit.isChecked()) str = adrstr + "01" "FFFFFF";
                master->addtofifo(WriteMemory, romid, str);
                logMsg("Write to memory " + str + " to address " + adrstr);
            }
        }
        else logMsg("error converting " + scratchpad.mid(8,2));
    }
    return true;
}




void ds2450::GetConfigStr(QString &str)
{
	AXplusB.GetConfigStr(str);
    str += logisdom::saveformat("ADC_resolution", QString("%1").arg(resolution.value()));
    str += logisdom::saveformat("IR_bit", QString("%1").arg(IR_bit.isChecked()));
    str += logisdom::saveformat("OE_bit", QString("%1").arg(OE_bit.isChecked()));
    str += logisdom::saveformat("OC_bit", QString("%1").arg(OC_bit.isChecked()));
}




void ds2450::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("ADC ")));
	AXplusB.setconfig(strsearch);
    bool ok;
    int res = logisdom::getvalue("ADC_resolution", strsearch).toInt(&ok);
    if (ok) resolution.setValue(res);
    int IR = logisdom::getvalue("IR_bit", strsearch).toInt(&ok);
    if (IR) IR_bit.setChecked(true);
    int OE = logisdom::getvalue("OE_bit", strsearch).toInt(&ok);
    if (OE) OE_bit.setChecked(true);
    int OC = logisdom::getvalue("OC_bit", strsearch).toInt(&ok);
    if (OC) OC_bit.setChecked(true);
}


