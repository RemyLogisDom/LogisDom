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
#include "net1wire.h"
#include "onewire.h"
#include "configwindow.h"
#include "alarmwarn.h"
#include "vmc.h"
#include "errlog.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "x10.h"



x10::x10(logisdom *Parent) : net1wire(Parent)
{
        framelayout = new QGridLayout(ui.frameguinet);
	Cm11Status = CM11_NOTHING;
	remoteSleep = 0;
	QDateTime now = QDateTime::currentDateTime();
	lastX10EmitON.setDate(now.date());
	lastX10EmitON.setTime(now.time());
}




void x10::init()
{
	type = NetType(Ezl50_X10);
	lastAddress.H = 0;
	lastAddress.M = 0;
	connect(this, SIGNAL(requestdone()), this, SLOT(delayfifonextX10()));
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(CM11_timeout()));
	connect(this, SIGNAL(X10Change()), parent->SwitchArea, SLOT(updateStatus()));
	int i = 0;

// setup Afficher
	Bouton6.setText(tr("Show"));
	framelayout->addWidget(&Bouton6, i++, 2, 1, 1);	
	connect(&Bouton6, SIGNAL(clicked()), this, SLOT(ShowDevice()));

// setup House Code combo
	for (int c=65; c<81; c++) ComboHouseCode.addItem(QChar(c));
	connect(&ComboHouseCode, SIGNAL(currentIndexChanged(int)), this , SLOT(SendX10Address()));
	framelayout->addWidget(&ComboHouseCode, i, 0, 1, 1);

// setup ComboDeviceCode combo
	for (int c=1; c<=16; c++) ComboDeviceCode.addItem(QString("%1").arg(c));
	connect(&ComboDeviceCode, SIGNAL(currentIndexChanged(int)), this , SLOT(SendX10Address()));
	framelayout->addWidget(&ComboDeviceCode, i, 1, 1, 1);

// setup CreateDev
	Bouton1.setText(tr("New unit"));
	connect(&Bouton1, SIGNAL(clicked()), this , SLOT(NewModule()));
	framelayout->addWidget(&Bouton1, i++, 2, 1, 1);

// setup buttons
	Bouton2.setText(tr("On"));
	framelayout->addWidget(&Bouton2, i, 0, 1, 1);
	connect(&Bouton2, SIGNAL(clicked()), this, SLOT(SendX10On()));

	Bouton3.setText(tr("Off"));
	framelayout->addWidget(&Bouton3, i++, 1, 1, 1);
	connect(&Bouton3, SIGNAL(clicked()), this, SLOT(SendX10Off()));

	Bouton4.setText(tr("Dim"));
	framelayout->addWidget(&Bouton4, i, 0, 1, 1);	
	connect(&Bouton4, SIGNAL(clicked()), this, SLOT(SendX10Dim()));

	Bouton5.setText("Bright");
	framelayout->addWidget(&Bouton5, i, 1, 1, 1);	
	connect(&Bouton5, SIGNAL(clicked()), this, SLOT(SendX10Bright()));

// setup power spin box
	framelayout->addWidget(&X10Power, i++, 2, 1, 1);
	X10Power.setSingleStep(4.5);
	X10Power.setSuffix(" %");
	X10Power.setDecimals(0);
	X10Power.setMaximum(99);
	X10Power.setMinimum(0);

	ui.EditType->setText("X10");
	retry = 0;
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readX10()));
	if (moduleipaddress != "0.0.0.0") connecttcp();
	connect(&TcpHeartBeatTimer, SIGNAL(timeout()), this, SLOT(X10HeartBeat()));
	TcpHeartBeatTimer.setSingleShot(true);
    QString configdata;
    parent->get_ConfigData(configdata);
    readConfigFile(configdata);
	//TcpHeartBeatTimer->start(30000);
}





void x10::GetConfigStr(QString &str)
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



void x10::setconfig(const QString &strsearch)
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




void x10::fifonext()
{
	bool ok;
	if (moduleipaddress == "0.0.0.0")
	{
		clearfifo();
		settraffic(Simulated);
		return;
	}
	if (Cm11Status != CM11_NOTHING) return;
next:
	if (fifoListEmpty()) return;		// si le fifo est vide, on quitte
	QString next = fifoListNext();
	QString O = getOrder(next);
	int order = getorder(O);
	QString req = getData(next);
	if (order == None)		
	{	
		fifoListRemoveFirst();
		request = None;
		goto next;
	}
	switch(order) 
	{
	case X10Address :
        Cm11Status = CM11_WAIT_CHECKSUM;
        setrequest(req);
        lastAddressRequest.H = InvertX10Codes[req.mid(2, 1).toInt(&ok, 16)];
        lastAddressRequest.M = InvertX10Codes[req.mid(3, 1).toInt(&ok, 16)];
		break;
	case X10Function :
		Cm11Status = CM11_WAIT_CHECKSUM;
		setrequest(req);
		break;
	default : 
		request = None;
		Cm11Status = CM11_NOTHING;
		fifoListRemoveFirst();
		break;
	}
}







