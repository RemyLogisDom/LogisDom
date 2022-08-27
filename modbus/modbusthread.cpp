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
#include "Winsock2.h"
#include "stdint.h"
#endif

#include <QElapsedTimer>
#include <QTimer>
#include "net1wire.h"
#include "modbusthread.h"
#include "globalvar.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif





modbusthread::modbusthread()
{
    moveToThread(this);
 }



modbusthread::~modbusthread()
{
    endLessLoop = false;
    if (socket) socket->abort();
}



void modbusthread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QTcpSocket MySocket;
    socket = &MySocket;
    MySocket.moveToThread(this);
    qintptr sd = MySocket.socketDescriptor();
	int set = 1;
	TCP_ID = 0;
#ifdef Q_OS_LINUX
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
    QString msg = "Thread starts, first connection";
    TCPconnect(MySocket, msg);
    while (endLessLoop)
    {
        log.clear();
        int multiplexMacro = 0;
        int adressMuxMax = 0;
        foreach (devmodbus *dev, modbusdevices)
        {
            if (dev->isM3MuxEnabled)
            {
                multiplexMacro ++;
                quint16 a = dev->address;
                if (a > adressMuxMax) adressMuxMax = a;
            }
            dev->muxHasBeenRead = false;
        }
        for (int n=0; n<adressMuxMax; n++) // number of loop = number of devices.
        {
            unsigned char req[maxLen];
            quint16 s = build_read_03_request(0, MuxInd, 2, &req[0]);
            qint64 V = 0;
            get(MySocket, &req[0], s, V);
            log += addTimeTag QString("Get Multipexor data turn %1").arg(n);
            msleep(500);
        }
        foreach (devmodbus *dev, modbusdevices)
        {
            if (!dev->isM3MuxEnabled)
            {
                if (!dev->autoRead) dev->checkRead();
                if (dev->autoRead | dev->readNow | !dev->initialRead)
                {
                    unsigned char req[maxLen];
                    quint16 slave = dev->slave;
                    quint16 address = dev->address;
                    quint16 function = dev->function;
                    quint16 s = build_read_request(slave, address, dev->resolution, function, &req[0]);
                    qint64 V = 0;
                    log += addTimeTag "Read " + dev->getname();
                    if (get(MySocket, &req[0], s, V))
                    {
                        QString v;
                        if (dev->ValUnsigned)
                        {
                            if (dev->resolution == 4) v = QString("%1").arg(quint64(V));
                            else if (dev->resolution == 2) v = QString("%1").arg(quint32(V));
                            else v = QString("%1").arg(quint16(V));
                        }
                        else
                        {
                            if (dev->resolution == 4) v = v = QString("%1").arg(qint64(V));
                            else if (dev->resolution == 2) v = v = QString("%1").arg(qint32(V));
                            else v = QString("%1").arg(qint16(V));
                        }
                        emit(setDeviceScratchpad(dev, v));
                        log += addTimeTag "Value : " + v;
                        if (logOnlyWrite) log.clear();
                        else saveLog();
                    }
                    msleep(100);
                }
            }
            if (!FIFOSpecial.isEmpty()) break;
        }
        while (!FIFOSpecial.isEmpty())
        {
            QString data = FIFOSpecial.first();
            QStringList parameters = data.split("/");
            if (parameters.count() == 3)
            {
                bool ok1, ok2, ok3;
                unsigned char req[maxLen];
                quint16 slave = quint16(parameters.at(0).toUInt(&ok1, 16));
                quint16 address = quint16(parameters.at(1).toUInt(&ok2, 16));
                quint16 value = quint16(parameters.at(2).toUInt(&ok3, 16));
                if (ok1 && ok2 && ok3)
                {
                    quint16 s = build_write_10_request(slave, address, value, &req[0]);
                    qint64 V = 0;
                    get(MySocket, &req[0], s, V);
                    // read back device
                    foreach (devmodbus *dev, modbusdevices)
                    {
                        if (!dev->isM3MuxEnabled)
                        {
                            unsigned char req[maxLen];
                            quint16 Rslave = dev->slave;
                            quint16 Raddress = dev->address;
                            quint16 function = dev->function;
                            if ((Rslave == slave) and (Raddress == address))
                            {
                                log += addTimeTag "Write command done " + dev->getname() + QString(" with value %1").arg(value);
                                quint16 s = build_read_request(slave, address, 1, function, &req[0]);
                                qint64 V = 0;
                                log += addTimeTag "Read " + dev->getname();
                                if (get(MySocket, &req[0], s, V))
                                {
                                    QString v; // = QString("%1").arg(V);
                                    if (dev->ValUnsigned)
                                    {
                                        //if (dev->resolution == 4) v = QString("%1").arg(quint64(V));
                                        //else if (dev->resolution == 2) v = QString("%1").arg(quint32(V));
                                        //else
                                        v = QString("%1").arg(quint16(V));
                                    }
                                    else
                                    {
                                        //if (dev->resolution == 4) v = v = QString("%1").arg(qint64(V));
                                        //else if (dev->resolution == 2) v = v = QString("%1").arg(qint32(V));
                                        //else
                                        v = QString("%1").arg(qint16(V));
                                    }
                                    emit(setDeviceScratchpad(dev, v));
                                    log += addTimeTag "Value : " + v;
                                    saveLog();
                                }
                            }
                        }
                    }
                }
                else
                {
                    log += addTimeTag "FIFOSpecial conversion error : " + data;
                }
            }
            mutexData.lock();
            FIFOSpecial.removeFirst();
            mutexData.unlock();
        }
        if (logOnlyWrite) log.clear();
        else saveLog();
        sleep(1);
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
}






