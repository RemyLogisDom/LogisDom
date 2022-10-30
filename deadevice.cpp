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
#include "deadevice.h"



deadevice::deadevice()
{
    ui.setupUi(this);
    connect(ui.listDevice, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(deviceChanged(QListWidgetItem *)));
    connect(ui.listOldDevice, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(OldDeviceChanged(QListWidgetItem *)));
    connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(ui.pushButtonSelect, SIGNAL(clicked()), this, SLOT(select()));
    ui.pushButtonSelect->setEnabled(false);
}



void deadevice::addOldDevice(const QString dev)
{
    if (!oldDevList.contains(dev))
    {
        oldDevList.append(dev);
        ui.listOldDevice->addItem(dev);
    }
}

void deadevice::addDevice(const QString dev)
{
    if (!newDevList.contains(dev))
    {
        newDevList.append(dev);
        ui.listDevice->addItem(dev);
    }
}



void deadevice::OldDeviceChanged(QListWidgetItem *idx)
{
    *oldDevice = idx->text();
    if (oldDevice->isEmpty()) ui.pushButtonSelect->setEnabled(false); else if (newDevice->isEmpty()) ui.pushButtonSelect->setEnabled(false); else ui.pushButtonSelect->setEnabled(true);
}




void deadevice::deviceChanged(QListWidgetItem *idx)
{
    *newDevice = idx->text();
    if (oldDevice->isEmpty()) ui.pushButtonSelect->setEnabled(false); else if (newDevice->isEmpty()) ui.pushButtonSelect->setEnabled(false); else ui.pushButtonSelect->setEnabled(true);
}



void deadevice::cancel()
{
    newDevice->clear();
    oldDevice->clear();
    close();
}



void deadevice::select()
{
    close();
}



