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



#ifndef HA7NETTHREAD_H
#define HA7NETTHREAD_H

#include <QtCore>
#include <QThread>

class ha7netthread : public QThread
{
    Q_OBJECT
public:
struct device
{
	QString RomID;
	QString Scratchpad;
	QString Scratchpad_1;
	QString Return;
	int SearchLevel;
	bool isValid;
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
public:
	ha7netthread();
	~ha7netthread();
	void run();
	QString moduleipaddress;
	QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
	int port;
    QString log, public_Log;
	QStringList log_List;
    void getLog(QString &str);
	bool GlobalConvert;
	bool logEnabled;
	bool endLessLoop;
	QMutex mutexData;
	int searchDelay;
	int fileIndex;
	QDateTime lastSearch;
	void addToFIFOSpecial(const QString &RomID, const QString &data, int Request_ID = 0);
	void addToFIFOSpecial(const QString &data);
	QList <device*> devices;
private:
	QString LockID;
	void logThis(const QString &str);
	void SearchAnalysis(const QString &data);
	int checkDevice(const QString RomID);
	void get(QString &Request, QString &Data);
	void FIFOAppend(int reqID, const QString &reqStr, int device_ID = -1, int scratchpad_ID = 0);
	void FIFOSpecialAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID = 0);
	void addDeviceReadings(bool ALL = true);
	QList <FIFOStruc*> FIFO;
	QList <FIFOStruc*> FIFOSpecial;
	int getConvertTime(device *dev);
	void readFIFOBuffer(const QString &htmlData);
	void readFIFOSpecialBuffer(const QString &htmlData);
	void processFIFO(bool ALL = true);
	void processFIFOSpecial();
	void saveLog();
signals:
    void newDevice(const QString&);
    void deviceReturn(const QString&, const QString&);
    void tcpStatusUpdate(const QString&);
private slots:
};

#endif // HA7NETTHREAD_H
