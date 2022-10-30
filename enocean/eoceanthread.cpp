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
#include "eoceanthread.h"
#include "globalvar.h"
#include <stdio.h>
#include "eoApiDef.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif

eoceanthread::eoceanthread()
{
    logEnabled = false;
	endLessLoop = true;
	fileIndex = 0;
    moveToThread(this);
}




eoceanthread::~eoceanthread()
{
    endLessLoop = false;
}



void eoceanthread::setAdress(const QString &adr)
{
    useTcp = false;
    useSerial = false;
    adress = adr;
    if (adress.split(".").count() == 4) useTcp = true;
    else
    {
        QStringList portList;
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
#ifdef WIN32
        portList.append(info.portName());
#else
        portList.append("/dev/" + info.portName());
#endif
        if (portList.contains(adr)) useSerial = true;
    }
}


void eoceanthread::setPort(int p)
{
    port = p;
}



void eoceanthread::run()
{
#define addTimeTag QDateTime::currentDateTime().toString("dd/MM HH:mm:ss:zzz ")
    endLessLoop = true;
	QElapsedTimer timer;
    timer.start();
    QTcpSocket MySocket;
    socket = &MySocket;
    //MySocket.moveToThread(this);
    QSerialPort MySerial;
    serial = &MySerial;
    //MySerial.moveToThread(this);
    int sd = MySocket.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(sd, SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

    if (!useTcp & !useSerial)
    {
        emit(logThis("IP address or Com Port not defined"));
        return;
    }
    if (useTcp)
    {
        QString msg = "Thread starts, first connection";
        TCPconnect(msg);
        emit(eoStatusChange());
        while (!checkBaseID() && endLessLoop)
        {
            emit(logThis(addTimeTag + " Check base ID fail retry"));
            sleep(1);
        }
    }
    if (useSerial)
    {
        serial->setPortName(adress);
        serial->setBaudRate(56700);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        if (!serial->open(QIODevice::ReadWrite))
        {
            emit(logThis("Failed to open " + adress + " " + serial->errorString()));
            endLessLoop = false;
            return ;
        }
        emit(eoStatusChange());
        emit(logThis(adress + " opened"));
        while (!checkBaseID() && endLessLoop)
        {
            emit(logThis(addTimeTag + " Check base ID fail retry"));
            sleep(1);
        }
    }
    learnMode = false;

    while (endLessLoop)
    {
        if (useTcp)
        {
            if (socket->state() == QAbstractSocket::ConnectedState)
            {
                MySocket.waitForReadyRead(100);
                if (MySocket.bytesAvailable())
                {
                    const QByteArray data = socket->readAll();
                    emit(logThis("Read : " + data.toHex().toUpper()));
                    //emit(logThis(QString("L = %1").arg(data.length())));
                    dataBuffer.append(data);
                    //emit(logThis("Data buffer : " + dataBuffer.toHex().toUpper()));
                    if (dataBuffer.at(0) == SER_SYNCH_CODE)
                    {
                        EOpacket packet;
                        if (decodeESP3(dataBuffer, packet))
                        {
                            emit(logThis("Data buffer : " + dataBuffer.toHex().toUpper()));
                            dataBuffer.clear();
                            if (isTeachIN(packet)) processTeach(packet); else processData(packet);
                        }
                    }
                    else
                    {
                        dataBuffer.clear();
                    }
                }
            }
            else
            {
                QString msg = "Try to reconnect";
                TCPconnect(msg);
            }
        }
        if (useSerial)
        {
            serial->waitForReadyRead(100);
            if (serial->bytesAvailable())
            {
                //do
                //{
                    const QByteArray data = serial->readAll();
                    emit(logThis("Read : " + data.toHex().toUpper()));
                    //emit(logThis(QString("L = %1").arg(data.length())));
                    dataBuffer.append(data);
                    //emit(logThis("Data buffer : " + dataBuffer.toHex().toUpper()));
                    if (dataBuffer.at(0) == SER_SYNCH_CODE)
                    {
                        EOpacket packet;
                        if (decodeESP3(dataBuffer, packet))
                        {
                            //emit(logThis("Data buffer : " + dataBuffer.toHex().toUpper()));
                            dataBuffer.clear();
                            if (isTeachIN(packet)) processTeach(packet); else processData(packet);
                        }
                    }
                    else
                    {
                        //emit(logThis("Clear buffer : " + dataBuffer.toHex().toUpper()));
                        dataBuffer.clear();
                    }
                //} while (serial->waitForReadyRead(100));
            }
        }
        if (!FIFO.isEmpty())
        {
            mutexData.lock();
            QString Request = FIFO.first()->Request;
            uint32_t destinationID = FIFO.first()->destinationID;
            FIFO.removeFirst();
            mutexData.unlock();
            bool ok;
            uint8_t rorg = uint8_t(Request.mid(0, 2).toUShort(&ok, 16));
            if (ok)
            {
                switch (rorg)
                {
                    case RORG_VLD :
                    {
                        EOpacket send; //eoMessage mytel = eoMessage(3);
                        send.Type = PACKET_RADIO;
                        send.data.append(RORG_VLD); //mytel.RORG = RORG_VLD;
                        uint8_t cmd = uint8_t(Request.mid(2, 2).toInt(&ok, 16));
                        send.data.append(cmd); //mytel.data[0] = cmd;
                        /*
                            CMD 0x1 - Actuator Set Output
                            CMD 0x2 - Actuator Set Local
                            CMD 0x3 - Actuator Status Query
                            CMD 0x4 - Actuator Status Response
                            CMD 0x5 - Actuator Set Measurement
                            CMD 0x6 - Actuator Measurement Query
                            CMD 0x7 - Actuator Measurement Response
                        */
                        switch (cmd)
                        {
                            case 0x01 :
                            {
                                //Data : D2010064 050EC3BD base_ID  80      Opt_data : 00 05164727 switch ID 40 00
                                if (Request.length() == 8)
                                {
                                    send.data.append(uint8_t(Request.mid(4, 2).toInt(&ok, 16))); //mytel.data[1] = uint8_t(Request.mid(4, 2).toInt(&ok, 16));
                                    send.data.append(uint8_t(Request.mid(6, 2).toInt(&ok, 16))); //mytel.data[2] = uint8_t(Request.mid(6, 2).toInt(&ok, 16));
                                    appendID(send.data, baseID);
                                    send.data.append(0x00);
                                    send.Optdata.append(0x00);
                                    appendID(send.Optdata, destinationID);
                                    send.Optdata.append(0x00);
                                    send.Optdata.append(0x00);
                                    sendESP3(send);
                                }
                            }break;
                            case 0x02 :
                            {
                            }break;
                            case 0x03 :
                            {
                                if (Request.length() == 6)
                                {
                                    send.data.append(uint8_t(Request.mid(4, 2).toInt(&ok, 16)));
                                    send.data.append(0x00);
                                    appendID(send.data, baseID);
                                    send.data.append(0x00);
                                    send.Optdata.append(0x00);
                                    appendID(send.Optdata, destinationID);
                                    send.Optdata.append(0x00);
                                    send.Optdata.append(0x00);
                                    sendESP3(send);
                                }
                            }break;
                            case 0x04 :
                            {
                            } break;
                            case 0x05 :
                            {
                            } break;
                        }
                    } break;
                }
            }
        }
    }
    if (useTcp)
    {
        MySocket.disconnectFromHost();
        if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
        tcpStatus = MySocket.state();
        emit(tcpStatusChange());
    }
    if (useSerial)
    {
        serial->close();
        emit(eoStatusChange());
        emit(logThis(adress + " closed"));
    }
}





void eoceanthread::TCPconnect(QString &Status)
{
    QElapsedTimer timer;
    emit(logThis("TCP Connect  to " + adress  + QString(":%1 ").arg(port) + Status));
    int tryConnect = 1;
    do
    {
        timer.start();
        emit(logThis(QString("TCP try connect %1").arg(tryConnect)));
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->disconnectFromHost();
            emit(logThis("Try Disconnect"));
            if (socket->state() == QAbstractSocket::ConnectedState) socket->waitForDisconnected();
            if (socket->state() == QAbstractSocket::UnconnectedState)
            {
                emit(logThis("Disconnect OK"));
            }
        }
        tcpStatus = socket->state();
        emit(tcpStatusChange());
        if (endLessLoop)
        {
            socket->connectToHost(adress, port);
            emit(logThis("Try Connect"));
            socket->waitForConnected();
            if (socket->state() == QAbstractSocket::ConnectedState)
            {
                emit(logThis("Connect OK"));
            }
            else
            {
                emit(logThis("Connection error : " + socket->errorString()));
                while (timer.elapsed() < 2000);
                timer.start();
            }
            tcpStatus = socket->state();
            emit(tcpStatusChange());
            tryConnect ++;
        }
        if (tryConnect > 10)
        {
            tryConnect = 0;
        }
    }
    while ((socket->state() != QAbstractSocket::ConnectedState) && (endLessLoop));
}




