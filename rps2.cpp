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
#include "errlog.h"
#include "alarmwarn.h"
#include "onewire.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "rps2.h"



rps2::rps2(logisdom *Parent) : net1wire(Parent)
{
	parameterNb = 0;
        framelayout = new QGridLayout(ui.frameguinet);
}



void rps2::init()
{
	type = NetType(TCP_RPS2Type);
	Buffer = "";
	retry = 0;
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readbuffer()));
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
	connecttcp();

	int i = 0;

	Bouton1.setText("Show");
	connect(&Bouton1, SIGNAL(clicked()), this, SLOT(ShowDevice()));
	framelayout->addWidget(&Bouton1, i++, 0, 1, 1);

// setup CreateDev
	Bouton2.setText(tr("New Device"));
	connect(&Bouton2, SIGNAL(clicked()), this , SLOT(NewDevice()));
	framelayout->addWidget(&Bouton2, i++, 0, 1, 1);

// setup RemoveDev
	//Bouton3.setText(tr("Remove Device"));
	//connect(&Bouton3, SIGNAL(clicked()), this , SLOT(RemoveModule()));
	//framelayout->addWidget(&Bouton3, i++, 0, 1, 1);

	ui.EditType->setText("RPS2");
        QString configdata;
        parent->get_ConfigData(configdata);
        readConfigFile(configdata);
}






void rps2::NewDevice()
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
	addDevice(nom, true);
}





void rps2::getConfig(QString&)
{
}




void rps2::setConfig(const QString&)
{
}



void rps2::readConfigFile(QString &configdata)
{
	QString ReadRomID;
	onewiredevice *device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	QString family = ReadRomID.right(2);
	if (ReadRomID.length() == 13) ReadRomID = ReadRomID.left(8) + port2Hex(port) + ReadRomID.right(5);
	if (ReadRomID.left(12) == (ip2Hex(moduleipaddress) + port2Hex(port)) && (family == familyRps2))
		if (!parent->configwin->deviceexist(ReadRomID))
		{
			device = parent->configwin->NewDevice(ReadRomID, this);
			if (device)
				if (!localdevice.contains(device)) localdevice.append(device);
			UpdateLocalDeviceList();
		}
	SearchLoopEnd;
}





void rps2::fifonext()
{
}






QString rps2::extractBuffer(const QString &data)
{
	QString extract;
	int etx1, etx2;
	Buffer += data;
	etx1 = Buffer.indexOf(ETX1);
	etx2 = Buffer.indexOf(ETX2);
	if ((etx1 == -1) or (etx2 == -1))
	{
		//GenMsg("No end of frame, wait next frame");
		return "";
	}
	if (etx2 != etx1 + 1)
	{
		GenMsg("Unexpected end of frame");
		Buffer.clear();
		return "";
	}
	Buffer.truncate(etx1);
	QStringList paramList = Buffer.split(SEP);
	if (parameterNb == 0)
	{
		// first time, simply get parameterNb
		if (paramList.count() > 0) parameterNb = paramList.count();
		GenMsg(QString("Set Nb Parameter to %1").arg(parameterNb));
		Buffer.clear();
		return "";
	}
	if (parameterNb != paramList.count())
	{
		GenMsg(QString("Unexpected Paramter number  %1   %2").arg(parameterNb).arg(paramList.count()));
		Buffer.clear();
		parameterNb = 0;
		return "";
	}
	extract = Buffer;
	Buffer.clear();
	return extract;
}






void rps2::readbuffer()
{
	QByteArray data;
	QString extract;
	QMutexLocker locker(&mutexreadbuffer);
more:
	data = tcp.readAll();
	QString logtxt;
	for (int n=0; n<data.length(); n++) logtxt += QString("[%1]").arg((unsigned char)data[n]);
	GenMsg(tr("Read Raw : ") + logtxt);
	extract = extractBuffer(data);
	if (extract.isEmpty()) return;
	TimeOut.stop();
	GenMsg(tr("Read : ") + extract);
	for (int n=0; n<localdevice.count(); n++)
		if (!localdevice[n]->setscratchpad(extract, true))
			GenError(34, localdevice[n]->getromid() + "  " + localdevice[n]->getname());
	if (tcp.bytesAvailable() > 0) goto more;
	TimeOut.start(30000);
}








void rps2::timeout()
{
	TimeOut.stop();
	GenError(33, name);
	reconnecttcp();
}





onewiredevice *rps2::addDevice(QString name, bool show)
{
	onewiredevice * device;
	for (int id=1; id<99; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + port2Hex(port) + QString("%1").arg(id, 3, 10, QChar('0')) + familyRps2);
		if (!parent->configwin->deviceexist(RomID))
		{
			device = parent->configwin->NewDevice(RomID, this);
			if (device)
			{
				device->setname(name);
				if (!localdevice.contains(device)) localdevice.append(device);
				UpdateLocalDeviceList();
				if (show) device->show();
				return device;
			}
		}
	}
	return nullptr;
}



