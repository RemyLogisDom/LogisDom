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





#include <QElapsedTimer>
#include <QTimer>
#include "net1wire.h"
#include "ha7sthread.h"
#include "globalvar.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif




ha7sthread::ha7sthread()
{
	logEnabled = false;
	GlobalConvert = false;
	endLessLoop = true;
    socket = nullptr;
    fileIndex = 0;
 }




ha7sthread::~ha7sthread()
{
    endLessLoop = false;
    if (socket) socket->abort();
}


void ha7sthread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QTcpSocket MySocket;
    socket = &MySocket;
    qintptr sd = MySocket.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
//#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
//#endif
    isRunning = true;
	int minute = -1;
	QDateTime tourneRead, lastTourneRead;
	tourneRead = QDateTime::currentDateTime();
	lastTourneRead = tourneRead;
    QString msg = "Thread starts, first connection";
    TCPconnect(MySocket, msg);
    while (endLessLoop)
    {
// Process FIFOSpecial
        log.clear();
        log += addTimeTag "********************     FIFOSpecial contains   *************************************";
        for (int n=0; n<FIFOSpecial.count(); n++) log += addTimeTag "" + FIFOSpecial.at(n)->Request;
        log += addTimeTag "*************************************************************************************";
        processFIFOSpecial(MySocket);
// Fill FIFO with device readings
        if (FIFO.isEmpty())	// if FIFO is empty add all request otherwise, finish last bunch
        {
            checkSearch(MySocket);
            log += addTimeTag "********************    Fill FIFO with device Reading  *******************************";
            addDeviceReadings();
        }
// *log FIFO content
        for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
        log += addTimeTag "**************************************************************************************";
// Get device reading info
        tourneRead = QDateTime::currentDateTime();
        minute = tourneRead.time().minute();
        processFIFO(MySocket);
        if (FIFO.isEmpty()) // avoid FIFO being aborted by FIFOspecial
        {
            while ((minute == QDateTime::currentDateTime().time().minute()) && endLessLoop)
            {
                if (!FIFOSpecial.isEmpty())
                {
                    processFIFOSpecial(MySocket);
                }
                if (FIFO.isEmpty())
                {
                    QThread::msleep(1000);
                    log += addTimeTag "***************    Fill FIFO with device Reading for reading till next minute  *********************";
                    if (searchDelay == 0) checkSearch(MySocket);
                    addDeviceReadings(false);
                    for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
                    log += addTimeTag "**************************************************************************************";
                }
                processFIFO(MySocket, false);
            }
            if (minute != QDateTime::currentDateTime().time().minute())
            {
                foreach (FIFOStruc *fifo, FIFO) delete fifo;
                FIFO.clear();
            }
        }
        saveLog();
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
    isRunning = false;
}




int ha7sthread::checkDevice(const QString RomID)
{
    bool found = false;
    int ID = -1;
    for (int n=0; n<devices.count(); n++)
    {
        if (devices.at(n)->RomID == RomID)
        {
            found = true;
            return n;
        }
    }
    if (!found)
    {
        device *dev = new device;
        dev->RomID = RomID;
        ID = devices.count();
        log += addTimeTag " Found new device : " + RomID + QString("  Index %1").arg(ID);
        devices.append(dev);
        emit(newDevice(devices.last()->RomID));
    }
    return ID;
}





void ha7sthread::TCPconnect(QTcpSocket &Socket, QString &Status)
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




void ha7sthread::get(QTcpSocket &Socket, QString &Request, QString &Data)
{
	Request += "\r";
	Data.clear();
    log += addTimeTag " SEND : " + Request;
	if (Socket.state() == QAbstractSocket::ConnectedState)
	{
        if (Socket.isValid()) Socket.write(Request.toLatin1()); else return;
        if (Socket.waitForBytesWritten())
		{
            QThread::msleep(200);
			if (Socket.waitForReadyRead())
			{
                Data.append(Socket.readAll());
                log += addTimeTag " GET : " + Data;
				int retry = 0;
				while ((!Data.endsWith(CR)) && (retry < 5))
				{
                    log += addTimeTag " DATA not complete wait more : " + Data;
					if (Socket.waitForReadyRead(1000)) Data.append(Socket.readAll());
					retry ++;
				}
				if (Data.endsWith(CR)) Data.chop(1);
                else log += addTimeTag " DATA not complete, abort : " + Data;
			}
			else
			{
				QString msg = "Socket error , reconnect, request = " + Request;
				TCPconnect(Socket, msg);
			}
		}
		else
		{
            log += addTimeTag "Error writing data : " + Request;
		}
	}
	else
	{
		QString msg = "Socket error before writing, reconnect, request = " + Request;
		TCPconnect(Socket, msg);
	}
}