QString eoceanthread::logPacket(const EOpacket &packet)
{
    QString log;
    QByteArray data;
    for (int n=0; n<packet.data.length(); n++) data.append(packet.data.at(n));
    if (data.isEmpty()) log.append("Data : Empty");
    else log.append("Data : " + data.toHex().toUpper());
    QByteArray Optdata;
    for (int n=0; n<packet.Optdata.length(); n++) Optdata.append(packet.Optdata.at(n));
    if (Optdata.isEmpty()) log.append(" Opt data : Empty");
    else log.append(" Opt data : " + Optdata.toHex().toUpper());
    return log;
}



void eoceanthread::appendID(QVector <uint8_t> &data, uint32_t ID)
{
    data.append((ID >> 24) & 0xFF);
    data.append((ID >> 16) & 0xFF);
    data.append((ID >> 8) & 0xFF);
    data.append((ID) & 0xFF);
}



void eoceanthread::processTeach(EOpacket &packet)
{
    if (!learnMode)
    {
        emit(logThis("Teach request ignored"));
        return;
    }
    QString log;
    log.append(QString("Learn packet type = %1 ").arg(packet.Type));
    log.append(logPacket(packet));
    emit(logThis(log));

    QString str;
    QString data_Str;
    uint8_t rorg = packet.data.at(0);
    switch (rorg)
    {
        case RORG_4BS:
        {     // A5-10-08 2C80 050F7A1A 00
            if (packet.data.count() < 10) break; // A580086A80050643CD00
            uint8_t func = packet.data.at(1) >> 2;
            uint8_t type = ((packet.data.at(1) & 3) << 5) | (packet.data.at(2) >> 3);
            //uint8_t manufacturer = (packet.data.at(2) & 7) << 8 | packet.data.at(3);
            device_ID = uint32_t(packet.data.at(5)) << 24 | uint32_t(packet.data.at(6)) << 16 | uint32_t(packet.data.at(7)) << 8 | packet.data.at(8);
            QString deviceID = QString("%1").arg(device_ID, 8, 16, QChar('0')).toUpper();
            QString deviceProfile = QString("%2-%3-%4").arg(uchar(rorg), 2, 16, QChar('0')).arg(uchar(func), 2, 16, QChar('0')).arg(uchar(type), 2, 16, QChar('0')).toUpper();
            switch (func)
            {
                case 0x20 :
                {
                    emit(logThis(addTimeTag + " A5-20 request : " + data_Str));
                    EOpacket response;
                    response.Type = packet.Type;
                    response.data.append(packet.data.at(0));
                    response.data.append(packet.data.at(1));
                    response.data.append(packet.data.at(2));
                    response.data.append(packet.data.at(3));
                    uint8_t DB0 = (1<<7) | // LRN type = 1=with EEP
                            (1<<6) | // 1=EEP is supported
                            (1<<5) | // 1=sender ID stored
                            (1<<4) | // 1=is LRN response
                            (0<<3);  // 0=is LRN packet
                    response.data.append(DB0);
                    response.data.append(packet.data.at(5));
                    response.data.append(packet.data.at(6));
                    response.data.append(packet.data.at(7));
                    response.data.append(packet.data.at(8));
                    response.data.append(packet.data.at(9));
                    if (packet.Optdata.count() == 7)
                    {
                        response.Optdata.append(0); // packet.Optdata.at(0)
                        appendID(response.Optdata, baseID);
                        response.Optdata.append(0); // packet.Optdata.at(5)
                        response.Optdata.append(0); // packet.Optdata.at(6)
                    }
                    sendESP3(response);
                    QString RomID = getRomID(rorg, func, type, device_ID);
                    emit(logThis(addTimeTag + "EnOcean A5-20 learning response done ..."));
                    emit(logThis(addTimeTag + " Device " + deviceID + " Learned-In EEP : " + deviceProfile));
                    emit(newDevice(RomID));
                    learnModeOff();
                } break;
                default:
                {
                    emit(logThis(addTimeTag + " Device " + deviceID + " Learned-In EEP : " + deviceProfile));
                    QString RomID = getRomID(rorg, func, type, device_ID);
                    emit(newDevice(RomID));
                    learnModeOff();
                } break;
            }
        } break;
        case RORG_UTE:
        {   // packet : D4 A0 01 46 00 0F 01 D2 05164727 00
            if (packet.data.count() != 13)
            {
                QString log;
                log.append("RORG_UTE wrong packet length " + logPacket(packet));
                emit(logThis(log));
                return;
                //Learn packet type = 1 Data : D4 A0 01 46 00 0F 01 D2 05164727 00 Opt data : 00FFFFFFFF4400
                //RORG_UTE wrong packet length Data : D4A00146000F01D20516472700 Opt data : 00FFFFFFFF4400
            }
            uint8_t devrorg = packet.data.at(7);
            uint8_t func = packet.data.at(6);
            uint8_t type = packet.data.at(5);
            //uint8_t manufacturer = (packet.data.at(4) & 7) << 8 | packet.data.at(3);
            //uint8_t Switch_Count = packet.data.at(2);
            //uint8_t CMD = packet.data.at(1) & 0x0F;
            device_ID = uint32_t(packet.data.at(8)) << 24 | uint32_t(packet.data.at(9)) << 16 | uint32_t(packet.data.at(10)) << 8 | packet.data.at(11);
            QString deviceID = QString("%1").arg(device_ID, 8, 16, QChar('0')).toUpper();
            QString deviceProfile = QString("%2-%3-%4").arg(uchar(rorg), 2, 16, QChar('0')).arg(uchar(func), 2, 16, QChar('0')).arg(uchar(type), 2, 16, QChar('0')).toUpper();
            switch (func)
            {
            // D4A00146000F01D20516472700
            // D4A0014600
            // 0F01D2    0F-01-D2   type func rorg
            // 05164727  ID
                case 0x01 :
                {
                    QString data_Str;
                    for (int n=0; n<packet.data.length(); n++) data_Str += QString("%1").arg(packet.data.at(n), 2, 16, QLatin1Char('0')).toUpper();
                    emit(logThis(addTimeTag + " UTE request : " + data_Str));
                    EOpacket response;
                    uint8_t DB6 = 1;
                    DB6 |= (TEACH_IN_ACCEPTED&0x3) << 4;
                    DB6 |= (UTE_DIRECTION_BIDIRECTIONAL&0x1) << 7;
                    response.Type = packet.Type;
                    response.data.append(packet.data.at(0));    // rorg D4
                    response.data.append(DB6);                  // acknoledge pairing
                    response.data.append(packet.data.at(2));
                    response.data.append(packet.data.at(3));
                    response.data.append(packet.data.at(4));
                    response.data.append(packet.data.at(5));
                    response.data.append(packet.data.at(6));
                    response.data.append(packet.data.at(7));
                    appendID(response.data, baseID);
                    response.data.append(packet.data.at(12));

                    response.Optdata.append(0);
                    response.Optdata.append(packet.data.at(8));    // Destination ID
                    response.Optdata.append(packet.data.at(9));
                    response.Optdata.append(packet.data.at(10));
                    response.Optdata.append(packet.data.at(11));
                    response.Optdata.append(0);
                    response.Optdata.append(0);

                    sendESP3(response);
                    emit(logThis(addTimeTag + "EnOcean learning UTE response done"));
                    QString RomID = getRomID(devrorg, func, type, device_ID);
                    //emit(logThis(addTimeTag + " Device " + deviceID + " Learned-In EEP : " + deviceProfile));
                    emit(newDevice(RomID));
                    learnModeOff();
                } break;
            }
        } break;
        case RORG_RPS :
        {
            emit(logThis(addTimeTag + " Device is switch"));
            //emit(newDevice());
        } break;
        default:
        {
        } break;
    }
}


