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
#include "fts800.h"



fts800::fts800(logisdom *Parent) : net1wire(Parent)
{
}



void fts800::init()
{
	Code1 = 0;
	Code2 = 0;
	NbVannes = 14;
    statusAdded = false;
	type = NetType(Ezl50_FTS800);
	Buffer = "";
	for (int i = 0; i < maxvannesFTS800; i++) VannePourcent[i] = 0;
	for (int i = 0; i < maxvannesFTS800; i++) devices[i] = nullptr;	
	modelvannes = new QStringListModel(this);
	listViewVanne.setModel(modelvannes);
	retry = 0;
	wait = default_wait;
	createonewiredevices();
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(this, SIGNAL(requestdone()), this, SLOT(fifonext()));
	TimerPause.setSingleShot(true);
	connect(&TimerPause, SIGNAL(timeout()), this, SLOT(FTS800fifonext()));

	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(connected()), this, SLOT(status()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readbuffer()));
	connecttcp();
        framelayout = new QGridLayout(ui.frameguinet);

	int i = 0;

	Bouton1.setText("Show");
	connect(&Bouton1, SIGNAL(clicked()), this, SLOT(ShowDevice()));
	framelayout->addWidget(&Bouton1, i++, 0, 1, 1);

// setup NbVanne menu
	lineEditNbVannes.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&lineEditNbVannes, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclickvannes(QPoint)));
	framelayout->addWidget(&lineEditNbVannes, i++, 0, 1, 1);

// setup Code 1 menu
	lineEditC1.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&lineEditC1, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclickcode1(QPoint)));
	framelayout->addWidget(&lineEditC1, i++, 0, 1, 1);

// setup Code 2 menu
	lineEditC2.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&lineEditC2, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclickcode2(QPoint)));
	framelayout->addWidget(&lineEditC2, i++, 0, 1, 1);
	
// setup Vannes list menu
	listViewVanne.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&listViewVanne, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklistvannes(QPoint)));
	framelayout->addWidget(&listViewVanne, i++, 0, 1, 1);

	Bouton2.setText("Status");
	framelayout->addWidget(&Bouton2, i, 0, 1, 1);	
	connect(&Bouton2, SIGNAL(clicked()), this, SLOT(status()));

	Bouton3.setText("Synchro");
	framelayout->addWidget(&Bouton3, i, 1, 1, 1);	
	connect(&Bouton3, SIGNAL(clicked()), this, SLOT(synchro()));

	ui.EditType->setText("FTS800");
	update();
}







void fts800::GetConfigStr(QString &str)
{
	str += "\n"  Net1Wire_Device  "\n";
	str += logisdom::saveformat("IPadress", moduleipaddress);
	str += logisdom::saveformat("Port", QString("%1").arg(port));
	str += logisdom::saveformat("Type", QString("%1").arg(type));
	str += logisdom::saveformat("TabName", name);
	if (ErrorLog->ui.checkBoxLogtoFile->isChecked()) str += logisdom::saveformat("Log", "1"); else str += logisdom::saveformat("Log", "0");
	if (ErrorLog->ui.checkBoxActivity->isChecked()) str += logisdom::saveformat("LogActivity", "1"); else str += logisdom::saveformat("LogActivity", "0");
	str += EndMark;
	str += "\n";
}





void fts800::setconfig(const QString &strsearch)
{
	bool logstate = false, logact = false;
	if (logisdom::getvalue("Log", strsearch) == "1") logstate = true;
	if (logisdom::getvalue("LogActivity", strsearch) == "1") logact = true;
	setipaddress(logisdom::getvalue("IPadress", strsearch));
	setport(logisdom::getvalue("Port", strsearch).toInt());
	ErrorLog->ui.checkBoxLogtoFile->setChecked(logstate);
	ErrorLog->ui.checkBoxActivity->setChecked(logact);
	init();
}





void fts800::fifonext()
{
//	QMutexLocker locker(&mutexFifonext);
	if (TimerPause.isActive()) return;
	pausefifo(250);
}





