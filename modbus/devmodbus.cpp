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
#include "modbus.h"
#include "devmodbus.h"




devmodbus::devmodbus(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	lastsavevalue = logisdom::NA;
	romid = RomID;
    Pui.setupUi(this);
    Pui.requestType->addItem("Read Holding Register (0x03)");
    Pui.requestType->addItem("Read Input Register(0x04)");
    Pui.Read_Mask->setInputMask("\0xHHHH");
    Pui.Read_Mask->setEnabled(false);
    family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Modbus device"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	MainValue = logisdom::NA;
    Coef.setText("1");
    Pui.gridLayout_2->addWidget(&Coef, 5, 1, 1, 2);
    setupLayout.addWidget(Pui.tabWidget, layoutIndex, 0, 1, logisdom::PaletteWidth);
    calculInterval.setName(tr("Read Interval"));
    setupLayout.addWidget(&calculInterval,  4, 1, 1, 1);
    connect(Pui.MuxEnable, SIGNAL(stateChanged(int)), this, SLOT(MuxEnableChange(int)));
    connect(Pui.Unsigned, SIGNAL(stateChanged(int)), this, SLOT(unsignedChange(int)));
    connect(Pui.Address, SIGNAL(textChanged(QString)), this, SLOT(addressChanged(QString)));
    connect(Pui.SlaveID, SIGNAL(textChanged(QString)), this, SLOT(slaveChanged(QString)));
    connect(Pui.requestType, SIGNAL(currentIndexChanged(int)), this, SLOT(functionChanged(int)));
    connect(Pui.SlaveID, SIGNAL(textChanged(QString)), this, SLOT(SlaveID_textChanged(QString)));
    connect(Pui.Address, SIGNAL(textChanged(QString)), this, SLOT(Address_textChanged(QString)));
    connect(Pui.lineEditMaxValue, SIGNAL(textChanged(QString)), this, SLOT(MaxValue_textChanged(QString)));
    connect(Pui.Write_Value, SIGNAL(textChanged(QString)), this, SLOT(Write_Value_textChanged(QString)));
    connect(Pui.hexSlaveID, SIGNAL(stateChanged(int)), this, SLOT(hexSlaveID_stateChanged(int)));
    connect(Pui.hexAddress, SIGNAL(stateChanged(int)), this, SLOT(hexAddress_stateChanged(int)));
    connect(Pui.Read_Mask_Enable, SIGNAL(stateChanged(int)), this, SLOT(Read_Mask_Enable_stateChanged(int)));
    connect(Pui.hexWrite, SIGNAL(stateChanged(int)), this, SLOT(hexWrite_stateChanged(int)));
    connect(Pui.pushButtonWrite, SIGNAL(clicked()), this, SLOT(pushButtonWrite_clicked()));
    connect(Pui.comboBoxResolution, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxResolution_currentIndexChanged(int)));
    connect(&calculInterval, SIGNAL(readChanged()), this, SLOT(readChanged()));
}





bool devmodbus::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
    scratchpad = devicescratchpad;
	logMsg(scratchpad);
    ScratchPad_Show.setText(scratchpad);
    ScratchPad_Show.setToolTip("");
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    if (scratchpad.isEmpty())
    {
        logMsg(romid + "  scratchpad empty !!!");
        return IncWarning();
    }
    bool ok;
    //double val = devicescratchpad.toInt(&ok, 16);
    double val = devicescratchpad.toInt(&ok);
    if (!ok)
	{
		error_Txt.setText(devicescratchpad);
		if (master && WarnEnabled.isChecked()) master->GenError(31, errorMsg);
		return IncWarning();
	}
    if (int(val) == logisdom::NA)
    {
        error_Txt.setText(devicescratchpad);
        if (master && WarnEnabled.isChecked()) master->GenError(60, errorMsg);
        return IncWarning();
    }
    //Write_Value.setValue(val);
    error_Txt.clear();
    if (Pui.Read_Mask_Enable->isChecked())
	{
        uint mask = Pui.Read_Mask->text().toUInt(&ok, 16);
		if (ok)
		{
            quint16 v = quint16(val) & mask;
			val = v;
		}
	}
    //QString str = "0x" + scratchpad + "  (" + QString("%1").arg(val) +")";
    //ScratchPad_Show.setToolTip(str);
    val = Coef.result(val);
    setMainValue(val, enregistremode);
	MainValueToStr();
    dataOk = true;
    initialRead = true;
    if (enregistremode)
    {
        if (!autoRead)
        {
            readNow = false;
            QString str = "Save done " + QDateTime::currentDateTime().toString() + "\n";
            str += "Next read will be : " + calculInterval.getNext();
            Pui.labelInfo->setText(str);
        }
        else
        {
            QString str = "Save done " + QDateTime::currentDateTime().toString() + "\n";
            str += "Next save will occur : " + saveInterval.getNext();
            Pui.labelInfo->setText(str);
        }
    }
    return true;
}




