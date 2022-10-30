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





#include <QtCore>
#include <QtNetwork>
#include "errlog.h"
#include "alarmwarn.h"
#include "onewire.h"
#include "formula.h"
#include "eoceanthread.h"
#include "configwindow.h"
#include "messagebox.h"
#include "eocean.h"


#ifdef WIN32
#include <windows.h>
#define SER_PORT "COM1"
#else
#define SER_PORT "/dev/ttyUSB0"//! the Serial Port Device
#endif


eocean::eocean(logisdom *Parent) : net1wire(Parent)
{
    uiw.setupUi(ui.frameguinet);
    //ui.checkBoxLog->setChecked(true);
    ui.fifolist->hide();
    ui.toolButtonClear->hide();
    ui.toolButtonClear->hide();
    ui.traffic->hide();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
#ifdef WIN32
        uiw.comboBoxComPort->addItem(info.portName());
#else
        uiw.comboBoxComPort->addItem("/dev/" + info.portName());
#endif
    eoThread.endLessLoop = true;
    eoThread.moveToThread(&eoThread);
    connect(uiw.comboBoxComPort, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(changeComPort(const QString &)));
    connect(uiw.TeachButtonON, SIGNAL(clicked()), this, SLOT(TeachON()));
    connect(uiw.TeachButtonOFF, SIGNAL(clicked()), this, SLOT(TeachOFF()));
    connect(uiw.TeachButtonClearLog, SIGNAL(clicked()), this, SLOT(ClearLog()));
    connect(&eoThread, SIGNAL(eoStatusChange()), this, SLOT(eoStatusChange()), Qt::QueuedConnection);
    connect(&eoThread, SIGNAL(logThis(QString)), this, SLOT(logThis(QString)), Qt::QueuedConnection);
    connect(&eoThread, SIGNAL(newDevice(QString)), this, SLOT(addNewDevice(QString)), Qt::QueuedConnection);
    connect(&eoThread, SIGNAL(deviceReturn(QString, QString)), this, SLOT(deviceReturn(QString, QString)), Qt::QueuedConnection);
    connect(&eoThread, SIGNAL(deviceReturn(QString, QString)), this, SLOT(logReturn(QString, QString)), Qt::QueuedConnection);
    connect(&eoThread, SIGNAL(tcpStatusChange()), this, SLOT(tcpStatusChange()), Qt::QueuedConnection);
    type = NetType(EOceanType);
    ui.EditType->setText("EnOcean");
}



void eocean::TeachON()
{
    eoThread.learnMode = true;
    logThis("EnOcean learning ON");
}


void eocean::TeachOFF()
{
    eoThread.learnMode = false;
    logThis("EnOcean learning OFF");
}



void eocean::ClearLog()
{
    ui.textBrowser->clear();
}


void eocean::switchOnOff(bool state)
{
    if (state)
    {
        if (LocalCatalog.isEmpty())
        {
            logThis("Local Catalog is empty");
        }
        else
        {
            QString str;
            for (int n=0; n<LocalCatalog.count(); n++) str.append(LocalCatalog.at(n) + "\n");
            logThis("Local Catalog : \n" + str);
        }
        eoThread.setAdress(moduleipaddress);
        eoThread.setPort(ui.portedit->text().toInt());
        OnOff = true;
        eoThread.start();
    }
    else
    {
        eoThread.endLessLoop = false;
        OnOff = false;
    }
}


void eocean::changeComPort(const QString& port)
{
   if (eoThread.isRunning()) return;
    ui.ipaddressedit->setText(port);
    moduleipaddress = port;
    eoThread.setAdress(moduleipaddress);
}


