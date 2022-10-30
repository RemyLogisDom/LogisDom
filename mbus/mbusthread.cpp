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



#include <QElapsedTimer>
#include <QTimer>
#include "net1wire.h"
#include "mbusthread.h"
#include "mbus.h"
#include "globalvar.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif



mbusthread::mbusthread()
{
    logEnabled = false;
    endLessLoop = true;
    readInterval = 0;
    searchMax = 10;
    searchDone = false;
 }




mbusthread::~mbusthread()
{
    endLessLoop = false;
}





void mbusthread::appendAdr(const QString &str)
{
    adrToRead.append(str);
}



void mbusthread::mbus_setType(mbus_frame &frame, int frame_type)
{
    frame.type = frame_type;
    switch (frame.type)
    {
        case MBUS_FRAME_TYPE_ACK:
        frame.start1 = MBUS_FRAME_ACK_START;
        break;

    case MBUS_FRAME_TYPE_SHORT:
        frame.start1 = MBUS_FRAME_SHORT_START;
        frame.stop   = MBUS_FRAME_STOP;
        break;

    case MBUS_FRAME_TYPE_CONTROL:
        frame.start1 = MBUS_FRAME_CONTROL_START;
        frame.start2 = MBUS_FRAME_CONTROL_START;
        frame.length1 = 3;
        frame.length2 = 3;
        frame.stop   = MBUS_FRAME_STOP;
        break;

    case MBUS_FRAME_TYPE_LONG:
        frame.start1 = MBUS_FRAME_LONG_START;
        frame.start2 = MBUS_FRAME_LONG_START;
        frame.stop   = MBUS_FRAME_STOP;
        break;
    }
}




