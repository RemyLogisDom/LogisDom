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





#include <QtGui>
#include <QUuid>
#include <QtNetwork>
#include "logisdom.h"
#include "globalvar.h"
#include "onewire.h"
#include "remote.h"
#include "errlog.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "htmlbinder.h"
#include "net1wire.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif



net1wire::net1wire(logisdom *Parent)
{
	ui.setupUi(this);
	parent = Parent;
	type = NoType;
	moduleipaddress = "";
	name = "";
	OnOff = true;
    tobeDeleted = false;
	runIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/config_run.png")), QIcon::Normal, QIcon::Off);
	stopIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/configclose.png")), QIcon::Normal, QIcon::Off);
	tcpIconUnconnectedState.addPixmap(QPixmap(QString::fromUtf8(":/images/images/disconnected.png")), QIcon::Normal, QIcon::Off);
	tcpIconHostLookupState.addPixmap(QPixmap(QString::fromUtf8(":/images/images/connecting.png")), QIcon::Normal, QIcon::Off);
	tcpIconConnectingState.addPixmap(QPixmap(QString::fromUtf8(":/images/images/connecting.png")), QIcon::Normal, QIcon::Off);
	tcpIconConnectedState.addPixmap(QPixmap(QString::fromUtf8(":/images/images/connected.png")), QIcon::Normal, QIcon::Off);
	tcpIconClosingState.addPixmap(QPixmap(QString::fromUtf8(":/images/images/disconnecting.png")), QIcon::Normal, QIcon::Off);
	htmlBind = new htmlBinder(parent, "", parent->configwin->htmlBindNetworkMenu->treeItem);
	connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));
	connect(ui.portedit, SIGNAL(editingFinished()), this, SLOT(changeport()));
	connect(ui.ipaddressedit, SIGNAL(editingFinished()), this, SLOT(setipaddress()));
	connect(ui.pushButtonShow, SIGNAL(clicked()), this, SLOT(ShowDevice()));
	connect(ui.toolButtonStart, SIGNAL(clicked()), this, SLOT(switchOn()));
	connect(ui.toolButtonStop, SIGNAL(clicked()), this, SLOT(switchOff()));
    connect(ui.toolButtonClear, SIGNAL(clicked()), parent->configwin, SLOT(supprimer()));
    ui.toolButtonStart->setToolTip(tr("Start communication"));
    ui.toolButtonStop->setToolTip(tr("Stop communication"));
    qintptr sd = tcp.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
//#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
//#endif
    connect(&tcp, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(TcpStateChanged(QAbstractSocket::SocketState)));
	connect(&TimerReqDone, SIGNAL(timeout()), this, SLOT(emitReqDone()));
	TimerReqDone.setSingleShot(true);
	TimerReqDone.setInterval(250);
	settraffic(Disabled);
	ErrorLog = parent->alarmwindow->newTab("no name");
}




net1wire::~net1wire()
{
	delete htmlBind;
	delete ErrorLog;
}




void net1wire::Lock(bool state)
{
    if (state) ui.toolButtonClear->hide();
    else ui.toolButtonClear->show();

}


QString net1wire::ip2Hex(const QString &ip)
{
	QString IPstr = IPtoHex(ip);
	if (IPstr.isEmpty()) IPstr = unID + port2Hex(port);
	return IPstr;
}



QString net1wire::IPtoHex(const QString &ip)
{
	bool ok;
	int p1 = ip.indexOf(".");		// get first point position
	if (p1 == -1) return "";
	int p2 = ip.indexOf(".", p1 + 1);	// get second point position
	if (p2 == -1) return "";
	int p3 = ip.indexOf(".", p2 + 1);	// get third point position
	if (p3 == -1) return "";
	int l = ip.length();
	QString N1 = ip.mid(0, p1);
	if (N1 == "") return "";
	QString N2 = ip.mid(p1 + 1, p2 - p1 - 1);
	if (N2 == "") return "";
	QString N3 = ip.mid(p2 + 1, p3 - p2 - 1);
	if (N3 == "") return "";
	QString N4 = ip.mid(p3 + 1, l - p3 - 1);
	if (N4 == "") return "";
	int n1 = N1.toInt(&ok);
	if (!ok) return "";
	int n2 = N2.toInt(&ok);
	if (!ok) return "";
	int n3 = N3.toInt(&ok);
	if (!ok) return "";
	int n4 = N4.toInt(&ok);
	if (!ok) return "";
	return QString("%1%2%3%4").arg(n1, 2, 16, QLatin1Char('0')).arg(n2, 2, 16, QLatin1Char('0')).arg(n3, 2, 16, QLatin1Char('0')).arg(n4, 2, 16, QLatin1Char('0')).toUpper();
}





