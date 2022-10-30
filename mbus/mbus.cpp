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
#include "alarmwarn.h"
#include "configwindow.h"
#include "onewire.h"
#include "messagebox.h"
#include "errlog.h"
#include "mbusthread.h"
#include "mbus.h"



mbus::mbus(logisdom *Parent) : net1wire(Parent)
{
    uiw.setupUi(ui.frameguinet);
    parent = Parent;
    type = NetType(MBusType);
    uiw.treeWidget->setColumnCount(5);
    connect(uiw.treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklist(QPoint)));
    setport(default_port_EZL50);
    uiw.ReadInterval->addItem(tr("1 Minute"));
    uiw.ReadInterval->addItem(tr("5 Minute"));
    uiw.ReadInterval->addItem(tr("10 Minute"));
    uiw.ReadInterval->addItem(tr("15 Minute"));
    uiw.ReadInterval->addItem(tr("20 Minute"));
    uiw.ReadInterval->addItem(tr("30 Minute"));
    uiw.ReadInterval->addItem(tr("1 hour"));
    uiw.ReadInterval->addItem(tr("1 Day"));
    connect(uiw.spinBoxSearchMax, SIGNAL(valueChanged(int)), this, SLOT(searchMaxChanged(int)));
    connect(uiw.pushButtonSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));
    connect(uiw.ReadInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(readIntervalChanged(int)));
    connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
    connect(&mbusThread, SIGNAL(tcpStatusUpdate(QString)), this, SLOT(tcpStatusUpdate(QString)), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(tcpStatusChange()), this, SLOT(tcpStatusChange()), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(newTreeAddress(int)), this, SLOT(newTreeAddress(int)), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(setTreeItem(QString)), this, SLOT(setTreeItem(QString)), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(setMainTreeItem(QString)), this, SLOT(setTreeItem(QString)), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(whatdoUDo(QString)), this, SLOT(whatdoUDo(QString)), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(ReadingDone()), this, SLOT(ReadingDone()), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(ReadingDevDone()), this, SLOT(ReadingDevDone()), Qt::QueuedConnection);
    connect(&mbusThread, SIGNAL(CheckDev()), this, SLOT(CheckDev()), Qt::QueuedConnection);
    connect(this, SIGNAL(appendAdr(QString)), &mbusThread, SLOT(appendAdr(QString)), Qt::QueuedConnection);
}



mbus::~mbus()
{
}



void mbus::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();
    mbusThread.moduleipaddress = moduleipaddress;
}



void mbus::init()
{
    ui.EditType->setText("M-Bus");
    ui.toolButtonClear->hide();
    ui.traffic->hide();
    mbusThread.endLessLoop = true;
    mbusThread.moduleipaddress = moduleipaddress;
    mbusThread.port = port;
    QString configdata;
    parent->get_ConfigData(configdata);
    QString ReadRomID;
    onewiredevice *device = nullptr;
    QString TAG_Begin = One_Wire_Device;
    QString TAG_End = EndMark;
    SearchLoopBegin
    ReadRomID = logisdom::getvalue("RomID", strsearch);
    QString MBusAdr = logisdom::getvalue("MBusAdr", strsearch);
    QString MBusDatId = logisdom::getvalue("MBusDatId", strsearch);
    QString family = ReadRomID.right(2);
    if ((ReadRomID.left(12) == ip2Hex(moduleipaddress) + port2Hex(port)) && (family == familyMBus))
        if (!parent->configwin->deviceexist(ReadRomID))
        {
            device = parent->configwin->NewDevice(ReadRomID, this);
            if (device)
            {
                if (!localdevice.contains(device))
                {
                    localdevice.append(device);
                    devicesScratchPad.append("");
                    UpdateLocalDeviceList();
                }
                QString config;
                config += logisdom::saveformat("MBusAdr", MBusAdr);
                config += logisdom::saveformat("MBusDatId", MBusDatId);
                device->setconfig(config);
            }
        }
    SearchLoopEnd
}




void mbus::startTCP()
{
    mbusThread.start(QThread::LowestPriority);
    /*QThread *thread = new QThread;
    mbusThread.moveToThread(thread);
    connect(thread, SIGNAL(started()), &mbusThread, SLOT(process()));
    connect(&mbusThread, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();*/
}



