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





#include "lcdonewire.h"
#include "configwindow.h"
#include "globalvar.h"
#include "server.h"
#include "inputdialog.h"
#include "devchooser.h"




lcdonewire::lcdonewire(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
    copyValue = new devchooser(parent);
    if (copyValue) copyValue->setHtmlBindControler(htmlBind);
    connectedDevice = nullptr;
    connect(copyValue, SIGNAL(deviceSelectionChanged()), this, SLOT(deviceSelectionChanged()));
    ui.gridLayout->addWidget(&trafficUi, 3, 3, 1, 2);
    lastsavevalue = logisdom::NA;
    lastCopyValue = logisdom::NA;
    romid = RomID;
    family = romid.right(4);
    ui.labelromid->setText(romid);
	if (master) ui.labelmaster->setText(master->getipaddress());
	MainValue = logisdom::NA;
    assignTry = 0;
    Unit.setText("");
    //connect(ui.pushButtonON, SIGNAL(clicked()), this, SLOT(clickX10On()));
    //connect(ui.pushButtonOFF, SIGNAL(clicked()), this, SLOT(clickX10Off()));
	//connect(this, SIGNAL(stateChanged()), parent->SwitchArea , SLOT(updateStatus()));
	//connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickX10On()));
    if (family == familyLCDOW_A)
    {
        ui.labelfamily->setText(tr("LCD One Wire 1"));
    }
    else
    {
        if (family == familyLCDOW_B) ui.labelfamily->setText(tr("LCD One Wire 2"));
        if (family == familyLCDOW_C) ui.labelfamily->setText(tr("LCD One Wire 3"));
        if (family == familyLCDOW_D) ui.labelfamily->setText(tr("LCD One Wire 4"));
    }
    //setupLayout.addWidget(&AXplusB, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    writeValue.setRange(-32767, 32768);
    setupLayout.addWidget(&writeValue, layoutIndex, 1, 1, 1);
    ButtonWrite.setText(tr("Write"));
    setupLayout.addWidget(&ButtonWrite, layoutIndex++, 2, 1, 1);
    connect(&ButtonWrite, SIGNAL(clicked()), this, SLOT(writePort()));

    ButtonWriteValText.setText(tr("Write Text"));
    setupLayout.addWidget(&ButtonWriteValText, layoutIndex, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    //textValue.setInputMask("xxxxxxxxxxxxxx");
    setupLayout.addWidget(&textValue, layoutIndex++, 0, 1, logisdom::PaletteWidth/2);
    connect(&ButtonWriteValText, SIGNAL(clicked()), this, SLOT(writeValTxt()));

    ButtonWritePrefix.setText(tr("Write Prefix"));
    setupLayout.addWidget(&ButtonWritePrefix, layoutIndex, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    //prefixStr.setInputMask("xxxxxxx");
    setupLayout.addWidget(&prefixStr, layoutIndex++, 0, 1, logisdom::PaletteWidth/2);
    connect(&ButtonWritePrefix, SIGNAL(clicked()), this, SLOT(writePrefix()));

    ButtonWriteSuffix.setText(tr("Write Suffix"));
    setupLayout.addWidget(&ButtonWriteSuffix, layoutIndex, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    //suffixStr.setInputMask("xxxxxxx");
    setupLayout.addWidget(&suffixStr, layoutIndex++, 0, 1, logisdom::PaletteWidth/2);
    connect(&ButtonWriteSuffix, SIGNAL(clicked()), this, SLOT(writeSuffix()));

    Fontsize.setRange(0, 2);
    TextFontSize.setRange(0, 2);
    XPos.setRange(0, 127);
    YPos.setRange(0, 7);
    XPosTxt.setRange(0, 127);
    YPosTxt.setRange(0, 7);
    Digits.setRange(0, 5);
    Virgule.setRange(0, 4);
    CoefMult.setRange(1, 255);
    CoefDiv.setRange(1, 255);

    Fontsize.setValue(1);
    TextFontSize.setValue(1);
    CoefMult.setValue(1);
    CoefDiv.setValue(1);
    //XPos.setValue(0);
    //YPos.setValue(0);
    //XPosTxt.setValue(0);
    //YPosTxt.setValue(0);
    Digits.setValue(2);
    CoefLabel.setText("Coef : ");
    setupLayout.addWidget(&CoefLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    Coef.setText("1");
    setupLayout.addWidget(&Coef, layoutIndex++, 1, 1, logisdom::PaletteWidth/2);
    //Virgule.setValue(0);

    copyValueLabel.setText("Value from device :");
    setupLayout.addWidget(&copyValueLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(copyValue, layoutIndex++, 1, 1, logisdom::PaletteWidth/2);

    Fontsize.setPrefix(tr("Font Size : "));
    TextFontSize.setPrefix(tr("Text Font Size : "));
    XPos.setPrefix(tr("Position X : "));
    YPos.setPrefix(tr("Position Y : "));
    XPosTxt.setPrefix(tr("Position text X : "));
    YPosTxt.setPrefix(tr("Position text Y : "));
    Digits.setPrefix(tr("Digits : "));
    Virgule.setPrefix(tr("Coma : "));
    CoefMult.setPrefix(tr("Coef Mult : "));
    CoefDiv.setPrefix(tr("Coef Div : "));

    boxConfig.setLayout(&configLayout);
    boxConfig.setTitle(tr("Configuration"));
    int configIndex = 0;
    setupLayout.addWidget(&boxConfig, layoutIndex++, 0, 1, logisdom::PaletteWidth);

    configLayout.addWidget(&XPos, configIndex, 0, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&YPos, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&XPosTxt, configIndex, 0, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&YPosTxt, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&Digits, configIndex, 0, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&Virgule, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&Fontsize, configIndex, 0, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&TextFontSize, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&CoefMult, configIndex, 0, 1, logisdom::PaletteWidth/2);
    configLayout.addWidget(&CoefDiv, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    if (family == familyLCDOW_A)
    {
        readConfig();
        ButtonReadConfig.setText(tr("Read configuration"));
        configLayout.addWidget(&ButtonReadConfig, configIndex, 0, 1, logisdom::PaletteWidth/2);
        connect(&ButtonReadConfig, SIGNAL(clicked()), this, SLOT(readConfig()));
    }
    ButtonWriteConfig.setText(tr("Write configuration"));
    configLayout.addWidget(&ButtonWriteConfig, configIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    connect(&ButtonWriteConfig, SIGNAL(clicked()), this, SLOT(writeConfig()));
}



/*

#define writeMemoryLCD 0xF5
#define ADDR_FontSize 1
#define ADDR_XPos 2
#define ADDR_YPos 3
#define ADDR_Digits 4
#define ADDR_Virgule 5
#define ADDR_Prefix 8
#define Prefix_MaxSize 24
#define ADDR_Suffix 32
#define Suffix_MaxSize 8
#define strMaxSize 30
#define Mem_Size 64


writeMemoryLCD map

write address 0
    Rien
    FontSize
    XPos
    YPos
    Digits
    Virgule
    Rien
    Rien
write address 8
    Prefix Bytes 0 - 7
write address 16
    Prefix Bytes 8 - 15
write address 24
    Prefix Bytes 15 - 23
write address 32
    Suffix Bytes 0 - 7
write address 40
    ValText Bytes 0 - ...


F5 + ID (1 Byte) + Address(1 Byte) + Data (8 Bytes) + crc(1 Bytes);
Write sequence for LCD 1 Wire
You must provide an hex code containing the command, address, 8 byte data and 0xFF for the crc feedback
The address is to point in the EEprom
The data is 8 bytes
The control is the crc
*/

lcdonewire::~lcdonewire()
{
}



void lcdonewire::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
    QAction Nom(tr("&Name"), this);
    QAction *selection;
	contextualmenu.addAction(&Lecture);
	if (!parent->isRemoteMode())
	{
        contextualmenu.addAction(&Nom);
    }
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
    if (selection == &Nom) changename();
}



void lcdonewire::writePort()
{
    assignMainValue(writeValue.value());
}





QString lcdonewire::toHex(QByteArray data)
{
    QString hex;
    for (int n=0; n<data.length(); n++) hex += QString("%1").arg(uchar(data[n]), 2, 16, QChar('0'));
    return hex.toUpper();
}





QString lcdonewire::fromHex(const QString hexstr)
{
    bool ok;
    QString result;
    int L = hexstr.length() / 2;
    for (int n=0; n<L; n++)
    {
        char ch = hexstr.mid(n*2, 2).toInt(&ok, 16);
        if (ok && ch) result.append(ch);
    }
    return result;
}



void lcdonewire::sendWriteValText(const QString hexstr)
{
    QString hex = hexstr.toUpper();
    logMsg("Write " + hex);
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(WriteValText, romid, hex);
        break;
#else
    case Ha7Net :
        //master->addtofifo(WriteValText, romid, hex);
        break;
#endif
    case TCP_HA7SType :
        master->addtofifo(WriteValText, romid, hex);
        break;
    case MultiGest :
        master->addtofifo(WriteValText, romid, hex);
        break;
        case RemoteType : master->getMainValue();
        break;
    }
}






void lcdonewire::sendReadMemory(const QString hexstr)
{
    QString hex = hexstr.toUpper();
    logMsg("Write " + hex);
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(ReadMemoryLCD, romid, hex);
        break;
#else
    case Ha7Net :
        //master->addtofifo(ReadMemoryLCD, romid, hex);
        break;
#endif
    case TCP_HA7SType :
        master->addtofifo(ReadMemoryLCD, romid, hex);
        break;
    case MultiGest :
        master->addtofifo(ReadMemoryLCD, romid, hex);
        break;
    }
}



void lcdonewire::writePrefix()
{
    bool ok;
//F5 + ID (1 Byte) + Address(1 Byte) + Data (8 Bytes) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = writeMemoryLCD;
// ID
    if (family == familyLCDOW_A) hex += "00";
    else if (family == familyLCDOW_B) hex += "01";
    else if (family == familyLCDOW_C) hex += "02";
    else if (family == familyLCDOW_D) hex += "03";
// Address
    hex += QString("%1").arg(uchar(ADDR_Prefix), 2, 16, QChar('0'));
// String
    QByteArray my;
    my.append(prefixStr.text().toLatin1());
    int L = my.length();
    if (L > 7) my.chop(L-7);
    while (my.length() < 7) { my.append(char(0)); }
 // Add nullptr terminsaison
    hex += toHex(my) + "00";
 // calc crc
    QVector <uint8_t> scratchpad;
    for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc = calcCRC8(scratchpad);
// add crc
     hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
     sendWriteValText(hex);
}




// F5 00 32 61 62 63 64 66 67 68 00 CC

void lcdonewire::writeSuffix()
{
    bool ok;
//F5 + ID (1 Byte) + Address(1 Byte) + Data (8 Bytes) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = writeMemoryLCD;
// ID
    if (family == familyLCDOW_A) hex += "00";
    else if (family == familyLCDOW_B) hex += "01";
    else if (family == familyLCDOW_C) hex += "02";
    else if (family == familyLCDOW_D) hex += "03";
// Address
    hex += QString("%1").arg(uchar(ADDR_Suffix), 2, 16, QChar('0'));
// String
    QByteArray my;
    my.append(suffixStr.text().toLatin1());
    int L = my.length();
    if (L > 7) my.chop(L-7);
    while (my.length() < 7) { my.append(char(0)); }
 // Add nullptr terminsaison
    hex += toHex(my) + "00";
 // calc crc
     //L = (hex.length() / 2);


     //byteVec_t scratchpadcrc = byteVec_t(L);
     //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);

     QVector <uint8_t> scratchpad;
     for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));

     //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
     uint8_t crccalc = calcCRC8(scratchpad);
     //qDebug() << QString("crc = %1, crc2 = %2").arg(crccalc).arg(crccalc2);

 // add crc
     hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
     sendWriteValText(hex);
}



void lcdonewire::writeConfig()
{
    bool ok;
//F5 + ID (1 Byte) + Address(1 Byte) + Data (8 Bytes) + crc(1 Bytes);
    if (parent->isRemoteMode()) return;
    QString hex = writeMemoryLCD;
// ID
    if (family == familyLCDOW_A) hex += "00";
    else if (family == familyLCDOW_B) hex += "01";
    else if (family == familyLCDOW_C) hex += "02";
    else if (family == familyLCDOW_D) hex += "03";
// Address
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// Nothing
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// FontSize
    hex += QString("%1").arg(uchar((Fontsize.value()) + TextFontSize.value() * 0x10) , 2, 16, QChar('0'));
// XPos
    hex += QString("%1").arg(uchar(XPos.value()), 2, 16, QChar('0'));
// YPos
    hex += QString("%1").arg(uchar(YPos.value()), 2, 16, QChar('0'));
// XposTxt
    hex += QString("%1").arg(uchar(XPosTxt.value()), 2, 16, QChar('0'));
// YposTxt
    hex += QString("%1").arg(uchar(YPosTxt.value()), 2, 16, QChar('0'));
// Digits
    hex += QString("%1").arg(uchar(Digits.value()), 2, 16, QChar('0'));
// Virgule
    hex += QString("%1").arg(uchar(Virgule.value()), 2, 16, QChar('0'));
// calc crc
    QVector <uint8_t> scratchpad;
    for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc = calcCRC8(scratchpad);
// add crc
     hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
     sendWriteValText(hex);
    hex = writeMemoryLCD;
 // ID
     if (family == familyLCDOW_A) hex += "00";
     else if (family == familyLCDOW_B) hex += "01";
     else if (family == familyLCDOW_C) hex += "02";
     else if (family == familyLCDOW_D) hex += "03";
// Address
    hex += QString("%1").arg(uchar(8), 2, 16, QChar('0'));
// CoefMult
    hex += QString("%1").arg(uchar(CoefMult.value()), 2, 16, QChar('0'));
// CoefDiv
    hex += QString("%1").arg(uchar(CoefDiv.value()), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// 0
    hex += QString("%1").arg(uchar(0), 2, 16, QChar('0'));
// calc crc
      //L = (hex.length() / 2);
      //scratchpadcrc = byteVec_t(L);
      //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);
      //crccalc = calcCRC8(&scratchpadcrc[0], L);
    scratchpad.clear();
    for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    crccalc = calcCRC8(scratchpad);
// add crc
      hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
      sendWriteValText(hex);
}




//AB 00 10 49 20 61 6D 20 62 65 61 75 20 67 6F 73 73 65 00 B2

void lcdonewire::writeValTxt()
{   bool ok;
    // AB + ID (1 Byte) + Length (max 20) + string + crc (1 Bytes) of all previous bytes including command AB
    if (parent->isRemoteMode()) return;

    QString hex = "AB";
// ID
    if (family == familyLCDOW_A) hex += "00";
    else if (family == familyLCDOW_B) hex += "01";
    else if (family == familyLCDOW_C) hex += "02";
    else if (family == familyLCDOW_D) hex += "03";
   QByteArray my;
   my.append(textValue.text().toLatin1());
// length
   hex += QString("%1").arg(my.length()+1, 2, 16, QChar('0'));
// string
   hex += toHex(my) + "00";
// calc crc
    //int L = (hex.length() / 2);
    //byteVec_t scratchpadcrc = byteVec_t(L);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);
    //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
   QVector <uint8_t> scratchpad;
   for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
   uint8_t crccalc = calcCRC8(scratchpad);
// add crc
    hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
    hex = hex.toUpper();
    sendWriteValText(hex);
    textDisplay = textValue.text();
    if (textValue.text() != textDisplay) textValue.text() = textDisplay;
}



void lcdonewire::SetOrder(const QString &order)
{
	if (order == NetRequestMsg[ReadTemp]) lecture();
}




QString lcdonewire::MainValueToStrLocal(const QString &str)
{
	QString S = str;
    ui.valueui->setText(S);
    return S;
}






void lcdonewire::sendText(double Value)
{
    if (UsetextValues.isChecked())
    {
        for (int n=0; n<TextValues.count(); n++)
        {
            QStringList list = TextValues.itemText(n).split("=");
            if (list.count() == 2)
            {
                bool ok;
                if (list.at(0).toDouble(&ok) == Value)
                {
                    if (ok)
                    {
                        if (textDisplay != list.at(1))
                        {
                            textValue.setText(list.at(1));
                            writeValTxt();
                        }
                    }
                    return;
                }
            }
        }
        if (textDisplay != "          ")
        {
            textDisplay = "          ";
            textValue.setText(textDisplay);
            writeValTxt();
        }
    }
}






void lcdonewire::lecture()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ReadCounter, romID_A);
            break;
#else
        case Ha7Net :
            setscratchpad(master->getScratchPad(romID_A));
            break;
#endif
        case TCP_HA7SType :
            setscratchpad(master->getScratchPad(romID_A));
            break;
        case MultiGest :
            setscratchpad(master->getScratchPad(romID_A));
            break;
        case RemoteType : master->getMainValue();
        break;
    }
}