void modbusthread::TCPconnect(QTcpSocket &Socket, QString &Status)
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
        if (Socket.state() == QAbstractSocket::UnconnectedState) log += addTimeTag "Disconnect OK";
		tcpStatus = Socket.state();
		emit(tcpStatusChange());
        if (endLessLoop)
        {
            Socket.connectToHost(moduleipaddress, port);
            log += addTimeTag "Try Connect";
            Socket.waitForConnected();
            if (Socket.state() == QAbstractSocket::ConnectedState) log += addTimeTag "Connect OK";
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





void modbusthread::handle_Excpetion(const unsigned char code)
{
	switch (code)
	{
		case 0x01	:
            log += addTimeTag "ILLEGAL FUNCTION";
/*The function code received in the query is not an
allowable action for the server (or slave). This
may be because the function code is only
applicable to newer devices, and was not
implemented in the unit selected. It could also
indicate that the server (or slave) is in the wrong
state to process a request of this type, for
example because it is unconfigured and is being
asked to return register values.*/
		break;
		case 0x02	:
            log += addTimeTag "ILLEGAL DATA ADDRESS";
/*The data address received in the query is not an
allowable address for the server (or slave). More
specifically, the combination of reference number
and transfer length is invalid. For a controller with
100 registers, the PDU addresses the first
register as 0, and the last one as 99. If a request
is submitted with a starting register address of 96
and a quantity of registers of 4, then this request
will successfully operate (address-wise at least)
on registers 96, 97, 98, 99. If a request is
submitted with a starting register address of 96
and a quantity of registers of 5, then this request
will fail with Exception Code 0x02 Illegal Data
Address since it attempts to operate on registers
96, 97, 98, 99 and 100, and there is no register
with address 100.*/
		break;
		case 0x03	:
            log += addTimeTag "ILLEGAL DATA VALUE";
/*A value contained in the query data field is not an
allowable value for server (or slave). This
indicates a fault in the structure of the remainder
of a complex request, such as that the implied
length is incorrect. It specifically does NOT mean
that a data item submitted for storage in a register
has a value outside the expectation of the
application program, since the MODBUS protocol
is unaware of the significance of any particular
value of any particular register.*/
		break;
		case 0x04	: //SLAVE DEVICE FAILURE
            log += addTimeTag "SLAVE DEVICE FAILURE";
/*
An unrecoverable error occurred while the server
(or slave) was attempting to perform the
requested action.*/
		break;
		case 0x05	: //ACKNOWLEDGE
           log += addTimeTag "ACKNOWLEDGE";
/*
Specialized use in conjunction with programming
commands.
The server (or slave) has accepted the request
and is processing it, but a long duration of time
will be required to do so. This response is
returned to prevent a timeout error from occurring
in the client (or master). The client (or master)
can next issue a Poll Program Complete message
to determine if processing is completed.*/
		break;
		case 0x06	: //SLAVE DEVICE BUSY
            log += addTimeTag "SLAVE DEVICE BUSY";
/*Specialized use in conjunction with programming
commands.
The server (or slave) is engaged in processing a
longduration program command. The client (or
master) should retransmit the message later when
the server (or slave) is free.*/
		break;
		case 0x08	: //MEMORY PARITY ERROR
            log += addTimeTag "MEMORY PARITY ERROR";
/*Specialized use in conjunction with function codes
20 and 21 and reference type 6, to indicate that
the extended file area failed to pass a consistency
check.*/							break;
		case 0x0A	: //GATEWAY PATH UNAVAILABLE
            log += addTimeTag "GATEWAY PATH UNAVAILABLE";
/*Specialized use in conjunction with gateways,
indicates that the gateway was unable to allocate
an internal communication path from the input
port to the output port for processing the request.
Usually means that the gateway is misconfigured
or overloaded.*/
		break;
		case 0x0B	: //GATEWAY TARGET DEVICE FAILED TO RESPOND
            log += addTimeTag "GATEWAY TARGET DEVICE FAILED TO RESPOND";
/*Specialized use in conjunction with gateways,
indicates that no response was obtained from the
target device. Usually means that the device is
not present on the network.*/
        break;
        default :
        log += addTimeTag "UNKOWN ERROR" + QString ("%1").arg(code);
        break;
	}
}






bool modbusthread::get(QTcpSocket &Socket, const unsigned char *request, const unsigned int inLen, qint64 &value)
{
	QString hex_request;
    for (quint16 n=0; n<inLen; n++) hex_request += QString("%1 ").arg(uchar(request[n]), 2, 16, QChar('0')).toUpper();
    log += addTimeTag " SEND : "+ hex_request;
	if (Socket.state() == QAbstractSocket::ConnectedState)
	{
        char req[inLen];
        uchar function = 0;
		for (quint16 n=0; n<inLen; n++) req[n] = request[n];
		if (Socket.isValid()) Socket.write(req, inLen);
		else return logisdom::NA;
		if (Socket.waitForBytesWritten())
		{
            msleep(200);
            if (Socket.waitForReadyRead(5000))
            {
                QByteArray Data;
				Data.append(Socket.readAll());
				quint16 read_length = Data.length();
				QString hex;
				for (int n=0; n<read_length; n++)
					hex += QString("%1 ").arg((unsigned char)Data.at(n), 2, 16, QChar('0')).toUpper();
                log += addTimeTag " GET : " + hex;
				unsigned char answer[read_length];
                for (int n=0; n<read_length; n++) answer[n] = uchar(Data.at(n));
				if (Modbus_TCP_Enable)
				{
                    if (read_length < 9)
                    {
                        log += addTimeTag QString("Not enough data " + hex);
                        return false;
                    }
					bool exception_check = false;
					if (answer[7] & 0x80) exception_check = true;
					if (exception_check)
					{
						handle_Excpetion(answer[8]);
						return false;
					}
					else
					{
						unsigned char ID_Read_High = answer[0];
                        unsigned char ID_Read_Low = answer[1];
						int TCP_ID_read = (ID_Read_High << 8) + ID_Read_Low;
						if (TCP_ID_read != TCP_ID)
						{
                            log += addTimeTag "Request ID Error";
							return false;
						}
						quint16 Next_length = answer[8];
						if (Next_length == 2)
						{
                            value = (answer[9] << 8) + answer[10];
						}
                        else if (Next_length == 4)
                        {
                            int index = (answer[9] << 8) | answer[10];
                            int value = (answer[11] << 8) | answer[12];
                            log += addTimeTag "Multiplex";
                            log += addTimeTag QString("index = %1").arg(index);
                            log += addTimeTag QString("value = %1").arg(value);
                            foreach (devmodbus *dev, modbusdevices)
                            {
                                if (dev->address == index)
                                {
                                    QString v = QString("%1").arg(value);
                                    emit(setDeviceScratchpad(dev, v));
                                    dev->muxHasBeenRead = true;
                                }
                            }
                        }
                        else
						{
                            log += addTimeTag "Program is not ready yet to handle data different than 2 Bytes";
						}
					}
				}
				else
				{
                    if (inLen > 1) function = request[1];
                    if ((function == 0x03) || (function == 0x04))
					{
                        if (read_length < 3) return false;
                        quint16 size = quint16(Data.at(2));
                        quint16 length = size + 5;	// in the answer size is byte n°3 (+3) and CRC is 2 bytes (+2)
                    if (read_length >= length)
                    {
                            //uchar answer[length];
                            uchar *answer = new uchar[length];
                            quint64 *answer64 = new quint64[length];
                            for (int n=0; n<length; n++) answer64[n] = uchar(Data.at(n));
                            for (int n=0; n<length; n++) answer[n] = uchar(Data.at(n));
                            quint16 crc_received = (answer[length-2] << 8) | answer[length-1];
                            quint16 crc_calc = calcCRC16(&answer[0], length-2);
                            if (crc_calc != crc_received)
                            {
                                log += addTimeTag " CRC Claculated : " + QString("%1 ").arg(crc_calc, 4, 16, QChar('0')).toUpper();
                                log += addTimeTag " CRC Received : " + QString("%1 ").arg(crc_received, 4, 16, QChar('0')).toUpper();
                                log += addTimeTag " Bad CRC";
								return false;
                            }
                            if (size == 8)
                            {
                                value = (answer64[9] << 56) | (answer64[10] << 48) | (answer64[7] << 40) | (answer64[8] << 32) | (answer64[5] << 24) | (answer64[6] << 16) | (answer64[3] << 8) | answer64[4];
                                //qDebug() << QString("L = 8 : %2").arg(value);
                            }
                            else
                            if (size == 4)
                            {
                                value = (answer64[5] << 24) | (answer64[6] << 16) | (answer64[3] << 8) | answer64[4];
                                //qDebug() << QString("L = 4 : %2").arg(value);
                            }
                            else
                            {
                                value = (answer64[3] << 8) | answer64[4];
                                //qDebug() << QString("L = 2 : %2").arg(value);
                            }
                            delete[] answer;
                            delete[] answer64;
                        }
						else return false;
					}
					else if (function == 0x06)
					{
						//for (int n=0; n<read_length; n++) answer[n] = (unsigned char)Data.at(n);
                        log += addTimeTag " Function finished";
					}
					else return false;
				}
			}
            else
            {
                QString msg = "No answer, reconnect socket, request = " + hex_request;
                TCPconnect(Socket, msg);
                return false;
            }
        }
		else
		{
            log += addTimeTag "Error writing data : " + hex_request;
			return false;
		}
	}
	else
	{
		QString msg = "Socket error before writing, reconnect, request = " + hex_request;
		TCPconnect(Socket, msg);
		return false;
	}
	return true;
}




bool modbusthread::write(QTcpSocket &Socket, const unsigned char *request, const unsigned int inLen, qint16 &value)
{
    QString hex_request;
    for (quint16 n=0; n<inLen; n++) hex_request += QString("%1 ").arg(uchar(request[n]), 2, 16, QChar('0')).toUpper();
    log += addTimeTag " SEND : "+ hex_request;
    if (Socket.state() == QAbstractSocket::ConnectedState)
    {
        char req[inLen];
        for (quint16 n=0; n<inLen; n++) req[n] = request[n];
        if (Socket.isValid()) Socket.write(req, inLen);
        else return logisdom::NA;
        if (!Socket.waitForBytesWritten(5000))
        {
            log += addTimeTag "Error writing data : " + hex_request;
            return false;
        }
    }
    else
    {
        QString msg = "Socket error before writing, reconnect, request = " + hex_request;
        TCPconnect(Socket, msg);
        return false;
    }
    return true;
}





void modbusthread::saveLog()
{
    if (!logEnabled)
    {
        log.clear();
        return;
    }
    if (!log.isEmpty()) emit(tcpStatusUpdate(log));
    log.clear();
}








void modbusthread::addToFIFOSpecial(const QString &data)
{
	if (FIFOSpecial.count() > 100) return;
    if (FIFOSpecial.contains(data)) return;
	mutexData.lock();
    FIFOSpecial.append(data);
	mutexData.unlock();
}



/* Table of CRC values for high-order byte */
const quint8 modbusthread::table_crc_hi[256] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
const quint8 modbusthread::table_crc_lo[256] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

quint16 modbusthread::build_read_request(quint16 slave, quint16 addr, quint16 nb, quint16 function, unsigned char *req)
{
    if (Modbus_TCP_Enable)
    {
        // Transaction ID
        if (TCP_ID < 32767) TCP_ID++; else TCP_ID = 0;
        req[0] = TCP_ID >> 8;
        req[1] = TCP_ID & 0x00ff;
        // Protocol Modbus
        req[2] = 0;
        req[3] = 0;
        // Length of following data
        req[4] = 0;
        req[5] = 6;
        // Unit
        req[6] = quint8(slave); //0xFF;
        req[7] = quint8(function);
        req[8] = addr >> 8;
        req[9] = addr & 0x00ff;
        req[10] = nb >> 8;
        req[11] = nb & 0x00ff;
        return 12;
    }
    else
    {
        req[0] = quint8(slave);
        req[1] = quint8(function);
        req[2] = addr >> 8;
        req[3] = addr & 0x00ff;
        req[4] = nb >> 8;
        req[5] = nb & 0x00ff;
        quint16 crc = calcCRC16(&req[0], 6);
        req[6] = crc >> 8;
        req[7] = crc & 0x00ff;
        return 8;
    }
}

quint16 modbusthread::build_read_03_request(quint16 slave, quint16 addr, quint16 nb, unsigned char *req)
{
	if (Modbus_TCP_Enable)
	{
		// Transaction ID
        if (TCP_ID < 32767) TCP_ID++; else TCP_ID = 0;
		req[0] = TCP_ID >> 8;
		req[1] = TCP_ID & 0x00ff;
		// Protocol Modbus
		req[2] = 0;
		req[3] = 0;
		// Length of following data
		req[4] = 0;
		req[5] = 6;
		// Unit
        req[6] = quint8(slave); //0xFF;
		req[7] = 0x03;
		req[8] = addr >> 8;
		req[9] = addr & 0x00ff;
		req[10] = nb >> 8;
		req[11] = nb & 0x00ff;
		return 12;
	}
	else
	{
        req[0] = quint8(slave);
		req[1] = 0x03;
		req[2] = addr >> 8;
		req[3] = addr & 0x00ff;
		req[4] = nb >> 8;
		req[5] = nb & 0x00ff;
		quint16 crc = calcCRC16(&req[0], 6);
		req[6] = crc >> 8;
		req[7] = crc & 0x00ff;
		return 8;
	}
}

quint16 modbusthread::build_write_10_request(quint16 slave, quint16 addr, quint16 val, unsigned char *req)
{
    quint16 nb = 1;
    int i = 0;
    if (Modbus_TCP_Enable)
	{
		// Transaction ID
		if (TCP_ID < 32767) TCP_ID++; else TCP_ID = 0;
        req[i++] = TCP_ID >> 8;
        req[i++] = TCP_ID & 0x00ff;
		// Protocol Modbus
        req[i++] = 0;
        req[i++] = 0;
		// Length of following data
        req[i++] = 0;
        req[i++] = 9;
		// Unit
        req[i++] = 0xFF;
        // Function code FC16
        req[i++] = 0x10;
        // Address of first register
        req[i++] = addr >> 8;
        req[i++] = addr & 0x00ff;
        // Index of register to write to
        req[i++] = nb >> 8;
        req[i++] = nb & 0x00ff;
        // Number of data byte to follow
        req[i++] = 0x02;
        // Data
        req[i++] = val >> 8;
        req[i++] = val & 0x00ff;
        return i;
	}
	else
	{
        req[i++] = quint8(slave);
        req[i++] = 0x10;
        req[i++] = addr >> 8;
        req[i++] = addr & 0x00ff;
        req[i++] = nb >> 8;
        req[i++] = nb & 0x00ff;
        req[i++] = 0x02;
        req[i++] = val >> 8;
        req[i++] = val & 0x00ff;
		quint16 crc = calcCRC16(&req[0], 9);
        req[i++] = crc >> 8;
        req[i++] = crc & 0x00ff;
        return i;
	}
}



void modbusthread::writeCommand(devmodbus *)
{
    qDebug() << "writeCommand";
    //writedevices.append(&dev);
    //writevalues.append(v);
}


/*
quint16 nb = 1;
req[0] = slave;
req[1] = 0x10;
req[2] = addr >> 8;
req[3] = addr & 0x00ff;
req[4] = nb >> 8;
req[5] = nb & 0x00ff;
req[6] = 0x02;
req[7] = val >> 8;
req[8] = val & 0x00ff;
quint16 crc = calcCRC16(&req[0], 9);
req[9] = crc >> 8;
req[10] = crc & 0x00ff;
return 11;
*/
/*
10 10 08 01 01 00 ED 13
10 10 08 01 01 00 02 02 1E ED 13
10 10 08 01 00 01 02 02 1D BD 2F
Slave Address 11
Function 10
Starting Address Hi 00
Starting Address Lo 01
No. of Registers Hi 00
No. of Registers Lo 02
Byte Count 04
Data Hi 00
Data Lo 0A
Data Hi 01
Data Lo 02
Error Check (LRC or CRC) 
*/

quint16 modbusthread::calcCRC16(const unsigned char *buffer, quint16 buffer_length)
 {
    quint8 crc_hi = 0xFF;
    quint8 crc_lo = 0xFF;
    unsigned int i;
    while (buffer_length--)
    {
	i = crc_hi ^ *buffer++;
	crc_hi = crc_lo ^ table_crc_hi[i];
	crc_lo = table_crc_lo[i];
    //if (logEnabled) log += addTimeTag QString(" buffer_length : %1" ).arg(buffer_length);
    //if (logEnabled) log += addTimeTag QString(" crc_hi : %1" ).arg(crc_hi);
    //if (logEnabled) log += addTimeTag QString(" crc_lo : %1" ).arg(crc_lo);
  }
  quint16 result = quint16(crc_hi << 8 | crc_lo);
  return result;
}