void eoceanthread::learnModeOff()
{
    emit(logThis("EnOcean learning OFF"));
    learnMode = false;
}


QString eoceanthread::getRomID(uint8_t rorg, uint8_t func, uint8_t type, uint32_t ID)
{
    QString deviceID = QString("%1").arg(ID, 8, 16, QChar('0')).toUpper();
    QString deviceProfile = QString("%2%3%4").arg((unsigned char)rorg, 2, 16, QChar('0')).arg((unsigned char)func, 2, 16, QChar('0')).arg((unsigned char)type, 2, 16, QChar('0')).toUpper();
    QString RomID = deviceID + deviceProfile + familyEOcean;
    return RomID;
}



void eoceanthread::processData(const EOpacket &packet)
{
    if (learnMode)
    {
        uint8_t rorg = packet.data.at(0);
        if (rorg == 0xF6)
        {
            emit(logThis("Switch Teach request"));
            device_ID = packet.data.at(2); device_ID <<= 8; device_ID += packet.data.at(3); device_ID <<= 8; device_ID += packet.data.at(4); device_ID <<= 8; device_ID += packet.data.at(5);
            QString deviceID = QString("%1").arg(quint32(device_ID), 8, 16, QChar('0')).toUpper();
            //emit(logThis(deviceID));
            //quint8 device_Data = packet.data.at(1);
            //QString deviceData = QString("%1").arg(uchar(device_Data), 2, 16, QChar('0')).toUpper();
            //emit(logThis(deviceData));
            QString RomID = getRomID(rorg, 0x01, 0x01, device_ID);
            //emit(logThis(addTimeTag + " Device " + deviceID + " Learned-In EEP : " + deviceProfile));
            emit(newDevice(RomID));
            learnModeOff();
            //55000707017A F6 30 00322C9F 3000FFFFFFFF41
        }
        return;
    }
    QString log;
    log.append(QString("Process packet type = %1 ").arg(packet.Type));
    log.append(logPacket(packet));
    emit(logThis(log));
    QString str;
    QString data_Str;
    uint8_t rorg = packet.data.at(0);
    if (packet.Type == PACKET_RESPONSE) {
        QByteArray data;
        for (int n=0; n<packet.data.length(); n++) data.append(packet.data.at(n));
        QString deviceID = QString("%1").arg(quint32(lastDestinationID), 8, 16, QChar('0')).toUpper();
        emit(logThis("PACKET_RESPONSE : " + data.toHex().toUpper() + " for " + QString(deviceID)));
        //QString deviceProfile = QString("%2").arg(uchar(rorg), 2, 16, QChar('0')).toUpper();
        //QString RomID = deviceID + deviceProfile + familyEOcean;
        //emit(deviceReturn(RomID, scratchpad));
    }
    if (packet.Type == PACKET_RADIO) {
    switch (rorg)
    {
        case RORG_4BS:
        {   // Process packet type = 1 Data : A5826A840F019FA33430 Opt data : 00FFFFFFFF4100
            QString s;
            QString log_Str;
            device_ID = packet.data.at(5); device_ID <<= 8; device_ID += packet.data.at(6); device_ID <<= 8; device_ID += packet.data.at(7); device_ID <<= 8; device_ID += packet.data.at(8);
            QString deviceID = QString("%1").arg(quint32(device_ID), 8, 16, QChar('0')).toUpper();
            //emit(logThis(deviceID));
            QString deviceRorg = QString("%2").arg(uchar(rorg), 2, 16, QChar('0')).toUpper();
            //emit(logThis(deviceRorg));
            for (int n=1; n<5; n++) data_Str += QString("%1").arg(packet.data.at(n), 2, 16, QLatin1Char('0')).toUpper();
            QString RomIDRorgOnly = deviceID + deviceRorg + familyEOcean;
// Process packet type = 1 Data : A532407F08050643CD00 Opt data : 00FFFFFFFF2A0
            if (deviceRorg == "A5")
            {
                /*  VALVE POS: Example in HEX "0x05 0x77 0x00 0x08"
                 DB3.7...DB3.0 = 0x05 = 5: new valve position is 5%
                 DB2.7...DB2.0 = 0x77 = 119: room temperature = 255 - 119 = 136 => 40 * 136 / 255 = 21,3 °C
                 DB1.7...DB1.0 = 0x00:
                o DB1.3 = 0: regular default radio cycle (no summer mode)
                o DB1.2 = 0: DB3.7...DB3.0 is set to valve position %
                DB0.7...DB0.0 = 0x08: Data telegram

                SET_TEMP: Example in HEX "0x80 0x81 0x04 0x08"
                 DB3.7...DB3.0 = 0x80 = 128: New target temperature is 40 * 128 / 255 = 20,1°C
                 DB2.7...DB2.0 = 0x81 = 129: room temperature = 255 - 129 = 126 => 40 * 126 / 255 = 19,8 °C
                 DB1.7...DB1.0 = 0x04:
                o DB1.3 = 0: regular default radio cycle (no summer mode)
                o DB1.2 = 1: DB3.7...DB3.0 is set to internal temp.-controller with default duty cycle (Summer
                bit not active)
                DB0.7...DB0.0 = 0x08: Data telegram */
                QString RomIDA52001 = deviceID + familyeoA52001;
                //emit(logThis("Check if exist " + RomIDA52001));
                int index = deviceRomIDs.indexOf(RomIDA52001);
                if (index != -1)
                {
                    emit(logThis("Get A52001EO " + RomIDA52001 + QString(" set value %1").arg(quint8((dev4BS_Send.at(index) >> 24) & 0xFF))));
                    EOpacket response;
                    response.Type = packet.Type;
                    response.data.append(packet.data.at(0));
                    if (dev4BS_Send.at(index) == 0xFFFFFFFF) response.data.append(packet.data.at(1));
                    else response.data.append(quint8(dev4BS_Send.at(index) >> 24)); // DB3.0-7 %Vanne
                    response.data.append(0x00);                                     // DB2.0-7 Temp Detector set to zero if % is used
                    response.data.append(0x00);                                     // DB1.3 Summer bit DB1.2 Mode Temp/%
                    response.data.append(0x08);                                     // data Telegram
                    response.data.append(packet.data.at(5));
                    response.data.append(packet.data.at(6));
                    response.data.append(packet.data.at(7));
                    response.data.append(packet.data.at(8));
                    response.data.append(packet.data.at(9));
                    if (packet.Optdata.count() == 7)
                    {
                        response.Optdata.append(0); // packet.Optdata.at(0)
                        appendID(response.Optdata, baseID);
                        response.Optdata.append(0); // packet.Optdata.at(5)
                        response.Optdata.append(0); // packet.Optdata.at(6))
                    }
                    sendESP3(response);
                    data_Str.clear();
                    data_Str += QString("%1").arg(response.data.at(1), 2, 16, QLatin1Char('0')).toUpper();
                    data_Str += QString("%1").arg(packet.data.at(2), 2, 16, QLatin1Char('0')).toUpper();
                    data_Str += QString("%1").arg(packet.data.at(3), 2, 16, QLatin1Char('0')).toUpper();
                    data_Str += QString("%1").arg(packet.data.at(4), 2, 16, QLatin1Char('0')).toUpper();
                }
                //else emit(logThis("No value to set for " + RomIDA52001));
            }
            emit(deviceReturn(RomIDRorgOnly, data_Str));
        } break;
        case RORG_VLD:
        { // D2 046080 05164727 00
            uint8_t cmd = uint8_t(packet.data.at(1));
/*
    CMD 0x1 - Actuator Set Output
    CMD 0x2 - Actuator Set Local
    CMD 0x3 - Actuator Status Query
    CMD 0x4 - Actuator Status Response
    CMD 0x5 - Actuator Set Measurement
    CMD 0x6 - Actuator Measurement Query
    CMD 0x7 - Actuator Measurement Response
*/
            switch (cmd)
            {
                case 0x01 :
                {
                } break;
                case 0x02 :
                {
                } break;
                case 0x03 :
                {
                } break;
                case 0x04 :// D20460E40516472700 Opt data : 00FFFFFFFF4000
                {
                    device_ID = packet.data.at(4); device_ID <<= 8; device_ID += packet.data.at(5); device_ID <<= 8; device_ID += packet.data.at(6); device_ID <<= 8; device_ID += packet.data.at(7);
                    QString deviceID = QString("%1").arg(qint32(device_ID), 8, 16, QChar('0')).toUpper();
                    QString deviceProfile = QString("%2").arg(uchar(rorg), 2, 16, QChar('0')).toUpper();
                    QString RomID = deviceID + deviceProfile + familyEOcean;
                    QString scratchpad;
                    for (int n=1; n<4; n++) scratchpad += QString("%1").arg(packet.data.at(n), 2, 16, QLatin1Char('0')).toUpper();
                    emit(deviceReturn(RomID, scratchpad));
                } break;
                case 0x05 :
                {
                } break;
                case 0x06 :
                {
                } break;
                case 0x07 :
                {
                } break;
                case 0x08 :
                {
                } break;
                case 0x09 :
                {
                } break;
                case 0x0A :
                {
                } break;
                case 0x0B :
                {
                } break;
                case 0x0C :
                {
                } break;
                case 0x0D :
                {
                } break;
                case 0x0E :
                {
                } break;
                case 0x0F :
                {
                } break;
            }
    } break;
        case RORG_RPS:
        {
            device_ID = packet.data.at(2); device_ID <<= 8; device_ID += packet.data.at(3); device_ID <<= 8; device_ID += packet.data.at(4); device_ID <<= 8; device_ID += packet.data.at(5);
            QString deviceID = QString("%1").arg(qint32(device_ID), 8, 16, QChar('0')).toUpper();
            QString deviceProfile = QString("%2").arg(uchar(rorg), 2, 16, QChar('0')).toUpper();
            QString RomID = deviceID + deviceProfile + familyEOcean;
            quint8 device_Data = packet.data.at(1);
            QString scratchpad = QString("%1").arg(uchar(device_Data), 2, 16, QChar('0')).toUpper();
            emit(deviceReturn(RomID, scratchpad));
        } break;
        default:
        {
        } break;
    }
    }
}