void ha7sthread::readFIFOBuffer(const QString &devicescratchpad)
{
    QString R2 = devicescratchpad.right(2);
    QString R14 = devicescratchpad.right(14);
    QString R18 = devicescratchpad.right(18);
	QString R26 = devicescratchpad.right(26);
	int deviceIndex = FIFO.first()->device_ID;
	switch (FIFO.first()->Request_ID)
	{
		case ConvertTemp	:
                    log += addTimeTag QString(" Wait for convesion : 1 second");
                    QThread::msleep(1000); break;
		case ConvertTempRomID	:
					if ((deviceIndex >= 0) && (deviceIndex < devices.count())) {
                    ulong convTime = getConvertTime(devices.at(deviceIndex));
                    log += addTimeTag QString(" Wait for convesion : %1ms").arg(convTime);
                    QThread::msleep(convTime + 250); }
					break;
        case ReadTemp           : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); } break;
        case ReadPIO            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R26; emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); } break;
        case ReadCounter        : if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                                    {
                                        if (FIFO.first()->scratchpad_ID == 0) devices.at(deviceIndex)->Scratchpad = devicescratchpad;
                                        else if (FIFO.first()->scratchpad_ID == 1) devices.at(deviceIndex)->Scratchpad_1 = devicescratchpad;
                                    } break;
        case ReadLCD            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = devicescratchpad; emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); } break;
        case ReadDualSwitch     : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R2; }	break; // emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); break;
        case ChannelAccessRead	: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R14; } break;
        case ReadADC            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R26; emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); } break;
        case ReadADCPage01h     : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad_1 = "page01h=(" + R26 + ")"; } break;
        case ReadPage           : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; }	break; //	emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); break;
        case ReadPage00h        : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); } break;
        case ReadPage01h        : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad_1 = "page01h=(" + R18 + ")"; }	break; // emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad)); break;
    }
}



void ha7sthread::processFIFOSpecial(QTcpSocket &MySocket)
{
	QString Data;
	while (!FIFOSpecial.isEmpty() && endLessLoop)
	{
		QString Req = FIFOSpecial.first()->Request;
		get(MySocket, Req, Data);
		readFIFOSpecialBuffer(Data);
        QThread::msleep(100);
        mutexData.lock();
		delete FIFOSpecial.first();
		FIFOSpecial.removeFirst();
		mutexData.unlock();
    }
}



void ha7sthread::processFIFO(QTcpSocket &MySocket, bool ALL)
{
    QString Data;
    while (!FIFO.isEmpty() && endLessLoop)
	{
        QString Req = FIFO.first()->Request;
        get(MySocket, Req, Data);
		readFIFOBuffer(Data);
        QThread::msleep(100);
		mutexData.lock();
		delete FIFO.first();
		FIFO.removeFirst();
		mutexData.unlock();
        if (FIFO.isEmpty()) return;
        if ((!FIFOSpecial.isEmpty()) && FIFO.first()->Request.startsWith("A")) return;
        if ((!ALL) && FIFO.first()->Request.startsWith("A")) return;
	}
}



