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



#ifndef MODBUSTHREAD_H
#define MODBUSTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>

#include "devmodbus.h"


class modbusthread : public QThread
{
	Q_OBJECT
public:
struct device
{
	quint16 slave;
	quint16 address;
    QString value;
    QString RomID;
    bool M3Mux;
    int isAlive;
    bool muxHasBeenRead;
};
#define maxLen 100
public:
	modbusthread();
	~modbusthread();
	void run();
	QString moduleipaddress;
    quint16 port;
    QString log;
    QTcpSocket *socket = nullptr;
    bool logEnabled = false, logOnlyWrite = false;
    quint16 MuxInd = 20;
    bool endLessLoop = true;
	QAbstractSocket::SocketState tcpStatus;
	QMutex mutexData;
    void addToFIFOSpecial(const QString &data);
	int fileIndex;
	static quint16 calcCRC16(const unsigned char *inBuf, quint16 inLen);
    bool Modbus_TCP_Enable = false;
    quint16 TCP_ID;
    QList <devmodbus*> modbusdevices;
private:
    bool get(QTcpSocket &Socket, const unsigned char *request, const unsigned int inLen, qint64 &value);
    bool write(QTcpSocket &Socket, const unsigned char *request, const unsigned int inLen, qint16 &value);
    void handle_Excpetion(const unsigned char code);
	void TCPconnect(QTcpSocket &Socket, QString &Status);
    QStringList FIFOSpecial;
	void saveLog();
    quint16 build_read_03_request(quint16 slave, quint16 addr, quint16 nb, unsigned char *req);
    quint16 build_read_request(quint16 slave, quint16 addr, quint16 nb, quint16 function, unsigned char *req);
    quint16 build_write_10_request(quint16 slave, quint16 addr, quint16 val, unsigned char *req);
	static const quint8 table_crc_lo[256];
	static const quint8 table_crc_hi[256];
signals:
    void newDevice(const QString&);
    void deviceReturn(const QString&, const QString&);
    void setDeviceScratchpad(devmodbus*, const QString&);
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
public slots:
    void writeCommand(devmodbus*);
};

#endif // MODBUSTHREAD_H
