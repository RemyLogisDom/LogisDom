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





#include "ledonewire.h"
#include "globalvar.h"
#include "server.h"
#include "inputdialog.h"



ledonewire::ledonewire(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
    ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	ui.labelromid->setText(romid);
    ui.labelfamily->setText(tr("Led/Volet One Wire"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	family = romid.right(2);
	zone = romid.right(4).left(2).toUpper();
	MainValue = logisdom::NA;
    ledState = logisdom::NA;
    voletState = logisdom::NA;
	Unit.setText(" %");
    lastOnValue = logisdom::NA;
    modSleepEnabled = false;
    manual = false;
    lastManual = false;
    ButtonOn.setText(cstr::toStr(cstr::ON));
    setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    ButtonFullOn.setText(cstr::toStr(cstr::ON) + " 100%");
    setupLayout.addWidget(&ButtonFullOn, layoutIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);

    ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOff, layoutIndex++, 0, 1, logisdom::PaletteWidth/2);

    ButtonDim.setText(cstr::toStr(cstr::Dim));
    setupLayout.addWidget(&ButtonDim, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    ButtonBright.setText(cstr::toStr(cstr::Brigth));
    setupLayout.addWidget(&ButtonBright, layoutIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);

    TTV.setPrefix(tr("TTV : "));
    TTV.setRange(0, 255);
    ui.spinBoxVolet->setRange(0, 255);
    setupLayout.addWidget(&TTV, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    ButtonWriteTTV.setText(tr("Write TTV "));
    setupLayout.addWidget(&ButtonWriteTTV, layoutIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);

    ModeTxt = tr("Mode");
    ModeBrightTxt = tr("Bright");
    ModeDimTxt = tr("Dim");
    ModeOnTxt = tr("On");
    ModeOffTxt = tr("Off");
    ModeFullOnTxt = tr("100%");
    ModeSleepTxt = tr("Sleep");

    htmlBind->addParameterCommand(ModeTxt, ModeBrightTxt, setModeBright);
    htmlBind->addParameterCommand(ModeTxt, ModeDimTxt, setModeDim);
    htmlBind->addParameterCommand(ModeTxt, ModeOnTxt, setModeOn);
    htmlBind->addParameterCommand(ModeTxt, ModeOffTxt, setModeOff);
    htmlBind->addParameterCommand(ModeTxt, ModeFullOnTxt, setModeFullOn);
    htmlBind->addParameterCommand(ModeTxt, ModeSleepTxt, setModeSleep);

    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickOn()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(clickOff()));
    connect(&ButtonFullOn, SIGNAL(clicked()), this, SLOT(clickFullOn()));
    connect(&ButtonDim, SIGNAL(clicked()), this, SLOT(clickDim()));
    connect(&ButtonBright, SIGNAL(clicked()), this, SLOT(clickBright()));
    connect(&ButtonWriteTTV, SIGNAL(clicked()), this, SLOT(clickWriteTTV()));

    connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(clickOn()));
    connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(clickOff()));
    connect(ui.pushButtonFULL, SIGNAL(clicked()), this, SLOT(clickFullOn()));
    connect(ui.pushButtonDIM, SIGNAL(clicked()), this, SLOT(clickDim()));
    connect(ui.pushButtonBRIGHT, SIGNAL(clicked()), this, SLOT(clickBright()));
    connect(ui.pushButtonOpen, SIGNAL(clicked()), this, SLOT(clickOpen()));
    connect(ui.pushButtonSet, SIGNAL(clicked()), this, SLOT(clickSetVolet()));
    connect(ui.pushButtonClose, SIGNAL(clicked()), this, SLOT(clickClose()));
    connect(ui.pushButtonSLEEP, SIGNAL(clicked()), this, SLOT(clickSleep()));
}



void ledonewire::remoteCommandExtra(const QString &cmd)
{
    if (cmd == setModeOn) ButtonOn.click();
    if (cmd == setModeOff) ButtonOff.click();
    if (cmd == setModeBright) ButtonBright.click();
    if (cmd == setModeDim) ButtonDim.click();
    if (cmd == setModeFullOn) ButtonFullOn.click();
    if (cmd == setModeSleep) { Dim(1) ; modSleepEnabled = true; }
}