void ha7sthread::checkSearch(QTcpSocket &MySocket)
{
	QString Data;
	QString Req;
	bool search = true;
	if (lastSearch.isValid())
	{
		if (searchDelay == -1) search = false;
		else if (searchDelay == 0) search = true;
		else if (lastSearch.secsTo(QDateTime::currentDateTime()) >= searchDelay)
		{
			lastSearch = QDateTime::currentDateTime();
			search = true;
		}
		else search = false;
	}
	else
	{
		lastSearch = QDateTime::currentDateTime();
	}
	if (search)
	{
		// Search sequence
					Req = "S";
					get(MySocket, Req, Data);
                    //if (Data.length() < 16) { get(MySocket, Req, Data); log += addTimeTag "Retry_1 initial search"; }
                    //if (Data.length() < 16) { get(MySocket, Req, Data); log += addTimeTag "Retry_2 initial search"; }
                    while ((Data.length() >= 16))
					{
						checkDevice(Data.left(16));
						Req = "s";
						get(MySocket, Req, Data);
                        //if (Data.length() < 16) { get(MySocket, Req, Data); log += addTimeTag "Retry_1 next search"; }
                        //if (Data.length() < 16) { get(MySocket, Req, Data); log += addTimeTag "Retry_2 next search"; }
                    }
		// log device listing
                    log += addTimeTag "********************     End of search / Device Listing   ********************************";
                    for (int n=0; n<devices.count(); n++) log += addTimeTag "" + QString("Device_%1 RomID : ").arg(n) + devices.at(n)->RomID + "   Original Scratchpad : " + devices.at(n)->Scratchpad;
                    log += addTimeTag "*************************************************************************************";
	}
	else
	{
		// Log device listing
                    log += addTimeTag "********************     Device Listing   ********************************";
                    for (int n=0; n<devices.count(); n++) log += addTimeTag "" + QString("Device_%1 RomID : ").arg(n) + devices.at(n)->RomID + "   Original Scratchpad : " + devices.at(n)->Scratchpad;
                    log += addTimeTag "*************************************************************************************";
	}
}




void ha7sthread::saveLog()
{
    if (logEnabled)
    {
        log += addTimeTag "*******************************    Device Scratchpad  **********************************";
        for (int n=0; n<devices.count(); n++) log += addTimeTag "" + devices.at(n)->RomID + "  Scratchpad : " + devices.at(n)->Scratchpad;
        log += addTimeTag "_________________________________FIN__________________________________\r\r";
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
}




void ha7sthread::readFIFOSpecialBuffer(const QString &devicescratchpad)
{
	if (devicescratchpad.isEmpty()) return;
	QString R8 = devicescratchpad.right(8);
    QString R16 = devicescratchpad.right(16);
    int deviceIndex = FIFOSpecial.first()->device_ID;
	switch (FIFOSpecial.first()->Request_ID)
	{
        case WritePIO :
        case setChannelOn :
        case setChannelOff :	if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
						{
							devices.at(deviceIndex)->Return = R8.right(2);
							devices.at(deviceIndex)->Scratchpad = ""; //R8.mid(2, 4);
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Return));
                        }
        break;
        case ChannelAccessWrite	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad = R16;
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad));
                        }
        break;
        case ReadMemoryLCD	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad_1 = devicescratchpad;
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad_1));
                        } break;
        case WriteLed	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad = devicescratchpad;
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad));
                        } break;
        default :
        case WriteMemory	:
        break;
    }
}




QString ha7sthread::getScratchPad(const QString &RomID, int scratchpad_ID)
{
	QMutexLocker locker(&mutexData);
	for (int n=0; n<devices.count(); n++)
	{
		if (devices.at(n)->RomID == RomID)
		{
			if (scratchpad_ID == 0)	return devices.at(n)->Scratchpad;
			if (scratchpad_ID == 1)	return devices.at(n)->Scratchpad_1;
		}
	}
	return "";
}




ulong ha7sthread::getConvertTime(device *dev)
{
	QString family = dev->RomID.right(2);
	if (family == family1820) return 200;
    if ((family == family1822) or (family == family18B20) or (family == family1825))
	{
		if (dev->Scratchpad.isEmpty()) return 750;
		bool ok;
        ulong resolution = dev->Scratchpad.mid(8, 2).toULong(&ok, 16);
		if (!ok) return 750;
        ulong P = resolution & 0x60;
        P /= 32;
        if (P == 3) return 750;
        if (P == 2) return 375;
        if (P == 1) return 190;
        if (P == 0) return 95;
        return 750;
	}
    return 0;
}