void fts800::FTS800fifonext()
{
//	QMutexLocker locker(&mutexFifonext);
	if (TimerPause.isActive()) return;
	settraffic(Connected);
	if ((statusAdded) and fifoListEmpty())
	{
		statusAdded = false;
		return;		// si le fifo est vide, on quitte
	}
	else if ((fifoListEmpty()) and (!statusAdded))
	{
		status();
	}
	QString Req ;
	QString next = fifoListNext();
	QString dataOrder = getOrder(next);
	int Order = getorder(dataOrder);
	switch (Order)
	{
		case Status : Req = getOrder(next); break;
		case Synchro : Req = getOrder(next); break;
		case SendOrder : Req = getData(next); break;
	}
	Req = Req + "\r";
	if (tcp.isOpen())
	{
		//for (int n = 0 ; n < Req.length(); n ++) writeTcp(Req[n].unicode());
        writeTcp(Req.toLatin1());
		settraffic(Waitingforanswer);
	}
	else
	{
		settraffic(Disabled);		
	}
	GenMsg(tr("Write : ") + getData(next));
	TimeOut.start(wait);
}






void fts800::status()
{
	pausefifo(2000);
	statusAdded = true;
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(Status,  ip);
}





void fts800::synchro()
{
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(Synchro,  ip);
}





QString fts800::extractBuffer(const QString &data)
{
	QString extract = "";
	int chdeb, chfin, L;
	Buffer += data;
	chdeb = Buffer.indexOf("<");
	chfin = Buffer.indexOf(">");
	L = Buffer.length();
	if ((chdeb == -1) and (chfin == -1))
	{
		//GenMsg("No begin No end found : ");
		//GenMsg("Buffer : " + Buffer);
		return "";
	}
	if ((chdeb != -1) and (chfin == -1))
	{
		//GenMsg("No end found : ");
		//GenMsg("Buffer : " + Buffer);
		return "";
	}
	if (chdeb > chfin)
	{
		Buffer = Buffer.right(L - chdeb);
		//GenMsg("No begin found : ");
		//GenMsg("Buffer : " + Buffer);
		return "";
	}
	if ((chdeb != -1) and (chfin != -1))
	{
		extract = Buffer.mid(chdeb + 1, chfin - chdeb - 1);
		Buffer = Buffer.right(L - chfin - 1);
		//GenMsg("begin found : ");
		//GenMsg("extract : " + extract);
		//GenMsg("Buffer : " + Buffer);
		return extract;
	}
	return extract;
}




void fts800::readbuffer()
{
bool ok;
int v, z, pos;
QString R;
	QByteArray data;
	QString extract;
	QMutexLocker locker(&mutexReadBuffer);
more:
	data = tcp.readAll();
	GenMsg(tr("Read : ") + data);
	extract = extractBuffer(data);
	if (extract.isEmpty()) return;
	if (fifoListCount() == 0)
	{
		pausefifo(2000);
		retry = 0;
		Buffer = "";
		return;
	}
	QString order;
	QString next = fifoListNext();
	QString dataOrder = getOrder(next);
	int Order = getorder(dataOrder);
	switch (Order)
	{
		case Status : order = getOrder(next); break;
		case Synchro : order = getOrder(next); break;
		case SendOrder : order = getData(next); break;
	}
	//GenMsg(tr("Extract : ") + extract);
	if (order == extract)
	{
	//GenMsg("fifo match");
		TimeOut.stop();
		retry = 0;
		wait = default_wait;
		fifoListRemoveFirst();
		emit requestdone();
		Buffer = "";
		return;
	}
	//GenMsg("forget");
	v = extract.toInt(&ok);
	if (ok)
	{
		//TimeOut->stop();
		pausefifo(2000);
		retry = 0;
		Buffer = "";
		return;
	}
	//GenMsg("fifo don't match");
	if ((order == NetRequestMsg[Status]) and (!extract.isEmpty()))
	{
	//GenMsg("Status analysis");
		TimeOut.stop();
		int control = 0;
		wait = default_wait;
		pos = extract.indexOf("Code1=") + 6;
		if (pos == -1)
			{ GenError(64, extract); goto C2; }
		z = extract.indexOf(" ", pos);
		if (z == -1)
			{ GenError(65, extract); goto C2; }
		R = extract.mid(pos, z - pos);
		v = R.toInt(&ok);
		if (!ok)
			{ GenError(66, R); goto C2; }
		Code1 = v;
		control ++;
	C2:
		pos = extract.indexOf("Code2=") + 6;
		if (pos == -1)
			{ GenError(67, extract); goto NZone; }
		z = extract.indexOf(" ", pos);
		if (z == -1)
			{ GenError(68, extract); goto NZone; }
		R = extract.mid(pos, z - pos);
		v = R.toInt(&ok);
		if (!ok)
			{ GenError(69, R); goto NZone; }
		Code2 = v;
		control ++;
	NZone:
		pos = extract.indexOf("NbZone=") + 7;
		if (pos == -1)
			{ GenError(70, extract); goto Zones; }
		z = extract.indexOf(" ", pos);
		if (z == -1)
			{ GenError(71,  extract); goto Zones; }
		R = extract.mid(pos, z - pos);
		v = R.toInt(&ok);
		if (!ok)
			{ GenError(72, R); goto Zones; }
		NbVannes = v;
		control ++;
	Zones:
		for (int n = 0; n < maxvannesFTS800; n ++)
		{
			R = QString("Zone%1=").arg(n + 1);
			pos = extract.indexOf(R);
			if (pos != -1)
			{
				pos += R.length();
				z = extract.indexOf(" ", pos);
				if (z == -1) { GenError(73, R); goto nextzone; }
				R = extract.mid(pos, z - pos);
				v = R.toInt(&ok);
				if (!ok)	{ GenError(74, R); goto nextzone; }
				if (!devices[n])
				{
					QString romid;
					romid = ip2Hex(moduleipaddress) + QString("V%1").arg(n + 1) + familySTA800;
					onewiredevice *device = parent->configwin->NewDevice(romid, this);
					if (device)
					{
						if (!localdevice.contains(device)) localdevice.append(device);
						UpdateLocalDeviceList();
						devices[n] = device;
					}
				}
                if (devices[n]) devices[n]->setMainValue(qRound(qreal((v*qreal(99)/qreal(255)))), true);
				VannePourcent[n] = v;
			}
		nextzone:;
		}
		if (control == 3)
		{
			update();						// update display
			fifoListRemoveFirst();
			retry = 0;
			Buffer = "";
			emit requestdone();
			return;
		}
	}
	if (order == NetRequestMsg[Synchro])
	{
	//GenMsg("Synchro sent");
		TimeOut.stop();
		retry = 0;
		fifoListRemoveFirst();
		emit requestdone();
		Buffer = "";
		return;
	}
	while (extractBuffer("") != "");
	TimeOut.stop();
	if (retry < 10)
	{
		retry ++;
		GenMsg((tr("Retry") + QString(" %1").arg(retry)));
	}
	else
	{
		TimeOut.stop();
		fifoListRemoveFirst();
		retry = 0;
		GenError(76, extract);
	}
	if (tcp.bytesAvailable() > 0) goto more;
	else emit requestdone();
}