QString net1wire::port2Hex(int P)
{
	return QString("%1").arg(P, 4, 16, QLatin1Char('0')).toUpper();
}




void net1wire::fifoListRemoveFirst()
{
	QMutexLocker locker(&mutexFifoList);
	if (ui.fifolist->count() > 0)
	{
		QListWidgetItem *item = ui.fifolist->takeItem(0);
		if (item) delete item;
	}
}


void net1wire::fifoListInsertFirst(const QString &order, int position)
{
	QMutexLocker locker(&mutexFifoList);
	QListWidgetItem *item = new QListWidgetItem(order);
	if (ui.fifolist->count() > 0)	ui.fifolist->insertItem(position, item);
	else ui.fifolist->addItem(item);
}



void net1wire::fifoListAdd(const QString &order)
{
	QMutexLocker locker(&mutexFifoList);
	QListWidgetItem *item = new QListWidgetItem(order);
	ui.fifolist->addItem(item);
}



QString net1wire::fifoListNext()
{
	QMutexLocker locker(&mutexFifoList);
	if (ui.fifolist->count() > 0)
	{
		QString next = ui.fifolist->item(0)->text();
		return next;
	}
	else return "";
}




QString net1wire::fifoListLast()
{
	QMutexLocker locker(&mutexFifoList);
	int count = ui.fifolist->count();
	if (count > 0)
	{
		QString last = ui.fifolist->item(count - 1)->text();
		return last;
	}
	else return "";
}




bool net1wire::fifoListEmpty()
{
	QMutexLocker locker(&mutexFifoList);
	if (ui.fifolist->count() != 0) return false;
	return true;
}






bool net1wire::fifoListContains(const QString &str)
{
	QMutexLocker locker(&mutexFifoList);
	for (int n=0; n<ui.fifolist->count(); n++)
		if (ui.fifolist->item(n)->text().contains(str)) return true;
	return false;
}




int net1wire::fifoListCount()
{
	QMutexLocker locker(&mutexFifoList);
	return ui.fifolist->count();
}




bool net1wire::isUploading()
{
	return false;
}





void net1wire::switchOnOff(bool state)
{
    if (state)
    {
        reconnecttcp();
    }
    else
    {
        if (tcp.state() == QAbstractSocket::ConnectedState) tcp.disconnectFromHost();
        else tcp.abort();
    }
}





void net1wire::switchOff()
{
	switch (type)
	{
		case Ha7Net :
		break;
		default : switchOnOff(false);
		break;
	}
}





void net1wire::switchOn()
{
	switch (type)
	{
		case Ha7Net :
		break;
		default : switchOnOff(true);
		break;
	}
}


void net1wire::init()
{
	Bouton1.setText(tr("No init function redefined"));
	framelayout->addWidget(&Bouton1, 0, 0, 1, 1);
}






void net1wire::UpdateLocalDeviceList()
{
	ui.localdevicecombolist->clear();
	for (int n=0; n<localdevice.count(); n++)
	{
        QString str = localdevice[n]->getromid() + "  " + localdevice[n]->getname();
		ui.localdevicecombolist->addItem(str);
	}
}




void net1wire::setrequest(const QString &)
{
}





void net1wire::fifonext()
{
}




void net1wire::ShowDevice()
{
	int index = ui.localdevicecombolist->currentIndex();
	if (index != -1)
	{
        if (localdevice[index]->isHidden()) localdevice[index]->show();
        else localdevice[index]->raise();
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int Xmax = screenGeometry.width();
        int Ymax = screenGeometry.height();
        int w = localdevice[index]->geometry().width();
        int h = localdevice[index]->geometry().height();
        if (localdevice[index]->geometry().x() > Xmax) localdevice[index]->setGeometry(Xmax/2, Ymax/2, w, h);
        if (localdevice[index]->geometry().y() > Ymax) localdevice[index]->setGeometry(Xmax/2, Ymax/2, w, h);
        parent->showPalette();
        parent->setPalette(&localdevice[index]->setup);
    }
}