void ledonewire::clickOn()
{
	On(true);
    modSleepEnabled = false;
	emit(stateChanged());
}




void ledonewire::clickOff()
{
	Off(true);
    modSleepEnabled = false;
    emit(stateChanged());
}



void ledonewire::clickDim()
{
    if (int(MainValue) == logisdom::NA)
    {
        lecture();
        if (int(MainValue) == logisdom::NA) return;
    }
    if (parent->isRemoteMode()) return;
    if (ledState >= 20) Dim(10);
    else Dim(2);
}




void ledonewire::Dim(int dimValue)
{
    // Command / Data / crc8Table
    //61 + data (1 Byte) + crc(1 Bytes);
    int v = ledState - dimValue;
    if (v < 0) v = 0;
    setPowerValue(v);
}




void ledonewire::setPowerValue(int value)
{
    QString hex = setPower + QString("%1").arg(value, 2, 16, QChar('0'));
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
    if (ui.radioButtonLED->isChecked()) setMainValue(value, true);
    ledState = value;
}



void ledonewire::clickBright()
{
// Command / Data / crc8Table
//61 + data (1 Byte) + crc(1 Bytes);
    if (ledState == logisdom::NA)
    {
        lecture();
        if (ledState == logisdom::NA) return;
    }
    int v;
    if (parent->isRemoteMode()) return;
    if (ledState >= 10) v = ledState + 10;
    else v = ledState + 2;
    if (v > 100) v = 100;
    QString hex = setPower + QString("%1").arg(v, 2, 16, QChar('0'));
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
    if (ui.radioButtonLED->isChecked()) setMainValue(v, true);
    ledState = v;
}





void ledonewire::clickWriteTTV()
{
// Command / Data / crc8Table
//6A + data (1 Byte) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = setTTV + QString("%1").arg(TTV.value(), 2, 16, QChar('0'));
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
}



void ledonewire::clickFullOn()
{
// Command / Data / crc8Table
//61 + data (1 Byte) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = setPower "64";
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
}




void ledonewire::On(bool)
{
// Command / Data / crc8Table
//62 + data (1 Byte) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = setPowerOn "00";
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
}





void ledonewire::Off(bool)
{
// Command / Data / crc8Table
//63 + data (1 Byte) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = setPowerOff "00";
    addCRC8(hex);
    hex += "FF";
    sendCommand(hex);
    lastOnValue = ledState;
}



void ledonewire::clickOpen()
{
    // Command / Data / crc8Table
    //64 + data (1 Byte) + crc(1 Bytes);
        if (parent->isRemoteMode()) return;
        QString hex = voletOpen "00";
        addCRC8(hex);
        hex += "FF";
        sendCommand(hex);
}


void ledonewire::clickClose()
{
    // Command / Data / crc8Table
    //65 + data (1 Byte) + crc(1 Bytes);
        if (parent->isRemoteMode()) return;
        QString hex = voletClose "00";
        addCRC8(hex);
        hex += "FF";
        sendCommand(hex);
}

void ledonewire::clickSleep()
{
    modSleepEnabled = true;
}


void ledonewire::clickSetVolet()
{
    // Command / Data / crc8Table
    //65 + data (1 Byte) + crc(1 Bytes);
        if (parent->isRemoteMode()) return;
        //int v = ui.spinBoxVolet->value();
        assignMainValueLocal(ui.spinBoxVolet->value());
        //QString hex;
        //if (v > 0)   hex = voletOpen + QString("%1").arg(v, 2, 16, QChar('0'));
        //if (v < 0)   hex = voletClose + QString("%1").arg(-v, 2, 16, QChar('0'));
        //addCRC8(hex);
        //hex += "FF";
        //sendCommand(hex);
}



void ledonewire::addCRC8(QString &hex)
{
    bool ok;
// calc crc
    //int L = (hex.length() / 2);
    //byteVec_t scratchpadcrc = byteVec_t(L);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);
    //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
    QVector <uint8_t> v;
    for (int n=0; n<hex.length(); n+=2) v.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc = onewiredevice::calcCRC8(v);
// add crc
    hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
}