void lcdonewire::lecturerec()
{
    QString romID_A = romid.left(16) + "_A";
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            master->addtofifo(ReadCounterRec, romID_A);
            break;
#else
        case Ha7Net :
            setscratchpad(master->getScratchPad(romID_A), true);
            break;
#endif
        case TCP_HA7SType :
            setscratchpad(master->getScratchPad(romID_A), true);
            break;
        case MultiGest :
            setscratchpad(master->getScratchPad(romID_A), true);
            break;
        case RemoteType : master->saveMainValue();
            break;
    }
}


//FA0000FFFFFFFFFFFFFFFFFF

void lcdonewire::readConfig(int ID, int Address)
{
    //ButtonReadConfig.setEnabled(false);
    //FA + ID (1 Byte) + Address(1 Byte) + Data (8 Bytes) + crc(1 Bytes);
        if (parent->isRemoteMode()) return;
        QString hex = readMemoryLCD;
        QString readRequest = "FFFFFFFFFFFFFFFFFF";        // 8 byte data + 1 byte crc
        if (ID == 3) hex += "03";
        else if (ID == 2) hex += "02";
        else if (ID == 1) hex += "01";
        else hex += "00";
        hex += QString("%1").arg(uchar(Address), 2, 16, QChar('0')) + readRequest;
        sendReadMemory(hex);
}