void x10::setrequest(const QString &req)
{
	bool ok;
	settraffic(Waitingforanswer);
	GenMsg("request : " + req);
	X10Checksum = 0;
	for (int n=0; n<req.length(); n+=2)
	{
		int N = req.mid(n, 2).toInt(&ok, 16);
		if (ok)
		{
			writeTcp((unsigned char)N);
			X10Checksum += N;
		}
		else GenMsg("toInt conversion error");
	}
	X10Checksum &= 0xff;
	TimeOut.start(2000);
	GenMsg(QString("X10 Checksum = %1").arg((unsigned char)X10Checksum));
}





void x10::readX10()
{
 	unsigned char c;
	settraffic(Connected);
	tcpdata.append(tcp.readAll());
	if (tcpdata.length() == 0) return;
	TimeOut.stop();
	QDateTime now = QDateTime::currentDateTime();
	//TcpHeartBeatTimer->stop();
	QString logtxt;
	for (int n=0; n<tcpdata.length(); n++) logtxt += QString("[%1]").arg((unsigned char)tcpdata[n]);
	if (!logtxt.isEmpty()) GenMsg(logtxt);
	c = tcpdata[0];
	int n, fct, deviceindex, dimbrigth;
	switch (Cm11Status)
	{
		case CM11_STATUS_CHECK :
			GenMsg("Status checked");
			Cm11Status = CM11_NOTHING;
			tcpdata.clear();			
		break;

		case CM11_WAIT_READY :
			if (c == CM11_READY)
			{
				CM11Ready();
				emit requestdone();
			}
			else
			{
				if (retry < 3)
				{
					if (c == CM11_POL) polRequest(); 
					else if (c == CM11_INIT)	InitRequest();
					else if (c == CM11_READY) CM11Ready();
					retry ++;
					GenMsg(QString("Ready state not received, retry %1").arg(retry));
					Cm11Status = CM11_NOTHING;
					emit requestdone();
				}
				else
				{
					retry = 0;	
					QString next = fifoListNext();
					GenMsg("Retry aborted : " + next);
					fifoListRemoveFirst();
					Cm11Status = CM11_NOTHING;
					emit requestdone();
				}
			}
			tcpdata.clear();			
		break;
		case CM11_WAIT_CHECKSUM :
			GenMsg(QString("LM11 Checksum = %1").arg((unsigned char)c));
                        if (uchar(X10Checksum) == uchar(c))
			{
				GenMsg("Checksum OK");
				writeTcp(0x00);
				lastAddress = lastAddressRequest;
				Cm11Status = CM11_WAIT_READY;
			}
			else
			{
				Cm11Status = CM11_NOTHING;
				if (retry < 3)
				{
					if (c == CM11_POL) polRequest(); 
					else if (c == CM11_INIT)	InitRequest();
					else if (c == CM11_READY) CM11Ready();
					retry ++;
					GenMsg(QString("Checksum fail, retry %1").arg(retry));
					emit requestdone();
				}
				else
				{
					retry = 0;	
					QString next = fifoListNext();
					GenMsg("Retry aborted : " + next);
					fifoListRemoveFirst();
					emit requestdone();
				}
			}
			tcpdata.clear();
		break;
		case CM11_POL_BYTE :
			if (tcpdata.length() > 2)
			{
				if (tcpdata.length() == tcpdata[0] + 1)
				{
					int mask = 1;
					dimbrigth = 0;
					fct = -1;
					for (n=2; n<tcpdata.length(); n++)
					{
						if ((tcpdata[1] & mask) == 0)
						{ // Address
							if ((fct == 4) or (fct == 5))
							{
								dimbrigth = int((tcpdata[n] & 0xF8) >> 3);
								GenMsg(QString("DimBrigth = %1").arg(dimbrigth));
							}
							else
							{
								lastAddress.H = InvertX10Codes[(tcpdata[n] & 0xF0) >> 4];
								lastAddress.M = InvertX10Codes[tcpdata[n] & 0x0F];
								GenMsg(QString("Address : ") + QChar(lastAddress.H + 65) + QString("%1").arg(lastAddress.M + 1));
							}
						}
						else
						{ // function
							GenMsg("Function : " + getFunctionText(tcpdata[n] & 0x0F)); // + "  House Code : " + QChar(InvertX10Codes[(tcpdata[n] & 0xF0) >> 4].toInt() + 65));
							fct = tcpdata[n] & 0x0F;
							for (deviceindex=0; deviceindex<parent->configwin->devicePtArray.count(); deviceindex++)
								if (parent->configwin->devicePtArray[deviceindex])
									if (parent->configwin->devicePtArray[deviceindex]->isX10Family())
										if (parent->configwin->devicePtArray[deviceindex]->getHouseCode() == lastAddress.H)
										if (parent->configwin->devicePtArray[deviceindex]->getModuleCode() == lastAddress.M)
										{
											switch (fct)
											{
												case 2 : parent->configwin->devicePtArray[deviceindex]->On(false);

													if (lastX10EmitON.secsTo(now) > 5)
													{
														GenMsg(QString("Remote Sleep : %1 seconds").arg(lastX10EmitON.secsTo(now)));
														remoteSleep = 0;
														lastX10EmitON.setDate(now.date());
														lastX10EmitON.setTime(now.time());
													}
													else
													{
														if (remoteSleep != 2)
														{
															GenMsg(QString("Remote Sleep step %1").arg(remoteSleep));
															 remoteSleep ++;
														}
														else
														{
															GenMsg("Remote Sleep");
															parent->configwin->devicePtArray[deviceindex]->Sleep();
														}
													}
												break;
												case 3 : parent->configwin->devicePtArray[deviceindex]->Off(false); break;
												case 4 : parent->configwin->devicePtArray[deviceindex]->Dim(false); break;
												case 5 : parent->configwin->devicePtArray[deviceindex]->Bright(false);
												break;
											}
										parent->SwitchArea->updateStatus();
										}
						}
						mask *= 2;
					}
					Cm11Status = CM11_NOTHING;
					tcpdata.clear();			
				}
				else Cm11Status = CM11_POL_BYTE;
			}
			emit(X10Change());
		break;
		default : 
			if (c == CM11_POL) polRequest(); 
			else if (c == CM11_INIT) 	InitRequest();
			else if (c == CM11_READY) CM11Ready();
			else Cm11Status = CM11_NOTHING;				
			tcpdata.clear();			
		break;
	}
	if ( tcpdata.length() > 0)
	{
		if (c == CM11_POL) polRequest();
		else if (c == CM11_INIT) InitRequest();
		else if (c == CM11_READY) CM11Ready();
	}
	//TcpHeartBeatTimer->start(10000);
}