void eoceanthread::saveConfig()
{
}



void eoceanthread::addtofifo(const QString &data)
{
    mutexData.lock();
    FIFOStruc *newFIFO = new FIFOStruc;
    newFIFO->Request = net1wire::getOrder(data);
    bool ok;
    newFIFO->destinationID = net1wire::getRomID(data).toUInt(&ok, 16);
    if (ok) FIFO.append(newFIFO);
    mutexData.unlock();
}



void eoceanthread::sendESP3(EOpacket &packet)
{
    QByteArray dataSend;
    dataSend.append(SER_SYNCH_CODE);
    uint8_t dataMSB = uint8_t(packet.data.length() >> 8);
    uint8_t dataLSB = uint8_t(packet.data.length() & 0x00FF);
    dataSend.append(char(dataMSB));
    dataSend.append(char(dataLSB));
    dataSend.append(char(packet.Optdata.length()));
    dataSend.append(char(packet.Type));
    uint8_t crcheader = 0;
    crcheader = crc8(crcheader, dataMSB);
    crcheader = crc8(crcheader, dataLSB);
    crcheader = crc8(crcheader, packet.Optdata.length());
    crcheader = crc8(crcheader, packet.Type);
    dataSend.append(crcheader);
    uint8_t crcdata = 0;
    for (int n=0; n<packet.data.length(); n++)
    {
        crcdata = crc8(crcdata, packet.data.at(n));
        dataSend.append(packet.data.at(n));
    }
    for (int n=0; n<packet.Optdata.length(); n++)
    {
        crcdata = crc8(crcdata, packet.Optdata.at(n));
        dataSend.append(packet.Optdata.at(n));
    }
    dataSend.append(crcdata);
    if (useTcp)
    {
        if (socket->isOpen())
        {
            socket->write(dataSend);
            socket->waitForBytesWritten(1000);
        }
        else
        {
            emit(logThis("Cannont write to closed socket"));
        }
    }
    if (useSerial)
    {
        if (serial->isOpen())
        {
            serial->write(dataSend);
            serial->waitForBytesWritten(1000);
        }
        else
        {
            emit(logThis("Cannont write to closed serial port"));
        }
    }
    QString log;
    log.append(QString("Send : "));
    log.append(logPacket(packet));
    emit(logThis(log));
}