bool lcdonewire::setscratchpad(const QString &scratchpad, bool enregistremode)
{
    ScratchPad_Show.setText(scratchpad);
    bool ok;
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    logMsg(scratchpad);
    if (family == familyLCDOW_A)
    {
        QString RomID_B = romid.left(17) + "B";
        QString RomID_C = romid.left(17) + "C";
        QString RomID_D = romid.left(17) + "D";
        onewiredevice *device_B;
        onewiredevice *device_C;
        onewiredevice *device_D;
        device_B = parent->configwin->DeviceExist(RomID_B);
        device_C = parent->configwin->DeviceExist(RomID_C);
        device_D = parent->configwin->DeviceExist(RomID_D);
        if (master)
        switch (master->gettype())
        {
#ifdef HA7Net_No_Thread
        case Ha7Net :
            if (device_B) device_B->setscratchpad(scratchpad, enregistremode);
            if (device_C) device_C->setscratchpad(scratchpad, enregistremode);
            if (device_D) device_D->setscratchpad(scratchpad, enregistremode);
            break;
#endif
        }
    }
    if (scratchpad.isEmpty())
    {
        logMsg(romid + "  scratchpad empty !!!");
        return IncWarning();
    }
    if (scratchpad == "5ADF01FFFFFFFFFFFFFFFFFFFFFF")
    {
        if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
        return IncWarning();		// No answer from device not connected
    }
    if (scratchpad == "0000000000000000000000000000")
    {
        if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
        return IncWarning();		// 1 Wire bus shorted
    }
    if (scratchpad.length() == 12) // 5A000005FCAA Write Value from HA7S
    {
       if (scratchpad.right(2) == "AA")
       {
           if (!checkCRC8(scratchpad.left(10)))
           {
               ScratchPad_Show.setText("Bad CRC (" + scratchpad +")");
               return false;
           }
           int ID = scratchpad.mid(2,2).toInt(&ok, 16);
           if (((family == familyLCDOW_A) && (ID == 0)) || ((family == familyLCDOW_B) && (ID == 1)) || ((family == familyLCDOW_C) && (ID == 2)) || ((family == familyLCDOW_D) && (ID == 3)))
           {
               QString VQint = scratchpad.mid(4, 4);
               double V = calcultemperature(VQint);
               assignedValue = V;
               if (ok)
               {
                   setMainValue(V, enregistremode);
                   sendText(V);
                   assignTry = 0;
               }
           }
           return true;
       }
       else
       {
           if (assignTry > 0)
           {
               assignMainValue(assignedValue);
               assignTry --;
           }
           if (master && WarnEnabled.isChecked()) master->GenError(25, errorMsg);
           return IncWarning();		// No answer from device not connected
       }
    }
    if (scratchpad.length() == 8) // 5A030000 Write value from HA7net
    {
        int ID = scratchpad.mid(2,2).toInt(&ok, 16);
        if (((family == familyLCDOW_A) && (ID == 0)) || ((family == familyLCDOW_B) && (ID == 1)) || ((family == familyLCDOW_C) && (ID == 2)) || ((family == familyLCDOW_D) && (ID == 3)))
        {
            QString VQint = scratchpad.mid(4, 4);
            double V = calcultemperature(VQint);
            assignedValue = V;
            if (ok)
            {
                setMainValue(V, enregistremode);
                sendText(V);
                assignTry = 0;
            }
        }
        return true;
    }
    // FA00000011000000000100A1     ReadMemoryLCD return need to read ID and Address and check CRC
    if ((scratchpad.length() > 22) && (scratchpad.left(2) == readMemoryLCD))
    {
        bool crcOk = checkCRC8(scratchpad);
        int ID = scratchpad.mid(2,2).toInt(&ok, 16);
        if ((!ok) && master && WarnEnabled.isChecked()) master->GenError(88, errorMsg + " ID");
        int Address = scratchpad.mid(4,2).toInt(&ok, 16);
        if ((!ok) && master && WarnEnabled.isChecked()) master->GenError(88, errorMsg + " Address");
        QString Data = scratchpad.mid(6,16);
        if (family == familyLCDOW_A)
        {
            if (Data == "FFFFFFFFFFFFFFFF")
            {
                ScratchPad_Show.setText("No answer, retry read Memory (" + scratchpad +")");
                readConfig(ID, Address);
                if (master->gettype() == Ha7Net) return false;
                return true;
            }
            if (!crcOk)
            {
                ScratchPad_Show.setText("Bad CRC, retry read Memory (" + scratchpad +")");
                readConfig(ID, Address);
                if (master->gettype() == Ha7Net) return false;
                return true;
            }
        }
        if (crcOk)
        {
            if ((family == familyLCDOW_A) && (ID == 0)) setLCDConfig(Address, Data);
            else if ((family == familyLCDOW_B) && (ID == 1)) setLCDConfig(Address, Data);
            else if ((family == familyLCDOW_C) && (ID == 2)) setLCDConfig(Address, Data);
            else if ((family == familyLCDOW_D) && (ID == 3)) setLCDConfig(Address, Data);
            if (family == familyLCDOW_A)
            {
                if (Address == 0) readConfig(ID, 8);
                else if (Address == 8) readConfig(ID, 16);
                else if (Address == 16) readConfig(ID, 32);
                else if (Address == 32)
                {
                    if (ID < 3) readConfig(ID+1, 0);
                }
            }
            ScratchPad_Show.setText("Read Memory OK");
            return true;
        }
    }
    if (!checkCRC16(scratchpad)) return IncWarning();
    int offset = 8;
    QString TStr;
    if (family == familyLCDOW_A)
    {
        TStr = scratchpad.mid(2+offset,2) + scratchpad.mid(0+offset,2);
    }
    else if (family == familyLCDOW_B)
    {
        TStr = scratchpad.mid(6+offset,2) + scratchpad.mid(4+offset,2);
    }
    else if (family == familyLCDOW_C)
    {
        TStr = scratchpad.mid(10+offset,2) + scratchpad.mid(8+offset,2);
    }
    else if (family == familyLCDOW_D)
    {
        TStr = scratchpad.mid(14+offset,2) + scratchpad.mid(12+offset,2);
    }
    double T = calcultemperature(TStr);
    setMainValue(T, enregistremode);
    sendText(T);
    return true;
}