void x10::polRequest()
{
	GenMsg("CM11 Poll signal");
	writeTcp(CM11_POL_ACK);
	tcpdata.clear();			
	Cm11Status = CM11_POL_BYTE;		
}




void x10::InitRequest()
{
	GenMsg("CM11 initialisation");
	writeTcp(0x9B);
	writeTcp(0x00);
	writeTcp(0x00);
	writeTcp(0x00);
	writeTcp(0x00);
	writeTcp(0x00);
	writeTcp(0x60);
	lastAddress.H = 0;
	lastAddress.M = 0;
	tcpdata.clear();			
	Cm11Status = CM11_NOTHING;				
}



void x10::CM11Ready()
{
	retry = 0;
	fifoListRemoveFirst();
	GenMsg("CM11 Ready");
	tcpdata.clear();			
	Cm11Status = CM11_NOTHING;
}





QString x10::getFunctionText(int function)
{
	switch (function)
	{
                case 0 : return (tr("All units Off"));
                case 1 : return (tr("All Light On"));
                case 2 : return (tr("On"));
                case 3 : return (tr("Off"));
                case 4 : return (tr("Dim"));
                case 5 : return (tr("Brigth"));
                case 6 : return (tr("All ligth Off"));
                case 7 : return (tr("Extended"));
                case 8 : return (tr("Hail Request"));
                case 9 : return (tr("Hail Ack"));
                case 13 : return (tr("Status On"));
                case 14 : return (tr("Status Off"));
                case 15 : return (tr("Status Request"));
		default : return (tr("Unkown"));
	}
}



void x10::SendX10On()
{
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	QString Code = "0" + X10_FUNC + X10CodesStr[ComboHouseCode.currentIndex()] + X10_ON;
	QString empty = "";
	addtofifo(X10Function, empty, Code);
}