void ha7sthread::addDeviceReadings(bool ALL)
{
	if (GlobalConvert)
	{
		FIFOAppend(Reset, "R");
		FIFOAppend(ConvertTemp, "W02CC44");
	}
	for(int index=0; index<devices.count(); index++)
	{
		QString RomID = devices.at(index)->RomID;
		QString family = RomID.right(2);
		if (!GlobalConvert)
		{
			if (getConvertTime(devices.at(index)))
			{
                //FIFOAppend(Reset, "R", index);
				FIFOAppend(MatchRom, "A" + RomID.left(16), index);
				FIFOAppend(ConvertTempRomID, "W0144", index);
			}
		}
        if ((family == family1822) || (family == family1820)  || (family == family18B20) || (family == family1825))
		{
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadTemp, "W0ABEFFFFFFFFFFFFFFFFFF", index);
		}
        else if (family == family2406)
        {
            //FIFOAppend(Reset, "R", index);
            FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadPIO, "W07F545FFFFFFFFFF", index);
        }
        else if (family == family2408)
		{
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadPIO, "W0DF08800FFFFFFFFFFFFFFFFFFFF", index);
		}
        else if (isFamily2413)
		{
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadDualSwitch, "W02F5FF", index);
		}
		else if ((family == family2423) && ALL)
		{
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadCounter, "W0EA5DF01FFFFFFFFFFFFFFFFFFFFFF", index);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadCounter, "W0EA5FF01FFFFFFFFFFFFFFFFFFFFFF", index, 1);
		}
        else if ((family == familyLCDOW))
        {
            //FIFOAppend(Reset, "R", index);
            FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadLCD, "W0EA5DF01FFFFFFFFFFFFFFFFFFFFFF", index);
        }
        else if ((family == familyLedOW))
        {
            //FIFOAppend(Reset, "R", index);
            FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadTemp, "W0ABEFFFFFFFFFFFFFFFFFF", index);
        }
        else if (family == family2438)
		{
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ConvertTempRomID, "W0144", index);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ConvertV, "W01B4", index);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(RecallMemPage00h, "W02B800", index);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadPage00h, "W0BBE00FFFFFFFFFFFFFFFFFF", index);
            FIFOAppend(RecallMemPage01h, "W02B801", index);
            //FIFOAppend(Reset, "R", index);
            FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadPage01h, "W0BBE01FFFFFFFFFFFFFFFFFF", index, 1);
        }
		else if (family == family2450)
		{
            //FIFOAppend(Reset, "R", index);
            FIFOAppend(MatchRom, "A" + RomID.left(16), index);
            FIFOAppend(ReadADCPage01h, "W0DAA0800FFFFFFFFFFFFFFFFFFFF", index, 1);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ConvertADC, "W053C0F00FFFF", index);
            //FIFOAppend(Reset, "R", index);
			FIFOAppend(MatchRom, "A" + RomID.left(16), index);
			FIFOAppend(ReadADC, "W0DAA0000FFFFFFFFFFFFFFFFFFFF", index);
        }
	}
}