void lcdonewire::setLCDConfig(int Address, const QString Data)
{
    bool ok;
/*  writeMemoryLCD map
    write address 0
        Rien
        FontSize
        XPos
        YPos
        XPosTxt
        YPosTxt
        Digits
        Decimal

    write address 16
        Prefix Bytes 0 - 7
     write address 32
        Suffix Bytes 0 - 7
    write address 40
        ValText Bytes 0 - ...       */
    if (Address == 0)
    {
        int v = Data.mid(2,2).toInt(&ok, 16);
        if (ok)
        {
            Fontsize.setValue(v & 0x0F);
            TextFontSize.setValue((v & 0xF0) >> 4);
        }
        v = Data.mid(4,2).toInt(&ok, 16);
        if (ok) XPos.setValue(v);
        v = Data.mid(6,2).toInt(&ok, 16);
        if (ok) YPos.setValue(v);
        v = Data.mid(8,2).toInt(&ok, 16);
        if (ok) XPosTxt.setValue(v);
        v = Data.mid(10,2).toInt(&ok, 16);
        if (ok) YPosTxt.setValue(v);
        v = Data.mid(12,2).toInt(&ok, 16);
        if (ok) Digits.setValue(v);
        v = Data.mid(14,2).toInt(&ok, 16);
        if (ok) Virgule.setValue(v);
        //QMessageBox::warning(this, romid, "Data : " + Data.mid(14,2), 1, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
    }
    if (Address == 8)
    {
        int v = Data.mid(0,2).toInt(&ok, 16);
        if (ok) CoefMult.setValue(v);
        v = Data.mid(2,2).toInt(&ok, 16);
        if (ok) CoefDiv.setValue(v);
    }
    if (Address == 16)
    {
        prefixStr.setText(fromHex(Data));
    }
    if (Address == 32)
    {
        suffixStr.setText(fromHex(Data));
    }
}




