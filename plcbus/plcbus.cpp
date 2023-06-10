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
#include "configwindow.h"
#include "onewire.h"
#include "alarmwarn.h"
#include "errlog.h"
#include "vmc.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "plcbus.h"



plcbus::plcbus(logisdom *Parent) : net1wire(Parent)
{
	PCLBUSStatus = PCLBUS_NOTHING;
	UserCode = 0;
	HouseCodeSeekIndex = 0;
	QDateTime now = QDateTime::currentDateTime();
	setUserCode(0);
}




void plcbus::init()
{
	type = NetType(Ezl50_PlcBus);
	connect(this, SIGNAL(requestdone()), this, SLOT(delayfifonextPLCBUS()));
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(PCLBUS_timeout()));
	connect(this, SIGNAL(X10Change()), maison1wirewindow->SwitchArea, SLOT(updateStatus()));
        framelayout = new QGridLayout(ui.frameguinet);
	int i = 0;

// setup User Code
	framelayout->addWidget(&UserCodeButton, i, 0, 1, 1);

// setup User Code
	deviceAck.setText(tr("Use Device Ack"));
	framelayout->addWidget(&deviceAck, i, 1, 1, 1);

// setup User Code
	devicePool.setText(tr("Device Pool State"));
	framelayout->addWidget(&devicePool, i++, 2, 1, 1);

// setup House Code combo
	for (int c=65; c<81; c++) ComboHouseCode.addItem(QChar(c));
	framelayout->addWidget(&ComboHouseCode, i, 0, 1, 1);

// setup ComboDeviceCode combo
	for (int c=1; c<=16; c++) ComboDeviceCode.addItem(QString("%1").arg(c));
	framelayout->addWidget(&ComboDeviceCode, i, 1, 1, 1);

// setup CreateDev
	Bouton1.setText(tr("New unit"));
	connect(&Bouton1, SIGNAL(clicked()), this , SLOT(NewModule()));
	framelayout->addWidget(&Bouton1, i++, 2, 1, 1);

// setup buttons
	Bouton2.setText(tr("On"));
	framelayout->addWidget(&Bouton2, i, 0, 1, 1);
	connect(&Bouton2, SIGNAL(clicked()), this, SLOT(ClickOn()));

	Bouton3.setText(tr("Off"));
	framelayout->addWidget(&Bouton3, i++, 1, 1, 1);
	connect(&Bouton3, SIGNAL(clicked()), this, SLOT(ClickOff()));

	Bouton4.setText(tr("Dim"));
//	framelayout->addWidget(&Bouton4, i, 0, 1, 1);	
//	connect(&Bouton4, SIGNAL(clicked()), this, SLOT(SendX10Dim()));

	Bouton5.setText("Bright");
//	framelayout->addWidget(&Bouton5, i, 1, 1, 1);	
//	connect(&Bouton5, SIGNAL(clicked()), this, SLOT(SendX10Bright()));

//	Bouton6.setText("FeedBack ON");
//	framelayout->addWidget(&Bouton6, i, 1, 1, 1);	
//	connect(&Bouton6, SIGNAL(clicked()), this, SLOT(FeedBackON()));

	ui.EditType->setText("PCLBUS");
	retry = 0;
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readPLCBus()));
	connecttcp();
	connect(&TcpHeartBeatTimer, SIGNAL(timeout()), this, SLOT(PCLBUS_HeartBeat()));
	TcpHeartBeatTimer.setSingleShot(false);
	connect(&UserCodeButton, SIGNAL(clicked()), this, SLOT(clickUserCode()));
        QString configdata;
        parent->get_ConfigData(configdata);
        readConfigFile(configdata);
	TcpHeartBeatTimer.start(10000);
}





void plcbus::clickUserCode()
{
	setUserCode();
}





void plcbus::setUserCode(int code)
{
	if (code == -1)
	{
		bool ok;
        int c = inputDialog::getIntegerPalette(this, tr("PCLBUS User Code"), tr("User Code : "), UserCode, 0, 249, 1, &ok, parent);
		if (!ok) return;
		UserCode = c;
	}
	else
	{
		if (code > 249) return;
		if (code < 0) return;
		UserCode = code;
	}
	QString txt = tr("User Code");
	UserCodeButton.setText(txt + QString (" %1").arg(UserCode));
}