void devmodbus::MuxEnableChange(int state)
{
    if (state == Qt::Checked) isM3MuxEnabled = true;
    else isM3MuxEnabled = false;
}



void devmodbus::unsignedChange(int state)
{
    if (state == Qt::Checked) ValUnsigned = true;
    else ValUnsigned = false;
}



void devmodbus::addressChanged(QString adr)
{
    int base = 10;
    if (Pui.hexAddress->isChecked()) base = 16;
    bool ok;
    int v = adr.toInt(&ok, base);
    if (ok) address = quint16(v);
}




void devmodbus::functionChanged(int f)
{
    if (f != -1) function = quint16(Pui.requestType->currentIndex() + 3);
}



void devmodbus::slaveChanged(QString id)
{
    int base = 10;
    if (Pui.hexSlaveID->isChecked()) base = 16;
    bool ok;
    int v = id.toInt(&ok, base);
    if (ok) slave =  quint16(v);
}



void devmodbus::lecture()
{
    if (autoRead)
    {
        if (!dataOk) IncWarning();
        dataOk = false;
    }
    else if (readNow)
    {
        if (!dataOk) IncWarning();
        dataOk = false;
    }
}




void devmodbus::lecturerec()
{
    if (autoRead)
    {
        if (!dataOk) { IncWarning(); return; }
        setscratchpad(scratchpad, true);
        dataOk = false;
    }
    else if (readNow)
    {
        if (!dataOk) { IncWarning(); return; }
        setscratchpad(scratchpad, true);
        dataOk = false;
    }
}






void devmodbus::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
    QList <QAction*> actions;
	contextualmenu.addAction(&Lecture);	
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
        if (Pui.groupBoxWrite->isChecked())
        {
            for (int n=0; n<textStrList.count(); n++)
            {
                QAction *action = new QAction(textStrList.at(n));
                contextualmenu.addAction(action);
                actions.append(action);
            }
        }
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
    if (selection == &Nom) changename();
    int index = actions.indexOf(selection);
    if (index != -1) assignMainValueLocal(textValList.at(index));
    for (int n=0; n<actions.count(); n++) delete actions.at(n);
}





QString devmodbus::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}






void devmodbus::SetOrder(const QString &)
{
}





void devmodbus::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("SlaveID", Pui.SlaveID->text());
    str += logisdom::saveformat("HexSlave", QString("%1").arg(Pui.hexSlaveID->isChecked()));
    str += logisdom::saveformat("AddressID", Pui.Address->text());
    str += logisdom::saveformat("HexAddress", QString("%1").arg(Pui.hexAddress->isChecked()));
    str += logisdom::saveformat("RequestType", QString("%1").arg(Pui.requestType->currentIndex()));
    str += logisdom::saveformat("Writable", QString("%1").arg(Pui.groupBoxWrite->isChecked()));
    str += logisdom::saveformat("MaskEnable", QString("%1").arg(Pui.Read_Mask_Enable->isChecked()));
    str += logisdom::saveformat("Unsigned", QString("%1").arg(Pui.Unsigned->isChecked()));
    str += logisdom::saveformat("Resolution", QString("%1").arg(Pui.comboBoxResolution->currentIndex()));
    str += logisdom::saveformat("ReadMask", Pui.Read_Mask->text());
    str += logisdom::saveformat("Coef", Coef.text());
    str += logisdom::saveformat("M3Mux", QString("%1").arg(Pui.MuxEnable->isChecked()));
    str += logisdom::saveformat("MaxValue", Pui.lineEditMaxValue->text());
    str += logisdom::saveformat("HexMax", QString("%1").arg(Pui.MaxCheckBox->isChecked()));
    str += logisdom::saveformat("NextReadInterval", calculInterval.getNext());
    str += logisdom::saveformat("ReadIntervalMode", calculInterval.getMode());
    str += logisdom::saveformat("ReadIntervalEnabled", calculInterval.getSaveMode());
}




