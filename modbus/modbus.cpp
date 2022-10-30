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




#include <QtWidgets/QFileDialog>
#include "net1wire.h"
#include "onewire.h"
#include "globalvar.h"
#include "errlog.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "modbusthread.h"
#include "modbus.h"



modbus::modbus(logisdom *Parent) : net1wire(Parent)
{
    uiw.setupUi(ui.frameguinet);
    ui.fifolist->hide();
    type = NetType(ModBus_Type);
    setport(default_port);
    connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
    connect(&TcpThread, SIGNAL(tcpStatusUpdate(QString)), this, SLOT(tcpStatusUpdate(QString)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(tcpStatusChange()), this, SLOT(tcpStatusChange()), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(setDeviceScratchpad(devmodbus*, QString)), this, SLOT(setDeviceScratchpad(devmodbus*, QString)), Qt::QueuedConnection);
}




modbus::~modbus()
{
    TcpThread.endLessLoop = false;
    TcpThread.quit();
}



void modbus::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();
    TcpThread.moduleipaddress = moduleipaddress;
    int id = 1;
    foreach (devmodbus *dev, TcpThread.modbusdevices)
    {
        QString RomID = (ip2Hex(moduleipaddress) + port2Hex(port) + QString("%1").arg(id, 3, 10, QChar('0')) + familyModBus);
        dev->changeRomID(RomID, moduleipaddress);
        id ++;
    }
}



void modbus::init()
{
    connect(uiw.pushButtonNewDevice, SIGNAL(clicked()), this, SLOT(NewDevice()));
    connect(uiw.pushButtonLoadSetup, SIGNAL(clicked()), this, SLOT(LoadSetup()));
    connect(uiw.pushButtonSaveSetup, SIGNAL(clicked()), this, SLOT(SaveSetup()));
    uiw.pushButtonSaveSetup->setEnabled(false);
    connect(uiw.Modbus_TCP_Enable, SIGNAL(toggled(bool)), this, SLOT(ModbusTCPChanged(bool)));
    connect(uiw.logOnlyWrite, SIGNAL(stateChanged(int)), this, SLOT(LogWriteChanged(int)));
    connect(uiw.clearLog, SIGNAL(clicked()), this, SLOT(clearLogText()));
    connect(uiw.MuxInd , SIGNAL(valueChanged(int)), this, SLOT(MuxIndChanged(int)));
    uiw.MuxVal->setEnabled(false);

    ui.toolButtonClear->hide();
	ui.traffic->hide();
	ui.EditType->setText("Modbus");

	TcpThread.endLessLoop = true;
	TcpThread.moduleipaddress = moduleipaddress;
	TcpThread.port = port;

	QString configdata;
	parent->get_ConfigData(configdata);
	readConfigFile(configdata);
}




void modbus::startTCP()
{
    TcpThread.start(QThread::LowestPriority);
}



void modbus::switchOnOff(bool state)
{
	if (state)
	{
		TcpThread.endLessLoop = true;
		OnOff = true;
		TcpThread.start(QThread::LowestPriority);
	}
	else
	{
		TcpThread.endLessLoop = false;
		OnOff = false;
	}
}




void modbus::addtofifo(const QString &order)
{
    TcpThread.addToFIFOSpecial(order);
}



void modbus::addtofifo(int order)
{
	TcpThread.addToFIFOSpecial(NetRequestMsg[order]);
}



void modbus::logEnabledChanged(int state)
{
	if (state) TcpThread.logEnabled = true;
	else TcpThread.logEnabled = false;
}



void modbus::clearLogText()
{
    ui.textBrowser->clear();
}



void modbus::tcpStatusUpdate(const QString &str)
{
    QString txt = ui.textBrowser->toPlainText();
    if(txt.length() > 10000)
    {
        txt = txt.right(9000);
        ui.textBrowser->setText(txt);
    }
    ui.textBrowser->append(str);
    ui.textBrowser->moveCursor(QTextCursor::End);
}



void modbus::ModbusTCPChanged(bool state)
{
	TcpThread.Modbus_TCP_Enable = state;
}


void modbus::LogWriteChanged(int state)
{
    TcpThread.logOnlyWrite = state;
}

void modbus::MuxIndChanged(int val)
{
    uiw.MuxVal->setValue(val + 1);
    TcpThread.MuxInd = quint16(val);
}



void modbus::tcpStatusChange()
{
	TcpStateChanged(TcpThread.tcpStatus);
}




void modbus::NewDevice()
{
	bool ok;
	retry:
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (parent->configwin->devicenameexist(nom))
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto retry;
	}
    onewiredevice *device = addDevice(nom, true);
    if (device)
    {
        parent->setPalette(&device->setup);
        parent->PaletteHide(false);
    }
}



void modbus::SaveSetup()
{
    // Name;Unit;Coef;Slave;Address;decimal; Request Type 3 or 4 Need 6 or 7 parameters
    // PV array input voltage; Volts; 1/100; 0x3100; 2; 4
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), name + ".cfg", tr("cfg (*.cfg)"), nullptr);
    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "Name;Unit;Coef;Slave;Address;decimal; Request Type 3 or 4 Need 6 or 7 parameters\n";
    foreach (devmodbus *dev, TcpThread.modbusdevices) out << dev->getSetup();
    file.close();
}