void ledonewire::sendCommand(const QString hexstr)
{
    QString hex = hexstr.toUpper();
    logMsg("Write " + hex);
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(WriteLed, romid, hex);
        break;
#else
    case Ha7Net :
        //master->addtofifo(WriteLed, romid, hex);
        break;
#endif
    case TCP_HA7SType :
        master->addtofifo(WriteLed, romid, hex);
        break;
    case MultiGest :
        master->addtofifo(WriteLed, romid, hex);
        break;
    case RemoteType : master->getMainValue();
        break;
    }
}




void ledonewire::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
    QAction ActionBright(tr("&Bright"), this);
    QAction ActionDim(tr("&Dim"), this);
    QAction ActionOn(tr("&On"), this);
    QAction ActionOff(tr("&Off"), this);
    QAction ActionFullOn(tr("100%"), this);
    QAction Action50(tr("50%"), this);
    QAction Action30(tr("30%"), this);
    QAction ActionSleep(tr("&Sleep"), this);
    QAction *selection;
    contextualmenu.addAction(&Lecture);
    contextualmenu.addAction(&ActionBright);
    contextualmenu.addAction(&ActionDim);
    contextualmenu.addAction(&ActionOn);
    contextualmenu.addAction(&ActionOff);
    contextualmenu.addAction(&ActionFullOn);
    contextualmenu.addAction(&Action50);
    contextualmenu.addAction(&Action30);
    contextualmenu.addAction(&ActionSleep);
    if (!parent->isRemoteMode())
	{
	}
	selection = contextualmenu.exec(event->globalPos());
    if (selection == &Lecture) lecture();
    if (selection == &ActionBright) clickBright();
    if (selection == &ActionDim) clickDim();
    if (selection == &ActionOn) clickOn();
    if (selection == &ActionOff) clickOff();
    if (selection == &ActionFullOn) clickFullOn();
    if (selection == &Action50) setPowerValue(50);
    if (selection == &Action30) setPowerValue(30);
    if (selection == &ActionSleep) { Dim(1); modSleepEnabled = true; }
}






void ledonewire::SetOrder(const QString &order)
{
	if (order == NetRequestMsg[ReadTemp]) lecture();
}




QString ledonewire::MainValueToStrLocal(const QString &str)
{
	QString S = str;
    if (ledState == logisdom::NA) ui.pushButtonPRC->setText(cstr::toStr(cstr::NA)); else ui.pushButtonPRC->setText(QString("%1%").arg(ledState));
    if (voletState == logisdom::NA) ui.pushButtonVolet->setText(cstr::toStr(cstr::NA)); else ui.pushButtonVolet->setText(QString("%1").arg(voletState));
    return S;
}




void ledonewire::lecture()
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






void ledonewire::lecturerec()
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
    if (modSleepEnabled)
    {
         Dim(1);
         if (ledState == 0) modSleepEnabled = false;
    }
}