void mbusthread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
#define PACKET_BUFF_SIZE 128
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
    QTcpSocket MySocket;
    int sd = MySocket.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(sd, SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
    int fileIndex = 0;
    int minute = -1;
    int minuteCheckDev = QDateTime::currentDateTime().time().minute();
    QString msg = "Thread starts, first connection";
    TCPconnect(MySocket, msg);
    while (endLessLoop)
    {
// Search sequence
        if (!searchDone)
        {
            searchMbus(MySocket);
            if (logEnabled) log += addTimeTag "********************     Adress found   ********************************";
            if (logEnabled) for (int n=0; n<devices.count(); n++) log += addTimeTag "Adress " + devices.at(n);
            if (logEnabled) log += addTimeTag "*************************************************************************************";
            searchDone = true;
        }
// Read All addresses
        QStringList adrStr;
        for (int n=0; n<devices.count(); n++)
        {
            bool ok;
            int adr = devices.at(n).toInt(&ok);
            if (ok)
            {
                QString s = QString("%1").arg(adr);
                if (!adrStr.contains(s))
                {
                    readMbus(MySocket, adr);
                    adrStr.append(s);
                }
            }
        }
        emit(ReadingDone());
        if (readInterval == mbus::Reading1d)
        {
            QString str = tr("Wait next day, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            int today = QDateTime::currentDateTime().date().day();
            while ((today == QDateTime::currentDateTime().date().day()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading1h)
        {
            QString str = tr("Wait next hour, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            int hour = QDateTime::currentDateTime().time().hour();
            while ((hour == QDateTime::currentDateTime().time().hour()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading30mn)
        {
            if (minute < 0) minute = QDateTime::currentDateTime().time().minute();
            minute += 30;
            while (minute > 59) minute -=60;
            QString str = tr("Wait 30mn, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            while ((minute != QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading20mn)
        {
            if (minute < 0) minute = QDateTime::currentDateTime().time().minute();
            minute += 20;
            while (minute > 59) minute -=60;
            QString str = tr("Wait 20mn, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            while ((minute != QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading15mn)
        {
            if (minute < 0) minute = QDateTime::currentDateTime().time().minute();
            minute += 15;
            while (minute > 59) minute -=60;
            QString str = tr("Wait 15mn, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            while ((minute != QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading10mn)
        {
            if (minute < 0) minute = QDateTime::currentDateTime().time().minute();
            minute += 10;
            while (minute > 59) minute -=60;
            QString str = tr("Wait 10mn, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            while ((minute != QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else if (readInterval == mbus::Reading5mn)
        {
            if (minute < 0) minute = QDateTime::currentDateTime().time().minute();
            minute += 5;
            while (minute > 59) minute -=60;
            QString str = tr("Wait 5mn, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            while ((minute != QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        else
        {
            QString str = tr("Wait next minute, last read : ") + QDateTime::currentDateTime().toString("dd-MM hh:mm");
            emit(whatdoUDo(str));
            minute = QDateTime::currentDateTime().time().minute();
            while ((minute == QDateTime::currentDateTime().time().minute()) && endLessLoop && searchDone) { MBusReadLoop; }
        }
        if (logEnabled)
        {
            QString filename = moduleipaddress;
            filename.remove(".");
            filename += QString("_%1.txt").arg(fileIndex);
            QFile file(filename);
            QTextStream out(&file);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            out << log;
            file.close();
            fileIndex++;
            if (fileIndex > 999) fileIndex = 0;
            emit(tcpStatusUpdate(log));
        }
        log.clear();
        sleep(1);
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
}




void mbusthread::readMbus(QTcpSocket &Socket, int adr)
{
    QString str = tr("Reading MBus address") + QString(" %1").arg(adr);
    emit(whatdoUDo(str));
    QByteArray data;
    mbus_frame frame;
    mbus_setType(frame, MBUS_FRAME_TYPE_SHORT);
    frame.control = MBUS_CONTROL_MASK_REQ_UD2 | MBUS_CONTROL_MASK_DIR_M2S; // data request from master to slave*/
    frame.address = adr;
    data.clear();
    if (logEnabled) log += addTimeTag QString("Get device data");
    get(Socket, frame, data, 1000);
    mbus_frame_data frame_data;
    if (!data.isEmpty())
    {
        mbus_frame reply;
        if (mbus_parse(reply, data)) return;
        QString logtxt;
        for (unsigned int n=0; n<reply.data_size; n++) logtxt += QString("[%1]").arg((unsigned char)reply.data[n]);
        if (logEnabled) log += addTimeTag "Data after parsing : " + logtxt;
        mbus_frame_data_parse(reply, frame_data, adr);
    }
// DonnÃ©e instantanÃ©e
    mbus_setType(frame, MBUS_FRAME_TYPE_CONTROL);
    frame.control = MBUS_CONTROL_MASK_SND_UD;
    frame.control_information = MBUS_CONTROL_INFO_REQUEST_RAM_READ;
    frame.address = adr;
    data.clear();
    if (logEnabled) log += addTimeTag QString("Get device real time data");
    get(Socket, frame, data, 1000);
    if (!data.isEmpty())
    {
        mbus_frame reply;
       // mbus_frame_data frame_data;
        if (mbus_parse(reply, data)) return;
        if (logEnabled)
        {
            QString logtxt;
            for (unsigned int n=0; n<reply.data_size; n++) logtxt += QString("[%1]").arg((unsigned char)reply.data[n]);
            log += addTimeTag "Real time data after parsing : " + logtxt;
        }
        mbus_frame_data_parse(reply, frame_data, adr);
    }
    for (int n=0; n<frame_data.data_var.records.count(); n++) delete frame_data.data_var.records.at(n);
}




void mbusthread::searchMbus(QTcpSocket &Socket)
{
    for (int adr=0; adr<searchMax; adr++)
    {
        QString str = tr("Searching/Reading MBus address") + QString(" %1").arg(adr);
        emit(whatdoUDo(str));
        mbus_frame frame;
        mbus_setType(frame, MBUS_FRAME_TYPE_SHORT);
        frame.control = MBUS_CONTROL_MASK_SND_NKE | MBUS_CONTROL_MASK_DIR_M2S; // data request from master to slave
        frame.address = adr;
        QByteArray data;
        if (logEnabled) log += addTimeTag QString("Search at adress %1").arg(adr);
        get(Socket, frame, data, 1000);
        if (!data.isEmpty())
        {
            if (data.at(0) == MBUS_FRAME_ACK_START)
            {
                emit(newTreeAddress(adr));
                QString adrStr = QString("%1").arg(adr);
                if (!devices.contains(adrStr)) devices.append(adrStr);
                mbus_setType(frame, MBUS_FRAME_TYPE_SHORT);
                frame.control = MBUS_CONTROL_MASK_REQ_UD2 | MBUS_CONTROL_MASK_DIR_M2S; // data request from master to slave
                frame.address = adr;
                data.clear();
                if (logEnabled) log += addTimeTag QString("Get device data");
                get(Socket, frame, data, 1000);
                mbus_frame_data frame_data;
                if (!data.isEmpty())
                {
                    mbus_frame reply;
                    mbus_parse(reply, data);
                    if (logEnabled)
                    {
                        QString logtxt;
                        for (unsigned int n=0; n<reply.data_size; n++) logtxt += QString("[%1]").arg((unsigned char)reply.data[n]);
                        log += addTimeTag "Data after parsing : " + logtxt;
                    }
                    mbus_frame_data_parse(reply, frame_data, adr);
                }
                // DonnÃ©e instantanÃ©e
                mbus_setType(frame, MBUS_FRAME_TYPE_CONTROL);
                frame.control = MBUS_CONTROL_MASK_SND_UD;
                frame.control_information = MBUS_CONTROL_INFO_REQUEST_RAM_READ;
                frame.address = adr;
                data.clear();
                if (logEnabled) log += addTimeTag QString("Get device real time data");
                get(Socket, frame, data, 1000);
                if (!data.isEmpty())
                {
                    mbus_frame reply;
                    //mbus_frame_data frame_data;
                    if (mbus_parse(reply, data)) return;
                    if (logEnabled)
                    {
                        QString logtxt;
                        for (unsigned int n=0; n<reply.data_size; n++) logtxt += QString("[%1]").arg((unsigned char)reply.data[n]);
                        log += addTimeTag "Real time data after parsing : " + logtxt;
                    }
                    mbus_frame_data_parse(reply, frame_data, adr);
                }
                for (int n=0; n<frame_data.data_var.records.count(); n++) delete frame_data.data_var.records.at(n);
            }
            else
            {
                if (logEnabled) log += addTimeTag "Unknown answer, may be data collision";
            }
        }
    }
}






int mbusthread::mbus_frame_pack(mbus_frame &frame, QByteArray &data)
{
    size_t i; //, offset = 0;
    if (mbus_frame_calc_length(frame) == -1) return -2;
    if (mbus_frame_calc_checksum(frame) == -1) return -3;
    switch (frame.type)
    {
        case MBUS_FRAME_TYPE_ACK:
            data.append(frame.start1);
            return data.count();

        case MBUS_FRAME_TYPE_SHORT:
            data.append(frame.start1);
            data.append(frame.control);
            data.append(frame.address);
            data.append(frame.checksum);
            data.append(frame.stop);
            return data.count();

        case MBUS_FRAME_TYPE_CONTROL:
            data.append(frame.start1);
            data.append(frame.length1);
            data.append(frame.length2);
            data.append(frame.start2);

            data.append(frame.control);
            data.append(frame.address);
            data.append(frame.control_information);

            data.append(frame.checksum);
            data.append(frame.stop);
            return data.count();

        case MBUS_FRAME_TYPE_LONG:
            data.append(frame.start1);
            data.append(frame.length1);
            data.append(frame.length2);
            data.append(frame.start2);

            data.append(frame.control);
            data.append(frame.address);
            data.append(frame.control_information);

            for (i = 0; i < frame.data_size; i++)
            {
                data.append(frame.data[i]);
            }

            data.append(frame.checksum);
            data.append(frame.stop);
            return data.count();
    default:
        return -5;
    }
    return -1;
}





int mbusthread::mbus_frame_calc_length(mbus_frame &frame)
{
    frame.length1 = frame.length2 = calc_length(frame);
    return 0;
}





quint8 mbusthread::calc_length(const mbus_frame &frame)
{
    switch(frame.type)
    {
    case MBUS_FRAME_TYPE_CONTROL:
        return 3;
    case MBUS_FRAME_TYPE_LONG:
        return frame.data_size + 3;
    default:
        return 0;
    }
}




int mbusthread::mbus_frame_calc_checksum(mbus_frame &frame)
{
    switch (frame.type)
    {
        case MBUS_FRAME_TYPE_ACK:
        case MBUS_FRAME_TYPE_SHORT:
        case MBUS_FRAME_TYPE_CONTROL:
        case MBUS_FRAME_TYPE_LONG:
        frame.checksum = calc_checksum(frame);
        break;
        default:
        return -1;
    }
    return 0;
}




quint8 mbusthread::calc_checksum(mbus_frame &frame)
{
    size_t i;
    quint8 cksum;
    switch(frame.type)
    {
        case MBUS_FRAME_TYPE_SHORT:
            cksum = frame.control;
            cksum += frame.address;
            break;

        case MBUS_FRAME_TYPE_CONTROL:
            cksum = frame.control;
            cksum += frame.address;
            cksum += frame.control_information;
            break;

        case MBUS_FRAME_TYPE_LONG:
            cksum = frame.control;
            cksum += frame.address;
            cksum += frame.control_information;
            for (i = 0; i < frame.data_size; i++)
                cksum += frame.data[i];
            break;
        case MBUS_FRAME_TYPE_ACK:
        default:
            cksum = 0;
    }
    return cksum;
}




void mbusthread::get(QTcpSocket &Socket, mbus_frame &frame, QByteArray &Data, int timeout)
{
    if (Socket.state() == QAbstractSocket::ConnectedState)
    {
        QByteArray Request;
        if (mbus_frame_pack(frame, Request) == -1) return;
        QString logtxt;
        for (int n=0; n<Request.length(); n++) logtxt += QString("[%1]").arg((unsigned char)Request[n]);
        if (logEnabled) log += addTimeTag "Request : " + logtxt;
        if (Socket.isValid()) Socket.write(Request); else return;
        if (Socket.waitForBytesWritten())
        {
            //msleep(200);
            int d = 0;
            while (Socket.waitForReadyRead(timeout))
            {
                d++;
                Data.append(Socket.readAll());
            }
            if (d)
            {
                if (logEnabled)
                {
                    logtxt.clear();
                    for (int n=0; n<Data.length(); n++) logtxt += QString("[%1]").arg((unsigned char)Data[n]);
                    log += addTimeTag "Read Raw : " + logtxt;
                }
            }
            else
            {
                    if (logEnabled) log += addTimeTag "No Data";
            }
        }
        else
        {
            if (logEnabled) log += addTimeTag "Error writing data : ";
        }
    }
    else
    {
        QString msg = "Socket error before writing, reconnect, request = ";
        TCPconnect(Socket, msg);
    }
}





int mbusthread::mbus_parse(mbus_frame &frame, QByteArray &data)
{
    if (logEnabled) log += addTimeTag "mbus_parse";
    size_t i, len;
    unsigned int data_size = data.size();
    if (data_size == 0)
    {
        if (logEnabled) log += addTimeTag "mbus_parse has no data";
        return -1;
    }
    switch (data.at(0))
    {
        case MBUS_FRAME_ACK_START:
        // OK, got a valid ack frame, require no more data
        frame.start1 = data.at(0);
        frame.type = MBUS_FRAME_TYPE_ACK;
        if (logEnabled) log += addTimeTag "OK, got a valid ack frame, require no more data";
        return 0;

        case MBUS_FRAME_SHORT_START:
        if (data_size < (unsigned int)MBUS_FRAME_BASE_SIZE_SHORT)
        {
            // OK, got a valid short packet start, but we need more data
            if (logEnabled) log += addTimeTag "OK, got a valid short packet start, but we need more data";
            return MBUS_FRAME_BASE_SIZE_SHORT - data_size;
        }
        if (data_size != (unsigned int)MBUS_FRAME_BASE_SIZE_SHORT)
        {
            if (logEnabled) log += addTimeTag "too much data... ?";
            // too much data... ?
            return -2;
        }
        // init frame data structure
        frame.start1   = data.at(0);
        frame.control  = data.at(1);
        frame.address  = data.at(2);
        frame.checksum = data.at(3);
        frame.stop     = data.at(4);
        frame.type = MBUS_FRAME_TYPE_SHORT;
        // verify the frame
        if (mbus_frame_verify(frame) != 0)
        {
            return -3;
        }
        // successfully parsed data
        if (logEnabled) log += addTimeTag "successfully parsed data";
        return 0;

        case MBUS_FRAME_LONG_START: // (also CONTROL)
        if (data_size < 3)
        {
            // OK, got a valid long/control packet start, but we need
            // more data to determine the length
            return 3 - data_size;
        }
        // init frame data structure
        frame.start1   = data.at(0);
        frame.length1  = data.at(1);
        frame.length2  = data.at(2);
        if (frame.length1 != frame.length2)
        {
            // not a valid M-bus frame
            return -2;
        }
        // check length of packet:
        len = frame.length1;
        if (data_size < size_t(MBUS_FRAME_FIXED_SIZE_LONG + len))
        {
            // OK, but we need more data
            return MBUS_FRAME_FIXED_SIZE_LONG + len - data_size;
        }
        // we got the whole packet, continue parsing
        frame.start2   = data.at(3);
        frame.control  = data.at(4);
        frame.address  = data.at(5);
        frame.control_information = data.at(6);
        frame.data_size = len - 3;
        for (i = 0; i < frame.data_size; i++)
        {
            frame.data[i] = data.at(7 + i);
        }
        frame.checksum = data.at(data_size-2); // data[6 + frame.data_size + 1]
        frame.stop     = data.at(data_size-1); // data[6 + frame.data_size + 2]
        if (frame.data_size == 0)
        {
            frame.type = MBUS_FRAME_TYPE_CONTROL;
        }
        else
        {
            frame.type = MBUS_FRAME_TYPE_LONG;
        }
        // verify the frame
        if (mbus_frame_verify(frame) != 0)
        {
            return -3;
        }
        // successfully parsed data
        return 0;
        default:
        // not a valid M-Bus frame header (start byte)
        return -4;
    }
}





int mbusthread::mbus_frame_verify(mbus_frame &frame)
{
    switch (frame.type)
    {
        case MBUS_FRAME_TYPE_ACK:
            return frame.start1 == (quint8)MBUS_FRAME_ACK_START;

        case MBUS_FRAME_TYPE_SHORT:
            if(frame.start1 != MBUS_FRAME_SHORT_START)
            return -1;
            break;

        case MBUS_FRAME_TYPE_CONTROL:
        case MBUS_FRAME_TYPE_LONG:
            if(frame.start1  != MBUS_FRAME_CONTROL_START ||
            frame.start2  != MBUS_FRAME_CONTROL_START ||
            frame.length1 != frame.length2 ||
            frame.length1 != calc_length(frame))
            return -1;
            break;

        default:
        return -1;
    }
    if (frame.stop != MBUS_FRAME_STOP || frame.checksum != calc_checksum(frame)) return -1;
    return 0;
}





int mbusthread::mbus_frame_data_parse(mbus_frame &frame, mbus_frame_data &data, int adr)
{
    if (frame.control_information == MBUS_CONTROL_INFO_RESP_FIXED)
    {
        data.type = MBUS_DATA_TYPE_FIXED;
        return mbus_data_fixed_parse(frame, data.data_fix, adr);
    }

    if (frame.control_information == MBUS_CONTROL_INFO_RESP_VARIABLE)
    {
        data.type = MBUS_DATA_TYPE_VARIABLE;
        int r = mbus_data_variable_parse(frame, data.data_var, adr);
        //for (int n=0; n<data.data_var.records.count(); n++) delete data.data_var.records.at(n);
        return r;
    }
    return -1;
}




void mbusthread::mbus_data_variable_print(mbus_data_variable &data, int adr)
{
    mbus_data_variable_header_print(data.header, adr);
// Variable data block HEADER is made of
//	DIB Data Information BLock made of
//		DIF : 1 byte
//		DIFE : 0-10 byte
//	VIB Value Information BLock made of
//		VIF : 1 byte
//		VIFE : 0-10 byte

// DIF is made of 8 bits
//	bit 7 : Extension
//	bit 6 : LSB of storage number
//	bit 5-4 : Function
//		00	Instantaneous value
//		01	Maximum value
//		10	Minimum value
//		11	Value during error state
//	bit 3-0 : data field type
//		0000	No data			0 Byte
//		1000	Selection for Readout	0 Byte
//		0001	8 Bit Integer		1 Byte
//		1001	2 digit BCD		1 Byte
//		0010	16 Bit Integer		2 Bytes
//		1010	4 digit BCD		2 Bytes
//		0011	24 Bit Integer		3 Bytes
//		1011	6 digit BCD		3 Bytes
//		0100	32 Bit Integer		4 Bytes
//		1100	8 digit BCD		4 Bytes
//		0101	32 Bit Real		4 Bytes
//		1101	variable length		N Bytes length is given byte the first byte of DIFE data called LVAR
//		0110	48 Bit Integer		6 Bytes
//		1110	12 digit BCD		6 Bytes
//		0111	64 Bit Integer		8 Bytes
//		1111	Special Functions	8 Bytes

//	LVAR = DIFE[0]     type (1101)
//	LVAR = 00h .. BFh : ASCII string with LVAR characters
//	LVAR = C0h .. CFh : positive BCD number with (LVAR - C0h) Â· 2 digits
//	LVAR = D0h .. DFH : negative BCD number with (LVAR - D0h) Â· 2 digits
//	LVAR = E0h .. EFh : binary number with (LVAR - E0h) bytes
//	LVAR = F0h .. FAh : floating point number with (LVAR - F0h) bytes [to be defined]
//	LVAR = FBh .. FFh : Reserved

//	Special function (1111) DIFE[0] =
//	0Fh	Start of manufacturer specific data structures to end of user data
//	1Fh	Same meaning as DIF = 0Fh + More records follow in next telegram
//	2Fh	Idle Filler (not to be interpreted), following byte = DIF
//	3Fh..6Fh	Reserved
//	7Fh	Global readout request (all storage#, units, tariffs, function fields)
    for (int n=0; n<data.records.count(); n++)
    {
        QString value = mbus_data_record_decode(data.records.at(n));
        QString unit = mbus_vib_unit_lookup(data.records.at(n)->drh.vib);
        QString comment = mbus_data_type(data.records.at(n)) + "  ";
        for (unsigned int i=0; i<data.records.at(n)->data_len; i++) comment += QString("[%1]").arg((unsigned char)data.records.at(n)->data[i], 2, 16, QChar('0')).toUpper();
        addTreeItem(adr, n, unit, value, comment, true);

    /*	QString dif = QString("%1").arg(data.records.at(n)->drh.dib.dif, 2, 16, QChar('0')).toUpper();
        QString D = "DIF";
        QString E = " ";
        addTreeItem(adr, n, D, dif, E);*/

    /*	for (unsigned int i=0; i<data.records.at(n)->drh.dib.ndife; i++)
        {
            if (i < difeMax)
            {
                QString dife = QString("%1").arg(data.records.at(n)->drh.dib.dife[i], 2, 16, QChar('0')).toUpper();
                QString P = QString("DIFE%1").arg(i+1);
                addTreeItem(adr, n, P, dife, E);
            }
            else
            {
                QString filename = "MBus_difeMax.txt";
                QFile file(filename);
                QTextStream out(&file);
                file.open(QIODevice::Append | QIODevice::Text);
                QString log = QString("nDIFE = %1\n").arg(data.records.at(n)->drh.dib.ndife);
                out << log;
                file.close();
            }
        }*/

/*		QString vif = QString("%1").arg(data.records.at(n)->drh.vib.vif, 2, 16, QChar('0')).toUpper();
        QString V = "VIF";
        addTreeItem(adr, n, V, vif, E);*/

/*		for (unsigned int i=0; i<data.records.at(n)->drh.vib.nvife; i++)
        {
            if (i < vifeMax)
            {
                QString vife = QString("%1").arg(data.records.at(n)->drh.vib.vife[i], 2, 16, QChar('0')).toUpper();
                QString P = QString("VIFE%1").arg(i+1);
                addTreeItem(adr, n, P, vife, E);
            }
        }
*/
//		log += addTimeTag "DIF Data Length = " + QString("%1").arg(data.records.at(n)->drh.dib.ndife);
//		log += addTimeTag "VIF Length = " + QString("%1").arg(data.records.at(n)->drh.vib.vif & 0x07);
//		log += addTimeTag "VIF Data Length = " + QString("%1").arg(data.records.at(n)->drh.vib.nvife);

        //log += addTimeTag "VIB = " + mbus_vib_unit_lookup(data.records.at(n)->drh.vib);
        // DIF
        //log += addTimeTag "DIF = " + QString("%1").arg(data.records.at(n)->drh.dib.dif);
        //log += addTimeTag "DIF.Extension = ";
        //log += (data.records.at(n)->drh.dib.dif & MBUS_DIB_DIF_EXTENSION_BIT) ? "Yes" : "No";
        //log += addTimeTag "DIF.Function = ";
        //log += (data.records.at(n)->drh.dib.dif & 0x30) ? "Minimum value" : "Instantaneous value";
        //log += addTimeTag "DIF.Data = " + QString("%1").arg(data.records.at(n)->drh.dib.dif & 0x0F);

        // VENDOR SPECIFIC
        //if (data.records.at(n)->drh.dib.dif == 0x0F || data.records.at(n)->drh.dib.dif == 0x1F) //MBUS_DIB_DIF_VENDOR_SPECIFIC)
        //{
        //	log += addTimeTag "VENDOR DATA = ";
        //	for (unsigned int j=0; j<data.records.at(n)->data_len; j++)
        //		log += QString("[%1]").arg((unsigned char)data.records.at(n)->data[j]);
        //	continue;
        //}

        // calculate length of data record
        //log += addTimeTag QString("DATA LENGTH = %1").arg(data.records.at(n)->data_len);

        // DIFE
        //for (unsigned int j=0; j<data.records.at(n)->drh.dib.ndife; j++)
        //{
        //	quint8 dife = data.records.at(n)->drh.dib.dife[j];
        //	log += addTimeTag "DIFE = " + QString("%1").arg(dife);
        //	log += addTimeTag "DIFE.Extension = ";
        //	log += (dife & MBUS_DIB_DIF_EXTENSION_BIT) ? "Yes" : "No";
        //	log += addTimeTag "DIFE.Function = ";
        //	log += (dife & 0x30) ? "Minimum value" : "Instantaneous value";
        //	log += addTimeTag "DIFE.Data = " + QString("%1").arg(dife & 0x0F);
        //}
        // VIB
        //log += addTimeTag "VIF = " + mbus_vif_unit_lookup(data.records.at(n)->drh.vib.vif);
    }
}





void mbusthread::mbus_data_variable_header_print(mbus_data_variable_header &header, int adr)
{
    QString ID = QString("%1").arg((unsigned char)header.id_bcd[3]);
    ID += QString("%1").arg((unsigned char)header.id_bcd[2]);
    ID += QString("%1").arg((unsigned char)header.id_bcd[1]);
    ID += QString("%1").arg((unsigned char)header.id_bcd[0]);
    QString E = " ";
    QString I = "ID";
    addTreeItem(adr, I, ID, E);

    QString manufacturer = QString("%1").arg((unsigned char)header.manufacturer[1]);
    manufacturer += QString("%1").arg((unsigned char)header.manufacturer[0]);
    QString T = tr("Manufacturer");
    addTreeItem(adr, T, manufacturer, E);

    QString version = QString("%1").arg((unsigned char)header.version);
    T = tr("Version");
    addTreeItem(adr, T, version, E);

    QString medium = QString("%1").arg((unsigned char)header.medium);
    T = tr("Medium");
    QString M = medium + " " + mbus_data_variable_medium_lookup(header.medium);
    addTreeItem(adr, T, M, E);

    QString access = QString("%1").arg((unsigned char)header.access_no);
    T = tr("Access #");
    addTreeItem(adr, T, access, E);

    QString status = QString("%1").arg((unsigned char)header.status);
    T = tr("Status");
    addTreeItem(adr, T, status, E);

    QString signature = QString("%1").arg((unsigned char)header.signature[1], 2, 16, QChar('0')).toUpper();
    signature += QString("%1").arg((unsigned char)header.signature[0], 2, 16, QChar('0')).toUpper();
    T = tr("Signature");
    addTreeItem(adr, T, signature, E);
}




const quint8 *mbusthread::mbus_decode_manufacturer(quint8 byte1, quint8 byte2)
{
    static quint8 m_str[4];

    int m_id;

    m_str[0] = byte1;
    m_str[1] = byte2;

    m_id = mbus_data_int_decode(m_str, 2);

    m_str[0] = (char)(((m_id>>10) & 0x001F) + 64);
    m_str[1] = (char)(((m_id>>5)  & 0x001F) + 64);
    m_str[2] = (char)(((m_id)     & 0x001F) + 64);
    m_str[3] = 0;

    return m_str;
}





QDateTime mbusthread::mbus_data_F_Format(quint8 *int_data)
{
//	first byte	         second byte
//	0 0 n5 n4 n3 n2 n1 n0    0 0 0 h4 h3 h2 h1 h0

//	h4..h0 code the hour (0..23), n5..n0 code the minute (0..59).
//	third byte        fourth byte
//	byte1 format G    byte2 format G
    int hh = int_data[0] & 0x3F;
    int mm = int_data[1] & 0x1F;
    int j = int_data[2] & 0x1F;
    int m = int_data[3] & 0x0F;
    int a = 2000 + ((int_data[2] & 0xE0) >> 5) + ((int_data[3] & 0xF0) >> 1);
    return QDateTime(QDate(a, m, j), QTime(hh, mm));
}





QDate mbusthread::mbus_data_G_Format(quint8 *int_data)
{
//	first byte	           second byte
//	a2 a1 a0 j4 j3 j2 j1 j0    a6 a5 a4 a3 M3 M2 M1 M0

//	j4..j0 code the day (1..31)
//	M3..M0 code the month (1..12)
//	a6..a0 code the year (0..99)
    int j = int_data[0] & 0x1F;
    int m = int_data[1] & 0x0F;
    int a = 2000 + ((int_data[0] & 0xE0) >> 5) + ((int_data[1] & 0xF0) >> 1);
    return QDate(a, m, j);
}




qreal mbusthread::mbus_data_float_decode(quint8 *int_data, size_t int_data_size)
{
    qreal val = logisdom::NA;
    long temp = 0, fraction;
    int sign,exponent;
    size_t i;
    if (int_data)
    {
        for (i = int_data_size; i > 0; i--)
        {
            temp = (temp << 8) + int_data[i-1];
        }
        // first bit = sign bit
        sign = (temp >> 31) ? -1 : 1;

        // decode 8 bit exponent
        exponent = ((temp & 0x7F800000) >> 23) - 127;

        // decode explicit 23 bit fraction
        fraction = temp & 0x007FFFFF;

        if ((exponent != -127) &&
            (exponent != 128))
        {
            // normalized value, add bit 24
            fraction |= 0x800000;
        }

        // calculate float value
        val = (qreal) sign * fraction * pow(2.0f, -23.0f + exponent);
    }
    return val;
}



quint32 mbusthread::mbus_data_int_decode(quint8 *int_data, size_t int_data_size)
{
    quint32 val = 0;
    size_t i;
    if (int_data)
    {
    for (i = int_data_size; i > 0; i--)
    {
        val = (val << 8) + int_data[i-1];
    }
    return val;
    }
    return -1;
}



quint64 mbusthread::mbus_data_long_decode(quint8 *int_data, size_t int_data_size)
{
    quint64 val = 0;
    size_t i;

    if (int_data)
    {
    for (i = int_data_size; i > 0; i--)
    {
        val = (val << 8) + int_data[i-1];
    }
    return val;
    }
    return -1;
}




void mbusthread::mbus_data_str_decode(QString &dst, mbus_data_record *record)
{
    for (quint16 n=0; n<record->data_len; n++) dst.append(QChar((char)record->data[n]));
/*    size_t i;
    record->data
    i = 0;
    dst[len] = '\0';
    while(len > 0)
    {
    dst[i++] = src[--len];
    }
    int LVAR = src[0];
    if ((LVAR >= 0) && (LVAR <= 0xBF))	// ASCII string with LVAR characters
    {
        log += addTimeTag QString("LVAR = %1").arg(LVAR);
        while(LVAR > 0) dst.append(QChar((char)src[--LVAR]));
    }
    else if ((LVAR >= 0xC0) && (LVAR <= 0xCF))	// positive BCD number with (LVAR - C0h) Â· 2 digits
    {
        dst ="Positive BCD number";
    }
    else if ((LVAR >= 0xD0) && (LVAR <= 0xDF))	// negative BCD number with (LVAR - D0h) Â· 2 digits
    {
        dst ="Negative BCD number";
    }
    else if ((LVAR >= 0xE0) && (LVAR <= 0xEF))	// binary number with (LVAR - E0h) bytes
    {
        dst ="Binary number";
    }
    else if ((LVAR >= 0xF0) && (LVAR <= 0xFA))	// floating point number with (LVAR - F0h) bytes [to be defined]
    {
        dst ="Floating point number";
    }*/
}



QString mbusthread::mbus_data_variable_medium_lookup(quint8 medium)
{
    switch (medium)
    {
        case MBUS_VARIABLE_DATA_MEDIUM_OTHER:		return tr("Other");
        case MBUS_VARIABLE_DATA_MEDIUM_OIL:		return tr("Oil");
        case MBUS_VARIABLE_DATA_MEDIUM_ELECTRICITY:	return tr("Electricity");
        case MBUS_VARIABLE_DATA_MEDIUM_GAS:		return tr("Gas");
        case MBUS_VARIABLE_DATA_MEDIUM_HEAT:		return tr("Heat");
        case MBUS_VARIABLE_DATA_MEDIUM_STEAM:		return tr("Steam");
        case MBUS_VARIABLE_DATA_MEDIUM_HOT_WATER:	return tr("Hot water");
        case MBUS_VARIABLE_DATA_MEDIUM_WATER:		return tr("Water");
        case MBUS_VARIABLE_DATA_MEDIUM_HEAT_COST:	return tr("Heat Cost Allocator");
        case MBUS_VARIABLE_DATA_MEDIUM_COMPR_AIR:	return tr("Compressed Air");
        case MBUS_VARIABLE_DATA_MEDIUM_COOL_OUT:	return tr("Cooling load meter: Outlet");
        case MBUS_VARIABLE_DATA_MEDIUM_COOL_IN:		return tr("Cooling load meter: Inlet");
        case MBUS_VARIABLE_DATA_MEDIUM_BUS:		return tr("Bus/System");
        case MBUS_VARIABLE_DATA_MEDIUM_COLD_WATER:	return tr("Cold water");
        case MBUS_VARIABLE_DATA_MEDIUM_DUAL_WATER:	return tr("Dual water");
        case MBUS_VARIABLE_DATA_MEDIUM_PRESSURE:	return tr("Pressure");
        case MBUS_VARIABLE_DATA_MEDIUM_ADC:		return tr("A/D Converter");
        case 0x0C:					return tr("Heat (Volume measured at flow temperature: inlet)");
        case 0x20: // - 0xFF				return tr("Reserved");
        default:					return tr("Unknown medium") + QString(" 0x%1").arg(medium, 2, 16, QChar('0')).toUpper();
    }
}



QString mbusthread::mbus_data_fixed_medium(mbus_data_fixed &data)
{
    int medium = (data.cnt1_type&0xC0)>>4 | (data.cnt2_type&0xC0)>>6;
    switch (medium)
    {
        case 0x00: return tr("Other");
        case 0x01: return tr("Oil");
        case 0x02: return tr("Electricity");
        case 0x03: return tr("Gas");
        case 0x04: return tr("Heat");
        case 0x05: return tr("Steam");
        case 0x06: return tr("Hot Water");
        case 0x07: return tr("Water");
        case 0x08: return tr("H.C.A.");
        case 0x09: return tr("Reserved");
        case 0x0A: return tr("Gas Mode 2");
        case 0x0B: return tr("Heat Mode 2");
        case 0x0C: return tr("Hot Water Mode 2");
        case 0x0D: return tr("Water Mode 2");
        case 0x0E: return tr("H.C.A. Mode 2");
        case 0x0F: return tr("Reserved");
        default: return tr("unknown");
    }
}



QString mbusthread::mbus_data_fixed_unit(int medium_unit_byte)
{
    switch (medium_unit_byte & 0x3F)
    {
        case 0x00: return tr("h,m,s");
        case 0x01: return tr("D,M,Y");
        case 0x02: return tr("Wh");
        case 0x03: return tr("10 Wh");
        case 0x04: return tr("100 Wh");
        case 0x05: return tr("kWh");
        case 0x06: return tr("10 kWh");
        case 0x07: return tr("100 kWh");
        case 0x08: return tr("MWh");
        case 0x09: return tr("10 MWh");
        case 0x0A: return tr("100 MWh");
        case 0x0B: return tr("kJ");
        case 0x0C: return tr("10 kJ");
        case 0x0E: return tr("100 kJ");
        case 0x0D: return tr("MJ");
        case 0x0F: return tr("10 MJ");
        case 0x10: return tr("100 MJ");
        case 0x11: return tr("GJ");
        case 0x12: return tr("10 GJ");
        case 0x13: return tr("100 GJ");
        case 0x14: return tr("W");
        case 0x15: return tr("10 W");
        case 0x16: return tr("100 W");
        case 0x17: return tr("kW");
        case 0x18: return tr("10 kW");
        case 0x19: return tr("100kW");
        case 0x1A: return tr("MW");
        case 0x1B: return tr("10 MW");
        case 0x1C: return tr("100 MW");
        case 0x29: return tr("l");
        case 0x2A: return tr("10 l");
        case 0x2B: return tr("100 l");
        case 0x3E: return tr("same but historic");
        default: return tr("unknown");
    }
}



QString mbusthread::mbus_unit_prefix(int exp)
{
    switch (exp)
    {
        case 0:  return "";
        case -3: return "m";
        case -6: return "my";
        case 1:  return "10";
        case 2:  return "100";
        case 3:  return "k";
        case 4:  return "10 k";
        case 5:  return "100 k";
        case 6:  return "M";
        case 9:  return "T";
        default:  return QString("10^%1").arg(exp);
    }
}




QString mbusthread::mbus_vif_unit_lookup(quint8 vif)
{
    int n;
    QString str;
    switch (vif)
    {
        // E000 0nnn Energy 10(nnn-3) W
        case 0x00:
        case 0x00+1:
        case 0x00+2:
        case 0x00+3:
        case 0x00+4:
        case 0x00+5:
        case 0x00+6:
        case 0x00+7:
            n = (vif & 0x07) - 3;
            str = "Energy (" + mbus_unit_prefix(n) + "Wh)";
            break;
        // 0000 1nnn          Energy       10(nnn)J     (0.001kJ to 10000kJ)
        case 0x08:
        case 0x08+1:
        case 0x08+2:
        case 0x08+3:
        case 0x08+4:
        case 0x08+5:
        case 0x08+6:
        case 0x08+7:
            n = (vif & 0x07);
            str = "Energy (" + mbus_unit_prefix(n) + "J)";
            break;
        // E001 1nnn Mass 10(nnn-3) kg 0.001kg to 10000kg
        case 0x18:
        case 0x18+1:
        case 0x18+2:
        case 0x18+3:
        case 0x18+4:
        case 0x18+5:
        case 0x18+6:
        case 0x18+7:
            n = (vif & 0x07);
            str = "Mass (" + mbus_unit_prefix(n-3) + "kg)";
            break;
        // E010 1nnn Power 10(nnn-3) W 0.001W to 10000W
        case 0x28:
        case 0x28+1:
        case 0x28+2:
        case 0x28+3:
        case 0x28+4:
        case 0x28+5:
        case 0x28+6:
        case 0x28+7:
            n = (vif & 0x07);
            str = "Power (" + mbus_unit_prefix(n-3) + "W)";
            break;
            // E011 0nnn Power 10(nnn) J/h 0.001kJ/h to 10000kJ/h
        case 0x30:
        case 0x30+1:
        case 0x30+2:
        case 0x30+3:
        case 0x30+4:
        case 0x30+5:
        case 0x30+6:
        case 0x30+7:
            n = (vif & 0x07);
            str = "Power (" + mbus_unit_prefix(n) + "J/h)";
            break;
        // E001 0nnn Volume 10(nnn-6) m3 0.001l to 10000l
        case 0x10:
        case 0x10+1:
        case 0x10+2:
        case 0x10+3:
        case 0x10+4:
        case 0x10+5:
        case 0x10+6:
        case 0x10+7:
            n = (vif & 0x07);
            str = "Volume (" + mbus_unit_prefix(n-6) + "m^3)";
            break;
        // E011 1nnn Volume Flow 10(nnn-6) m3/h 0.001l/h to 10000l/
        case 0x38:
        case 0x38+1:
        case 0x38+2:
        case 0x38+3:
        case 0x38+4:
        case 0x38+5:
        case 0x38+6:
        case 0x38+7:
            n = (vif & 0x07);
            str = "Volume flow (" + mbus_unit_prefix(n-6) + "m^3/h)";
            break;
        // E100 0nnn Volume Flow ext. 10(nnn-7) m3/min 0.0001l/min to 1000l/min
        case 0x40:
        case 0x40+1:
        case 0x40+2:
        case 0x40+3:
        case 0x40+4:
        case 0x40+5:
        case 0x40+6:
        case 0x40+7:
            n = (vif & 0x07);
            str = "Volume (" + mbus_unit_prefix(n-7) + "m^3/min)";
            break;
        // E100 1nnn Volume Flow ext. 10(nnn-9) m3/s 0.001ml/s to 10000ml/
        case 0x48:
        case 0x48+1:
        case 0x48+2:
        case 0x48+3:
        case 0x48+4:
        case 0x48+5:
        case 0x48+6:
        case 0x48+7:
            n = (vif & 0x07);
            str = "Volume (" + mbus_unit_prefix(n-9) + "m^3/s)";
            break;
        // E101 0nnn Mass flow 10(nnn-3) kg/h 0.001kg/h to 10000kg/
        case 0x50:
        case 0x50+1:
        case 0x50+2:
        case 0x50+3:
        case 0x50+4:
        case 0x50+5:
        case 0x50+6:
        case 0x50+7:
            n = (vif & 0x07);
            str = "Mass flow  (" + mbus_unit_prefix(n-3) + "kg/h)";
            break;
        // E101 10nn Flow Temperature 10(nn-3) Â°C 0.001Â°C to 1Â°C
        case 0x58:
        case 0x58+1:
        case 0x58+2:
        case 0x58+3:
            n = (vif & 0x03);
            str = "Flow temperature (" + mbus_unit_prefix(n-3) + "deg C)";
            break;
        // E101 11nn Return Temperature 10(nn-3) Â°C 0.001Â°C to 1Â°C
        case 0x5C:
        case 0x5C+1:
        case 0x5C+2:
        case 0x5C+3:
            n = (vif & 0x03);
            str = "Return temperature (" + mbus_unit_prefix(n-3) + "deg C)";
            break;
        // E110 10nn Pressure 10(nn-3) bar 1mbar to 1000mbar
        case 0x68:
        case 0x68+1:
        case 0x68+2:
        case 0x68+3:
            n = (vif & 0x03);
            str = "Pressure (" + mbus_unit_prefix(n-3) + "bar)";
            break;
        // E010 00nn On Time
        // nn = 00 seconds
        // nn = 01 minutes
        // nn = 10   hours
        // nn = 11    days
        // E010 01nn Operating Time coded like OnTime
        case 0x20:
        case 0x20+1:
        case 0x20+2:
        case 0x20+3:
        case 0x24:
        case 0x24+1:
        case 0x24+2:
        case 0x24+3:
            {
                if (vif & 0x4) str += "Operating time ";
                else str += "On time ";
                switch (vif & 0x03)
                {
                    case 0x00: str += "(seconds)"; break;
                    case 0x01: str += "(minutes)"; break;
                    case 0x02: str += "(hours)"; break;
                    case 0x03: str += "(days)"; break;
                }
            }
            break;

        // E110 110n Time Point
        // n = 0        date
        // n = 1 time & date
        // data type G
        // data type F
        case 0x6C:
        case 0x6C+1:

            if (vif & 0x1) str = "Time Point (time & date)";
            else str = "Time Point (date)";
            break;
            // E110 00nn    Temperature Difference   10(nn-3)K   (mK to  K)
        case 0x60:
        case 0x60+1:
        case 0x60+2:
        case 0x60+3:
            n = (vif & 0x03);
            str = "Temperature Difference (" + mbus_unit_prefix(n-3) + "deg C)";
            break;
        // E110 01nn External Temperature 10(nn-3) Â°C 0.001Â°C to 1Â°C
        case 0x64:
        case 0x64+1:
        case 0x64+2:
        case 0x64+3:
            n = (vif & 0x03);
            str = "External temperature (" + mbus_unit_prefix(n-3) + "deg C)";
            break;
        // E110 1110 Units for H.C.A. dimensionless
        case 0x6E:
            str = "Units for H.C.A.";
            break;
        // E110 1111 Reserved
        case 0x6F:
            str = "Reserved";
            break;
        // Custom VIF
        case 0x7C:
            str = "Custom VIF";
            break;
        // Fabrication No
        case 0x78:
            str = "Fabrication number";
            break;
        // Manufacturer specific: 7Fh / FF
        case 0x7F:
        case 0xFF:
            str = "Fabrication specific";
            break;
        default:
            str = tr("Unknown VIF") + QString(" 0x%1").arg(vif, 2, 16, QChar('0')).toUpper();
            break;
    }
    return str;
}





QString mbusthread::mbus_data_type(mbus_data_record *record)
{
    QString str;
    int dif = record->drh.dib.dif & 0x0F;
    switch (dif)
    {
        case 0x00: // no data	break;
        case 0x01: // 1 byte integer (8 bit)
            str = "1 byte integer (8 bit)";
            break;
        case 0x02: // 2 byte integer (16 bit)
            str = "2 byte integer (16 bit)";
            break;
        case 0x03: // 3 byte integer (24 bit)
            str = "3 byte integer (24 bit)";
            break;
        case 0x04: // 4 byte integer (32 bit)
            str = "4 byte integer (32 bit)";
            break;
        case 0x05: // 4 byte real (32 bit)
            str = "4 byte real (32 bit)";
            break;
        case 0x06: // 6 byte integer (48 bit)
            str = "6 byte integer (48 bit)";
            break;
        case 0x07: // 8 byte integer (64 bit)
            str = "8 byte integer (64 bit)";
            break;
        case 0x09: // 2 digit BCD (8 bit)
            str = "2 digit BCD (8 bit)";
            break;
        case 0x0A: // 4 digit BCD (16 bit)
            str = "4 digit BCD (16 bit)";
            break;
        case 0x0B: // 6 digit BCD (24 bit)
            str = "6 digit BCD (24 bit)";
            break;
        case 0x0C: // 8 digit BCD (32 bit)
            str = "8 digit BCD (32 bit)";
            break;
        case 0x0D: // variable length
            str = "Variable length";
            break;
        case 0x0E: // 12 digit BCD (48 bit)
            str = "12 digit BCD (48 bit)";
            break;
        case 0x0F: // special functions
            str = "Special functions";
            break;
        default : str = tr("Unknown DIF") + QString(" 0x%1").arg(record->drh.dib.dif & 0x0F, 2, 16, QChar('0')).toUpper();
            break;
        }
    return str;
}





void mbusthread::mbus_data_str_type(QString &dst, const quint8 *src)
{
    int LVAR = src[0];
    if ((LVAR >= 0) && (LVAR <= 0xBF))	// ASCII string with LVAR characters
    {
        dst ="ASCII string with LVAR characters";
    }
    else if ((LVAR >= 0xC0) && (LVAR <= 0xCF))	// positive BCD number with (LVAR - C0h) Â· 2 digits
    {
        dst ="Positive BCD number";
    }
    else if ((LVAR >= 0xD0) && (LVAR <= 0xDF))	// negative BCD number with (LVAR - D0h) Â· 2 digits
    {
        dst ="Negative BCD number";
    }
    else if ((LVAR >= 0xE0) && (LVAR <= 0xEF))	// binary number with (LVAR - E0h) bytes
    {
        dst ="Binary number";
    }
    else if ((LVAR >= 0xF0) && (LVAR <= 0xFA))	// floating point number with (LVAR - F0h) bytes [to be defined]
    {
        dst ="Floating point number";
    }
}




QString mbusthread::mbus_data_record_decode(mbus_data_record *record)
{
    QString str;
    int dif = record->drh.dib.dif & 0x0F;
    switch (dif)
    {
        case 0x00: // no data	break;
        case 0x01: // 1 byte integer (8 bit)
            str = QString("%1").arg(mbus_data_int_decode(record->data, 1));
            break;
        case 0x02: // 2 byte integer (16 bit)
            str = QString("%1   ").arg(mbus_data_int_decode(record->data, 2));
            //str += mbus_data_G_Format(record->data).toString("dd-MM-yyyy");
            break;
        case 0x03: // 3 byte integer (24 bit)
            str = QString("%1").arg(mbus_data_int_decode(record->data, 3));
            break;
        case 0x04: // 4 byte integer (32 bit)
            str = QString("%1   ").arg(mbus_data_int_decode(record->data, 4));
            //str += mbus_data_F_Format(record->data).toString("dd-MM-yyyy hh:mm");
            break;
        case 0x05: // 4 byte real (32 bit)
            str = QString("%1   ").arg(mbus_data_float_decode(record->data, 4));
            //str += mbus_data_F_Format(record->data).toString("dd-MM-yyyy hh:mm");
            break;
        case 0x06: // 6 byte integer (48 bit)
            str = QString("%1").arg(mbus_data_long_decode(record->data, 6));
            break;
        case 0x07: // 8 byte integer (64 bit)
            str = QString("%1").arg(mbus_data_long_decode(record->data, 8));
            break;
        case 0x09: // 2 digit BCD (8 bit)
            str = QString("%1").arg(mbus_data_bcd_decode(record->data, 1));
            break;
        case 0x0A: // 4 digit BCD (16 bit)
            str = QString("%1").arg(mbus_data_bcd_decode(record->data, 2));
            break;
        case 0x0B: // 6 digit BCD (24 bit)
            str = QString("%1").arg(mbus_data_bcd_decode(record->data, 3));
            break;
        case 0x0C: // 8 digit BCD (32 bit)
            str = QString("%1").arg(mbus_data_bcd_decode(record->data, 4));
            break;
        case 0x0D: // variable length
            if(record->data_len <= 0xBF) mbus_data_str_decode(str, record);
            break;
        case 0x0E: // 12 digit BCD (48 bit)
            str = QString("%1").arg(mbus_data_bcd_decode(record->data, 6));
           break;
        case 0x0F: // special functions
            //mbus_data_bin_decode(buff, record->data, record->data_len, sizeof(buff));
            break;
        default: str = tr("Unknown DIF") + QString(" 0x%1").arg(record->drh.dib.dif & 0x0F, 2, 16, QChar('0')).toUpper();
            break;
    }
    return str;
}



QString mbusthread::mbus_vib_unit_lookup(mbus_value_information_block &vib)
{
    QString str;
    if (vib.vif == (quint8)0xFD) // first type of VIF extention: see table 8.4.4
    {
        if (vib.nvife == 0) str = "Missing VIF extension";
        else if (vib.vife[0] == 0x08 || vib.vife[0] == 0x88)
        {
            // E000 1000
            str = "Access Number (transmission count)";
        }
        else if (vib.vife[0] == 0x09|| vib.vife[0] == 0x89)
        {
            // E000 1001
            str = "Medium (as in fixed header)";
        }
        else if (vib.vife[0] == 0x0A || vib.vife[0] == 0x8A)
        {
            // E000 1010
            str = "Manufacturer (as in fixed header)";
        }
        else if (vib.vife[0] == 0x0B || vib.vife[0] == 0x8B)
        {
            // E000 1010
            str = "Parameter set identification";
        }
        else if (vib.vife[0] == 0x0C || vib.vife[0] == 0x8C)
        {
            // E000 1100
            str = "Model / Version";
        }
        else if (vib.vife[0] == 0x0D || vib.vife[0] == 0x8D)
        {
            // E000 1100
            str = "Hardware version";
        }
        else if (vib.vife[0] == 0x0E || vib.vife[0] == 0x8E)
        {
            // E000 1101
            str = "Firmware version";
        }
        else if (vib.vife[0] == 0x0F || vib.vife[0] == 0x8F)
        {
            // E000 1101
            str = "Software version";
        }
        else if (vib.vife[0] == 0x10)
        {
            // VIFE = E001 0000 Customer location
            str = "Customer location";
        }
        else if (vib.vife[0] == 0x11)
        {
            // VIFE = E001 0001 Customer
            str = "Customer";
        }
        else if (vib.vife[0] == 0x16)
        {
            // VIFE = E001 0110 Password
            str = "Password";
        }
        else if (vib.vife[0] == 0x17 || vib.vife[0] == 0x97)
        {
            // VIFE = E001 0111 Error flags
            str = "Error flags";
        }
        else if (vib.vife[0] == 0x1A)
        {
            // VIFE = E001 1010 Digital output (binary)
            str = "Digital output (binary)";
        }
        else if (vib.vife[0] == 0x1B)
        {
            // VIFE = E001 1011 Digital input (binary)
            str = "Digital input (binary)";
        }
        else if ((vib.vife[0] & 0x70) == 0x40)
        {
            // VIFE = E100 nnnn 10^(nnnn-9) V
            int n = (vib.vife[0] & 0x0F);
            str = QString("%1 V").arg(mbus_unit_prefix(n-9));
        }
        else if ((vib.vife[0] & 0x70) == 0x50)
        {
            // VIFE = E101 nnnn 10nnnn-12 A
            int n = (vib.vife[0] & 0x0F);
            str = QString("%1 A").arg(mbus_unit_prefix(n-12));
        }
        else if ((vib.vife[0] & 0xF0) == 0x70)
        {
            // VIFE = E111 nnn Reserved
            str = "Reserved VIF extension";
        }
        else
        {
            str = QString("Unrecongized VIF extension: %1").arg(vib.vife[0]);
        }
        return str;
    }
    else if (vib.vif == 0x7C)
    {
        // custom VIF
        //str = QString("%1").arg(vib.custom_vif);
        return str;
    }
    else if (vib.vif == 0xFC && (vib.vife[0] & 0x78) == 0x70)
    {
        // custom VIF
        //int n = (vib.vife[0] & 0x07);
        //str = QString("%1 %2").arg(mbus_unit_prefix(n-6)).arg(vib.custom_vif);
        return str;
    }
    return mbus_vif_unit_lookup(vib.vif); // no extention, use VIF
}




void mbusthread::mbus_dif_value_lookup(mbus_data_information_block &dib)
{
    qreal value = 0;
    int data_len = dib.dif & 0x07;
    if (dib.dif & 0x08)	// BCD Coding
    {
        if (logEnabled) log += addTimeTag "BCD Coding";
        int coef = 1;
        for (int n=0; n<data_len; n++)
        {
            int H = dib.dife[n] & 0xF0;
            int L = dib.dife[n] & 0x0F;
            value += (L + 10*H) *coef;
            coef *= 100;
        }
    }
    else	// Integer Coding
    {
        if (logEnabled) log += addTimeTag "Integer Coding";
        int coef = 1;
        for (int n=0; n<data_len; n++)
        {
            value += dib.dife[n] * coef;
            coef *= 10;
        }
    }
    dib.value = value;
    if (logEnabled) log += addTimeTag QString("Value = %1").arg(dib.value);
}



qint32 mbusthread::mbus_data_bcd_decode(quint8 *bcd_data, size_t bcd_data_size)
{
    qint32 val = 0;
    size_t i;

    if (bcd_data)
    {
    for (i = bcd_data_size; i > 0; i--)
    {
        val = (val * 10) + ((bcd_data[i-1]>>4) & 0xF);
        val = (val * 10) + ( bcd_data[i-1]     & 0xF);
    }
    return val;
    }
    return -1;
}



int mbusthread::mbus_data_fixed_parse(mbus_frame &frame, mbus_data_fixed &data, int)
{
    if (logEnabled) log += addTimeTag "Data is Fixed size";
    data.id_bcd[0] = frame.data[0];
    data.id_bcd[1] = frame.data[1];
    data.id_bcd[2] = frame.data[2];
    data.id_bcd[3] = frame.data[3];
    data.tx_cnt = frame.data[4];
    data.status = frame.data[5];
    data.cnt1_type = frame.data[6];
    data.cnt2_type = frame.data[7];
    data.cnt1_val[0] = frame.data[8];
    data.cnt1_val[1] = frame.data[9];
    data.cnt1_val[2] = frame.data[10];
    data.cnt1_val[3] = frame.data[11];
    data.cnt2_val[0] = frame.data[12];
    data.cnt2_val[1] = frame.data[13];
    data.cnt2_val[2] = frame.data[14];
    data.cnt2_val[3] = frame.data[15];
    return 1;
}


#define NITEMS(x) (sizeof(x)/sizeof(x[0]))


int mbusthread::mbus_data_variable_parse(mbus_frame &frame, mbus_data_variable &data, int adr)
{
    if (logEnabled) log += addTimeTag "Data is Variable size";
    size_t i, j;

    // parse header
    i = sizeof(mbus_data_variable_header);
    if(frame.data_size < i)	return -1;

    // first copy the variable data fixed header
    data.header.id_bcd[0]       = frame.data[0];
    data.header.id_bcd[1]       = frame.data[1];
    data.header.id_bcd[2]       = frame.data[2];
    data.header.id_bcd[3]       = frame.data[3];
    data.header.manufacturer[0] = frame.data[4];
    data.header.manufacturer[1] = frame.data[5];
    data.header.version         = frame.data[6];
    data.header.medium          = frame.data[7];
    data.header.access_no       = frame.data[8];
    data.header.status          = frame.data[9];
    data.header.signature[0]    = frame.data[10];
    data.header.signature[1]    = frame.data[11];

    while (i < frame.data_size)
    {
        mbus_data_record *record = new mbus_data_record;
        // Skip filler dif=2F
        if ((frame.data[i] & 0xFF) == MBUS_DIB_DIF_IDLE_FILLER)
        {
          i++;
          continue;
        }
        if (record == nullptr)
        {
            if (logEnabled) log += addTimeTag "record == NULL";
            return (-2);
        }
// read and parse DIB
        // DIF
        record->drh.dib.dif = frame.data[i];
        if ((record->drh.dib.dif == MBUS_DIB_DIF_MANUFACTURER_SPECIFIC) || (record->drh.dib.dif == MBUS_DIB_DIF_MORE_RECORDS_FOLLOW))
        {
            i++;
            //log += addTimeTag QString("i++ DIF");
            // just copy the remaining data as it is vendor specific
            record->data_len = frame.data_size - i;
            for (j=0; j<record->data_len; j++)
            {
                record->data[j] = frame.data[i++];
                //log += addTimeTag QString("i++ record->data[j]");
            }
            // append the record and move on to next one
            //data.records.append(record);
            continue;
        }
        // calculate length of data record
        record->data_len = mbus_dif_datalength_lookup(record->drh.dib.dif);

        //log += addTimeTag QString("record->data_len %1").arg(record->data_len);
        // read DIF extensions
        record->drh.dib.ndife = 0;

       while ((frame.data[i] & MBUS_DIB_DIF_EXTENSION_BIT) && (record->drh.dib.ndife < NITEMS(record->drh.dib.dife)))
        //while ((frame.data[i] & MBUS_DIB_DIF_EXTENSION_BIT) && record->drh.dib.ndife < difeMax)
        {
            //log += addTimeTag QString("Extension bit found, frame.data[i] = %1").arg(frame.data[i]);
            quint8 dife = frame.data[i+1];
            record->drh.dib.dife[record->drh.dib.ndife] = dife;
            record->drh.dib.ndife++;
            i++;
            //log += addTimeTag QString("i++ MBUS_DIB_DIF_EXTENSION_BIT");
        }
        i++;
        //log += addTimeTag QString("i++ tout seul");

        // VIF
        record->drh.vib.vif = frame.data[i];
        //log += addTimeTag QString("i++ VIF");
        // VIFE
        record->drh.vib.nvife = 0;
        //while ((frame.data[i] & MBUS_DIB_VIF_EXTENSION_BIT) && (record->drh.vib.nvife < vifeMax))
        while ((frame.data[i] & MBUS_DIB_VIF_EXTENSION_BIT) && (record->drh.vib.nvife < NITEMS(record->drh.vib.vife)))
          {
            quint8 vife = frame.data[i+1];
            record->drh.vib.vife[record->drh.vib.nvife] = vife;
            record->drh.vib.nvife++;
            i++;
            //log += addTimeTag QString("i++ VIFE");
        }
        i++;
        //if (i > frame.data_size)
        //{
            //delete record;
            //log += addTimeTag QString("Premature end of record at DIF");
            //return -1;
        //}
        // calculate data variable length
        if((record->drh.dib.dif & 0x0D) == 0x0D)
        {
            if (frame.data[i] <= (quint8)0xBF)
            {
                record->data_len = frame.data[i++];
                //log += addTimeTag QString("i++ 1");
            }
            else if ((frame.data[i] >= (quint8)0xC0) && (frame.data[i] <= (quint8)0xCF))
            {
                record->data_len = (frame.data[i++] - 0xC0) * 2;
                //log += addTimeTag QString("i++ 2");
            }
            else if ((frame.data[i] >= (quint8)0xD0) && (frame.data[i] <= (quint8)0xDF))
            {
                record->data_len = (frame.data[i++] - 0xD0) * 2;
                //log += addTimeTag QString("i++ 3");
            }
            else if ((frame.data[i] >= (quint8)0xE0) && (frame.data[i] <= (quint8)0xEF))
            {
                record->data_len = frame.data[i++] - 0xE0;
                //log += addTimeTag QString("i++ 4");
            }
            else if ((frame.data[i] >= (quint8)0xF0) && (frame.data[i] <= (quint8)0xFA))
            {
                record->data_len = frame.data[i++] - (quint8)0xF0;
                //log += addTimeTag QString("i++ 5");
            }
        }
        // copy data
        for (j=0; j<record->data_len; j++)
        {
            //log += addTimeTag QString("Copy data %1 = %2").arg(j).arg(frame.data[i]);
            record->data[j] = frame.data[i++];
        }
        // append the record and move on to next one
        data.records.append(record);
    }
    mbus_data_variable_print(data, adr);
    return 0;
}




unsigned char mbusthread::mbus_dif_datalength_lookup(unsigned char dif)
{
    switch (dif & MBUS_DATA_RECORD_DIF_MASK_DATA)
    {
        case 0x0:
            return 0;
        case 0x1:
            return 1;
        case 0x2:
            return 2;
        case 0x3:
            return 3;
        case 0x4:
            return 4;
        case 0x5:
            return 4;
        case 0x6:
            return 6;
        case 0x7:
            return 8;

        case 0x8:
            return 0;
        case 0x9:
            return 1;
        case 0xA:
            return 2;
        case 0xB:
            return 3;
        case 0xC:
            return 4;
        case 0xD:
            // variable data length,
            // data length stored in data field
            return 0;
        case 0xE:
            return 6;
        case 0xF:
            return 8;

        default: // never reached
            return 0x00;

    }
}




void mbusthread::TCPconnect(QTcpSocket &Socket, QString &Status)
{
    if (logEnabled) log += addTimeTag "TCP Connect  " + Status;
    int tryConnect = 1;
    do
    {
        if (logEnabled) log += addTimeTag QString("TCP try connect %1").arg(tryConnect);
        if (Socket.state() == QAbstractSocket::ConnectedState)
        {
            Socket.disconnectFromHost();
            if (logEnabled) log += addTimeTag "Try Disconnect";
            if (Socket.state() == QAbstractSocket::ConnectedState) Socket.waitForDisconnected();
            if (Socket.state() != QAbstractSocket::ConnectedState)
            {
                if (logEnabled) log += addTimeTag "Socket disconnected";
                tcpStatus = Socket.state();
                emit(tcpStatusChange());
            }
            else
            {
                if (logEnabled) log += addTimeTag "Socket failed to disconnect";
            }
        }
        else
        {
            if (logEnabled) log += addTimeTag "Socket not connected";
        }
        tcpStatus = Socket.state();
        emit(tcpStatusChange());
        Socket.connectToHost(moduleipaddress, port);
        if (logEnabled) log += addTimeTag "Try Connect";
        Socket.waitForConnected();
        if (Socket.state() == QAbstractSocket::ConnectedState)
        {
            if (logEnabled) log += addTimeTag "Connect OK";
        }
        tcpStatus = Socket.state();
        emit(tcpStatusChange());
        tryConnect ++;
    }
    while (Socket.state() != QAbstractSocket::ConnectedState);
}





void mbusthread::addTreeItem(int adr, const QString &parameter, const QString &value, const QString &comment)
{
    QString str = QString("Adr %1").arg(adr) + "=" + parameter + "=" + value + "=" + comment;
    emit(setTreeItem(str));
}




void mbusthread::addTreeItem(int adr, int record, const QString &parameter, const QString &value, const QString &comment, bool mainTreeItem)
{
    if (mainTreeItem)
    {
        QString str = QString("Adr %1").arg(adr) + "=" + QString("Dat %1").arg(record) + "=" + parameter + "=" + value + "=" + comment;
        emit(setMainTreeItem(str));
    }
    else
    {
        QString str = QString("Adr %1").arg(adr) + "=" + QString("Dat %1").arg(record) + "=" + parameter + "=" + value + "=" + comment;
        emit(setTreeItem(str));
    }
}