QString net1wire::GenError(int ErrID, const QString Msg)
{
	QString txt;
	QMutexLocker locker(&MutexGenMsg);
	txt = ErrorLog->SetError(ErrID, Msg);
//	parent->alarmwindow->ui.errortab->setCurrentIndex(errortabindex);
	if (ErrorLog->ui.checkBoxLogtoFile->isChecked()) log(Msg);
	return txt;
}





void net1wire::GenMsg(const QString Msg)
{
	QMutexLocker locker(&MutexGenMsg);
	if (!ErrorLog->ui.checkBoxActivity->isChecked()) return;
	ErrorLog->AddMsg(Msg);
	if (ErrorLog->ui.checkBoxLogtoFile->isChecked()) log(Msg);
}







void net1wire::settabtraffic(int state)
{
	QMutexLocker locker(&mutexsettraffic);
	switch (state)
	{
		case Disabled :		break;
		case Waitingforanswer :	break;
		//case Connecting :	emit(TcpStateConnecting(this)); break;
        case Disconnected :	emit(TcpStateDisonnected(this)); ui.ipaddressedit->setEnabled(true); ui.portedit->setEnabled(true); break;
        case Connected :	emit(TcpStateConnected(this)); ui.ipaddressedit->setEnabled(false); ui.portedit->setEnabled(false); break;
		case Paused :		break;
		case Simulated :	break;
		default :		break;
	}
}




void net1wire::setTobeDeleted()
{
    tobeDeleted = true;
}



void net1wire::settraffic(int state)
{
	QMutexLocker locker(&mutexsettraffic);
	switch (state)
	{
		case Disabled :
		ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_off.png")));
			ui.traffic->setToolTip(tr("Enabled"));
		break;
		
		case Waitingforanswer : 
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_yellow.png")));
			ui.traffic->setToolTip(tr("Waiting for answer"));
		break;
		
		case Connecting :
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_red_yellow.png")));
			ui.traffic->setToolTip(tr("Waiting connection"));
		break;
		
		case Disconnected :
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_red.png")));
			ui.traffic->setToolTip(tr("Disconnected"));
		break;

		case Connected :
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_green.png")));
			ui.traffic->setToolTip(tr("Connected"));
		break;
		
		case Paused :
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_red_yellow.png")));
			ui.traffic->setToolTip(tr("Pausing"));
		break;

		case Simulated :
			ui.traffic->setPixmap(QPixmap(QString::fromUtf8(":/images/images/trafficlight_on.png")));
			ui.traffic->setToolTip(tr("Simulated mode"));
		break;
		
		default :
		break;
	}
	htmlBind->setParameter("Status", ui.traffic->toolTip());
}






bool net1wire::isGlobalConvert()
{
	return false;
}




void net1wire::remoteCommand(const QString &)
{
}



bool  net1wire::removeDeviceFromCatalog(onewiredevice *device)
{
    //qDebug() << name;
    //qDebug() << device->getromid();
    //int dev = localdevice.indexOf(device);
    //qDebug() << QString("Dev index %1").arg(dev);
    if (localdevice.removeOne(device))
    {
        UpdateLocalDeviceList();
        return true;
    }
    return false;
}


bool net1wire::isDataValid()
{
	return false;	
}







int net1wire::getport()
{
	return port;
}






QString net1wire::getname()
{
	return name;
}






void net1wire::setname(const QString &Name)
{
	name = Name;
	parent->alarmwindow->setTabName(ErrorLog, Name);
	htmlBind->setName(name);
	ErrorLog->setLogFileName(Name);
}






void net1wire::setport(int Port)
{
	ui.portedit->setText(QString("%1").arg(Port));
	port = Port;
}






void net1wire::changeport()
{
	port = ui.portedit->text().toInt();
}



