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
#include <QtGui>
#include "globalvar.h"
#include "logisdom.h"
#include "configwindow.h"
#include "commonstring.h"
#include "onewire.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "devchooser.h"
#include "devfinder.h"



devchooser::devchooser(logisdom *Parent)
{
	qRegisterMetaType<onewiredevice*>();
	parent = Parent;
	htmlBindControler = nullptr;
	setLayout(&setupLayout);
    setupLayout.addWidget(&chooseButton, 0, 1, 1, 1);
	chooseButton.setText(tr("not assigned"));
	connect(&chooseButton, SIGNAL(clicked()), this, SLOT(chooseClick()));
    QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
    ButtonDel.setIcon(removeIcon);
    ButtonDel.setIconSize(QSize(logisdom::statusIconSize/2, logisdom::statusIconSize/2));
    ButtonDel.setToolTip(tr("Remove"));
    const QSize BUTTON_SIZE = QSize(22, 22);
    ButtonDel.setMaximumSize(BUTTON_SIZE);
    connect(&ButtonDel, SIGNAL(clicked()), this, SLOT(delClick()));
	connect(parent->configwin, SIGNAL(newDeviceAdded(onewiredevice *)), this, SLOT(newDeviceAdded(onewiredevice *)));
}





devchooser::~devchooser()
{
}



void devchooser::Lock(bool state)
{
    lockStatus = state;
    updateLock();
}



void devchooser::updateLock()
{
    if (lockStatus)
    {
        setupLayout.removeWidget(&ButtonDel);
        ButtonDel.hide();
    }
    else
    {
        setupLayout.addWidget(&ButtonDel, 0, 0, 1, 1);
        ButtonDel.show();
    }
}



void devchooser::addFamily(QString Family)
{
	AcceptedFamily << Family;
}



void devchooser::setHtmlBindControler(htmlBinder *htmlBind)
{
	htmlBindControler = htmlBind;
	onewiredevice *device = parent->configwin->DeviceExist(RomID);
	if (device) device->htmlBindControler = htmlBindControler;
}



void devchooser::setStyleSheet(QString style)
{
	chooseButton.setStyleSheet(style);
}


void devchooser::delClick()
{
    emit(deleteRequest(this));
}


void devchooser::chooseClick()
{
    onewiredevice *device = parent->configwin->DeviceExist(RomID);
    devfinder *devFinder;
    devFinder = new devfinder(parent);
    devFinder->originalDevice = device;
    foreach (QString str, AcceptedFamily) qDebug() << str;

    foreach (onewiredevice* dev, parent->configwin->devicePtArray)
    {
        if (dev->plugin_interface) {
            if (dev->plugin_interface->acceptCommand(dev->getromid()) && (dev->htmlBindControler == nullptr))
                devFinder->devicesList.append(dev);
        }
        if ((AcceptedFamily.contains(dev->getfamily()) && (dev->htmlBindControler == nullptr)) or (AcceptedFamily.isEmpty()))
            devFinder->devicesList.append(dev);
    }
    if (devFinder->devicesList.count() == 0)
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), tr("No compatible device could be found"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
    }
    devFinder->sort();
    devFinder->exec();
    if (device == devFinder->choosedDevice) return;
    if (device) device->htmlBindControler = nullptr;
    device = devFinder->choosedDevice;
    delete devFinder;
    if (device)
    {
        if (device->htmlBindControler)
            if (!AcceptedFamily.isEmpty())
            {
                messageBox::warningHide(this, cstr::toStr(cstr::MainName), device->getname() + " " + tr("is already controled"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                return;
            }
        device->htmlBindControler = htmlBindControler;
        setRomID(device->getromid());
    }
    else
    {
        setRomID("");
    }
    emit(deviceSelectionChanged());
}




void devchooser::freeDevice()
{
    onewiredevice *olddevice = parent->configwin->DeviceExist(RomID);
    if (olddevice) olddevice->htmlBindControler = nullptr;
    RomID = "";
}




void devchooser::setRomID(QString romid)
{
    onewiredevice *olddevice = parent->configwin->DeviceExist(RomID);
    if (olddevice)
    {
        olddevice->htmlBindControler = nullptr;
        disconnect(olddevice, SIGNAL(DeviceConfigChanged(onewiredevice *)), this, SLOT(DeviceConfigChanged(onewiredevice *)));
        disconnect(olddevice, SIGNAL(DeviceValueChanged(onewiredevice *)), this, SLOT(DeviceValueChanged(onewiredevice *)));
    }
    RomID = romid;
    onewiredevice *device = parent->configwin->DeviceExist(RomID);
    if (!device)
    {
		if (RomID.isEmpty()) chooseButton.setText("not assigned");
		else chooseButton.setText(RomID);
        return;
	}
    device->htmlBindControler = htmlBindControler;
	DeviceConfigChanged(device);
    connect(device, SIGNAL(DeviceConfigChanged(onewiredevice*)), this, SLOT(DeviceConfigChanged(onewiredevice*)));
    connect(device, SIGNAL(DeviceValueChanged(onewiredevice*)), this, SLOT(DeviceValueChanged(onewiredevice*)));
    emit(deviceSelectionChanged());
}





QString devchooser::getRomID()
{
	return RomID;
}




QString devchooser::getName()
{
	onewiredevice *device = parent->configwin->DeviceExist(RomID);
	if (device) return device->getname();
	return "not assigned";
}





onewiredevice *devchooser::device()
{
	onewiredevice *device = nullptr;
	device = parent->configwin->DeviceExist(RomID);
	return device;
}




void devchooser::newDeviceAdded(onewiredevice *device)
{
    if (!device) return;
    if (device->getromid() == RomID)
	{
        chooseButton.setText(device->getname() + " (" + device->MainValueToStr()+ ")");
	    setRomID(RomID);
        emit(deviceSelectionChanged());
	}
}




void devchooser::DeviceConfigChanged(onewiredevice *device)
{
    if (!device) { chooseButton.setText(tr("not defined")); return; }
    if (device->getromid() != RomID) { chooseButton.setText(RomID); return; }
    chooseButton.setText(device->getname() + " (" + device->MainValueToStr()+ ")");
}



void devchooser::DeviceValueChanged(onewiredevice *device)
{
    if (!device) return;
    if (device->getromid() == RomID) chooseButton.setText(device->getname() + " (" + device->MainValueToStr()+ ")");
}