// 55 00 01 00 02 65 00 00

bool eoceanthread::decodeESP3(QByteArray &buf, EOpacket &packet)
{
    QVector <uint8_t> dataBuffer;
    for (int n=0; n<buf.length(); n++) dataBuffer.append(uint8_t(buf.at(n)));
    if (dataBuffer.length() > 6)
    {
        int dataLenght = int(dataBuffer.at(1) << 8) + dataBuffer.at(2);
        int optDataLength = dataBuffer.at(3);
        int packetLength = 7 + dataLenght + optDataLength;
        if (packetLength == dataBuffer.length())
        {
            packet.Type = dataBuffer.at(4);
            uint8_t crcheader = uint8_t(dataBuffer.at(5));
            uint8_t crccalc = 0;
            for (int n=1; n<5; n++) crccalc = crc8(crccalc, uint8_t(dataBuffer.at(n)));
            if (crcheader != crccalc) return false;
            uint8_t crcdata = uint8_t(dataBuffer.at(packetLength-1));
            crccalc = 0;
            for (int n=6; n<packetLength-1; n++) crccalc = crc8(crccalc, uint8_t(dataBuffer.at(n)));
            if (crcdata != crccalc) return false;
            for (int n=0; n<dataLenght; n++) packet.data.append(quint8(dataBuffer.at(n+6)));
            for (int n=(6+dataLenght); n<(packetLength-1); n++) packet.Optdata.append(quint8(dataBuffer.at(n)));

            //emit(logThis(QString("Decode packet type = %1").arg(packet.Type)));
            //emit(logThis("Decode data : " + packet.data.toHex().toUpper()));
            //emit(logThis("Decode Optdata : " + packet.Optdata.toHex().toUpper()));

//55000A 07 01 EB A56966830F019FA33430 00FFFFFFFF3A00 F8

            return true;


     /*
            if (dataBuffer.at(4) == PACKET_RADIO)    // ERP1
            {
                uint8_t crc = uint8_t(dataBuffer.at(5));
                uint8_t crccalc = 0;
                for (int n=1; n<5; n++) crccalc = crc8(crccalc, uint8_t(dataBuffer.at(n)));
                if (crc == crccalc) // CRC Header check
                {
                    uint8_t crc = uint8_t(dataBuffer.at(packetLength-1));
                    uint8_t crccalc = 0;
                    for (int n=6; n<packetLength-1; n++) crccalc = crc8(crccalc, uint8_t(dataBuffer.at(n)));
                    if (crc == crccalc) // CRC Data check
                    {
                        QString hex = dataBuffer.toHex().toUpper();
                        emit(logThis("ERP1 : " + hex));

                        //uint8_t subTelNum = uint8_t(dataBuffer.at(6 + dataLenght));
                        //emit(logThis(QString("subTelNum = %1").arg(subTelNum)));

                        uint32_t dest_ID = uint32_t(dataBuffer.at(7 + dataLenght)) << 24 | uint32_t(dataBuffer.at(8 + dataLenght)) << 16 | uint32_t(dataBuffer.at(9 + dataLenght)) << 8 | dataBuffer.at(10 + dataLenght);
                        QString str_ID = QString("%1").arg(dest_ID, 8, 16, QChar('0')).toUpper();
                        emit(logThis("Destination ID : " + str_ID));

                        uint8_t dBm = uint8_t(dataBuffer.at(11 + dataLenght));
                        emit(logThis(QString("dBm = %1").arg(dBm)));

                        emit(logThis("ERP1 : " + hex));
                        if (learnMode)
                        {

                        }
                        else
                        {
                            processData(dataBuffer.mid(6, dataLenght));
                        }
                    }
                }
                else
                {
                    QString hex = dataBuffer.toHex().toUpper();
                    emit(logThis("Wrong Header CRC packet aborted " + hex));
                }
            }
            else if (dataBuffer.at(4) == PACKET_RESPONSE)
            { // 55 0005 01 02 DB 00FF8F5D80 0A EF

            }
            dataBuffer.clear();
            return;*/
        }
        else if (packetLength < dataBuffer.length())
        {
            buf.clear();
            return false;
        }
    }
    return false;
}