void net1wire::getDeviceList(const QString RomID, QStringList &DevList)
{
    QString family = RomID.right(2);
    if (family == family2406)
    {
        DevList.append(RomID + "_A");
        //DevList.append(RomID + "_B");
    }
    else if (family == family2408)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
        DevList.append(RomID + "_C");
        DevList.append(RomID + "_D");
        DevList.append(RomID + "_E");
        DevList.append(RomID + "_F");
        DevList.append(RomID + "_G");
        DevList.append(RomID + "_H");
    }
    else if (isFamily2413)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
    }
    else if (family == family2423)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
    }
    else if (family == family2450)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
        DevList.append(RomID + "_C");
        DevList.append(RomID + "_D");
    }
    else if (family == family2438)
    {
        DevList.append(RomID + "_T");
        DevList.append(RomID + "_V");
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_I");
    }
    else if (family == familyLCDOW)
    {
        DevList.append(RomID + "_A");
        DevList.append(RomID + "_B");
        DevList.append(RomID + "_C");
        DevList.append(RomID + "_D");
    }
    else DevList.append(RomID);
}




void net1wire::newDevice(const QString &RomID)
{
    QStringList DevList;
    getDeviceList(RomID, DevList);
    for (int n=0; n<DevList.count(); n++)
    {
        createDevice(DevList.at(n));
    }
}




