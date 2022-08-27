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




#include <QtNetwork>
#include <QUuid>
#include <QTcpServer>
#include <QDateTime>

#include "globalvar.h"
#include "connection.h"
#include "server.h"

#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif



Server::Server(logisdom *Parent)
{
	clientconnected = 0;
	parent = Parent;
    connect(this, SIGNAL(newConnection()), this, SLOT(newSocket()));
}




void Server::sendAll()
{
	for (int n=0; n<SocketList.count(); n++)
		SocketList[n]->sendAll();
}



void Server::newSocket()
{
    QTcpSocket *socket = nextPendingConnection();
    qintptr sd = socket->socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
//#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
//#endif

    Connection *connection = new Connection(socket, this);
/*
void Server::incomingConnection(int socketDescriptor)
{
    connection->setSocketDescriptor(socketDescriptor);
//        connect(connection, SIGNAL(clientbegin(Connection*)), this, SLOT(clientBegin(Connection*)), Qt::BlockingQueuedConnection);
}

 */
    connect(connection, SIGNAL(clientend(Connection*)), this, SLOT(clientEnd(Connection*)));
    connect(connection, SIGNAL(newRequest(QString)), this, SLOT(emitNewRequest(QString)));
    SocketList.append(connection);
    emit clientbegin(connection);
}



void Server::transfertToOthers(QString order, Connection *client)
{
	if (client)
	{
		for (int n=0; n<SocketList.count(); n++)
			if (client != SocketList[n]) SocketList[n]->transferCommand(order);
	}
//	else
//	{
//		for (int n=0; n<SocketList.count(); n++) SocketList[n]->transferCommand(order);
//	}
}



void Server::SaveConfigStr(QString &str)
{
	int count = ConnectionUsers.count();
	for (int n=0; n<count; n++)
	 {
 		str += "\n"  Remote_User  "\n";
		str += logisdom::saveformat(Remote_User_Name, ConnectionUsers[n].Name);
		str += logisdom::saveformat(Remote_User_Psw, ConnectionUsers[n].PassWord);
		str += logisdom::saveformat(Remote_User_Privilege, QString("%1").arg(ConnectionUsers[n].Rigths));
		str += EndMark;
		str += "\n";
	 }
}





void Server::readconfigfile(QString &configdata)
{
	bool ok;
	QString TAG_Begin = Remote_User;
	QString TAG_End = EndMark;
	QString Name, Psw;
	int Privilege;
	SearchLoopBegin
	Name = logisdom::getvalue(Remote_User_Name, strsearch);
	Psw = logisdom::getvalue(Remote_User_Psw, strsearch);
	Privilege = logisdom::getvalue(Remote_User_Privilege, strsearch).toInt(&ok);
	if ((!Name.isEmpty()) and (!Psw.isEmpty()) and (ok))
	{
		UsersLogin NewUser;
		NewUser.Name = Name;
		NewUser.PassWord = Psw;
		NewUser.Rigths = Privilege;
		ConnectionUsers.append(NewUser);
	}
	SearchLoopEnd
}



int Server::GetNewId(int indexuser)
{
    QMutexLocker locker(&GetID);
    QDateTime now = QDateTime::currentDateTime();
    UserIDs newID;
    newID.WebID = IDGen.createUuid().toString();
    newID.WebID.remove("\{");
    newID.WebID.remove("}");
    newID.timeout = now.addSecs(timeLimit);
    newID.Rigths = ConnectionUsers[indexuser].Rigths;
    ConnectionIDs.append(newID);
    return (ConnectionIDs.count() - 1);
}




void Server::setLastPageWeb(const QString &ID, const QString &pageweb)
{
	int count = ConnectionIDs.count();
	for (int n=0; n<count; n++)
		if (ConnectionIDs[n].WebID == ID)
		{
			ConnectionIDs[n].LastPageWeb = pageweb;
		}
}




QString Server::getLastPageWeb(const QString &ID)
{
	int count = ConnectionIDs.count();
	for (int n=0; n<count; n++)
		if (ConnectionIDs[n].WebID == ID)
		{
			return ConnectionIDs[n].LastPageWeb;
		}
	return "";
}


int Server::isIDValable(const QString &ID)
{
        QMutexLocker locker(&GetID);
        QDateTime now = QDateTime::currentDateTime();
	int count;
	bool found;
	do
	{
		found = false;
		count = ConnectionIDs.count();
		for (int n=0; n<count; n++)
		{
			if (ConnectionIDs[n].timeout.secsTo(now) > 0)
			{
				ConnectionIDs.removeAt(n);
				found = true;
				break;
			}
		}
	} while (found);
	count = ConnectionIDs.count();
	for (int n=0; n<count; n++)
		if (ConnectionIDs[n].WebID == ID)
		{
			ConnectionIDs[n].timeout = now.addSecs(timeLimit);
			return n;
		}
	return -1;
}





void Server::clientEnd(Connection *client)
{
    SocketList.takeAt(SocketList.indexOf(client));
    emit clientend(client);
}



void Server::emitNewRequest(QString str)
{
    emit newRequest(str);
}



int Server::clients()
{
	return SocketList.count();
}