void fts800::timeout()
{
	TimeOut.stop();
	if (tcp.bytesAvailable() > 0)
	{
		readbuffer();
		return;
	}
	wait += default_wait;
	retry ++;
	if (retry > 20)
	{
		retry = 0;
		fifoListRemoveFirst();
		request = None;
		GenError(85, name);
		reconnecttcp();
	}
	fifonext();
}







void fts800::setNbVannes(int Nb)
{
	if ((Nb != 0) and (Nb <= maxvannesFTS800))
	{
		QString Data = QString("NbZone==%1").arg(Nb);
		QString ip = ip2Hex(moduleipaddress);
		addtofifo(SendOrder, ip, Data);
	}
}







void fts800::rightclicklistvannes(const QPoint & pos)
{
bool ok;
	QMenu contextualmenu;
	QModelIndex index = listViewVanne.currentIndex();
	if (index.row() == -1) return;
 	QAction contextualaction0(tr("&Modify percentage valve %1").arg(index.row() + 1), this);
 	QAction contextualaction1(tr("&Show vanne %1").arg(index.row() + 1), this);
 	QAction contextualaction2(tr("&Assign code valve %1").arg(index.row() + 1), this);
	contextualmenu.addAction(&contextualaction0);
	contextualmenu.addAction(&contextualaction1);
	contextualmenu.addAction(&contextualaction2);
	QAction *selection = contextualmenu.exec(listViewVanne.mapToGlobal(pos));
	if (selection == &contextualaction0)
	{
		int i = inputDialog::getIntegerPalette(this, tr("Percentage Valve"),
                        tr("Percentage Valve %1 :").arg(index.row() + 1), qRound(qreal((VannePourcent[index.row()]*qreal(99)/qreal(255)))), 0, 99, 1, &ok, parent);
		if (ok)	setVannePourCent(index.row() + 1, i);
	}
	if (selection == &contextualaction1)
	{
		//onewiredevice **device = parent->configwin->DeviceArray();
        for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
            if (parent->configwin->devicePtArray[n]->getromid() == moduleipaddress + QString("V%1").arg(index.row() + 1) + familySTA800)
                parent->configwin->devicePtArray[n]->show();
    }
	if (selection == &contextualaction2) AssignCode(index.row() + 1);
}