void plcbus::fifonext()
{
//	QMutexLocker locker(&mutexFifonext);
	if (PCLBUSStatus != PCLBUS_NOTHING) return;
next:
	if (fifoListEmpty()) return;		// si le fifo est vide, on quitte
	QString next = fifoListNext();
	QString O = getOrder(next);
	int order = getorder(O);
	QString req = getData(next);
	if (order == None)		
	{	
		fifoListRemoveFirst();
		goto next;
	}
	switch(order) 
	{
		case PCLBUSCommand :
			PCLBUSStatus = PCLBUS_CHECK;
			setrequest(req);
			break;
		default : 
			PCLBUSStatus = PCLBUS_NOTHING;
			fifoListRemoveFirst();
			break;
	}
}







void plcbus::setrequest(const QString &req)
{
	bool ok;
	QString logtxt;
	settraffic(Waitingforanswer);
	int length = req.length() / 2 + 1;
	writeTcp(STX);
        logtxt += QString("[%1]").arg(uchar(STX));
        writeTcp(uchar(length));
	logtxt += QString("[%1]").arg(length);
	writeTcp(UserCode);
	logtxt += QString("[%1]").arg(UserCode);
	for (int n=0; n<req.length(); n+=2)
	{
		int N = req.mid(n, 2).toInt(&ok, 16);
		if (ok)
		{
			writeTcp((unsigned char)N);
			logtxt += QString("[%1]").arg(N);
		}
		else GenMsg("toInt conversion error");
	}
	writeTcp(ETX);
	logtxt += QString("[%1]").arg((unsigned char)ETX);
	GenMsg("request : " + logtxt);
	TimeOut.start(2000);
}





