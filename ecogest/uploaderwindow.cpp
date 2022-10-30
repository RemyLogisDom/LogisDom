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
#include "uploaderwindow.h"
#include <QtGui>
#include <QtWidgets/QFileDialog>
#include "logisdom.h"
#include "errlog.h"
#include "ecogest.h"
#include "messagebox.h"
#include "alarmwarn.h"





uploaderwindow::uploaderwindow(ecogest *Parent)
{
	parent = Parent;
	ui.setupUi(this);
	TimeOut = new QTimer(this);
	tcp = new QTcpSocket(this);
	retry = 0;
	isRunning = false;
	connect(tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(tcp, SIGNAL(readyRead()), this, SLOT(readbuffer()));
	connect(TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
	status = standby;
	connect(ui.pushButtonGo, SIGNAL(clicked()), this, SLOT(go()));
	connect(ui.pushButtonForce, SIGNAL(clicked()), this, SLOT(force()));
	connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(stop()));
	ui.pushButtonStop->setEnabled(false);
	settraffic(Unused);
}




uploaderwindow::~uploaderwindow()
{
	disconnect(ui.pushButtonGo, SIGNAL(clicked()), this, SLOT(go()));
	disconnect(ui.pushButtonForce, SIGNAL(clicked()), this, SLOT(force()));
	disconnect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(stop()));
}





void uploaderwindow::displayUiTxt(const QString &text)
{
	QMutexLocker locker(&txtMutex);
	ui.textEdit->append(text);
}






void uploaderwindow::setipaddress(const QString &adr)
{
	moduleipaddress = adr;
	displayUiTxt("IP = " + adr);
}





void uploaderwindow::setport(int Port)
{
	port = Port;
	displayUiTxt(QString("Port = %1").arg(Port));
}




void uploaderwindow::go()
{
	if (isRunning) return;
	parent->uploadStarted = true;
	isRunning = true;
	ui.pushButtonGo->setEnabled(false);
	ui.pushButtonForce->setEnabled(false);
	ui.pushButtonStop->setEnabled(true);
	displayUiTxt(tr("Upload process starts request"));
	if (status != standby)
	{
		displayUiTxt(tr("Status in not in standby, request ignored"));
		return;
	}
	status = restart;
	connecttcp();
}




void uploaderwindow::force()
{
	isRunning = true;
	parent->uploadStarted = true;
	ui.pushButtonGo->setEnabled(false);
	ui.pushButtonForce->setEnabled(false);
	ui.pushButtonStop->setEnabled(true);
	displayUiTxt(tr("Upload process force start request"));
	if (status != standby)
	{
		displayUiTxt(tr("Status in not in standby, request ignored"));
		return;
	}
	status = norestart;
	connecttcp();
}





void uploaderwindow::stop()
{
	parent->uploadStarted = false;
	TimeOut->stop();
	displayUiTxt(tr("Upload process aborted"));
	status = standby;
	tcp->disconnectFromHost();
	settraffic(Disconnected);
	isRunning = false;
	ui.pushButtonGo->setEnabled(true);
	ui.pushButtonForce->setEnabled(true);
	ui.pushButtonStop->setEnabled(false);
}




void uploaderwindow::connecttcp()
{
	settraffic(Connecting);
	tcp->abort();
	tcp->connectToHost(moduleipaddress, port);
	displayUiTxt(tr("Try Tcp connection"));
}