void eocean::init()
{
    if (LocalCatalog.isEmpty())
    {
        logThis("Local Catalog is empty");
    }
    else
    {
        QString str;
        for (int n=0; n<LocalCatalog.count(); n++) str.append(LocalCatalog.at(n) + "\n");
        logThis("Local Catalog : \n" + str);
    }
    QString configdata;
    parent->get_ConfigData(configdata);
    QString ReadRomID, MasterIP;
    onewiredevice *device = nullptr;
    QString TAG_Begin = One_Wire_Device;
    QString TAG_End = EndMark;
    SearchLoopBegin
    ReadRomID = logisdom::getvalue("RomID", strsearch);
    QString family = ReadRomID.right(2);
    QString RomID = ReadRomID;
    if (family.left(1) == "_")
    {
        RomID.chop(2);
        family = RomID.right(2);
    }
    if (family == familyEOcean)
    {
        if (LocalCatalog.contains(ReadRomID) | upgradeCatalogInfo)
        {
            device = parent->configwin->DeviceExist(ReadRomID);
            if (!device)
            {
                device = parent->configwin->NewDevice(ReadRomID, this);
                if (device)
                {
                    if (!localdevice.contains(device))
                    {
                        localdevice.append(device);
                        UpdateLocalDeviceList();
                    }
                }
            }
        }
    }
    SearchLoopEnd
    eoThread.endLessLoop = true;
    eoThread.setAdress(moduleipaddress);
    eoThread.setPort(ui.portedit->text().toInt());
    eoThread.start();
}



eocean::~eocean()
{
    eoThread.endLessLoop = false;
    eoThread.quit();
}




void eocean::addtofifo(const QString &order)
{
    eoThread.addtofifo(order);
}



QString eocean::writeScratchPad(const QString& RomID, const QString& data_Str)
{
    //qDebug() << "writeScratchPad : " + RomID + " = " + data_Str;
    bool ok;
    int index = eoThread.deviceRomIDs.indexOf(RomID);
    quint64 n = data_Str.toULongLong(&ok, 16);
    if (index == -1)
    {
        eoThread.deviceRomIDs.append(RomID);
        if (!ok) n = 0xFFFFFFFF;
        eoThread.dev4BS_Send.append(n);
    }
    index = eoThread.deviceRomIDs.indexOf(RomID);
    if (index == -1) return "";
    eoThread.dev4BS_Send.replace(index, n);
    return "";
}



void eocean::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();
    eoThread.setAdress(moduleipaddress);
}



void eocean::setport(int Port)
{
    ui.portedit->setText(QString("%1").arg(Port));
    port = Port;
    eoThread.setPort(port);
}



void eocean::setConfig(const QString &strsearch)
{
    QString catalog = logisdom::getvalue("EoCatalog", strsearch);
    if (catalog.isEmpty()) upgradeCatalogInfo = true;
    else LocalCatalog = catalog.split(",");
}



void eocean::getConfig(QString &str)
{
    QString catalog;
    for (int n=0; n<localdevice.count(); n++)
    {
        catalog.append(localdevice.at(n)->getromid());
        if (n != (localdevice.count() - 1)) catalog.append(",");
    }
    str += logisdom::saveformat("EoCatalog", catalog);
}



void eocean::tcpStatusChange()
{
    TcpStateChanged(eoThread.tcpStatus);
}



void eocean::logReturn(const QString RomID, const QString scratchpad)
{
    QString log_Str;
    log_Str = QDateTime::currentDateTime().toString("dd/MM HH:mm:ss:zzz ") + " : Device " + RomID + " setscratchpad : " + scratchpad;
    logThis(log_Str);
}


void eocean::logThis(const QString str)
{
    if (!ui.checkBoxLog->isChecked()) return;
    QString txt = ui.textBrowser->toPlainText();
    if(txt.length() > 10000)
    {
        txt = txt.right(9000);
        ui.textBrowser->setText(txt);
    }
    ui.textBrowser->append(str);
    ui.textBrowser->moveCursor(QTextCursor::End);
}



