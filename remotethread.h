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



#ifndef REMOTETHREAD_H
#define REMOTETHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>
#include "logisdom.h"
#include "tcpdata.h"

class remotethread : public QThread
{
    Q_OBJECT
struct FIFOStruc
	{
		QString Request;
		int Request_ID;
	};
public:
	remotethread(logisdom *Parent);
	logisdom *parent;
	void run();
	QString moduleipaddress;
    quint16 port;
	QString log;
    QString trace;
	bool endLessLoop;
	QString UserName, PassWord;
	bool passWordDone;
	bool logEnabled;
	void addToFIFOSpecial(int Request_ID, const QString &data);
	void addToFIFOSpecial(const QString &data);
	void addToFIFOSpecial(int Request_ID);
	QAbstractSocket::SocketState tcpStatus;
	QStringList FIFOs;
private:
	QMutex mutexData;
	bool Admin;
	QList <FIFOStruc*> FIFOSpecial;
	void get(QTcpSocket &Socket, QString &Request, tcpData &data);
    void checkUserName(tcpData &data);
    void checkPassWord(tcpData &data);
	void checkGetConfigFile(tcpData &data);
	void checkGetDevicesConfig(tcpData &data);
	void checkGetMainValue(tcpData &data);
	void checkGetFIFOSpecial(tcpData &data);
	void checkGetFile(tcpData &data);
	void logFile(QString txt);
	void getFifoString(QString &str);
signals:
    void configReady(const QString&);
    void deviceConfigReady(const QString&);
    void getMainValueReady(const QString&);
	void reloadGrahp();
	void tcpStatusChange();
    void traceUpdate(const QString&);
public slots:

};

#endif // REMOTETHREAD_H
