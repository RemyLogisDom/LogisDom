/****************************************************************************
**
** Copyright (C) 2018 Remy CARISIO.
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



#ifndef MASTERTHREAD_H
#define MASTERTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>

class masterthread : public QThread
{
    Q_OBJECT
public:
struct device
{
	QString RomID;
	QString channel;
	QString localName;
	QString Scratchpad;
	QString Scratchpad_1;
	QString Return;
};
private:
struct FIFOStruc
{
	QString RomID;
	QString Request;
	int Request_ID;
	int device_ID;
	int scratchpad_ID;
};
enum EcoGestDetectors
	{
		TankLow = 0, TankHigh, HeaterIn, HeaterOut, HeatingOut, SolarIn, SolarOut, LastDetector,
		devServo1, devServo2, devServo3, devServo4, devServo5, devFlowA, devFlowB, devFlowC, devInterlock,
		devSwitchA, devSwitchB, devSwitchC, devSwitchD, devSwitchE, devSwitchF, devFluidicMode, devSolarMode, allDetector
	};
public:
	masterthread();
	~masterthread();
	void run();
    QString log, logfile;
    QTcpSocket *socket;
	QString moduleipaddress;
	QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
	int port;
	bool GlobalConvert;
	bool logEnabled;
	bool endLessLoop;
	int modeProcess;
	QAbstractSocket::SocketState tcpStatus;
    QMutex mutexData;
	void addToFIFOSpecial(const QString &RomID, const QString &data, int Request_ID = 0);
	void addToFIFOSpecial(const QString &data);
    QString lastFIFOAdd;
	QList <device*> devices;
	int searchDelay;
	int fileIndex;
	QDateTime lastSearch;
private:
	void removeDuplicates();
	QString getStr(int index);
    void TCPconnect(QTcpSocket &Socket, const QString &Status);
	void GlobalSearchAnalysis(const QString &data);
	void LocalSearchAnalysis(const QString &data);
	void GetScratchPadsAnalysis(const QString &data);
	void GetStatusAnalysis(const QString &data);
	void FIFOAppend(int reqID, const QString &reqStr, int device_ID = 0, int scratchpad_ID = 0);
	void FIFOSpecialAppend(int reqID, const QString &reqStr, const QString &RomID, int device_ID, int scratchpad_ID = 0);
	void addDeviceReadings(bool ALL = true);
	QList <FIFOStruc*> FIFO;
	QList <FIFOStruc*> FIFOSpecial;
	QString extractBuffer(QTcpSocket &socket);
	void checkSendData(QTcpSocket &socket);
	void processFIFOSpecial(QTcpSocket &socket);
	void processFIFO(QTcpSocket &socket, bool All = true);
	void checkSearch();
	void saveLog();
	QString Buffer;
	int getConvertTime(device *dev);
	void readFIFOBuffer(const QString &data);
	void readFIFOSpecialBuffer(const QString &data);
signals:
    void newDevice(const QString&, const QString&);
    void deviceReturn(const QString&, const QString&);
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
    void newStatus(const QString&);
	void modeChanged(int);
private slots:
};

#endif // MASTERTHREAD_H