double lcdonewire::calcultemperature(const QString &THex)
{
    bool ok;
    bool s = false;
    short int T = THex.toInt(&ok, 16);
    if (!ok) return logisdom::NA;
    if (T & 0x8000)
    {
        T = 0xFFFF - T + 1;
        s = true;
    }
    double R = Coef.result(T);
    if (s) return double(-R);
        else return double(R);
}



void lcdonewire::deviceSelectionChanged()
{
    if (connectedDevice) disconnect(connectedDevice, SIGNAL(DeviceValueChanged(onewiredevice*)), this, SLOT(DeviceValueChanged(onewiredevice*)));
    if (!copyValue) return;
    connectedDevice = copyValue->device();
    if (!connectedDevice) return;
    connect(connectedDevice, SIGNAL(DeviceValueChanged(onewiredevice*)), this, SLOT(DeviceValueChanged(onewiredevice*)));
}



void lcdonewire::DeviceValueChanged(onewiredevice *device)
{
    if (MainValue == logisdom::NA) return;
    if (device)
    {
        double v = device->getMainValue();
        if (v != lastCopyValue)
        {
            lastCopyValue = v;
            assignMainValue(v);
        }
    }
}



void lcdonewire::assignMainValueLocal(double value)
{
    //QMessageBox::warning(this, device->MainValueToStr(), "Data : ", 1, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
    double R = value / Coef.value();
    if (assignTry != 0) assignTry = 3;
    bool ok;
    /*
    5A + Index(1 byte) + Value (2 bytes) + crc (1 bytes) + "FF";
      Write sequence for LCD 1 Wire use the Write PIO 5A command code like DS2408
      You must provide an hex code containing the command, index, value, control and FF for the feedback
      The index is 0 to 3 as the LCD controleur can handle 4 values
      The value is 2 bytes (signed int)
      The controle is the complement ~ of the value
      FF will let the LCD controleur to send a return code in case of good data control
    */
    if (parent->isRemoteMode()) return;
    qint16 data = (int)round(R);
    QString hex = writePIO;
    if (family == familyLCDOW_A) hex += "00";
    if (family == familyLCDOW_B) hex += "01";
    if (family == familyLCDOW_C) hex += "02";
    if (family == familyLCDOW_D) hex += "03";
    hex += QString("%1").arg(data, 4, 16, QChar('0')).right(4);
// calc crc
    //int L = (hex.length() / 2);
    //byteVec_t scratchpadcrc = byteVec_t(L);
    //for (int n=0; n<L; n++) scratchpadcrc[n] = hex.mid(n * 2, 2).toInt(&ok, 16);
    //uint8_t crccalc = calcCRC8(&scratchpadcrc[0], L);
    QVector <uint8_t> scratchpad;
    for (int n=0; n<hex.length(); n+=2) scratchpad.append(uint8_t(hex.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc = calcCRC8(scratchpad);
// add crc
    hex += QString("%1").arg(crccalc, 2, 16, QChar('0'));
    hex += "FF";
    hex = hex.toUpper();
logMsg("Write " + hex);
    switch (master->gettype())
    {
#ifdef HA7Net_No_Thread
    case Ha7Net :
        master->addtofifo(WriteLed, romid, hex);
        break;
#else
    case Ha7Net :
        //if (!command.isEmpty()) master->addtofifo(setChannelOff, romid, command);
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



double lcdonewire::getMaxValue()
{
    return 32767;
}



bool lcdonewire::isSwitchFamily()
{
    return true;
}



bool lcdonewire::isDimmmable()
{
    return true;
}


void lcdonewire::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("Coef", Coef.text());
    str += logisdom::saveformat("copyValue", copyValue->getRomID());
    AXplusB.GetConfigStr(str);
}





void lcdonewire::setconfig(const QString &strsearch)
{
    QString Name = logisdom::getvalue("Name", strsearch);
    if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(tr("LCD_Value ")));
    QString coefstr = logisdom::getvalue("Coef", strsearch);
    if (!coefstr.isEmpty()) Coef.setText(coefstr);
    AXplusB.setconfig(strsearch);
    QString RomID = logisdom::getvalue("copyValue", strsearch);
    if (!RomID.isEmpty()) copyValue->setRomID(RomID);
}