bool eoceanthread::checkBaseID()
{
    EOpacket packet;
    packet.Type = PACKET_COMMON_COMMAND;
    packet.data.append(CO_RD_IDBASE);
    sendESP3(packet);
    if (useTcp)
    {
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->waitForReadyRead(100);
            if (socket->bytesAvailable())
            {
                const QByteArray data = socket->readAll();
                dataBuffer.append(data);
            }
        }
        else
        {
            QString msg = "Try to reconnect";
            TCPconnect(msg);
        }
    }
    if (useSerial)
    {
        serial->waitForReadyRead(1000);
        if (serial->bytesAvailable())
        {
            const QByteArray data = serial->readAll();
            dataBuffer.append(data);
        }
    }
    if (dataBuffer.at(0) == SER_SYNCH_CODE)
    {
        EOpacket packet;
        if (decodeESP3(dataBuffer, packet))
        {
            dataBuffer.clear();
            if ((packet.Type == PACKET_RESPONSE) && (packet.data.length() == 5) && (packet.Optdata.length() == 1))
            {
                baseID = packet.data.at(1) << 24 | packet.data.at(2) << 16 | packet.data.at(3) << 8 | packet.data.at(4);
                QString IDstr = QString("%1").arg(quint32(baseID), 8, 16, QChar('0')).toUpper();
                QString str = QString(" Remaining Writes : %2").arg(quint32(packet.Optdata.at(0)));
                emit(logThis("Base ID : 0x" + IDstr + str));
                return true;
            }
        }
    }
    else
    {
        emit(logThis("Clear buffer : " + dataBuffer.toHex().toUpper()));
        dataBuffer.clear();
    }
    return false;
}