void modbus::LoadSetup()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", "");
    QFile file(fileName);
    if (file.exists())
    {
        int devIndex = 0;
        int devCount = localdevice.count();
        if (devCount != 0)
        {
            if (messageBox::questionHide(this, tr("Append ?"), tr("Replace existing devices ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No)
                devIndex = devCount;
        }
        if (file.open(QIODevice::ReadOnly) )
        {
            QTextStream in(&file);
            QString text;
            text = in.readLine();
            while (!text.isEmpty())
            {
                QStringList parameters = text.split(";");
                int Pcount = parameters.count();
                if ((Pcount == 6) or (Pcount == 7))
                {
                    QString str;
                    str += logisdom::saveformat("SlaveID", QString("%1").arg(1));
                    QString adr = parameters.at(4);
                    adr = adr.remove(" ");
                    if (adr.startsWith("0x"))
                    {
                        str += logisdom::saveformat("HexAddress", QString("%1").arg(1));
                        str += logisdom::saveformat("AddressID", adr.remove("0x"));
                    }
                    else str += logisdom::saveformat("AddressID", adr);
                    str += logisdom::saveformat("Writable", QString("%1").arg(0));
                    str += logisdom::saveformat("MaskEnable", QString("%1").arg(0));
                    str += logisdom::saveformat("ReadMask", "");
                    str += logisdom::saveformat("Coef", parameters.at(2));
                    str += logisdom::saveformat("M3Mux", QString("%1").arg(0));
                    bool P6Ok = true;
                    if (Pcount == 7)
                    {
                        int requestType = parameters.at(6).toInt(&P6Ok);
                        if (P6Ok)
                        {
                            if (requestType == 4) str += logisdom::saveformat("RequestType", QString("%1").arg(1));
                            else str += logisdom::saveformat("RequestType", QString("%1").arg(0));
                        }
                    }
                    bool ok;
                    int dec = parameters.at(5).toInt(&ok);
                    if (!ok) dec = 0;
                    if (P6Ok)
                    {
                        if (devIndex >= devCount)
                        {
                            onewiredevice *device = addDevice(parameters.at(0), true);
                            device->setCfgStr(str);
                            device->Unit.setText(parameters.at(1));
                            device->Decimal.setValue(dec);
                        }
                        else
                        {
                            localdevice.at(devIndex)->setname(parameters.at(0));
                            localdevice.at(devIndex)->setCfgStr(str);
                            localdevice.at(devIndex)->Unit.setText(parameters.at(1));
                            localdevice.at(devIndex)->Decimal.setValue(dec);
                        }
                    }
                    devIndex ++;
                }
                text = in.readLine();
            }
            file.close();
        }
    }
}





onewiredevice *modbus::addDevice(QString name, bool show)
{
    onewiredevice *device = nullptr;
    for (int id=1; id<99; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + port2Hex(port) + QString("%1").arg(id, 3, 10, QChar('0')) + familyModBus);
        if (!parent->configwin->deviceexist(RomID))
		{
            device = createNewDevice(RomID);
			if (device)
			{
				device->setname(name);
				if (show) device->show();
                return device;
            }
        }
	}
    return nullptr;
}



void modbus::getConfig(QString &str)
{
    if (uiw.Modbus_TCP_Enable->isChecked()) str += logisdom::saveformat("ModbusTCP", "1"); else str += logisdom::saveformat("Modbus_TCP_Enable", "0");
    str += logisdom::saveformat("MuxInd", QString("%1").arg(uiw.MuxInd->value()));
}




void modbus::setConfig(const QString &strsearch)
{
    bool TCPEnable = false;
    if (logisdom::getvalue("ModbusTCP", strsearch) == "1") TCPEnable = true;
    uiw.Modbus_TCP_Enable->setChecked(TCPEnable);
    ModbusTCPChanged(TCPEnable);
    bool ok;
    int ind = logisdom::getvalue("MuxInd", strsearch).toInt(&ok);
    if (!ok) ind = 20;
    if (ind < 0) ind = 20;
    uiw.MuxInd->setValue(ind);
    uiw.MuxVal->setValue(ind + 1);
}




void modbus::readConfigFile(QString &configdata)
{
	QString ReadRomID;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	QString family = ReadRomID.right(2);
	if (ReadRomID.length() == 13) ReadRomID = ReadRomID.left(8) + port2Hex(port) + ReadRomID.right(5);
	if (ReadRomID.left(12) == (ip2Hex(moduleipaddress) + port2Hex(port)) && (family == familyModBus))
        createNewDevice(ReadRomID);
    SearchLoopEnd
}




onewiredevice *modbus::createNewDevice(QString RomID)
{
    onewiredevice *device = nullptr;
    if (!parent->configwin->deviceexist(RomID))
    {
        device = parent->configwin->NewDevice(RomID, this);
        devmodbus* dev = dynamic_cast<devmodbus*>(device);
        if (device)
        {
            if (dev)
            {
                if (!localdevice.contains(device))
                {
                    localdevice.append(device);
                    UpdateLocalDeviceList();
                }
                if (!TcpThread.modbusdevices.contains(dev)) TcpThread.modbusdevices.append(dev);
            }
            uiw.pushButtonSaveSetup->setEnabled(true);
        }
    }
    return device;
}



void modbus::setDeviceScratchpad(devmodbus *dev, QString v)
{
    dev->setscratchpad(v);
}
