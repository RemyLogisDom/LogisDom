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



#ifndef TELEINFOTHREAD_H
#define TELEINFOTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>


class teleinfothread : public QThread
{
	Q_OBJECT
public:
static const char CR = 0x0D;		// Frame End
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
public:
    teleinfothread();
    ~teleinfothread();
    void run();
    bool isRunning = false;
	QString moduleipaddress;
	int port;
    QString log, logfile;
    QTcpSocket *socket;
	bool logEnabled;
	bool endLessLoop;
	QAbstractSocket::SocketState tcpStatus;
private:
	void TCPconnect(QTcpSocket &Socket, QString &Status);
signals:
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
private slots:
};

#endif // TELEINFOTHREAD_H