bool eoceanthread::isTeachIN(EOpacket &packet)
{
    uint8_t rorg = packet.data.at(0);
    if ((rorg == RORG_4BS) && (packet.data.at(4) & 0x08) == 0x00) return true;
    else if (((rorg == RORG_1BS) && (packet.data.at(1) & 0x08) == 0x00)) return true;
    else if (rorg == GP_TI) return true;
    else if (rorg == RORG_UTE) return true;
    else if (rorg == RORG_SEC_TI) return true;
    return false;
}


uint8_t eoceanthread::crc8(uint8_t CRC, uint8_t Data)
{
    return CRC8Table[CRC ^ Data];
}

uint16_t eoceanthread::crc16(uint16_t CRC, uint16_t Data)
{
    return CRC16Table[CRC ^ Data];
}

const uint8_t eoceanthread::CRC8Table[256] =
{	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
    0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
    0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
    0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
    0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
    0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
    0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
    0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
    0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
    0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 	};

const uint16_t eoceanthread::CRC16Table[256] =
{    	0,  4489,  8978, 12955, 17956, 22445, 25910, 29887,
     35912, 40385, 44890, 48851, 51820, 56293, 59774, 63735,
      4225,   264, 13203,  8730, 22181, 18220, 30135, 25662,
     40137, 36160, 49115, 44626, 56045, 52068, 63999, 59510,
      8450, 12427,   528,  5017, 26406, 30383, 17460, 21949,
     44362, 48323, 36440, 40913, 60270, 64231, 51324, 55797,
     12675,  8202,  4753,   792, 30631, 26158, 21685, 17724,
     48587, 44098, 40665, 36688, 64495, 60006, 55549, 51572,
     16900, 21389, 24854, 28831,  1056,  5545, 10034, 14011,
     52812, 57285, 60766, 64727, 34920, 39393, 43898, 47859,
     21125, 17164, 29079, 24606,  5281,  1320, 14259,  9786,
     57037, 53060, 64991, 60502, 39145, 35168, 48123, 43634,
     25350, 29327, 16404, 20893,  9506, 13483,  1584,  6073,
     61262, 65223, 52316, 56789, 43370, 47331, 35448, 39921,
     29575, 25102, 20629, 16668, 13731,  9258,  5809,  1848,
     65487, 60998, 56541, 52564, 47595, 43106, 39673, 35696,
     33800, 38273, 42778, 46739, 49708, 54181, 57662, 61623,
      2112,  6601, 11090, 15067, 20068, 24557, 28022, 31999,
     38025, 34048, 47003, 42514, 53933, 49956, 61887, 57398,
      6337,  2376, 15315, 10842, 24293, 20332, 32247, 27774,
     42250, 46211, 34328, 38801, 58158, 62119, 49212, 53685,
     10562, 14539,  2640,  7129, 28518, 32495, 19572, 24061,
     46475, 41986, 38553, 34576, 62383, 57894, 53437, 49460,
     14787, 10314,  6865,  2904, 32743, 28270, 23797, 19836,
     50700, 55173, 58654, 62615, 32808, 37281, 41786, 45747,
     19012, 23501, 26966, 30943,  3168,  7657, 12146, 16123,
     54925, 50948, 62879, 58390, 37033, 33056, 46011, 41522,
     23237, 19276, 31191, 26718,  7393,  3432, 16371, 11898,
     59150, 63111, 50204, 54677, 41258, 45219, 33336, 37809,
     27462, 31439, 18516, 23005, 11618, 15595,  3696,  8185,
     63375, 58886, 54429, 50452, 45483, 40994, 37561, 33584,
     31687, 27214, 22741, 18780, 15843, 11370,  7921,  3960 };