void net1wire::createDevice(QString RomID)
{
	onewiredevice *device = parent->configwin->NewDevice(RomID, this);
	if (device)
	{
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
}





void net1wire::deviceReturn(const QString romid, const QString scratchpad)
{
    QString family8 = romid.right(8);
    QString family = romid.right(2);
	if (scratchpad.isEmpty()) return;
    if (family == family2406)
    {
        QString RomID_A = romid + "_A";
        onewiredevice *deviceA = parent->configwin->DeviceExist(RomID_A);
        if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());
        QString RomID_B = romid + "_B";
        onewiredevice *deviceB = parent->configwin->DeviceExist(RomID_B);
        if (deviceB) deviceB->setscratchpad(scratchpad, deviceB->isAutoSave());
    }
    else if (isFamily2413)
    {
		QString RomID_A = romid + "_A";
		onewiredevice *deviceA = parent->configwin->DeviceExist(RomID_A);
        if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());
		QString RomID_B = romid + "_B";
		onewiredevice *deviceB = parent->configwin->DeviceExist(RomID_B);
        if (deviceB) deviceB->setscratchpad(scratchpad, deviceB->isAutoSave());
	}
    else if (family == family2408)
	{
		QString RomID_A = romid + "_A";
		onewiredevice *deviceA = parent->configwin->DeviceExist(RomID_A);
        if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());
		QString RomID_B = romid + "_B";
		onewiredevice *deviceB = parent->configwin->DeviceExist(RomID_B);
        if (deviceB) deviceB->setscratchpad(scratchpad, deviceB->isAutoSave());
		QString RomID_C = romid + "_C";
		onewiredevice *deviceC = parent->configwin->DeviceExist(RomID_C);
        if (deviceC) deviceC->setscratchpad(scratchpad, deviceC->isAutoSave());
		QString RomID_D = romid + "_D";
		onewiredevice *deviceD = parent->configwin->DeviceExist(RomID_D);
        if (deviceD) deviceD->setscratchpad(scratchpad, deviceD->isAutoSave());
		QString RomID_E = romid + "_E";
		onewiredevice *deviceE = parent->configwin->DeviceExist(RomID_E);
        if (deviceE) deviceE->setscratchpad(scratchpad, deviceE->isAutoSave());
		QString RomID_F = romid + "_F";
		onewiredevice *deviceF = parent->configwin->DeviceExist(RomID_F);
        if (deviceF) deviceF->setscratchpad(scratchpad, deviceF->isAutoSave());
		QString RomID_G = romid + "_G";
		onewiredevice *deviceG = parent->configwin->DeviceExist(RomID_G);
        if (deviceG) deviceG->setscratchpad(scratchpad, deviceG->isAutoSave());
		QString RomID_H = romid + "_H";
		onewiredevice *deviceH = parent->configwin->DeviceExist(RomID_H);
        if (deviceH) deviceH->setscratchpad(scratchpad, deviceH->isAutoSave());
	}
    else if ((family == family2450) || (family == familyLCDOW))
    {
        QString RomID_A = romid + "_A";
        onewiredevice *deviceA = parent->configwin->DeviceExist(RomID_A);
        if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());
        QString RomID_B = romid + "_B";
        onewiredevice *deviceB = parent->configwin->DeviceExist(RomID_B);
        if (deviceB) deviceB->setscratchpad(scratchpad, deviceB->isAutoSave());
        QString RomID_C = romid + "_C";
        onewiredevice *deviceC = parent->configwin->DeviceExist(RomID_C);
        if (deviceC) deviceC->setscratchpad(scratchpad, deviceC->isAutoSave());
        QString RomID_D = romid + "_D";
        onewiredevice *deviceD = parent->configwin->DeviceExist(RomID_D);
        if (deviceD) deviceD->setscratchpad(scratchpad, deviceD->isAutoSave());
    }
    else if (family == family2438)
	{
		QString RomID_T = romid + "_T";
		onewiredevice *deviceT = parent->configwin->DeviceExist(RomID_T);
        if (deviceT) deviceT->setscratchpad(scratchpad, deviceT->isAutoSave());
		QString RomID_V = romid + "_V";
		onewiredevice *deviceV = parent->configwin->DeviceExist(RomID_V);
        if (deviceV) deviceV->setscratchpad(scratchpad, deviceV->isAutoSave());
		QString RomID_A = romid + "_A";
		onewiredevice *deviceA = parent->configwin->DeviceExist(RomID_A);
        if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());
	}
    else if (family == familyEOcean)
    {
        //qDebug() << romid;
        onewiredevice *device = parent->configwin->EoDeviceExist(romid);
        if (localdevice.contains(device))
            if (device) device->setscratchpad(scratchpad, device->isAutoSave());

        QString RomID_T = romid + "_T";
        onewiredevice *deviceT = parent->configwin->EoDeviceExist(RomID_T);
        if (localdevice.contains(deviceT))
            if (deviceT) deviceT->setscratchpad(scratchpad, deviceT->isAutoSave());

        QString RomID_H = romid + "_H";
        onewiredevice *deviceH = parent->configwin->EoDeviceExist(RomID_H);
        if (localdevice.contains(deviceT))
            if (deviceH) deviceH->setscratchpad(scratchpad, deviceH->isAutoSave());

        QString RomID_A = romid + "_A";
        onewiredevice *deviceA = parent->configwin->EoDeviceExist(RomID_A);
        if (localdevice.contains(deviceA))
            if (deviceA) deviceA->setscratchpad(scratchpad, deviceA->isAutoSave());

        QString RomID_B = romid + "_B";
        onewiredevice *deviceB = parent->configwin->EoDeviceExist(RomID_B);
        if (localdevice.contains(deviceB))
            if (deviceB) deviceB->setscratchpad(scratchpad, deviceB->isAutoSave());
    }
    else
	{
		onewiredevice *device = parent->configwin->DeviceExist(romid);
        if (device) device->setscratchpad(scratchpad, device->isAutoSave());
	}
}



void net1wire::convert()
{
}



void net1wire::closeTCP()
{
    disconnectall();
    tcp.close();
}




void net1wire::startTCP()
{
}


bool net1wire::receiveData()
{
    return false;
}



void net1wire::disconnectall()
{
    disconnect(&tcp, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, nullptr, nullptr);
}








QString net1wire::getipaddress()
{
	return moduleipaddress;
}




void net1wire::emitReqDone()
{
	if (TimerReqDone.isActive()) return;
	emit requestdone();
}





void net1wire::connecttcp(bool force)
{
    if (!OnOff) return;
    QDateTime now = QDateTime::currentDateTime();
    if (!lastConnectionRequest.isValid()) lastConnectionRequest = now.addSecs(-61);
    if ((lastConnectionRequest.secsTo(now) > 60) or (!lastConnectionRequest.isValid()) or force)
    {
        settraffic(Connecting);
        settabtraffic(Connecting);
        tcp.abort();
        tcp.connectToHost(moduleipaddress, port);
        lastConnectionRequest = QDateTime::currentDateTime();
        GenMsg(tr("Try Tcp connection"));
    }
}