void devmodbus::setconfig(const QString &strsearch)
{
	bool ok;
	if (name.isEmpty())
	{
		QString Name = logisdom::getvalue("Name", strsearch);
		if (!Name.isEmpty()) setname(assignname(Name));
		else setname(assignname(tr("ModBus device ")));
	}
    int hexS = logisdom::getvalue("HexSlave", strsearch).toInt(&ok);
    if ((ok) && (hexS)) Pui.hexSlaveID->setCheckState(Qt::Checked);
    QString s = logisdom::getvalue("SlaveID", strsearch);
    Pui.SlaveID->setText(s);
    slaveChanged (s);
    int hexA = logisdom::getvalue("HexAddress", strsearch).toInt(&ok);
    if ((ok) && (hexA)) Pui.hexAddress->setCheckState(Qt::Checked);
    QString adr = logisdom::getvalue("AddressID", strsearch);
    Pui.Address->setText(adr);
    addressChanged(adr);
    int ReqType = logisdom::getvalue("RequestType", strsearch).toInt(&ok);
    if (ok)
    {
        Pui.requestType->setCurrentIndex(ReqType);
        functionChanged(ReqType);
    }
    int Wr = logisdom::getvalue("Writable", strsearch).toInt(&ok);
    if ((ok) && (Wr)) Pui.groupBoxWrite->setChecked(true);
	int Msk = logisdom::getvalue("MaskEnable", strsearch).toInt(&ok);
    if ((ok) && (Msk))
	{
        Pui.Read_Mask_Enable->setChecked(true);
        Read_Mask_Enable_stateChanged(Qt::Checked);
	}
    int Res = logisdom::getvalue("Resolution", strsearch).toInt(&ok);
    if (ok)
    {
        Pui.comboBoxResolution->setCurrentIndex(Res);
        comboBoxResolution_currentIndexChanged(Res);
    }
    else comboBoxResolution_currentIndexChanged(0);
    int Uns = logisdom::getvalue("Unsigned", strsearch).toInt(&ok);
    if ((ok) && (Uns))
    {
        Pui.Unsigned->setChecked(true);
        ValUnsigned = true;
    }
    QString coefstr = logisdom::getvalue("Coef", strsearch);
    if (!coefstr.isEmpty()) Coef.setText(coefstr);
	QString RdMsk = logisdom::getvalue("ReadMask", strsearch);
    if (!RdMsk.isEmpty()) Pui.Read_Mask->setText(RdMsk);
    int M3 = logisdom::getvalue("M3Mux", strsearch).toInt(&ok);
    if ((ok) && (M3))
    {
        Pui.MuxEnable->setChecked(true);
        isM3MuxEnabled = true;
    }
    QString MValue = logisdom::getvalue("MaxValue", strsearch);
    Pui.lineEditMaxValue->setText(MValue);
    int hexM = logisdom::getvalue("HexMax", strsearch).toInt(&ok);
    if ((ok) && (hexM)) Pui.MaxCheckBox->setCheckState(Qt::Checked);
    QString next = logisdom::getvalue("NextReadInterval", strsearch);
    if (next.isEmpty()) calculInterval.setNext(QDateTime::currentDateTime());
        else calculInterval.setNext(QDateTime::fromString (next, Qt::ISODate));
    int savemode = logisdom::getvalue("ReadIntervalMode", strsearch).toInt(&ok);
    if (ok) calculInterval.setMode(savemode);
    int saveen = logisdom::getvalue("ReadIntervalEnabled", strsearch).toInt(&ok);
    if (ok)
    {
        if (saveen) calculInterval.setSaveMode(true);
        else calculInterval.setSaveMode(false);
    }
    else calculInterval.setSaveMode(false);
    if (calculInterval.enabled) autoRead = false;
    else autoRead = true;
}