bool ledonewire::setscratchpad(const QString &scratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	bool ok;
    //QString scratchpad = scratchpa;
    ScratchPad_Show.setText(scratchpad);
    logMsg(scratchpad);
    // "610054AA"
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    if (scratchpad.length() == 8)
    {
       if (scratchpad.right(2) == "AA")
       {
           QString command = scratchpad.left(2);
           int data = scratchpad.mid(2,2).toInt(&ok, 16);
           if ((command == setPower) && ok) { ledState = data; if (ui.radioButtonLED->isChecked()) setMainValue(ledState, true); }
           else if (command == setPowerOn) { ledState = lastOnValue; if (ui.radioButtonLED->isChecked()) setMainValue(ledState, true);}
           else if (command == setPowerOff) { ledState = 0; if (ui.radioButtonLED->isChecked()) setMainValue(ledState, true); }
           else if (command == voletOpen) { voletState += data; if (voletState > TTV.value()) voletState = TTV.value(); if (ui.radioButtonVolet->isChecked()) setMainValue(voletState, true); }
           else if (command == voletClose) { voletState -= data; if (voletState < 0) voletState = 0; if (ui.radioButtonVolet->isChecked()) setMainValue(voletState, true); }
           return true;
       }
       else
       {
           if (master && WarnEnabled.isChecked()) master->GenError(25, errorMsg);
           return IncWarning();		// No answer from device not connected
       }
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
	// CRC Check
	//logMsg("CRC Check");
    //int L = (scratchpad.length() / 2) - 1;
    //if (L <= 0)
    //{
    //    if (master && WarnEnabled.isChecked()) master->GenError(26, errorMsg);
    //    return IncWarning();
    //}
    //byteVec_t scratchpadcrc = byteVec_t(L+1);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = scratchpad.mid(n * 2, 2).toInt(&ok, 16);
    //uint8_t crc = scratchpad.right(2).toInt(&ok, 16);
    //logMsg(QString("crc (%1)").arg(crc));
    //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
    //logMsg(QString("crccalc (%1)").arg(crccalc));
    //if (crc != crccalc)
    //{
    //	if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
    //	return IncWarning();		// CRC Wrong
    //}
    //else logMsg(QString("CRC ok (%1)").arg(crccalc));
    if (!checkCRC8(scratchpad)) return IncWarning();
    ledState = scratchpad.mid(0,2).toInt(&ok, 16);
    voletState = scratchpad.mid(2,2).toInt(&ok, 16);
    int ttv = scratchpad.mid(4,2).toInt(&ok, 16);
    int voletManual = scratchpad.mid(14,2).toInt(&ok, 16) & 0x0001;
    if (ok && voletManual && (lastManual != voletManual)) manual = true;
    lastManual = voletManual;
    //logMsg(QString("Manual (%1)").arg(manual));
    if (!TTV.isActiveWindow()) TTV.setValue(ttv);
    if (ui.radioButtonLED->isChecked()) setMainValue(double(ledState), enregistremode);
    else setMainValue(double(voletState), enregistremode);
	return true;
}




void ledonewire::assignMainValueLocal(double value)
{
    bool ok;
    double assignedValue = value;
    MainValue = assignedValue;
// Command / Data / crc8Table
// 61 + data (1 Byte) + crc(1 Bytes);
    if (int(MainValue) == logisdom::NA)
    {
        lecture();
        if (int(MainValue) == logisdom::NA) return;
    }
    int v = int(assignedValue);
    QString hex;
    if (ui.radioButtonLED->isChecked())
    {
        if (parent->isRemoteMode()) return;
        if (assignedValue < 0) v = 0;
        if (assignedValue > 100) v = 100;
        hex = setPower + QString("%1").arg(v, 2, 16, QChar('0'));
    }
    else
    {
        if (int(value) == 0) hex = voletClose "00";
        else if (value >= TTV.value()) hex = voletOpen "00";
        else if (value > voletState) hex = voletOpen + QString("%1").arg(int((value - voletState)), 2, 16, QChar('0'));
        else if (value < voletState) hex = voletClose + QString("%1").arg(int((voletState - value)), 2, 16, QChar('0'));
        //addCRC8(hex);
        //hex += "FF";
        //sendCommand(hex);
    }
// calc crc
    //int L = (hex.length() / 2);
    //byteVec_t scratchpadcrc = byteVec_t(L);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);
    //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
    QVector <uint8_t> hexv;
    for (int n=0; n<hex.length(); n+=2) hexv.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc = calcCRC8(hexv);
// add crc
    hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
    hex += "FF";
    sendCommand(hex);
}




double ledonewire::getMaxValue()
{
    if (ui.radioButtonLED->isChecked()) return 100;
    else return TTV.value();
}



bool ledonewire::isSwitchFamily()
{
    return true;
}



bool ledonewire::isDimmmable()
{
    return true;
}



bool ledonewire::isManual()
{
    if (manual)
    {
        manual = false;
        return true;
    }
    return false;
}


void ledonewire::writescratchpad()
{
}





void ledonewire::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("LED_Mode", QString("%1").arg(ui.radioButtonLED->isChecked()));
}




void ledonewire::setconfig(const QString &strsearch)
{
    bool ok;
    QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(tr("LED Driver")));
    int led = logisdom::getvalue("LED_Mode", strsearch).toInt(&ok);
    if (ok)
    {
        if (led) ui.radioButtonLED->setChecked(Qt::Checked);
        else ui.radioButtonVolet->setChecked(Qt::Checked);
    }
}




