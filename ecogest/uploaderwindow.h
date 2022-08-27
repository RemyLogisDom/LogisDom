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



#ifndef UPLOADERWINDOW_H
#define UPLOADERWINDOW_H
#include <QtNetwork>
#include "logisdom.h"
#include "ui_firmware.h"

class ecogest;


const uchar pic_ID = 0xC1;

class uploaderwindow : public QDialog
{
	Q_OBJECT
#define TIMEOUT 1000
#define bufferSize 100
#define adressUnset 0xFFFF
#define picMemorySize 0x10000
#define picConfigMemorySize 0x10
enum pic_status
{
	standby, restart, norestart, WaitPicID, wait_forK, wait_forS
};
public:
enum NetTraffic
{
	Unused, Connecting, Disconnected, Retry, Connected, Finished, Failed
};
	QString fileName;
	void setipaddress(const QString &adr);
	void setport(int Port);
	QString moduleipaddress;
	int port;
	void connecttcp();
private slots:
	void readbuffer();
	void timeout();
	void tcpconnected();
	void tcpdisconnected();
	void tcpConnectionError(QAbstractSocket::SocketError socketError);
public slots:
	void force();
	void go();
	void stop();
private:
    uchar picMemory[picMemorySize];
	int memoryPtr, configPtr;
	unsigned char picConfigMemory[picConfigMemorySize];
	void sendConfigByte(int adr);
	bool isRunning;
	QTimer *TimeOut;
	QTcpSocket *tcp;
	int ProgessMax;
	QStringList senddata;
	void settraffic(int state);
	void sendnextdata();
	void finished(QString msg, bool closeMe);
	QString loaddata();
	QString fileNameValid;
	int status, retry;
	ecogest *parent;
public:
	Ui::firmware ui;
	QMutex txtMutex;
	uploaderwindow(ecogest *Parent);
	~uploaderwindow();
private slots:
	void displayUiTxt(const QString &text);
};



#endif




