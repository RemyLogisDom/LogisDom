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




#include "configwindow.h"
#include "devfinder.h"



devfinder::devfinder(logisdom *Parent, int opt)
{
    parent = Parent;
    options = opt;
    ui.setupUi(this);
    choosedDevice = nullptr;
    originalDevice = nullptr;
    done = false;
    connect(ui.lineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
    connect(ui.listMaster, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(masterChanged(QListWidgetItem *)));
    connect(ui.listDevice, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(deviceChanged(QListWidgetItem *)));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui.pushButtonSelect, SIGNAL(clicked()), this, SLOT(select()));
    connect(ui.pushButtonClear, SIGNAL(clicked()), this, SLOT(clear()));
    connect(ui.listDevice, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(select(QModelIndex)));
    ui.pushButtonSelect->setEnabled(false);
    net1wires.clear();
    ui.listMaster->clear();
    for (int n=0; n<parent->configwin->net1wirearray.count(); n++)
        net1wires.append(parent->configwin->net1wirearray.at(n));
    for (int n=0; n<parent->logisdomInterfaces.count(); n++)
        plugins.append(parent->logisdomInterfaces.at(n));
    ui.listMaster->addItem(tr("All"));
    ui.listMaster->addItem(tr("Virtual device"));
    for (int n=0; n<net1wires.count(); n++) {
        ui.listMaster->addItem(net1wires.at(n)->getname()); }
    for (int n=0; n<plugins.count(); n++) {
        ui.listMaster->addItem(plugins.at(n)->getName()); }
}





void devfinder::sort()
{
    if (devicesList.isEmpty()) {
        for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
            devicesList.append(parent->configwin->devicePtArray[n]); }
    devices.clear();
    ui.listDevice->clear();
    int index = ui.listMaster->currentIndex().row();
    QListWidgetItem *item = ui.listMaster->currentItem();
    QString masterName;
    if (item) masterName = item->text();
    if (index < 1)    // All
    {
        for (int n=0; n<devicesList.count(); n++)
        {
            if (((devicesList.at(n)->getname()).contains(ui.lineEdit->text(), Qt::CaseInsensitive)) or (ui.lineEdit->text().isEmpty()))
            {
                devices.append(devicesList.at(n));
                ui.listDevice->addItem(devices.last()->getname());
            }
        }
    }
    else if (index == 1)  // Virtual device
    {
        for (int n=0; n<devicesList.count(); n++)
        {
            if ((!devicesList.at(n)->master) and (!devicesList.at(n)->plugin_interface))
            {
                if (((devicesList.at(n)->getname()).contains(ui.lineEdit->text(), Qt::CaseInsensitive)) or (ui.lineEdit->text().isEmpty()))
                {
                    devices.append(devicesList.at(n));
                    ui.listDevice->addItem(devices.last()->getname());
                }
            }
        }
    }
    else if (index > 1)
    {
        for (int n=0; n<devicesList.count(); n++)
        {
            if (devicesList.at(n)->master) {
                if (devicesList.at(n)->master->getname() == masterName) {
                    if (((devicesList.at(n)->getname()).contains(ui.lineEdit->text(), Qt::CaseInsensitive)) or (ui.lineEdit->text().isEmpty())) {
                        devices.append(devicesList.at(n));
                        ui.listDevice->addItem(devices.last()->getname()); } } }
            if (devicesList.at(n)->plugin_interface) {
                if (devicesList.at(n)->plugin_interface->getName() == masterName) {
                    if (((devicesList.at(n)->getname()).contains(ui.lineEdit->text(), Qt::CaseInsensitive)) or (ui.lineEdit->text().isEmpty())) {
                        devices.append(devicesList.at(n));
                        ui.listDevice->addItem(devices.last()->getname()); } } }
        }
    }
    if (devices.count() == 0) ui.pushButtonSelect->setEnabled(false);
}



void devfinder::textChanged(const QString &)
{
    ui.listDevice->clear();
    sort();
}


void devfinder::masterChanged(QListWidgetItem *)
{
    sort();
}



void devfinder::deviceChanged(QListWidgetItem *)
{
    ui.pushButtonSelect->setEnabled(true);
    int index = ui.listDevice->currentIndex().row();
    if ((index != -1) && (index < devices.count())) choosedDevice = devices.at(index);
}




void devfinder::closeEvent(QCloseEvent *)
{
    if (!done) choosedDevice = originalDevice;
}



void devfinder::cancel()
{
    close();
}



void devfinder::select(QModelIndex idx)
{
    int index = idx.row();
    if ((index != -1) && (index < devices.count())) choosedDevice = devices.at(index);
    done = true;
    close();
}


void devfinder::select()
{
    done = true;
    close();
}



void devfinder::clear()
{
    choosedDevice = nullptr;
    done = true;
    close();
}