void net1wire::reconnecttcp()
{
    if (!OnOff) return;
    if (tcp.state() == QAbstractSocket::ConnectedState) tcp.disconnectFromHost();
    settraffic(Disconnected);
    settabtraffic(Disconnected);
    connecttcp(true);
}






void net1wire::tcpconnected()
{
    GenMsg(tr("Tcp Connection successfull"));
    settraffic(Connected);
    settabtraffic(Connected);
    fifonext();
}







void net1wire::tcpConnectionError(QAbstractSocket::SocketError)
{
    settabtraffic(Disconnected);
    GenError(80, tcp.errorString() + " " + moduleipaddress);
}




void net1wire::TcpStateChanged(QAbstractSocket::SocketState state)
{
    switch (state)
    {
        case QAbstractSocket::UnconnectedState : ui.toolButtonTcpState->setIcon(tcpIconUnconnectedState);
            ui.toolButtonTcpState->setToolTip(tr("Not connected"));
            settabtraffic(Disconnected);
        break;
        case QAbstractSocket::HostLookupState : ui.toolButtonTcpState->setIcon(tcpIconConnectingState);
            ui.toolButtonTcpState->setToolTip(tr("Host lookup"));
            settabtraffic(Disconnected);
        break;
        case QAbstractSocket::ConnectingState : ui.toolButtonTcpState->setIcon(tcpIconConnectingState);
            ui.toolButtonTcpState->setToolTip(tr("Connecting"));
            settabtraffic(Disconnected);
        break;
        case QAbstractSocket::ConnectedState : ui.toolButtonTcpState->setIcon(tcpIconConnectedState);
            ui.toolButtonTcpState->setToolTip(tr("Connected"));
            settabtraffic(Connected);
        break;
        case QAbstractSocket::ClosingState : ui.toolButtonTcpState->setIcon(tcpIconClosingState);
            ui.toolButtonTcpState->setToolTip(tr("Disconnecting"));
            settabtraffic(Disconnected);
        break;
        default: break;
    }
}




void net1wire::tcpdisconnected()
{
    settraffic(Disconnected);
    settabtraffic(Disconnected);
    GenError(81, tcp.errorString() + " " + moduleipaddress);
    if (!isUploading())
    {
        QTimer::singleShot(10000, this, SLOT(connecttcp()));
    }
}





void net1wire::writeTcp(char c)
{
    switch (tcp.state())
    {
        case QTcpSocket::UnconnectedState : connecttcp(); break;
        case QTcpSocket::HostLookupState : break;
        case QTcpSocket::ConnectingState : break;
        case QTcpSocket::ConnectedState :
            tcp.putChar(c);
        //	if (!tcp.waitForBytesWritten()) GenMsg("net1wire waitForBytesWritten failed");
            break;
        case QTcpSocket::BoundState : break;
        case QTcpSocket::ClosingState : break;
        default : break;
    }

}




void net1wire::writeTcp(const QByteArray req)
{
    switch (tcp.state())
    {
        case QTcpSocket::UnconnectedState : connecttcp(); break;
        case QTcpSocket::HostLookupState : break;
        case QTcpSocket::ConnectingState : break;
        case QTcpSocket::ConnectedState :
            tcp.write(req);
            //if (!tcp.waitForBytesWritten(10000))
            //{
            //	GenMsg("net1wire waitForBytesWritten failed");
            //	connecttcp();
            //}
            break;
        case QTcpSocket::BoundState : break;
        case QTcpSocket::ClosingState : break;
        default : break;
    }

}




bool net1wire::logenabled()
{
	return ErrorLog->ui.checkBoxLogtoFile->isChecked();
}




bool net1wire::logactenabled()
{
	return ErrorLog->ui.checkBoxActivity->isChecked();
}





void net1wire::setlogenabled(bool state)
{
	ErrorLog->ui.checkBoxLogtoFile->setChecked(state);
}


void net1wire::setlogactenabled(bool state)
{
	ErrorLog->ui.checkBoxActivity->setChecked(state);
}





void net1wire::log(const QString &text)
{
	parent->logthis(name, text);
}