void mbus::switchOnOff(bool state)
{
    if (state)
    {
        mbusThread.endLessLoop = true;
        OnOff = true;
        mbusThread.start(QThread::LowestPriority);
        /*QThread *thread = new QThread;
        mbusThread.moveToThread(thread);
        connect(thread, SIGNAL(started()), &mbusThread, SLOT(process()));
        connect(&mbusThread, SIGNAL(finished()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();*/
    }
    else
    {
        mbusThread.endLessLoop = false;
        OnOff = false;
    }
}




void mbus::logEnabledChanged(int state)
{
    if (state) mbusThread.logEnabled = true;
    else mbusThread.logEnabled = false;
}



void mbus::searchMaxChanged(int max)
{
    mbusThread.searchMax = max;
}



void mbus::searchClicked()
{
    mbusThread.searchDone = false;
}





void mbus::readIntervalChanged(int index)
{
    int delay = 0;
    if (index != -1) delay = index;
    mbusThread.readInterval = delay;
}




onewiredevice *mbus::addDevice()
{
    onewiredevice *device;
    for (int id=1; id<999; id++)
    {
        QString RomID = (ip2Hex(moduleipaddress) + port2Hex(port) + QString("%1").arg(id, 3, 10, QChar('0')) + familyMBus);
        device = parent->configwin->DeviceExist(RomID);
        if (!device)
        {
            device = parent->configwin->NewDevice(RomID, this);
            device->setname(RomID);
            if (!localdevice.contains(device)) localdevice.append(device);
            devicesScratchPad.append("");
            UpdateLocalDeviceList();
            device->show();
            return device;
        }
    }
    return nullptr;
}




void mbus::tcpStatusUpdate(const QString &str)
{
    ui.textBrowser->setText(str);
}



void mbus::tcpStatusChange()
{
    TcpStateChanged(mbusThread.tcpStatus);
}




void mbus::newTreeAddress(int adr)
{
    for (int n=0; n<uiw.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *topItem = uiw.treeWidget->topLevelItem(n);
        if (topItem->text(0) == QString("Adr %1").arg(adr)) return;
    }
    QTreeWidgetItem *item = new QTreeWidgetItem(uiw.treeWidget, 0);
    item->setText(0, QString("Adr %1").arg(adr));
}




QTreeWidgetItem *mbus::getItem(const QString &adr, const QString &parameter)
{
    for (int n=0; n<uiw.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *topItem = uiw.treeWidget->topLevelItem(n);
        if (topItem)
        {
            if (topItem->text(0) == adr)
            {
                for (int i=0; i<topItem->childCount(); i++)
                    if (topItem->child(i)->text(0) == parameter)
                        return topItem->child(i);
            }
        }
    }
    return nullptr;
}