bool devmodbus::isDimmmable()
{
    return Pui.groupBoxWrite->isChecked();
}


double devmodbus::getMaxValue()
{
    if (Pui.lineEditMaxValue->text().isEmpty()) return 0xFFFF;
    int base = 10;
    if (Pui.MaxCheckBox->isChecked()) base = 16;
    bool ok;
    int v = Pui.lineEditMaxValue->text().toInt(&ok, base);
    if (ok) return double(v);
    return 0xFFFF;
}


QString devmodbus::getSetup()
{
    // Name;Unit;Coef;Slave;Address;decimal; Request Type 3 or 4 Need 6 or 7 parameters
    QString slave_str = QString("%1").arg(slave);
    if (Pui.hexSlaveID->isChecked()) slave_str = "0x" + QString("%1").arg(slave, 4, 16).toUpper();
    QString address_str = QString("%1").arg(address);
    if (Pui.hexAddress->isChecked()) address_str = "0x" + QString("%1").arg(address, 4, 16).toUpper();
    QString str = name + ";" + getunit() + ";" + Coef.text() + ";" + slave_str + ";" + address_str + ";" + QString("%1").arg(Decimal.value()) + ";" + QString("%1").arg(function) + "\n";
    return str;
}



void devmodbus::changeRomID(QString RomID, QString ip)
{
    RomIDButton.setText(RomID);
    ui.labelmaster->setText(ip);
    htmlBind->setParameter("RomID", RomID);
    dataLoader->romID = RomID;
    romid = RomID;
}


void devmodbus::assignMainValueLocal(double value)
{
    double V = value / Coef.value();
    QString v;
    if (ValUnsigned)
   {
       //if (resolution == 4) v = QString("%1").arg(quint64(round(V)));
       //else if (resolution == 2) v = QString("%1").arg(quint32(round(V)));
       //else
       v = QString("%1").arg(quint16(round(V)),  4, 16, QChar('0')).toUpper().right(4);
   }
   else
   {
       //if (resolution == 4) v = v = QString("%1").arg(qint64(round(V)));
       //else if (resolution == 2) v = v = QString("%1").arg(qint32(round(V)));
       //else
       v = QString("%1").arg(qint16(round(V)), 4, 16, QChar('0')).toUpper().right(4);
   }

    //qint16 data = qint16(round(R));
    QString order = QString("%1/").arg(slave, 2, 16, QChar('0')).toUpper();
    order += QString("%1/").arg(address, 4, 16, QChar('0')).toUpper();
    order += v;
    //qDebug() << order;
    //order += QString("%1").arg(qint16(data), 4, 16, QChar('0')).toUpper().right(4);
    master->addtofifo(order);
}




void devmodbus::SlaveID_textChanged(const QString &str)
{
    int base = 10;
    if (Pui.hexSlaveID->isChecked()) base = 16;
    bool ok;
    int ID = str.toInt(&ok, base);
    if (ok) Pui.SlaveID->setToolTip("Value " + str + " is correct, 0x" + QString("%1").arg(ID, 2, 16, QChar('0')).toUpper() + " in hexadecimal");
    else Pui.SlaveID->setToolTip("Value " + str + " is not correct");
}



void devmodbus::hexSlaveID_stateChanged(int)
{
    SlaveID_textChanged(Pui.SlaveID->text());
}