void plcbus::readPLCBus()
{
	QMutexLocker locker(&mutexreadPLCBus);
	settraffic(Connected);
more:
	int FrameLength = 0, dataLength = 0;
	tcpdata.append(tcp.readAll());
	FrameLength = tcpdata.length();
	QString logtxt;
	for (int n=0; n<tcpdata.length(); n++) logtxt += QString("[%1]").arg((unsigned char)tcpdata[n]);
	if (!logtxt.isEmpty()) GenMsg("Read : " + logtxt);	
	if (FrameLength == 0)
	{
		GenMsg("Read PLC Bus no data");
		return;
	}
	if (char(tcpdata[0]) != STX)
	{
		GenMsg("Read PLC Bus frame not starting with STX");
		return;
	}
	if (FrameLength > 2) dataLength = tcpdata[1];
	if (FrameLength < dataLength + 3)
	{
		GenMsg("Full frame not received yet");
		return;
	}
	if (tcpdata[dataLength + 2] != ETX)
	{
		GenMsg("Frame not ending with ETX");
		return;
	}
//	TimeOut->stop();
	QByteArray data = tcpdata.mid(2, dataLength);
	tcpdata.remove(0, dataLength + 3);
	bool requestFinished = false;
	if (fifoListEmpty())
	{
		// Analyse PCLBUS Status
	}
	else
	{
		// Check request answer
		bool ok = true;
//		QString O = getOrder(fifolist[0]);
//		int order = getorder(O);
		QString next = fifoListNext();
        QByteArray req;
        req.append(getData(next).toLatin1());
		int Command = data[2] & 0x1F;	// remove extra bit info added for request ACK
		int feedback = data[2] & ACK_PULSE;
		int codeH, codeM, state;
		switch(PCLBUSStatus) 
		{
			case PCLBUS_NOTHING :
				TimeOut.stop();
				requestFinished = true;
			break;

			case PCLBUS_CHECK :
// Check is returned UserCode is our UserCode
				if (data[0] != (char)UserCode) ok = false;
				//if (data[1] != req[0]) ok = false;
				//if (data[2] != req[1]) ok = false;
				//if (data[3] != req[2]) ok = false;
				if (ok)
// returned data = request, data is correct
				{
					switch(Command)
					{
						case PLC_ON :
						case PLC_OFF :
							if (feedback)
							{
								PCLBUSStatus = PCLBUS_ACK;
								TimeOut.start(2000);
							}
							else
							{
								PCLBUSStatus = PCLBUS_NOTHING;
								requestFinished = true;
							}
						break;
						case PLC_GET_ON_ID_PULSE :
							PCLBUSStatus = PCLBUS_DATA;
							TimeOut.start(2000);
						break;
						default :
							PCLBUSStatus = PCLBUS_NOTHING;
						break;
					}
				}
				else GenMsg("Request check error");
			break;

			case PCLBUS_ACK :
				codeH = data[1] / 16;
				codeM = data[1] & 0x0F;
				state = data[3];
				for (int n=0; n<localdevice.count(); n++)
				{
					if ((localdevice[n]->getHouseCode() == codeH) && (localdevice[n]->getModuleCode() == codeM))
					{
/*						switch(Command)
						{
							case PLC_ON :
								localdevice[n]->On(false);
							break;
							case PLC_OFF :
								localdevice[n]->Off(false);
							break;
						}*/
						if (state == 100) localdevice[n]->On(false);
						if (state == 0) localdevice[n]->Off(false);
					}
				}
				PCLBUSStatus = PCLBUS_NOTHING;
				requestFinished = true;
				break;

			case PCLBUS_DATA :
				requestFinished = true;
				switch(Command) 
				{
					case PLC_GET_ON_ID_PULSE :
//		 GenMsg("PLC_GET_ON_ID_PULSE");	
					int codeMArray[16];
					int bit = 1;
					for (int n=0; n<8; n++)
					{
						if (bit & data[4]) codeMArray[n] = 1; else codeMArray[n] = 0;
						bit *= 2;
					}
					bit = 1;
					for (int n=0; n<8; n++)
					{
						if (bit & data[3]) codeMArray[n + 8] = 1; else codeMArray[n + 8] = 0;
						bit *= 2;
					}
					codeH = data[1] / 16;
					for (int n=0; n<localdevice.count(); n++)
					{
						if (localdevice[n]->getHouseCode() == codeH)
						{
							bool PCLStatus = false;
						//	bool deviceStatus = false;
							if (codeMArray[localdevice[n]->getModuleCode()]) PCLStatus = true;
							if (PCLStatus) localdevice[n]->On(false);
							else localdevice[n]->Off(false);
						//	if (localdevice[n]->getMainValue() > 0) deviceStatus = true;
						//	if ((PCLStatus != deviceStatus) && (PCLStatus)) localdevice[n]->On(false);
						//	else if ((PCLStatus != deviceStatus) && (!PCLStatus)) localdevice[n]->Off(false);
						//	else if ((localdevice[n]->getMainValue() == logisdom::NA) && (PCLStatus)) localdevice[n]->On(false);
						//	else if ((localdevice[n]->getMainValue() == logisdom::NA) && (!PCLStatus)) localdevice[n]->Off(false);
						}
					}
					PCLBUSStatus = PCLBUS_NOTHING;
					break;
				}
				break;
			default: 
				requestFinished = true;
		}

		if (requestFinished == true)
		{
			TimeOut.stop();
			fifoListRemoveFirst();
			retry = 0;
			emit requestdone();
		}
/*		else
		{
			PCLBUSStatus = PCLBUS_NOTHING;
			if (retry < 10)
			{
				retry ++;
				GenMsg(QString("PCLBUS Request corrupted, retry %1").arg(retry));
			}
			else
			{
				retry = 0;	
				GenMsg("Retry aborted : " + fifolist[0]);
				modelnet1wire->setStringList(fifolist);
				if (tcp->state() == QAbstractSocket::ConnectedState) tcp->disconnect(); else connecttcp();
			}
		}*/
	}
	if (tcp.bytesAvailable() > 0) goto more;
	if (!tcpdata.isEmpty()) goto more;
}