int net1wire::gettype()
{
	return type;
}







void net1wire::mousePressEvent(QMouseEvent*)
{
/*	QLineEdit *childlineedit = dynamic_cast<QLineEdit*>(childAt(event->pos()));
	if(!childlineedit) return;
	else
	{
		if (childlineedit == ui.lineEditNbVannes)
		{
			contextualmenu = new QMenu;
			contextualaction = new QAction *[2];
         contextualaction[0] = new QAction("AddEvent", this);
			contextualaction[1] = new QAction("Supprimer", this);
			contextualmenu->addAction(contextualaction[0]);
			contextualmenu->addAction(contextualaction[1]);
			QAction *selection =  contextualmenu->exec(event->globalPos());
		}
	}*/

	//if (event->child() == ui.lineEditC1())
	//{
		
	//}

/*	if (event->button() == Qt::RightButton)
		{
		 contextualmenu = new QMenu;
		 contextualaction = new QAction *[2];
         contextualaction[0] = new QAction("AddEvent", this);
		 contextualaction[1] = new QAction("Supprimer", this);
		 contextualmenu->addAction(contextualaction[0]);
		 contextualmenu->addAction(contextualaction[1]);
		 QAction *selection =  contextualmenu->exec(event->globalPos());
		}

	if (event->button() != Qt::LeftButton) return;*/
}





// Fifo sequence =  (order) {RomID} [Data] 

void net1wire::addtofifo(const QString &order)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TimerReqDone.stop();
	QString Order = getFifoString(order, "", "");
	if (ui.fifolist->count() > fifomax)
	{
		GenError(83, order);
		TimerReqDone.start();
		return;		// si fifo trop plein on quitte
	}
	if ((Order != NetRequestMsg[Reset]) and (fifoListContains(Order)))
	{
		TimerReqDone.start();
		return;
	}
//	if ((priority) and  (ui.fifolist->count() > 2)) fifoListInsertFirst(Order);
//	else
	fifoListAdd(Order);
//	removeDuplicates();
	GenMsg(" addtofifo -> " + Order);
	TimerReqDone.start();
}




void net1wire::addtofifo(int order)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TimerReqDone.stop();
	if (!(order < LastRequest)) return;
	QString Order = getFifoString(NetRequestMsg[order], "", "");
	if (fifoListCount() > fifomax)
	{
		GenError(83, Order);
		TimerReqDone.start();
		return;		// si fifo trop plein on quitte
	}
	if ((Order != NetRequestMsg[Reset]) and (fifoListContains(Order)))
	{
		TimerReqDone.start();
		return;
	}
//	if ((priority) and  (ui.fifolist->count() > 2)) fifoListInsertFirst(Order);
//	else
	fifoListAdd(Order);
//	removeDuplicates();
	GenMsg(" addtofifo -> " + Order);
	TimerReqDone.start();
}





void net1wire::addtofifo(int order, const QString &RomID, bool priority)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TimerReqDone.stop();
	if (!(order < LastRequest)) return;
	QString Order = getFifoString(NetRequestMsg[order], RomID, "");
	if (fifoListCount() > fifomax)
	{
		GenError(83, Order);
		TimerReqDone.start();
		return;		// si fifo trop plein on quitte
	}
	if ((Order != NetRequestMsg[Reset]) and (fifoListContains(Order)))
	{
		TimerReqDone.start();
		return;
	}
	if ((priority) and  (fifoListCount() > 2)) fifoListInsertFirst(Order, 3);
		else if ((order == SetUserName) or (order == SetPassWord)) fifoListInsertFirst(Order);
	else
		fifoListAdd(Order);
//	removeDuplicates();
	GenMsg(" addtofifo -> " + Order);
	TimerReqDone.start();
}





void net1wire::addtofifo(int order, const QString &RomID, const QString &Data, bool priority)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TimerReqDone.stop();
	if (!(order < LastRequest)) return;
	QString Order = getFifoString(NetRequestMsg[order], RomID, Data);
	if (fifoListCount() > fifomax)
	{
		GenError(83, Order);
		TimerReqDone.start();
		return;		// si fifo trop plein on quitte
	}
	if ((Order != NetRequestMsg[Reset]) and (fifoListContains(Order)))
	{
		TimerReqDone.start();
		return;
	}
    if ((priority) and (ui.fifolist->count() > 2)) fifoListInsertFirst(Order);
    else fifoListAdd(Order);