void devmodbus::Address_textChanged(const QString &str)
{
    int base = 10;
    if (Pui.hexAddress->isChecked()) base = 16;
    bool ok;
    int ID = str.toInt(&ok, base);
    if (ok) Pui.Address->setToolTip("Value " + str + " is correct, 0x" + QString("%1").arg(ID, 2, 16, QChar('0')).toUpper() + " in hexadecimal");
    else Pui.Address->setToolTip("Value " + str + " is not correct");
}



void devmodbus::readChanged()
{
    QString str = "Read condition has changed\n";
    if (calculInterval.enabled) autoRead = false;
    else autoRead = true;
    if (autoRead) str += "Auto read enabled, value is continously read\n and saved every interval chosen\n";
    else str +="Read will be every : " + QString("%1").arg(calculInterval.getSecs()) + " seconds\n";
    str += "Next read will be : " + calculInterval.getNext();
    str += "\n" + QDateTime::currentDateTime().toString();
    Pui.labelInfo->setText(str);
}


void devmodbus::checkRead()
{
    QString str;
    if (autoRead)
    {
        str += "Auto read enabled, value is continously read\nand saved every interval chosen";
        return;
    }
    if (readNow)
    {
        str += "Oups ...\n";
        str += "New read request while waiting for read\n";
        return;
    }
    readNow = calculInterval.isitnow();
    if (readNow)
    {
        str += "Wait to be read\n";
        str += "Next read will be : " + calculInterval.getNext();
    }
    else
    {
        str += "No read needed for now\n";
        str += "Next read will be : " + calculInterval.getNext();
    }
    Pui.labelInfo->setText(str);
}



void devmodbus::MaxValue_textChanged(const QString &str)
{
    int base = 10;
    if (Pui.MaxCheckBox->isChecked()) base = 16;
    bool ok;
    int ID = str.toInt(&ok, base);
    if (ok) Pui.lineEditMaxValue->setToolTip("Value " + str + " is correct, 0x" + QString("%1").arg(ID, 2, 16, QChar('0')).toUpper() + " in hexadecimal");
    else Pui.lineEditMaxValue->setToolTip("Value " + str + " is not correct 0xFFFF will be used");
}


void devmodbus::hexAddress_stateChanged(int)
{
    Address_textChanged(Pui.Address->text());
}



void devmodbus::Write_Value_textChanged(const QString &str)
{
    int base = 10;
    if (Pui.hexWrite->isChecked()) base = 16;
    bool ok;
    int ID = str.toInt(&ok, base);
    if (ok) Pui.Write_Value->setToolTip("Value " + str + " is correct, 0x" + QString("%1").arg(ID, 2, 16, QChar('0')).toUpper() + " in hexadecimal");
    else Pui.Write_Value->setToolTip("Value " + str + " is not correct");
}




void devmodbus::hexWrite_stateChanged(int)
{
    Write_Value_textChanged(Pui.Write_Value->text());
}




void devmodbus::Read_Mask_Enable_stateChanged(int state)
{
    if (state)
    {
        Pui.Read_Mask->setEnabled(true);
    }
    else
    {
        Pui.Read_Mask->setEnabled(false);
    }
}




void devmodbus::comboBoxResolution_currentIndexChanged(int index)
{
    if (index == 2) resolution = 4;
    else if(index == 1) resolution = 2;
    else resolution = 1;
}


void devmodbus::pushButtonWrite_clicked()
{
//17:27:17:400  addToFIFOSpecial : 10/0894/0002
    int base = 10;
    if (Pui.hexWrite->isChecked()) base = 16;
    bool ok;
    int W = Pui.Write_Value->text().toInt(&ok, base);
    //double R = W / Coef.value();
    //qint16 data = qint16(round(R));
    //QString order = QString("%1/").arg(getSlave(), 2, 16, QChar('0')).toUpper();
    //order += QString("%1/").arg(getAddress(), 4, 16, QChar('0')).toUpper();
    //order += QString("%1").arg(data, 4, 16, QChar('0')).toUpper().right(4);
    //master->addtofifo(order);
    assignMainValueLocal(W);
}




