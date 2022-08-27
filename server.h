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



#ifndef SERVER_H
#define SERVER_H
#include <QtGui>
#include <QTcpServer>
#include <QDateTime>
#include <QUuid>
#include "logisdom.h"

class Connection;

class Server : public QTcpServer
{
    Q_OBJECT
#define timeLimit 86400
    friend class configwindow;
public:
enum RemoteRigths
{
	NoRigths, ReadOnly, FullControl
};
struct UsersLogin
{
	QString Name;
	QString PassWord;
	int Rigths;
};
struct UserIDs
{
	QString WebID;
	QString LastMenu;
	QString LastPageWeb;
	QDateTime timeout;
	int Rigths;
};
	QUuid IDGen;
	Server(logisdom *Parent);
	logisdom *parent;
	int clients();
	QList<UsersLogin> ConnectionUsers;
	QList<UserIDs> ConnectionIDs;
	void SaveConfigStr(QString &str);
	void readconfigfile(QString &configdata);
	int GetNewId(int indexuser);
	int isIDValable(const QString &ID);
	void setLastPageWeb(const QString &ID, const QString &pageweb);
	QString getLastPageWeb(const QString &ID);
	void transfertToOthers(QString order, Connection *client = nullptr);
private:
	int clientconnected;
	QList<Connection*> SocketList;
	QString usersonnected;
        QMutex GetID;
public slots:
	void sendAll();
private slots:
	void clientEnd(Connection*);
    void emitNewRequest(QString);
    void newSocket();
signals:
    void newRequest(QString);
    void clientend(Connection*);
	void clientbegin(Connection*);
};

#endif
