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



#ifndef EOCEANTHREAD_H
#define EOCEANTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMutex>


#define SER_SYNCH_CODE 			        0x55
#define SER_HEADER_NR_BYTES             4
#define SER_INTERBYTE_TIME_OUT 			100


class eoceanthread : public QThread
{
    Q_OBJECT
public:
struct FIFOStruc
{
    QString Request;
    uint32_t destinationID;
};
struct EOpacket
{
    uint8_t Type;
    QVector <uint8_t> data;
    QVector <uint8_t> Optdata;
};
    eoceanthread();
    ~eoceanthread();
    void setPort(int);
    void setAdress(const QString&);
    bool useTcp = false;
    bool useSerial = false;
    void run();
	bool logEnabled;
	bool endLessLoop;
	QAbstractSocket::SocketState tcpStatus;
	QString getStatus;
	QMutex mutexData;
    int fileIndex;
    QStringList deviceRomIDs;
    QList <quint64> dev4BS_Send;
    void saveConfig();
    int logAll = 0;
    uint32_t device_ID;
    void addtofifo(const QString &order);
    bool learnMode = false;
private:
    QList <FIFOStruc*> FIFO;
    uint32_t lastDestinationID;
    QTcpSocket *socket;
    QSerialPort *serial;
    QString adress;
    int port;
    QByteArray dataBuffer;
    static uint8_t crc8(uint8_t CRC, uint8_t Data);
    static uint16_t crc16(uint16_t CRC, uint16_t Data);
    const static uint8_t CRC8Table[256];
    const static uint16_t CRC16Table[256];
    void sendESP3(EOpacket &);
    bool decodeESP3(QByteArray &, EOpacket &);
    void processData(const EOpacket &);
    void processTeach(EOpacket &);
    bool isTeachIN(EOpacket &packet);
    bool checkBaseID();
    uint32_t baseID;
    QString getRomID(uint8_t rorg, uint8_t func, uint8_t type, uint32_t ID);
    QString logPacket(const EOpacket&);
    void appendID(QVector <uint8_t> &data, uint32_t ID);
    void learnModeOff();
    void TCPconnect(QString &Status);
signals:
    void newDevice(QString);
    void deviceReturn(const QString&, const QString&);
    void eoStatusChange();
	void newStatus();
    void logThis(const QString&);
    void tcpStatusChange();
};

#endif // EOCEANTHREAD_H