void ha7sthread::addToFIFOSpecial(const QString &RomID, const QString &data, int Request_ID)
{
	if (FIFOSpecial.count() > 100) return;
	QString S;
	if (RomID.isEmpty())
	{
		S.clear();
		FIFOSpecialAppend(Request_ID, data, 0);
	}
	else
	{
		for (int index=0; index<devices.count(); index++)
		{
			if (devices.at(index)->RomID == RomID.left(16))
			{
				switch (Request_ID)
				{
					case setChannelOn :
                        if (data.isEmpty()) log += addTimeTag RomID + " setChannelOn empty data";
						else
						{
							S = "W045A" + data + "FF";
                            //FIFOSpecialAppend(Reset, "R", index);
							FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(setChannelOn, S.toUpper(), index);
						}
						break;
					case setChannelOff :
                        if (data.isEmpty()) log += addTimeTag RomID + " setChannelOff empty data";
						else
						{
							S = "W045A" + data + "FF";
                            //FIFOSpecialAppend(Reset, "R", index);
							FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(setChannelOff, S.toUpper(), index);
						}
						break;
                    case WriteScratchpad00 :
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteScratchpad00 empty data";
						else
						{
                            S = QString("W%1").arg(uchar((data.length()/2 + 2)), 2, 16, QChar('0')) + "4E00" + data;
                            //FIFOSpecialAppend(Reset, "R", index);
							FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(WriteScratchpad00, S.toUpper(), index);
						}
						break;
                    case CopyScratchpad00 :
                        //FIFOSpecialAppend(Reset, "R", index);
						FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
						FIFOSpecialAppend(CopyScratchpad00, "W024800", index);
						break;
                    case WriteMemory :
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteMemory empty data";
                        else
                        {
                            S = QString("W%1").arg(uchar((data.length()/2 + 1)), 2, 16, QChar('0')) + "55" + data;
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(WriteMemory, S.toUpper(), index);
                        }
                        break;
                    case WriteValText	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteValText empty data";
                        else
                        {
                            S = QString("W%1").arg(uchar((data.length()/2)), 2, 16, QChar('0')) + data;
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(WriteMemory, S.toUpper(), index);
                        }
                        break;
                    case WriteLed	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteLed empty data";
                        else
                        {
                            S = QString("W%1").arg(uchar((data.length()/2)), 2, 16, QChar('0')) + data;
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(WriteLed, S.toUpper(), index);
                        }
                        break;
                    case ReadMemoryLCD	:
                        if (data.isEmpty()) log += addTimeTag RomID + " ReadMemoryLCD empty data";
                        else
                        {
                            S = QString("W%1").arg(uchar((data.length()/2)), 2, 16, QChar('0')) + data;
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(ReadMemoryLCD, S.toUpper(), index);
                        }
                        break;
                    case WritePIO :
                        if (data.isEmpty()) log += addTimeTag RomID + " WritePIO empty data";
                        else
                        {
                            S = "W045A" + data + "FF";
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(WritePIO, S.toUpper(), index);
                        }
                        break;
                    case ChannelAccessWrite :
                        if (data.isEmpty()) log += addTimeTag RomID + " ChannelAccessWrite empty data";
                        else
                        {
                            S = QString("W%1").arg(uchar((data.length()/2 + 3)), 2, 16, QChar('0')) + "F5" + data + "FFFF";
                            //FIFOSpecialAppend(Reset, "R", index);
                            FIFOSpecialAppend(MatchRom, "A" + RomID.left(16), index);
                            FIFOSpecialAppend(ChannelAccessWrite, S.toUpper(), index);
                        }
                        break;
                }
			}
		}
	}
}





void ha7sthread::addToFIFOSpecial(const QString &data)
{
	if (FIFOSpecial.count() > 100) return;
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->RomID = "";
	newFIFO->Request_ID = -1;
	newFIFO->Request = data;
	newFIFO->scratchpad_ID = -1;
	newFIFO->device_ID = -1;
	mutexData.lock();
	FIFOSpecial.append(newFIFO);
	mutexData.unlock();
}




void ha7sthread::FIFOAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID)
{
	mutexData.lock();
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = reqID;
	newFIFO->Request = reqStr;
	newFIFO->scratchpad_ID = scratchpad_ID;
	if ((device_ID >= 0) && (device_ID < devices.count())) newFIFO->RomID = devices.at(device_ID)->RomID; else newFIFO->RomID = "";
	newFIFO->device_ID = device_ID;
	FIFO.append(newFIFO);
	mutexData.unlock();
}






void ha7sthread::FIFOSpecialAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID)
{
	mutexData.lock();
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = reqID;
	newFIFO->Request = reqStr;
	newFIFO->scratchpad_ID = scratchpad_ID;
	if ((device_ID >= 0) && (device_ID < devices.count())) newFIFO->RomID = devices.at(device_ID)->RomID; else newFIFO->RomID = "";
	newFIFO->device_ID = device_ID;
	FIFOSpecial.append(newFIFO);
	mutexData.unlock();
}