//	removeDuplicates();
	GenMsg(" addtofifo -> " + Order);
	TimerReqDone.start();
}






void net1wire::clearfifo()
{
	QMutexLocker locker(&mutexFifoList);
	ui.fifolist->clear();
}





void net1wire::removeDuplicates()
{
}



void net1wire::addremotefifo(QString &str)
{
	TimerReqDone.stop();
	QMutexLocker locker(&mutexFifoList);
	QListWidgetItem *item = new QListWidgetItem(str);
	ui.fifolist->addItem(item);
	GenMsg(" addtofifo -> " + str);
	TimerReqDone.start();
	//emit requestdone();
}




int net1wire::getorder(QString &str)
{
	for (int n=0; n<LastRequest; n++)
		if (NetRequestMsg[n] == str) return n;
	return 0;
}




QString net1wire::getData(const QString &str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("[");
	end = str.indexOf("]");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}




QString net1wire::getOrder(const QString &str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("*");
	end = str.indexOf("#");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}




QString net1wire::getRomID(const QString &str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("{");
	end = str.indexOf("}");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}




QString net1wire::getFifoString(const QString Order, const QString RomID, const QString Data)
{
	QString str;
	if (!Order.isEmpty()) str += "*" + Order + "#";
	if (!RomID.isEmpty()) str += "{" + RomID + "}"; else if (!ip2Hex(moduleipaddress).isEmpty()) str += "{" + ip2Hex(moduleipaddress) + "}";
	if (!Data.isEmpty()) str += "[" + Data + "]";
	return str;
}





void net1wire::voiralldevice()
{
	for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
		if (parent->configwin->devicePtArray[n]->getMaster() == this) parent->configwin->devicePtArray[n]->show();
}





void net1wire::searchbutton()
{
	addtofifo(Search);
}




void net1wire::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();			// mise en mémoire de l'adresse IP
}




void net1wire::setipaddress(const QString &adr)
{
	moduleipaddress = adr;			// mise en mémoire de l'adresse IP
	ui.ipaddressedit->setText(adr);		// affichage de l'adresse IP
    if (adr == simulator) settraffic(Simulated);
}





void net1wire::setunID(const QString &ID)
{
	unID = ID;
}





QString net1wire::getUid()
{
	return unID;
}



void net1wire::getMainValue()
{
}


void net1wire::saveMainValue()
{
}



QString net1wire::getScratchPad(const QString &, int)
{
	return "";
}


bool net1wire::checkDevConfig(const QString &, int)
{
    return true;
}


QString net1wire::writeScratchPad(const QString &, int)
{
	return "";
}

QString net1wire::writeScratchPad(const QString&, const QString&)
{
    return "";
}


void net1wire::setName(QString Name)
{
	name = Name;
}



void net1wire::getCfgStr(QString &str)
{
    if (tobeDeleted) return;
	str += "\n"  Net1Wire_Device  "\n";
	str += logisdom::saveformat("IPadress", moduleipaddress);
	str += logisdom::saveformat("UUID", unID);
	str += logisdom::saveformat("Port", QString("%1").arg(port));
	str += logisdom::saveformat("Type", QString("%1").arg(type));
    str += logisdom::saveformat("TabName", name, true);
	getConfig(str);
	ErrorLog->getCfgStr(str);
	str += EndMark;
	str += "\n";
}



void net1wire::setCfgStr(const QString &strsearch)
{
	setipaddress(logisdom::getvalue("IPadress", strsearch));
	setport(logisdom::getvalue("Port", strsearch).toInt());
	QString ID = logisdom::getvalue("UUID", strsearch);
	if (!ID.isEmpty()) unID = ID;
	setConfig(strsearch);
	ErrorLog->setCfgStr(strsearch);
	if (!parent->logTag) ErrorLog->ui.checkBoxActivity->setChecked(false);
	init();
}




void net1wire::getConfig(QString &)
{
}


void net1wire::setConfig(const QString &)
{
}