void x10::NewModule()
{
	bool ok;
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	bool paletteHidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	retry:
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if ((!ok) || nom.isEmpty())
	{
		parent->PaletteHide(paletteHidden);
		return;
	}
	if (parent->configwin->devicenameexist(nom))
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto retry;
	}
	QStringList X10type;
	X10type << familyAM12 << familyLM12;
    QString DevType = inputDialog::getItemPalette(this, tr("Module Type"), tr("Module Type : "), X10type, 0, false, &ok, parent);
	if ((!ok) || DevType.isEmpty())
	{
		parent->PaletteHide(paletteHidden);
		return;
	}
	onewiredevice * device;
	for (int id=1; id<999; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + QString("%1").arg(id, 3, 10, QChar('0')) + DevType);
		if (!parent->configwin->deviceexist(RomID))
		{
			device = parent->configwin->NewDevice(RomID, this);
			if (device)
			{
				device->setname(nom);
				int H  = ComboHouseCode.currentIndex();
				int M  = ComboDeviceCode.currentIndex();
				device->setHouseCode(H, M);
				if (!localdevice.contains(device)) localdevice.append(device);
				UpdateLocalDeviceList();
				device->show();
				parent->PaletteHide(paletteHidden);
				return;
			}
		}
	}
	if (messageBox::questionHide(this, tr("Problem ?"), tr("Unit creation impossible\nRetry ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes) goto retry;
	parent->PaletteHide(paletteHidden);
}





void x10::readConfigFile(QString &configdata)
{
	QString ReadRomID, MasterIP, H, M;
	onewiredevice * device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	MasterIP = logisdom::getvalue("Master", strsearch);
	if (ReadRomID.left(8) == ip2Hex(moduleipaddress).toUpper()) 
		if (!parent->configwin->deviceexist(ReadRomID))
		{
			device = parent->configwin->NewDevice(ReadRomID, this);
			if (device)
				if (!localdevice.contains(device)) localdevice.append(device);
			UpdateLocalDeviceList();
		}
	SearchLoopEnd;
}




void x10::SendX10Dim()
{
	bool ok;
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
        int code = X10_FUNC.toInt(&ok, 16) + 8 * (int((X10Power.value() / 99 * 22)));
	QString Code = QString("%1").arg(code, 2, 16, QChar('0')).toUpper() + X10CodesStr[ComboHouseCode.currentIndex()] + X10_DIM;
	QString empty = "";
	addtofifo(X10Function, empty, Code);
}








void x10::SendX10Off()
{
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	QString Code = "0" + X10_FUNC + X10CodesStr[ComboHouseCode.currentIndex()] + X10_OFF;
	QString empty = "";
	addtofifo(X10Function, empty, Code);
}








void x10::X10HeartBeat()
{
	Cm11Status = CM11_NOTHING;	
	QString code = "0" + X10_ADDR + X10CodesStr[lastAddress.H] + X10CodesStr[lastAddress.M];
	QString empty = "";
	if (fifoListCount() == 0) addtofifo(X10Address, empty, code); else fifonext();
}






void x10::CM11_timeout()
{
//	if (tcp->bytesAvailable() > 0)
//	{
//		readX10();
//		return;
//	}
	Cm11Status = CM11_NOTHING;				
	if (retry < 3)
	{
		retry ++;
		GenMsg(QString("CM11 timed out, retry %1").arg(retry));
		emit requestdone();
	}
	else
	{
		retry = 0;	
		QString next = fifoListNext();
		GenMsg("Retry aborted : " + next);
		fifoListRemoveFirst();
		emit requestdone();
	}
}







void x10::SendX10Bright()
{
	bool ok;
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	int code = X10_FUNC.toInt(&ok, 16) + 8 * ((int)(X10Power.value() / 99 * 22));
	QString Code = QString("%1").arg(code, 2, 16, QChar('0')).toUpper() + X10CodesStr[ComboHouseCode.currentIndex()] + X10_BRIGHT;
	QString empty = "";
	addtofifo(X10Function, empty, Code);
}






void x10::SendX10Address()
{
	if (ComboHouseCode.currentIndex() == -1) return;
	if (ComboDeviceCode.currentIndex() == -1) return;
	QString Code = "0" + X10_ADDR + X10CodesStr[ComboHouseCode.currentIndex()] + X10CodesStr[ComboDeviceCode.currentIndex()];
	QString empty = "";
	addtofifo(X10Address, empty, Code);
}





void x10::delayfifonextX10()
{
	QTimer::singleShot(500, this, SLOT(fifonext()));
}




