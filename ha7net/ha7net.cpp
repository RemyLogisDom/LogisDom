/****************************************************************************
**
** Copyright (C) 2019 Remy CARISIO.
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
#ifndef HA7Net_No_Thread

#include "globalvar.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "onewire.h"
#include "messagebox.h"
#include "errlog.h"
#include "ha7netthread.h"
#include "ha7net.h"



ha7net::ha7net(logisdom *Parent) : net1wire(Parent)
{
	HttpThread = nullptr;
	newThread();
	type = NetType(Ha7Net);
	setport(default_port_http);
	LockID = "";
	searchInterval.addItem(tr("Search Once"));
	searchInterval.addItem(tr("Continus Search"));
	searchInterval.addItem(tr("Search 5 minutes"));
	searchInterval.addItem(tr("Search 10 minutes"));
	searchInterval.addItem(tr("Search 30 minutes"));
	searchInterval.addItem(tr("Search 1 hours"));
	searchInterval.addItem(tr("Search 6 hours"));
	searchInterval.addItem(tr("Search 1 day"));
	connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
        connect(&logUpdate , SIGNAL(timeout()), this, SLOT(updateLog()), Qt::QueuedConnection);
	logUpdate.setSingleShot(false);
	logUpdate.start(1000);
}



ha7net::~ha7net()
{
}





void ha7net::init()
{
	framelayout = new QGridLayout(ui.frame);
	int i = 0;

	Bouton2.setText(tr("Show All"));
	connect(&Bouton2, SIGNAL(clicked()), this, SLOT(voiralldevice()));
	framelayout->addWidget(&Bouton2, i++, 0, 1, 1);
	
	Bouton6.setText(tr("Ha7Net config"));
	connect(&Bouton6, SIGNAL(clicked()), this, SLOT(Ha7Netconfig()));
	framelayout->addWidget(&Bouton6, i++, 0, 1, 1);

	Global_Convert.setText(tr("Global Convert"));
	framelayout->addWidget(&Global_Convert, i++, 0, 1, 1);
	connect(&Global_Convert, SIGNAL(toggled(bool)), this, SLOT(Global_ConvertChanged(bool)));

	framelayout->addWidget(&searchInterval, i++, 0, 1, 1);
	connect(&searchInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(searchIntervalChanged(int)));

	ui.EditType->setText("HA7Net");

	if (moduleipaddress == simulator) LockEnable.setChecked(false);
	ui.toolButtonTcpState->hide();
	ui.toolButtonClear->hide();
	ui.traffic->hide();
	HttpThread->moduleipaddress = moduleipaddress;
	HttpThread->port = port;
}




void ha7net::newThread()
{
	HttpThread = new ha7netthread;
	HttpThread->endLessLoop = true;
	HttpThread->moduleipaddress = moduleipaddress;
	HttpThread->port = port;
	HttpThread->start(QThread::LowestPriority);
    connect(HttpThread, SIGNAL(newDevice(QString)), this, SLOT(newDeviceSlot(QString)), Qt::QueuedConnection);
    connect(HttpThread, SIGNAL(deviceReturn(QString, QString)), this, SLOT(deviceReturn(QString, QString)), Qt::QueuedConnection);
    connect(HttpThread, SIGNAL(tcpStatusUpdate(QString)), this, SLOT(tcpStatusUpdate(QString)), Qt::QueuedConnection);
}




void ha7net::setipaddress(const QString &adr)
{
	moduleipaddress = adr;			// mise en mémoire de l'adresse IP
	ui.ipaddressedit->setText(adr);		// affichage de l'adresse IP
	if (adr == simulator) settraffic(Simulated);
	if (HttpThread)	HttpThread->moduleipaddress = moduleipaddress;
}




void ha7net::setport(int Port)
{
	ui.portedit->setText(QString("%1").arg(Port));
	port = Port;
	if (HttpThread)	HttpThread->port = port;
}


void ha7net::switchOnOff(bool state)
{
	if (state)
	{
		HttpThread->endLessLoop = true;
		OnOff = true;
		HttpThread->start(QThread::LowestPriority);
	}
	else
	{
		HttpThread->endLessLoop = false;
		OnOff = false;
	}
}




void ha7net::addtofifo(const QString &order)
{
	HttpThread->addToFIFOSpecial(order);
}




void ha7net::addtofifo(int order)
{
	HttpThread->addToFIFOSpecial(NetRequestMsg[order]);
}




void ha7net::addtofifo(int order, const QString &RomID, bool priority)
{
	QString Data = "";
	HttpThread->addToFIFOSpecial(RomID, Data, order);
}


void ha7net::addtofifo(int order, const QString &RomID, const QString &Data, bool priority)
{
	HttpThread->addToFIFOSpecial(RomID, Data, order);
}




QString ha7net::getScratchPad(const QString &RomID, int scratchpad_ID)
{
	if (!HttpThread->isRunning()) return "";
	return HttpThread->getScratchPad(RomID.left(16), scratchpad_ID);
}



void ha7net::logEnabledChanged(int state)
{
	if (state) HttpThread->logEnabled = true;
	else HttpThread->logEnabled = false;
}




void ha7net::searchIntervalChanged(int index)
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
	HttpThread->searchDelay = delay;
}





void ha7net::Global_ConvertChanged(bool state)
{
	HttpThread->GlobalConvert = state;
}




void ha7net::newDeviceSlot(const QString romid)
{
	if (!romid.isEmpty()) newDevice(romid);
}



void ha7net::tcpStatusUpdate(const QString str)
{
    ui.textBrowser->setText(str);
}



void ha7net::updateLog()
{
    QString str;
    HttpThread->getLog(str);
    ui.textBrowser->setText(str);
}



void ha7net::Ha7Netconfig()
{
	  messageBox::aboutHide(this, tr("Ha7net Configuration"),
                           tr("Admin login :  user 'admin' password 'eds'\n"
                           "Default IP adress 192.168.0.250\n"
                           "Change Lock idle time out int the Miscellaneous tab to 5 seconds\n"
                           "\n"
                           "\n\n"));
}




void ha7net::getConfig(QString &str)
{
	if (LockEnable.isChecked()) str += logisdom::saveformat("LockID", "1"); else str += logisdom::saveformat("LockID", "0");
	if (Global_Convert.isChecked()) str += logisdom::saveformat("GlobalConvert", "1"); else str += logisdom::saveformat("GlobalConvert", "0");
	str += logisdom::saveformat("searchInterval", QString("%1").arg(searchInterval.currentIndex()));
}





void ha7net::setConfig(const QString &strsearch)
{
	bool Lock = false, GlobalConvert = false;
	if (getvalue("LockID", strsearch) == "1") Lock = true;
	if (getvalue("GlobalConvert", strsearch) == "1") GlobalConvert = true;
	Global_Convert.setChecked(GlobalConvert);
	if (GlobalConvert) HttpThread->GlobalConvert = true;
	LockEnable.setChecked(Lock);
	Global_Convert.setChecked(GlobalConvert);
	bool ok;
	double SI = logisdom::getvalue("searchInterval", strsearch).toDouble(&ok);
	if (!ok) SI = 1;
	if (SI < 0) SI = 1;
	searchInterval.setCurrentIndex(SI);
}



bool  ha7net::checkbusshorted(const QString &data)
{
// search buffer for    "Exception_String_0" TYPE="text" VALUE="Short detected on 1-Wire Bus"
	int i;
	QByteArray strsearch;
	strsearch = "\"Exception_String_0\" TYPE=\"text\" VALUE=\"Short detected on 1-Wire Bus\"";
	i = data.indexOf(strsearch);
	if (i != -1) return true; else return false;
}






bool  ha7net::checkLockIDCompliant(const QString &data)
{
// search buffer for   "A BLOCK of data may not be empty"
	int j, k;
	QByteArray strsearch;
	j = data.indexOf("Lock ID");
	k = data.indexOf("does not exist");
	if ((j != -1) and (k != -1)) return true; 
	return false;
}




#endif // HA7Net_No_Thread