void fts800::rightclickvannes(const QPoint & pos)
{
bool ok;

	QMenu *contextualmenu = new QMenu(this);
	QAction **contextualaction = new QAction *[1];
	contextualaction[0] = new QAction(tr("Modify valve number"), this);
	contextualmenu->addAction(contextualaction[0]);
	QAction *selection = contextualmenu->exec(lineEditNbVannes.mapToGlobal(pos));
	if (selection == contextualaction[0])
	{
        int i = inputDialog::getIntegerPalette(this, tr("Valves"), tr("Valve number :"), NbVannes, 1, maxvannesFTS800, 1, &ok, parent);
		if (ok)
		{
			setNbVannes(i);
		}
	}
}









void fts800::rightclickcode1(const QPoint & pos)
{
bool ok;

	QMenu *contextualmenu = new QMenu(this);
	QAction **contextualaction = new QAction *[1];
 	contextualaction[0] = new QAction(tr("Modify Code 1"), this);
	contextualmenu->addAction(contextualaction[0]);
	QAction *selection = contextualmenu->exec(lineEditC1.mapToGlobal(pos));
	if (selection == contextualaction[0])
	{
        int i = inputDialog::getIntegerPalette(this, tr("Code 1"), tr("Code 1 :"), Code1, 0, 99, 1, &ok, parent);
		if (ok)
		{
			setCode1(i);
		}
	}
}







void fts800::rightclickcode2(const QPoint & pos)
{
bool ok;

	QMenu *contextualmenu = new QMenu(this);
	QAction **contextualaction = new QAction *[1];
 	contextualaction[0] = new QAction(tr("Modify Code 2"), this);
	contextualmenu->addAction(contextualaction[0]);
	QAction *selection = contextualmenu->exec(lineEditC2.mapToGlobal(pos));
	if (selection == contextualaction[0])
	{
        int i = inputDialog::getIntegerPalette(this, tr("Code 2"), tr("Code 2 :"), Code2, 0, 99, 1, &ok, parent);
		if (ok)
		{
			setCode2(i);
		}
	}
}









void fts800::pausefifo(int delay)
{
	settraffic(Paused);
	TimerPause.start(delay);
}









void fts800::update()
{
	listvannes.clear();
        for (int n = 0; n < NbVannes; n ++)	listvannes << QString("Vanne %1 = %2%").arg(n + 1).arg(qRound(qreal((VannePourcent[n]*qreal(99)/qreal(255)))));
	modelvannes->setStringList(listvannes);
	lineEditNbVannes.setText(tr("%1 vannes STA 800").arg(NbVannes));
	lineEditC1.setText(tr("Code 1 : %1").arg(Code1));
	lineEditC2.setText(tr("Code 2 : %1").arg(Code2));
}





void fts800::AssignCode(int code)
{
	if ((code < 0) or (code > 14)) return;
	QString data = QString("SetZ%1").arg(code);
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(SendOrder, ip, data);
}







void fts800::setCode1(int code)
{
	if ((code < 0) or (code > 99)) return;
	QString data = QString("Code1==%1").arg(code);
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(SendOrder, ip, data);
}







void fts800::setCode2(int code)
{
	if ((code < 0) or (code > 99)) return;
	QString data = QString("Code2==%1").arg(code);
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(SendOrder, ip, data);
}



int fts800::getcode1()
{
	return Code1;
}



int fts800::getcode2()
{
	return Code2;
}



int fts800::getNbVanne()
{
	return NbVannes;
}



int fts800::getVannePourcent(int vanne)
{
	return VannePourcent[vanne];
}




void fts800::createonewiredevices()
{
	QString romid;
//	for (int n = 0; n < NbVannes; n ++)
	for (int n = 0; n < 1; n ++)
	{
		romid = ip2Hex(moduleipaddress) + QString("V%1").arg(n + 1) + familySTA800;
		onewiredevice *device = parent->configwin->NewDevice(romid, this);
		if (device)
			if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
}






void fts800::setVannePourCent(int vanne, int prc)
{
	if (vanne < 1) return;
	if (vanne > 99) return;
	if (prc < 0) return;
	if (prc > 99) return;
        QString data = QString("Zone%1==%2").arg(vanne).arg(int((prc*255/99)));
	QString ip = ip2Hex(moduleipaddress);
	addtofifo(SendOrder, ip, data);	// "Zoneii==xxx"  i = zone  xxx = percent
}




