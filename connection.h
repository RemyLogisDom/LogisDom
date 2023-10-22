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



#ifndef CONNECTION_H
#define CONNECTION_H

class Server;
#include <QWidget>
#include <QTcpSocket>


class Connection : public QWidget
{
	Q_OBJECT
#define CRequest "request"
#define CUser "user"
#define CPsw "password"
#define CUId "webid"
#define CMenuId "menuid"
public:
    Connection(QTcpSocket *socket, Server *parent);
	~Connection();
    QTcpSocket *tcp;
    QString ip;
	void sendScratchPads();
	void sendMainValue();
	QString getName();
	void transferCommand(QString order);
signals:
	void clientend(Connection*);
    void newRequest(QString);
protected:
private slots:
	void processReadyRead();
	void clientEnd();
public slots:
	void sendAll();
private:
	int Privilege;
	bool isHttp;
	Server *Parent;
	QString webFolder;
	QString getData(QString str);
	QString getOrder(QString str);
	QString getRomID(QString str);
	void extractrequest(QByteArray &data, QString *str);
	QString extractBuffer(const QString &data);
	QString UserName;
	QString PassWord;
	QByteArray buffer;
	void writeHeader(QByteArray dataType, bool compressed, long datasize, QByteArray order, QByteArray headerExtraData);
	void writeToClient(QByteArray data);
	bool writeWebFile(const QString &Data, const QString &type);
    bool writePng(const QString &Data);
};

#endif