QTreeWidgetItem *mbus::getItem(const QString &adr, const QString &record, const QString &parameter)
{
    for (int n=0; n<uiw.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *topItem = uiw.treeWidget->topLevelItem(n);
        if (topItem)
        {
            if (topItem->text(0) == adr)
            {
                for (int i=0; i<topItem->childCount(); i++)
                {
                    QTreeWidgetItem *lowerItem = topItem->child(i);
                    if (lowerItem->text(0) == record)
                    {
                        for (int j=0; j<lowerItem->childCount(); j++)
                        {
                            if (lowerItem->child(j)->text(0) == parameter)
                                return lowerItem->child(j);
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}



QTreeWidgetItem *mbus::getTree(const QString &adr)
{
    for (int n=0; n<uiw.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *topItem = uiw.treeWidget->topLevelItem(n);
        if (topItem->text(0) == adr) return topItem;
    }
    return nullptr;
}




QTreeWidgetItem *mbus::getTree(const QString &adr, const QString &record)
{
    for (int n=0; n<uiw.treeWidget->topLevelItemCount(); n++)
    {
        QTreeWidgetItem *topItem = uiw.treeWidget->topLevelItem(n);
        if (topItem->text(0) == adr)
        {
            for (int i=0; i<topItem->childCount(); i++)
            {
                QTreeWidgetItem *lowerItem = topItem->child(i);
                if (lowerItem->text(0) == record)
                {
                    return lowerItem;
                }
            }
            QTreeWidgetItem *Item = new QTreeWidgetItem(topItem);
            Item->setText(0, record);
            return Item;
        }
    }
    return nullptr;
}





void mbus::ReadingDone()
{
    for (int n=0; n<localdevice.count(); n++)
        localdevice.at(n)->lecturerec();
}




void mbus::ReadingDevDone()
{
    while (!devicetoread.isEmpty()) devicetoread.takeFirst()->masterlecturerec();
}




void mbus::CheckDev()
{
    QStringList list;
    devicetoread.clear();
    for (int n=0; n<localdevice.count(); n++)
    {
        if (localdevice.at(n)->hastoread())
        {
            QString str;
            localdevice.at(n)->GetConfigStr(str);
            QString MBusAdr = logisdom::getvalue("MBusAdr", str);
            if (!list.contains(MBusAdr))
            {
                emit(appendAdr(MBusAdr));
                list.append(MBusAdr);
            }
            devicetoread.append(localdevice.at(n));
        }
    }
}



void mbus::rightclicklist(const QPoint &pos)
{
    QTreeWidgetItem *item = uiw.treeWidget->currentItem();
    QTreeWidgetItem *record = item->parent();
    if (!record) return;
    QString Adr = record->text(0);
    QString DatId = item->text(0);
    QString RomID = item->text(4);
    if (!RomID.isEmpty())
        if (parent->configwin->deviceexist(RomID))
        {
            QMenu contextualmenu;
            QAction actionDelDevice(tr("&Remove device"), this);
            contextualmenu.addAction(&actionDelDevice);
            QAction *selection = contextualmenu.exec(uiw.treeWidget->mapToGlobal(pos));
            if (selection == &actionDelDevice)
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Remove device"));
                msgBox.setInformativeText(tr("Do you want to remove the device ?"));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes)
                {
                    //onewiredevice *dev = nullptr;
                    for (int n=0; n<localdevice.count(); n++)
                    {
                        if (localdevice.at(n)->getromid() == RomID)
                        {
                            //dev = localdevice.at(n);
                            //parent->configwin->removeDevice(dev);
                            localdevice.removeAt(n);
                            msgBox.setText(tr("Done"));
                            msgBox.setInformativeText(tr("Please save and restart the application"));
                            msgBox.setStandardButtons(QMessageBox::Ok);
                            msgBox.exec();
                            break;
                        }
                    }
                }
            }
            return;
        }
    QMenu contextualmenu;
    QModelIndex index = uiw.treeWidget->currentIndex();
    if (index.row() == -1) return;
    QAction actionNewDevice(tr("&Create New device"), this);
    contextualmenu.addAction(&actionNewDevice);
    QAction *selection = contextualmenu.exec(uiw.treeWidget->mapToGlobal(pos));
    if (selection == &actionNewDevice)
    {
        onewiredevice *device = addDevice();
        if (device)
        {
            QString config;
            config += logisdom::saveformat("MBusAdr", Adr);
            config += logisdom::saveformat("MBusDatId", DatId);
            //config += logisdom::saveformat("MBusDatStr", DatStr.replace('(', "[").replace(')', "]"));
            device->setconfig(config);
            QString RomID = device->getromid();
            item->setText(4, RomID);
            setScratchPad(RomID, item->text(1));
            device->setscratchpad(item->text(1));
        }
    }
}




QString mbus::getScratchPad(const QString &RomID, int)
{
    for (int n=0; n<localdevice.count(); n++)
        if (localdevice.at(n)->getromid() == RomID)
        {
            return devicesScratchPad.at(n);
        }
    return "";
}




void mbus::setScratchPad(const QString &RomID, const QString &Value)
{
    for (int n=0; n<localdevice.count(); n++)
        if (localdevice.at(n)->getromid() == RomID)
        {
            localdevice.at(n)->setscratchpad(Value, true);
            devicesScratchPad.replace(n, Value);
            return;
        }
}



void mbus::setTreeItem(const QString &parameters)	// adr/name/Value
{
    QStringList list = parameters.split("=");
    if ((list.count() == 4) or (list.count() == 5))
    {
        QTreeWidgetItem *item = getItem(list.at(0), list.at(1));
        if (!item)
        {
            QTreeWidgetItem *tree = getTree(list.at(0));
            if (tree)
            {
                item = new QTreeWidgetItem(tree);
                for (int n=0; n<localdevice.count(); n++)
                {
                    QString str;
                    localdevice.at(n)->GetConfigStr(str);
                    QString MBusAdr = logisdom::getvalue("MBusAdr", str);
                    QString MBusDatId = logisdom::getvalue("MBusDatId", str);
                    if ((MBusAdr == list.at(0)) and (MBusDatId == list.at(1)))
                    {
                        item->setText(4, localdevice.at(n)->getromid());
                        localdevice.at(n)->setscratchpad(list.at(3));
                    }
                }
            }
            else return;
        }
        if (item)
        {
            item->setText(0, list.at(1));
            item->setText(1, list.at(2));
            item->setText(2, list.at(3));
            if (list.count() == 5) item->setText(3, list.at(4));
            if (!item->text(4).isEmpty()) setScratchPad(item->text(4), list.at(3));
        }
    }
}





void mbus::setMainTreeItem(const QString &parameters)	// adr/name/Value
{
    QStringList list = parameters.split("=");
    if (list.count() == 5)
    {
        QTreeWidgetItem *item = getItem(list.at(0), list.at(1), list.at(2));
        if (!item)
        {
            QTreeWidgetItem *branch = getTree(list.at(0), list.at(1));
            if (branch)
            {
                    item = new QTreeWidgetItem(branch);
                    branch->setText(2, list.at(2));
                    branch->setText(1, list.at(3));
                    item->setText(0, list.at(2));
                    item->setText(1, list.at(3));
                    item->setText(2, list.at(4));
                    QString MBusAdr = list.at(0);
                    QString MBusDatId = list.at(1);
                    QString MBusDatStr = list.at(2);
                                        QString configdata;
                                        parent->get_ConfigData(configdata);
                                        QString ReadRomID;
                    onewiredevice *device = nullptr;
                    QString TAG_Begin = One_Wire_Device;
                    QString TAG_End = EndMark;
                    SearchLoopBegin
                    ReadRomID = logisdom::getvalue("RomID", strsearch);
                    QString ReadMBusAdr = logisdom::getvalue("MBusAdr", strsearch);
                    QString ReadMBusDatId = logisdom::getvalue("MBusDatId", strsearch);
                    QString ReadMBusDatStr = logisdom::getvalue("MBusDatStr", strsearch).replace('[', "(").replace(']', ")");
                    QString family = ReadRomID.right(2);
                    if ((ReadRomID.left(12) == ip2Hex(moduleipaddress) + port2Hex(port)) && (family == familyMBus))
                        if (!parent->configwin->deviceexist(ReadRomID))
                        {
                            if ((ReadMBusAdr == MBusAdr) && (ReadMBusDatId == MBusDatId) && (ReadMBusDatStr == MBusDatStr))
                            {
                                device = parent->configwin->NewDevice(ReadRomID, this);
                                if (device)
                                {
                                    item->setText(3, ReadRomID);
                                    if (!localdevice.contains(device))
                                    {
                                        localdevice.append(device);
                                        devicesScratchPad.append("");
                                        UpdateLocalDeviceList();
                                    }
                                    QString config;
                                    config += logisdom::saveformat("MBusAdr", MBusAdr);
                                    config += logisdom::saveformat("MBusDatId", MBusDatId);
                                    config += logisdom::saveformat("MBusDatStr", MBusDatStr.replace('(', "[").replace(')', "]"));
                                    device->setconfig(config);
                                    setScratchPad(ReadRomID, item->text(1));
                                    device->setscratchpad(item->text(1));
                                    return;
                                }
                            }
                        }
                    SearchLoopEnd
            }
            else return;
        }
        if (item)
        {
            item->setText(0, list.at(2));
            item->setText(1, list.at(3));
            item->setText(2, list.at(4));
            if (!item->text(4).isEmpty()) setScratchPad(item->text(4), list.at(3));
        }
    }
}


void mbus::whatdoUDo(const QString str)	// adr/name/Value
{
    uiw.labelStatus->setText(str);
}




void mbus::getConfig(QString &str)
{
    str += logisdom::saveformat("searchMax", QString("%1").arg(uiw.spinBoxSearchMax->value()));
    str += logisdom::saveformat("ReadInterval", QString("%1").arg(uiw.ReadInterval->currentIndex()));
}





void mbus::setConfig(const QString &strsearch)
{
    bool ok;
    double SM = logisdom::getvalue("searchMax", strsearch).toDouble(&ok);
    if (ok)
    {
        uiw.spinBoxSearchMax->setValue(int(SM));
        searchMaxChanged(int(SM));
    }
    double RI = logisdom::getvalue("ReadInterval", strsearch).toDouble(&ok);
    if (!ok) RI = 1;
    if (RI < 0) RI = 1;
    uiw.ReadInterval->setCurrentIndex(int(RI));
    readIntervalChanged(int(RI));
}