void plcbus::NewModule()
{
	bool ok;
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	retry:
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (maison1wirewindow->configwin->devicenameexist(nom))
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto retry;
	}
//	QStringList X10type;
//	X10type << familyAM12 << familyLM12;
//	QString DevType = QInputDialog::getItem(this, tr("Module Type"), tr("Module Type : "), X10type, 0, false, &ok);
//	if (!ok) return;
//	if (DevType.isEmpty()) return;
	onewiredevice * device;
	for (int id=1; id<999; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + QString("%1").arg(id, 3, 10, QChar('0')) + familyPLCBUS);
		if (!maison1wirewindow->configwin->deviceexist(RomID))
		{
			device = maison1wirewindow->configwin->NewDevice(RomID, this);
			if (device)
			{
				device->setname(nom);
				int H  = ComboHouseCode.currentIndex();
				int M  = ComboDeviceCode.currentIndex();
				device->setHouseCode(H, M);
				if (!localdevice.contains(device)) localdevice.append(device);
				UpdateLocalDeviceList();
				device->show();
				return;
			}
		}
	}
	if (messageBox::questionHide(this, tr("Problem ?"), tr("Unit creation impossible\nRetry ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) goto retry;
}






void plcbus::getConfig(QString &str)
{
	str += logisdom::saveformat("UserCode", QString("%1").arg(UserCode));
	str += logisdom::saveformat("FeedBack", QString("%1").arg(deviceAck.isChecked()));
	str += logisdom::saveformat("DevicePool", QString("%1").arg(devicePool.isChecked()));
}





void plcbus::setConfig(const QString &strsearch)
{
	bool ok;
	int code = logisdom::getvalue("UserCode", strsearch).toInt(&ok);
	if (ok) setUserCode(code); else setUserCode(0);
	int F = logisdom::getvalue("FeedBack", strsearch).toInt(&ok);
	if (ok && (F != 0)) deviceAck.setCheckState(Qt::Checked);
	int P = logisdom::getvalue("DevicePool", strsearch).toInt(&ok);
	if (ok && (P != 0)) devicePool.setCheckState(Qt::Checked);
}






void plcbus::readConfigFile(QString &configdata)
{
	QString ReadRomID, MasterIP;
	onewiredevice * device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	MasterIP = logisdom::getvalue("Master", strsearch);
	if (ReadRomID.left(8) == ip2Hex(moduleipaddress).toUpper()) 
		if (!maison1wirewindow->configwin->deviceexist(ReadRomID))
		{
			device = maison1wirewindow->configwin->NewDevice(ReadRomID, this);
			if (device)
				if (!localdevice.contains(device)) localdevice.append(device);
			UpdateLocalDeviceList();
		}
	SearchLoopEnd;
}






void plcbus::ClickOn()
{
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	// Unit Code
	QString Code = QString("%1").arg((ComboHouseCode.currentIndex() * 16 + ComboDeviceCode.currentIndex()), 2, 16, QChar('0')).toUpper();
	// Command
	if (deviceAck.isChecked())	Code += QString("%1").arg(PLC_ON + ACK_PULSE, 2, 16, QChar('0')).toUpper();
	else Code += QString("%1").arg(PLC_ON, 2, 16, QChar('0')).toUpper();
	// Data 0000
	Code += "0000";
	QString empty = "";
	addtofifo(PCLBUSCommand, empty, Code);
}





void plcbus::ClickOff()
{
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	// Unit Code
	QString Code = QString("%1").arg((ComboHouseCode.currentIndex() * 16 + ComboDeviceCode.currentIndex()), 2, 16, QChar('0')).toUpper();
	// Command
	if (deviceAck.isChecked())	Code += QString("%1").arg(PLC_OFF + ACK_PULSE, 2, 16, QChar('0')).toUpper();
	else Code += QString("%1").arg(PLC_OFF, 2, 16, QChar('0')).toUpper();
	// Data 0000
	Code += "0000";
	QString empty = "";
	addtofifo(PCLBUSCommand, empty, Code);
}






void plcbus::FeedBackON()
{
	if (ComboDeviceCode.currentIndex() == -1) return;
	// Unit Code
	QString Code = QString("%1").arg((ComboHouseCode.currentIndex() * 16 + ComboDeviceCode.currentIndex()), 2, 16, QChar('0')).toUpper();
	// Command
	Code += QString("%1").arg(PLC_GET_ON_ID_PULSE, 2, 16, QChar('0')).toUpper();
	// Data 0000
	Code += "0000";
	QString empty = "";
	addtofifo(PCLBUSCommand, empty, Code);
}





void plcbus::PCLBUS_HeartBeat()
{
//	if (!devicePool.isChecked()) return;
// Get House Code List
	QList <int> HoudeCodeList;
	for (int n=0; n<localdevice.count(); n++)
	{
		int HC = localdevice[n]->getHouseCode();
		if (!HoudeCodeList.contains(HC)) HoudeCodeList.append(HC);
	}
	if (HoudeCodeList.count() == 0) HouseCodeSeekIndex = -1;
	int codeSeek;
	if (HouseCodeSeekIndex == -1) codeSeek = 0;
	else
	if (HouseCodeSeekIndex < HoudeCodeList.count())
		{
			codeSeek = HoudeCodeList[HouseCodeSeekIndex];
		}
		else
		{
			HouseCodeSeekIndex = 0;
			codeSeek = HoudeCodeList[0];
		}
	// Unit Code
	QString Code = QString("%1").arg(codeSeek * 16, 2, 16, QChar('0')).toUpper();
	// Command
	Code += QString("%1").arg(PLC_GET_ON_ID_PULSE, 2, 16, QChar('0')).toUpper();
	// Data 0000
	Code += "0000";
	QString empty = "";
	addtofifo(PCLBUSCommand, empty, Code);
	HouseCodeSeekIndex++;
}





void plcbus::PCLBUS_timeout()
{
	QString next = fifoListNext();
	if (tcp.bytesAvailable() > 0)
	{
		readPLCBus();
		return;
	}
	switch (PCLBUSStatus)
	{
		case PCLBUS_NOTHING:
			if (retry < 10)
			{
				retry ++;
				GenMsg(QString("PCLBUS timed out, retry %1").arg(retry));
				emit requestdone();
			}
			else
			{
				retry = 0;	
				GenMsg("Retry aborted : " + next);
				fifoListRemoveFirst();
				emit requestdone();
				reconnecttcp();
				//if (tcp.state() == QAbstractSocket::ConnectedState) tcp.disconnect(); else connecttcp();
			}
		break;
		case PCLBUS_ACK :
			if (retry < 3)
			{
				retry ++;
				GenMsg(QString("PCLBUS ACK timed out, device not present on the network, retry %1").arg(retry));
			}
			else
			{
				retry = 0;	
				GenMsg("Retry aborted : " + next);
				fifoListRemoveFirst();
			}
			PCLBUSStatus = PCLBUS_NOTHING;
			emit requestdone();
		break;
		case PCLBUS_DATA :
			if (retry < 3)
			{
				retry ++;
				GenMsg(QString("PCLBUS DATA timed out, retry %1").arg(retry));
			}
			else
			{
				retry = 0;	
				GenMsg("Retry aborted : " + next);
				fifoListRemoveFirst();
			}
			PCLBUSStatus = PCLBUS_NOTHING;
			emit requestdone();
		break;
		case PCLBUS_CHECK :
			if (retry < 3)
			{
				retry ++;
				GenMsg(QString("PCLBUS CHECK timed out, retry %1").arg(retry));
			}
			else
			{
				retry = 0;	
				GenMsg("Retry aborted : " + next);
				fifoListRemoveFirst();
			}
			PCLBUSStatus = PCLBUS_NOTHING;
			emit requestdone();
		break;
		default:
			if (fifoListCount() > 0) fifoListRemoveFirst();
			PCLBUSStatus = PCLBUS_NOTHING;
			emit requestdone();
	}
}






void plcbus::delayfifonextPLCBUS()
{
	QTimer::singleShot(500, this, SLOT(fifonext()));
}




