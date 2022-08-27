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



#ifdef Q_OS_WIN32
#include "windows.h"
#include "Winsock2.h"
#endif



#include <QElapsedTimer>
#include <QTimer>
#include "net1wire.h"
#include "teleinfothread.h"
#include "globalvar.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif



teleinfothread::teleinfothread()
{
	logEnabled = false;
	endLessLoop = true;
    socket = nullptr;
 }




teleinfothread::~teleinfothread()
{
    endLessLoop = false;
    if (socket) socket->abort();
}


void teleinfothread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QTcpSocket MySocket;
    socket = &MySocket;
    qintptr sd = MySocket.socketDescriptor();
#ifdef Q_OS_LINUX
    int set = 1;
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
//#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
//#endif
    isRunning = true;
    QString msg = "Thread starts, first connection";
    TCPconnect(MySocket, msg);
    while (endLessLoop)
    {
        log.clear();
        sleep(1);
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
    isRunning = false;
}





void teleinfothread::TCPconnect(QTcpSocket &Socket, QString &Status)
{
    log += addTimeTag "TCP Connect  " + Status;
	int tryConnect = 1;
	do
	{
        log += addTimeTag QString("TCP try connect %1").arg(tryConnect);
		if (Socket.state() == QAbstractSocket::ConnectedState)
		{
			Socket.disconnectFromHost();
            log += addTimeTag "Try Disconnect";
            if (Socket.state() == QAbstractSocket::ConnectedState) Socket.waitForDisconnected();
		}
		if (Socket.state() == QAbstractSocket::UnconnectedState)
		{
            log += addTimeTag "Disconnect OK";
		}
		tcpStatus = Socket.state();
		emit(tcpStatusChange());
        if (endLessLoop)
        {
            Socket.connectToHost(moduleipaddress, port);
            log += addTimeTag "Try Connect";
            Socket.waitForConnected();
            if (Socket.state() == QAbstractSocket::ConnectedState)
            {
                log += addTimeTag "Connect OK";
            }
            tcpStatus = Socket.state();
            emit(tcpStatusChange());
            tryConnect ++;
        }
        if (tryConnect > 10)
        {
            log.clear();
            tryConnect = 0;
        }
    }
    while ((Socket.state() != QAbstractSocket::ConnectedState) && (endLessLoop));
}

































