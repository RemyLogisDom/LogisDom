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



#ifndef REMOTE_H
#define REMOTE_H
#include "tcpdata.h"
#include "remotethread.h"
#include "net1wire.h"



class remote : public net1wire
{
	Q_OBJECT
public:
	remote(logisdom *Parent);
	void init(QString userName, QString password);
	void add2fifo(const QString &order, bool priority = false);
	void add2fifo(int order, bool priority = false);
	void addtofifo(int order, const QString &Data);
	QString getUserName();
	QString getPassWord();
	bool isAdmin();
private slots:
	void saveMainValue();
	void logEnabledChanged(int);
    void configReady(const QString&);
    void deviceConfigReady(const QString&);
    void traceUpdate(const QString&);
    void getMainValueReady(const QString&);
private:
	tcpData Data;
	QString UserName, PassWord;
	bool Admin, saveRequest;
	remotethread *TcpThread;
	void switchOnOff(bool state);
public slots:
	void addGetFiletoFifo(QString name, QString folder = "");
	void addGetDatFiletoFifo(QString name);
	void addCommandtoFifo(QString command);
};

#endif
