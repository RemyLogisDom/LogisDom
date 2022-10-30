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




#include "net1wire.h"

#include "onewire.h"
#include "globalvar.h"
#include "errlog.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "ha7sthread.h"
#include "ha7s.h"



ha7s::ha7s(logisdom *Parent) : net1wire(Parent)
{
	//qRegisterMetaType<QAbstractSocket::SocketState>();
    ui.fifolist->hide();
    framelayout = new QGridLayout(ui.frameguinet);
    type = NetType(TCP_HA7SType);
    setport(default_port_EZL50);
    searchInterval.addItem(tr("Search Once"));
    searchInterval.addItem(tr("Continus Search"));
    searchInterval.addItem(tr("Search 5 minutes"));
    searchInterval.addItem(tr("Search 10 minutes"));
    searchInterval.addItem(tr("Search 30 minutes"));
    searchInterval.addItem(tr("Search 1 hours"));
    searchInterval.addItem(tr("Search 6 hours"));
    searchInterval.addItem(tr("Search 1 day"));
    connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
    connect(&HA7SThread, SIGNAL(newDevice(QString)), this, SLOT(newDeviceSlot(QString)), Qt::QueuedConnection);
    connect(&HA7SThread, SIGNAL(deviceReturn(QString, QString)), this, SLOT(deviceReturn(QString, QString)), Qt::QueuedConnection);
    connect(&HA7SThread, SIGNAL(tcpStatusUpdate(QString)), this, SLOT(tcpStatusUpdate(QString)), Qt::QueuedConnection);
    connect(&HA7SThread, SIGNAL(tcpStatusChange()), this, SLOT(tcpStatusChange()), Qt::QueuedConnection);
    connect(&searchInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(searchIntervalChanged(int)));
    connect(&Global_Convert, SIGNAL(toggled(bool)), this, SLOT(Global_ConvertChanged(bool)));
}




ha7s::~ha7s()
{
    HA7SThread.endLessLoop = false;
}



void ha7s::startTCP()
{
    HA7SThread.start(QThread::LowestPriority);
    /*QThread *thread = new QThread;
    HA7SThread.moveToThread(thread);
    connect(thread, SIGNAL(started()), &HA7SThread, SLOT(process()));
    connect(&HA7SThread, SIGNAL(finished()), thread, SLOT(quit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();*/
}



void ha7s::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();
    HA7SThread.moduleipaddress = moduleipaddress;
}


void ha7s::closeTCP()
{
    HA7SThread.endLessLoop = false;
}


void ha7s::init()
{
	int i = 0;

   // framelayout->addWidget(&serialPort, i++, 0, 1, 1);

    Bouton2.setText(tr("Show All"));
	connect(&Bouton2, SIGNAL(clicked()), this, SLOT(voiralldevice()));
	framelayout->addWidget(&Bouton2, i++, 0, 1, 1);

	Global_Convert.setText(tr("Global Convert"));
	framelayout->addWidget(&Global_Convert, i++, 0, 1, 1);

	framelayout->addWidget(&searchInterval, i++, 0, 1, 1);

	ui.toolButtonClear->hide();
	ui.traffic->hide();
	ui.EditType->setText("HA7S");

    HA7SThread.endLessLoop = true;
    HA7SThread.moduleipaddress = moduleipaddress;
    HA7SThread.port = port;
}





void ha7s::switchOnOff(bool state)
{
	if (state)
	{
        HA7SThread.endLessLoop = true;
		OnOff = true;
        HA7SThread.start(QThread::LowestPriority);
        /*QThread *thread = new QThread;
        HA7SThread.moveToThread(thread);
        connect(thread, SIGNAL(started()), &HA7SThread, SLOT(process()));
        connect(&HA7SThread, SIGNAL(finished()), thread, SLOT(quit()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();*/
    }
	else
	{
        HA7SThread.endLessLoop = false;
		OnOff = false;
	}
}



bool ha7s::isGlobalConvert()
{
	return Global_Convert.isChecked();
}



void ha7s::addtofifo(const QString &order)
{
    HA7SThread.addToFIFOSpecial(order);
}


void ha7s::addtofifo(int order)
{
    HA7SThread.addToFIFOSpecial(NetRequestMsg[order]);
}


void ha7s::addtofifo(int order, const QString &RomID, bool)
{
	QString Data = "";
    HA7SThread.addToFIFOSpecial(RomID, Data, order);
}


void ha7s::addtofifo(int order, const QString &RomID, const QString &Data, bool)
{
    HA7SThread.addToFIFOSpecial(RomID, Data, order);
}


QString ha7s::getScratchPad(const QString &RomID, int scratchpad_ID)
{
    if (!HA7SThread.isRunning) return "";
    return HA7SThread.getScratchPad(RomID.left(16), scratchpad_ID);
}


void ha7s::logEnabledChanged(int state)
{
    if (state) HA7SThread.logEnabled = true;
    else HA7SThread.logEnabled = false;
}



void ha7s::newDeviceSlot(const QString &romid)
{
    if (!romid.isEmpty()) newDevice(romid);
}



void ha7s::tcpStatusUpdate(const QString &str)
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





void ha7s::searchIntervalChanged(int index)
{
	int delay = -1;
	switch (index)
	{
		case searchOnce : delay = -1; break;
		case searchConstant : delay = 0; break;
		case search5mn : delay = 300; break;
		case search10mn : delay = 600; break;
		case search30mn : delay = 1800; break;
		case search1hr : delay = 3600; break;
		case search6hr : delay = 21600; break;
		case search1day : delay = 86400; break;
	}
    HA7SThread.searchDelay = delay;
}




void ha7s::Global_ConvertChanged(bool state)
{
    HA7SThread.GlobalConvert = state;
}



void ha7s::tcpStatusChange()
{
    TcpStateChanged(HA7SThread.tcpStatus);
}




void ha7s::getConfig(QString &str)
{
	if (Global_Convert.isChecked()) str += logisdom::saveformat("GlobalConvert", "1"); else str += logisdom::saveformat("GlobalConvert", "0");
	str += logisdom::saveformat("searchInterval", QString("%1").arg(searchInterval.currentIndex()));
}





void ha7s::setConfig(const QString &strsearch)
{
	bool GlobalConvert = false;
	if (logisdom::getvalue("GlobalConvert", strsearch) == "1") GlobalConvert = true;
	Global_Convert.setChecked(GlobalConvert);
    if (GlobalConvert) HA7SThread.GlobalConvert = true;
	bool ok;
	double SI = logisdom::getvalue("searchInterval", strsearch).toDouble(&ok);
	if (!ok) SI = 1;
	if (SI < 0) SI = 1;
	searchInterval.setCurrentIndex(SI);
	searchIntervalChanged(SI);
}