void uploaderwindow::tcpconnected()
{
	QString req;
	memoryPtr = 0;
	configPtr = 3;
	QString str = loaddata();
	bool ok = false;
	if (str == "Load data ok") ok = true;
	if (!ok)
	{
		messageBox::warningHide(this, tr("Firmware upload"), tr("Error in file") + " : " + str, maison1wirewindow, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		stop();
		return;
	}
// get version
	QByteArray data;
	for (int n=0; n<picMemorySize; n++) data.append(picMemory[n]);
	QString version = logisdom::getvalue("LogisDom Controler Version", data);
	if (version.isEmpty())
	{
		messageBox::warningHide(this, tr("Process aborted"), tr("File version could not be found"), maison1wirewindow, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		stop();
		return;
	}
	QString txt = tr("File") + " " + fileNameValid + " " + tr("has been checked and is valid") + "\r\n" + tr("The process will take few minutes");
	txt += "\r\n" + tr("Network connection must not be interrupted during the upload");
	txt += "\r\n" + tr("File version is") + " : " + version;
	txt += "\r\n" + tr("Do you really want to start uploading the firmware ?");
	if ((messageBox::questionHide(this, tr("Confirm upload ?"), txt, maison1wirewindow, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes))
	{
		stop();
		return;
	}
	displayUiTxt(str);
	switch (status)
	{
		case restart :
			retry = 0;
			displayUiTxt(tr("Tcp Connection successfull"));
			displayUiTxt(tr("Send Reboot command"));
			req = "Reboot\r";
			for (int n=0 ; n<req.length(); n++) tcp->putChar(req[n].unicode());
	//		settraffic(Connected);
			displayUiTxt(tr("Wait for S"));
			status = wait_forS;
			TimeOut->start(1000);
		break;
		case norestart :
			retry = 0;
			displayUiTxt(tr("Tcp Connection successfull"));
			displayUiTxt(tr("Send C1"));
			tcp->putChar(pic_ID);
			displayUiTxt(tr("Wait for Pic ID"));
	//		settraffic(Connected);
			status = WaitPicID;
			TimeOut->start(200);
		break;
	}
}




void uploaderwindow::tcpdisconnected()
{
	settraffic(Disconnected);
	parent->GenError(81, tcp->errorString() + " " + moduleipaddress);
}






void uploaderwindow::tcpConnectionError(QAbstractSocket::SocketError)
{
	settraffic(Disconnected);
	parent->GenError(80, tcp->errorString() + " " + moduleipaddress);
}





void uploaderwindow::settraffic(int state)
{
	QMutexLocker locker(&txtMutex);
	switch (state)
	{
		case Connecting :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/stop.png")));
			ui.labelTrafficPix->setToolTip(tr("Connecting"));
			ui.labelTrafficTxt->setText(tr("Connecting"));
		break;
		
		case Disconnected :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/stop.png")));
			ui.labelTrafficPix->setToolTip(tr("Disconnected"));
			ui.labelTrafficTxt->setText(tr("Disconnected"));
		break;

		case Connected :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/refresh.png")));
			ui.labelTrafficPix->setToolTip(tr("Connected"));
			ui.labelTrafficTxt->setText(tr("Connected"));
		break;

		case Retry :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/pause.png")));
			ui.labelTrafficPix->setToolTip(tr("Retry"));
			ui.labelTrafficTxt->setText(tr("Retry"));
		break;

		case Finished :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/cpu_preferences.png")));
			ui.labelTrafficPix->setToolTip(tr("Finished"));
			ui.labelTrafficTxt->setText(tr("Finished"));
		break;

		case Failed :
			ui.labelTrafficPix->setPixmap(QPixmap(QString::fromUtf8(":/images/images/cpu_stop.png")));
			ui.labelTrafficPix->setToolTip(tr("Failed"));
			ui.labelTrafficTxt->setText(tr("Failed"));
		break;

		case Unused :
		default :
			ui.labelTrafficPix->setPixmap(QPixmap(""));
			ui.labelTrafficPix->setToolTip(tr(""));
			ui.labelTrafficTxt->setText(tr(""));
		break;
	}
}






void uploaderwindow::readbuffer()
{
	unsigned char c;
	TimeOut->stop();
	settraffic(Connected);
	QByteArray tcpdata;
	tcpdata.clear();
	tcpdata.append(tcp->readAll());
	if (tcpdata.length() == 0) return;
	QString logtxt;
//	if (ui.checkBoxLog->isChecked())
//	{
//		for (int n=0; n<tcpdata.length(); n++) logtxt += QString("[%1]").arg((unsigned char)tcpdata[n]);
//		displayUiTxt(logtxt);
//	}
	c = tcpdata[0];
	switch (status)
	{
		case WaitPicID :
			if (c == 79)
			{
				displayUiTxt("Pic identified");
				status = wait_forK;
				if (tcpdata.endsWith('K')) sendnextdata();
			}
			return;
		case wait_forK :
			if (c == 'K')
			{
				if (memoryPtr < 0xFF01)
				{
					displayUiTxt(QString("%1 : ok").arg(memoryPtr - 64, 4, 16, QChar('0')).toUpper());
				}
				sendnextdata();
			}
			else if (c == 'N')
			{
				displayUiTxt("Checksum error");
				memoryPtr -=64;
				sendnextdata();
			}
			return;
		case wait_forS :
			if (c == 'S')
			{
				displayUiTxt(tr("Send C1"));
				tcp->putChar(pic_ID);
				displayUiTxt(tr("Wait for Pic ID"));
				settraffic(Connected);
				status = WaitPicID;
			}
			return;
		default: 	parent->GenError(48, QString("Status %1 not defined : ").arg(status));
		break;
	}
}







QString uploaderwindow::loaddata()
{
	QString line;
	int _32bitAddress = 0;
	QFile file;
	QString getFileName;
	file.setFileName(QString("..") + QDir::separator() + QString("..") + QDir::separator() + QString("MultiGest Dev 18F") + QDir::separator() + "src" + QDir::separator() + fileName);
	if (!file.exists()) file.setFileName(fileName);
	if (!file.exists())
	{
		getFileName = QFileDialog::getOpenFileName(this, tr("Select hex file "), "", "hex (*.hex)");
		if (getFileName.isEmpty()) return "Canceled by operator";
		file.setFileName(getFileName);
	}
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		parent->GenError(48, "Cannot open hex file  " + file.fileName());
		return QString("Cannot open hex file  " + file.fileName());
	}
	else
	{
		displayUiTxt("File " +  file.fileName() + " opened");
		fileNameValid = file.fileName();
		QTextStream in(&file);
		senddata.clear();
		// clear picMemory
		for (int n=0; n<picMemorySize; n++) picMemory[n] = 0xFF;
		for (int n=0; n<picConfigMemorySize; n++) picConfigMemory[n] = 0xFF;
		while (!in.atEnd())
		{
			// read file line
			QString line = in.readLine();
			senddata.append(line);
			//displayUiTxt("Read line " + line);
			// verify checksum
			bool ok;
            uchar checksum = uchar(line.right(2).toUShort(&ok, 16));
			if (!ok) return QString("checksum toInt Error : " + line);
			line.chop(2);	// remove checksum string
			// compute checksum
            uchar NumberOfData = uchar(line.mid(1, 2).toUShort(&ok, 16));
			if (!ok) return QString("NumberOfData toInt Error : " + line);
			line.remove(0, 3);
			unsigned char CS = NumberOfData;
			QStringList data;
			while (!line.isEmpty())
			{
				QString s = line.left(2);
				data.append(s);
                uchar c = uchar(s.toUShort(&ok, 16));
				if (!ok) return QString("Data toInt Error : " + s);
				CS += c;
				line.remove(0, 2);
			}
			CS = -CS;
			//displayUiTxt(QString("Checksum calculated :  %1").arg((short)CS, 2, 16, QChar('0')));
			//if (checksum == CS) displayUiTxt("Line checksum ok");
			if (checksum != CS) return QString("Line checksum fail");
			//displayUiTxt(QString("Checksum calculated :  %1").arg((short)CS, 2, 16, QChar('0')));
			if (NumberOfData + 3 < data.count()) return QString("Missing data");
			if (NumberOfData + 3 > data.count()) return QString("Extra data");
			QString adressStr = data[0] + data[1];
			int dataType = data[2].toInt(&ok, 16);
			if (!ok) return QString("dataType toInt Error : " + adressStr);
			int address = adressStr.toInt(&ok, 16);
			if (!ok) return QString("Address toInt Error : " + adressStr);
			QString _32bitAddressStr;
			QString str;
			switch (dataType)
			{
				case 0 :	// data record
					if (_32bitAddress == 0)	// Program memory space
					{
						if (address >= picMemorySize - NumberOfData) return QString("Address out of range : " + adressStr);
							for (int n=0; n<NumberOfData; n++)
							{
                                picMemory[address + n] = uchar(data[n + 3].toUShort(&ok, 16));
								if (!ok) return QString("Data toInt Error : " + adressStr);
							}
					}
					else if (_32bitAddress == 0x30)
					{
						if (address == 0)	// Configuration bit memory space
						{
							for (int n=0; n<NumberOfData; n++)
							{
								if (n<picConfigMemorySize)
								{
                                    picConfigMemory[n] = uchar(data[n + 3].toUShort(&ok, 16));
									if (!ok) return QString("DataConfig toInt Error : " + adressStr);
								}
							}
						}
					}
				break;
				case 1 : // End of file
					// place boot vector at 0x0000 to bootloader exit jump (7FA0 - 4) * 2 = 0xFF38
					for (int n=0; n<4; n++) picMemory[0xFF38 + n] = picMemory[n];
					// place boot vector at 0x0000 to bootloader begin 0x7FF0
					picMemory[0] = 0xA0;
					picMemory[1] = 0xEF;
					picMemory[2] = 0x7F;
					picMemory[3] = 0xF0;
					if (ui.checkBoxLog->isChecked())
					{
						displayUiTxt("End of File");
						for (int n=0; n<picMemorySize/16; n++)
						{
							str = "";
                            for (int d=0; d<16; d++) str += QString("%1").arg(ushort(picMemory[n*16 + d]), 2, 16, QChar('0'));
                            displayUiTxt((QString("%1:").arg(int(n*16), 4, 16, QChar('0')) + str).toUpper());
						}
						displayUiTxt("Config bit memory");
						str = "";
                        for (int d=0; d<16; d++) str += QString("%1").arg(ushort(picConfigMemory[d]), 2, 16, QChar('0'));
						displayUiTxt(str.toUpper());
					}
				break;
				case 2 : // Extended Segment Address Record
				break;
				case 3 : // Start Segment Address Record (Only for 80x86)
				break;
				case 4 : // Extended Linear Address Record
					if (NumberOfData != 2) return QString("Unsupported NumberOfData for dataType 4");
					_32bitAddressStr = data[3] + data[4];
					_32bitAddress = _32bitAddressStr.toInt(&ok, 16);
					if (!ok) return QString("_32bitAddress toInt Error : " + _32bitAddressStr);
				break;
				case 5 : // Start Linear Address Record
				break;
			}
		}
		file.close();
		ui.progressBar->setMinimum(0);
		ui.progressBar->setMaximum(picMemorySize);
		ProgessMax = senddata.count();
		ui.progressBar->setValue(0);
	}
	return "Load data ok";
}






void uploaderwindow::timeout()
{
	switch (status)
	{
		case WaitPicID :
        tcp->putChar(char(pic_ID));
		displayUiTxt("Time Out WaitPicID");
			break;
		case wait_forK :
			displayUiTxt("Time Out wait_forK");
			break;
		case wait_forS :
            tcp->putChar(char(pic_ID));
			displayUiTxt("Time Out wait_forS");
			break;
		default: ;
	}
	if (retry <20)
	{
		retry ++;
		settraffic(Retry);
		TimeOut->start(200);
	}
	else
	{
		finished("Time out, upload failed", false);
	}
}





void uploaderwindow::sendnextdata()
{
	bool log = ui.checkBoxLog->isChecked();
//	int keepAlive = 0;
	if (memoryPtr < 0xFF01)	// Program memory space
	{
//		bool noffff = false;
//		for (int d=0; d<64; d++)
//		{
//			if (picMemory[memoryPtr + d] != 0xFF)
//			{
//				 noffff = true;
//			}
//			else
//			{
//				keepAlive++;
//				if (keepAlive < 10) memoryPtr+=64; else noffff = true;
//			}
//		}
//		if (noffff)
		{
//			keepAlive = 0;
			QString str = "\n";
			if (log)
			{
				for (int n=0; n<4; n++)
				{
                    str += (QString("%1:").arg(int(memoryPtr + n*16), 4, 16, QChar('0'))).toUpper();
					for (int d=0; d<16; d++)
                        str += QString("%1").arg(ushort(picMemory[memoryPtr + d + n*16]), 2, 16, QChar('0')).toUpper();
					str += "\n";
				}
				displayUiTxt(str);
				str = "";
			}
			unsigned char Checksum = 0;
            unsigned char high = uchar(memoryPtr >> 8);
            unsigned char low = uchar(memoryPtr & 0x00FF);
			Checksum += high;
			Checksum += low;
			Checksum += 64;
			tcp->putChar(0x00);		// upper byte of address
            tcp->putChar(char(high));		// high byte of address
            tcp->putChar(char(low));		// low byte of address
			tcp->putChar(64);		// number of data
			if (log)
			{
                str += QString("%1").arg(ushort(0), 2, 16, QChar('0')).toUpper();
                str += QString("%1").arg(ushort(high), 2, 16, QChar('0')).toUpper();
                str += QString("%1").arg(ushort(low), 2, 16, QChar('0')).toUpper();
                str += QString("%1").arg(ushort(64), 2, 16, QChar('0')).toUpper();
			}
			for (int n=0; n<64; n++)
			{
				Checksum += picMemory[memoryPtr];
                tcp->putChar(char(picMemory[memoryPtr]));
                if (log) str += QString("%1").arg(ushort(picMemory[memoryPtr]), 2, 16, QChar('0')).toUpper();
				memoryPtr++;
			}
            if (log) str += QString("%1").arg(ushort(Checksum), 2, 16, QChar('0')).toUpper();
			tcp->putChar(-Checksum);
			if (log) displayUiTxt("send : " + str);
		}
//		else memoryPtr += 64;
		ui.progressBar->setValue(memoryPtr);
	}
	else if (configPtr < picConfigMemorySize)
	{
		switch (configPtr)
		{
			case 0x1 : sendConfigByte(configPtr++); break;
			case 0x2 : sendConfigByte(configPtr++); break;
			case 0x3 : sendConfigByte(configPtr++); break;	// 300003h CONFIG2H    WDTPS3 WDTPS2 WDTPS1 WDTPS0 WDTEN
			case 0x4 : finished("Upload done", true); break;
			case 0x5 : //configPtr++; break;
			case 0x6 : //configPtr++; break;
			case 0x7 : //configPtr++; break;
			case 0x8 : //configPtr++; break;
			case 0x9 : //configPtr++; break;
			case 0xA : //configPtr++; break;
			case 0xB : //configPtr++; break;
			case 0xC : //configPtr++; break;
			case 0xD : //configPtr++; break;
			default: finished("Upload done", true); break;
		}
	}
}






void uploaderwindow::sendConfigByte(int adr)
{
	bool log = ui.checkBoxLog->isChecked();
	QString str = "\n";
	unsigned char Checksum = 0;

    tcp->putChar(char(0xB0));		// 30h
	Checksum += 0xB0;

	tcp->putChar(0);			// high byte of address

    tcp->putChar(char(adr));		// low byte of address
	Checksum += adr;

	tcp->putChar(1);			// CFG
	Checksum += 1;

    tcp->putChar(char(picConfigMemory[adr]));	// data
	Checksum += picConfigMemory[adr];

	tcp->putChar(-Checksum);		// checksum

    if (log) str += QString("Config %1 : ").arg(ushort(adr), 2, 16, QChar('0')).toUpper();
    if (log) str += QString("%1").arg(ushort(picConfigMemory[adr]), 2, 16, QChar('0')).toUpper();
	if (log) displayUiTxt("send : " + str);
}






void uploaderwindow::finished(QString msg, bool closeMe)
{
	TimeOut->stop();
	displayUiTxt(msg);
	displayUiTxt(tr("Upload process finished"));
	status = standby;
	tcp->disconnectFromHost();
	if (closeMe) settraffic(Finished); else  settraffic(Failed);
	isRunning = false;
	ui.progressBar->setValue(picMemorySize);
	ui.pushButtonGo->setEnabled(true);
	ui.pushButtonForce->setEnabled(false);
	ui.pushButtonStop->setEnabled(false);
}