void eocean::addNewDevice(const QString &RomID)
{
    QStringList DevList;
    getDeviceList(RomID, DevList);
    //qDebug() << DevList.first();
    onewiredevice *device = parent->configwin->DeviceExist(DevList.first());
    if (device)
    {
        // check if device enroled with this master
        if (device->master == this)
        {
            messageBox::warningHide(this, RomID, tr("Device ") + RomID + "\n" + tr("Already enroled"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        }
        else    // this is another master
        {
            // check is EPP prifile is A52001
            if (device->getromid().right(8) == familyeoA52001)
            {
                QString name = device->getMaster()->getname();
                messageBox::warningHide(this, RomID, tr("Device ") + RomID + "\n" + tr(" is already enroled\nIt will be remove from ") + name + " " + tr("catalog\nPlease save the configuration and restart the application"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            }
            else    // ask if must be removes from other catalog
            {
                if ((messageBox::questionHide(this, RomID, tr("Device ") + RomID + "\n" + tr(" is already enroled\nDo you want to remove it from ") + name + " " + tr("catalog") , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
                {
                    QStringList DevList;
                    getDeviceList(RomID, DevList);
                    int moved_dev = 0;
                    for (int n=0; n<DevList.count(); n++)
                    {
                        onewiredevice *dev = parent->configwin->DeviceExist(DevList.at(n));
                        if (dev->master->removeDeviceFromCatalog(dev))
                        {
                            moved_dev++;
                            if (!localdevice.contains(dev)) localdevice.append(dev);
                            UpdateLocalDeviceList();
                            dev->show();
                        //qDebug() << DevList.at(n);
                        }
                    }
                    if (moved_dev == DevList.count())
                    {
                        messageBox::warningHide(this, RomID, tr("Device ") + RomID + "\n" + tr(" has been tranfered\nPlease save the configuration and restart the application"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                    }
                    else
                    {
                        messageBox::warningHide(this, RomID, tr("Device ") + RomID + "\n" + tr(" could not be tranfered"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                    }
                }
                else
                {
                    if (messageBox::questionHide(this, tr("Add EnOcean device"), tr("Do you want to add this device to the catalog ?\nDevice ID : ") + RomID.mid(0, 8) + "\nEEP : " + RomID.mid(8, 6) , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
                    {
                        QStringList DevList;
                        getDeviceList(RomID, DevList);
                        for (int n=0; n<DevList.count(); n++)
                        {
                            logThis("Add device : " + DevList.at(n));
                            onewiredevice *device = parent->configwin->DeviceExist(DevList.at(n));
                            if (!device)
                            {
                                logThis("This kind of device is not yet supported");
                                return;
                            }
                            if (!localdevice.contains(device)) localdevice.append(device);
                            UpdateLocalDeviceList();
                            device->show();
                        }
                    }
                }
            }
        }
        return;
    }
    if (messageBox::questionHide(this, tr("Add EnOcean device"), tr("Do you want to add this device ?\nDevice ID : ") + RomID.mid(0, 8) + "\nEEP : " + RomID.mid(8, 6) , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
    {
        QStringList DevList;
        getDeviceList(RomID, DevList);
        for (int n=0; n<DevList.count(); n++)
        {
            logThis("Create device : " + DevList.at(n));
            onewiredevice *device = parent->configwin->NewDevice(DevList.at(n), this);
            if (!device)
            {
                logThis("This kind of device is not yet supported");
                return;
            }
            if (!localdevice.contains(device)) localdevice.append(device);
            UpdateLocalDeviceList();
            device->show();
        }
    }
}




void eocean::getDeviceList(const QString RomID, QStringList &DevList)
{

    //QString family = RomID.right(2);
    QString family8 = RomID.right(8);
    if ((family8.left(4) == QString(familyeoA504XX).left(4)) && (family8.right(2) == QString(familyeoA504XX).right(2)))
    {
        DevList.append(RomID + "_T");
        DevList.append(RomID + "_H");
    }
    else if (family8 == familyeoD20112)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
    }
    else
    {
        //qDebug() << RomID;
        DevList.append(RomID);
    }
}




void eocean::eoStatusChange()
{
    if (eoThread.endLessLoop)
    {
        settraffic(Connected);
        settabtraffic(Connected);
        TcpStateChanged(QTcpSocket::ConnectedState);
    }
    else
    {
        settraffic(Disconnected);
        settabtraffic(Disconnected);
        TcpStateChanged(QTcpSocket::UnconnectedState);
    }
}





void eocean::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		{
         //QMenu contextualmenu;
         //QAction Search(tr("&Search"), this);
         //contextualmenu.addAction(&Search);
         //QAction *selection;
         //selection = contextualmenu.exec(event->globalPos());
         //if (selection == &GetSP) addtofifo(GetScratchPads);
         //if (selection == &GetST) addtofifo(GetStatus);
		}
	if (event->button() != Qt::LeftButton) return;
}





