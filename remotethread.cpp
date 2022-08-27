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





#include "globalvar.h"
#include "remotethread.h"
#include "net1wire.h"
#include "quazip.h"
#include "quazipfile.h"

#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif



remotethread::remotethread(logisdom *Parent)
{
	parent = Parent;
	moveToThread(this);
	passWordDone = false;
	logEnabled = false;
}



void remotethread::checkGetConfigFile(tcpData &data)
{
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray DataByte;
	data.getData(DataByte);
	QString configStr;
	configStr = codec->toUnicode(DataByte);
	if (configStr.isEmpty())
	{
		log = "Data empty\n";
		logFile(log);
		return;
	}
	QString Header;
	Header.append(data.Header);
	log = "checkGetConfigFile done : " + Header + "\n";
	logFile(log);
    emit(configReady(configStr));
}




void remotethread::checkGetDevicesConfig(tcpData &data)
{
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray DataByte;
	data.getData(DataByte);
	QString deviceConfig;
	deviceConfig = codec->toUnicode(DataByte);
	if (deviceConfig.isEmpty())
	{
		log = "Data empty\n";
		logFile(log);
		return;
	}
	QString Header;
	Header.append(data.Header);
	log = "checkGetDeviceConfig done : " + Header + "\n";
	logFile(log);
    emit(deviceConfigReady(deviceConfig));
}





void remotethread::checkGetMainValue(tcpData &data)
{
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray DataByte;
	data.getData(DataByte);
	QString deviceConfig;
	deviceConfig = codec->toUnicode(DataByte);
	if (deviceConfig.isEmpty())
	{
		log = "Data empty\n";
		logFile(log);
		return;
	}
	QString Header;
	Header.append(data.Header);
	log = "checkGetMainValue done : " + Header + "\n";
	logFile(log);
    emit(getMainValueReady(deviceConfig));
}





void remotethread::checkGetFile(tcpData &data)
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QString Header;
	Header.append(data.Header);
//	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QByteArray DataByte;
	data.getData(DataByte);
    QString folder = logisdom::getvalue(NetRequestMsg[setFolder], Header);
    if (folder.endsWith("\\")) folder.chop(1);
    if (!folder.isEmpty())
	{
		if (!QDir().exists(folder))
		{
			if (logEnabled) log += addTimeTag "Try to create Dir " + folder;
			if (!QDir().mkdir(folder))
			{
				if (logEnabled) log += addTimeTag "Cannot create Dir " + folder;
			}
			if (logEnabled) log += addTimeTag "Dir " + folder + " created";
		}
	}
	QString filename;
	if (folder.isEmpty()) filename = logisdom::getvalue(NetRequestMsg[GetFile], Header);
        else filename = folder + QDir::separator() + logisdom::getvalue(NetRequestMsg[GetFile], Header);
    if (!filename.isEmpty())
    {
        QFile file(filename);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(DataByte);
            file.close();
        }
        else
        {
            if (logEnabled) log += addTimeTag "cannot open file " + filename;
        }
    }

	int index = 0;
	while (index < FIFOSpecial.count())
	{
		bool found = false;
		QString item = FIFOSpecial.at(index)->Request;
		QString fifoorder = net1wire::getOrder(item);
		QString fifofilenamme = logisdom::getvalue(NetRequestMsg[GetDatFile], fifoorder);
		if (logEnabled) log += addTimeTag "Try Remove extra : " + fifoorder + "   " + fifofilenamme;
        if (fifoorder.contains(NetRequestMsg[GetDatFile]))
		{
			// check if file is here in a global zip file
			int posUnderScore = fifofilenamme.indexOf("_");
			if (posUnderScore > 0)
			{
				QString romid = fifofilenamme.left(posUnderScore);
				QString year = fifofilenamme.right(4);
				QString zipfilename = parent->getrepertoiredat() + romid + "_" + year + ".zip";
				QFile file(zipfilename);
				if (file.exists())
				{
					QuaZip zip(zipfilename);
					if(zip.open(QuaZip::mdUnzip))
					{
                                                QTextCodec *codec = zip.getFileNameCodec();
                                                zip.setFileNameCodec(codec);
                                                //zip.setFileNameCodec("IBM866");
						QuaZipFileInfo info;
						QuaZipFile zipFile(&zip);
						QString name;
						for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
						{
							if (zip.getCurrentFileInfo(&info))
							{
								if(zipFile.getZipError() == UNZ_OK)
								{
									name = zipFile.getActualFileName();
									if (name == fifofilenamme + dat_ext)
									{
										found = true;
										if (logEnabled) log += addTimeTag "Remove extra : " + name;
									}
								}
							}
						}
						zip.close();
						if(zip.getZipError() != UNZ_OK)
							if (logEnabled) log += addTimeTag "Zip file close error : " + zipfilename;
					}
				}
			}
        }
		if (found)
		{
			mutexData.lock();
			delete FIFOSpecial.at(index);
			FIFOSpecial.removeAt(index);
			mutexData.unlock();
		}
		else index ++;
	}
	bool found = false;
	for (int n=0; n<FIFOSpecial.count(); n++)
		if (FIFOSpecial.at(n)->Request.contains(NetRequestMsg[GetFile]) or FIFOSpecial.at(n)->Request.contains(NetRequestMsg[GetDatFile])) found = true;
	if (!found) emit(reloadGrahp());
	if (logEnabled) log += addTimeTag "Files " + filename + " Done";
}





void remotethread::checkGetFIFOSpecial(tcpData &data)
{
	if (FIFOSpecial.first()->Request_ID == -1)
	{

	}
	else switch (FIFOSpecial.first()->Request_ID)
	{
		case GetFile	:
		case GetDatFile	: checkGetFile(data); break;
	}
}





void remotethread::checkUserName(tcpData &data)
{
	QString Header;
	Header.append(data.Header);
	log = "Check UserName done : " + Header + "\n";
	logFile(log);
}




void remotethread::checkPassWord(tcpData &data)
{
	QString Header;
	Header.append(data.Header);
	QString rigths = logisdom::getvalue(RigthsTypeStr, Header);
	if (rigths.indexOf(RigthsAdminStr) != -1) Admin = true;
	log = "Check PassWord done : " + Header + "\n";
	logFile(log);
	passWordDone = true;
}




void remotethread::logFile(QString txt)
{
	//if (!logEnabled);
	return;
	QString filename = moduleipaddress;
	filename.remove(".");
	filename += QString(".txt");
	QFile file(filename);
	QTextStream out(&file);
	file.open(QIODevice::Append | QIODevice::Text);
	out << txt;
	file.close();
}




void remotethread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QElapsedTimer timer;
    QString Req;
	QTcpSocket MySocket;
    qintptr sd = MySocket.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(int(sd), SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
//#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
//#endif
    int fileIndex = 0;
	tcpData Data;
    MySocket.connectToHost(moduleipaddress, port);
    MySocket.waitForConnected();
    if (MySocket.state() == QAbstractSocket::ConnectedState)
        log = addTimeTag moduleipaddress + "  is now connected\n";
    else
        log = addTimeTag moduleipaddress +  "  could not connect\n";
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
    logFile(log);
    passWordDone = false;
    while (endLessLoop)
    {
        log.clear();
        if (!passWordDone)
        {
            Req = NetRequestMsg[SetUserName] + " = (" + UserName + ")";
            if (logEnabled) log += addTimeTag Req;
            getFifoString(Req);
            get(MySocket, Req, Data);
            checkUserName(Data);
            msleep(250);
            Req = NetRequestMsg[SetPassWord] + " = (" + PassWord + ")";
            if (logEnabled) log += addTimeTag Req;
            getFifoString(Req);
            get(MySocket, Req, Data);
            checkPassWord(Data);
            msleep(250);
            Req = NetRequestMsg[GetConfigFile];
            if (logEnabled) log += addTimeTag Req;
            getFifoString(Req);
            get(MySocket, Req, Data);
            checkGetConfigFile(Data);
            msleep(250);
            Req = NetRequestMsg[GetDevicesConfig];
            if (logEnabled) log += addTimeTag Req;
            getFifoString(Req);
            get(MySocket, Req, Data);
            checkGetDevicesConfig(Data);
            msleep(250);
            Req = NetRequestMsg[GetMainValue];
            if (logEnabled) log += addTimeTag Req;
            getFifoString(Req);
            get(MySocket, Req, Data);
            checkGetMainValue(Data);
        }
        timer.restart();
        while (!FIFOSpecial.isEmpty() && endLessLoop && (timer.elapsed() < 30000))
        {
//				if (logEnabled) log += addTimeTag "------------------------------------------------------------------------------";
//				QString Req = FIFOSpecial.first()->Request;
//				get(MySocket, Req, Data);
//				readFIFOSpecialBuffer(Data);
            //status += FIFOSpecial.first()->Request;
            msleep(250);
            Req = FIFOSpecial.first()->Request;
            if (Req.isEmpty()) Req = NetRequestMsg[FIFOSpecial.first()->Request_ID];
            if (!Req.isEmpty())
            {
                if (logEnabled) log += addTimeTag Req;
                getFifoString(Req);
                get(MySocket, Req, Data);
                checkGetFIFOSpecial(Data);
            }
            mutexData.lock();
            delete FIFOSpecial.first();
            FIFOSpecial.removeFirst();
            mutexData.unlock();
        }
        msleep(250);
        Req = NetRequestMsg[SaveMainValue];
        if (logEnabled) log += addTimeTag Req;
        getFifoString(Req);
        get(MySocket, Req, Data);
        checkGetMainValue(Data);
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
            //QString *logRead = new QString;
            //*logRead = log;
            //emit(tcpStatusUpdate(logRead));
        }
        timer.restart();
        while ((timer.elapsed() < 10000) && (FIFOSpecial.isEmpty()) && endLessLoop) sleep(1);
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
}




void remotethread::addToFIFOSpecial(int Request_ID, const QString &data)
{
	if (FIFOSpecial.count() > 100) return;
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = Request_ID;
	newFIFO->Request = data;
	mutexData.lock();
	FIFOSpecial.append(newFIFO);
	mutexData.unlock();
}




void remotethread::addToFIFOSpecial(const QString &data)
{
	if (FIFOSpecial.count() > 100) return;
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = -1;
	newFIFO->Request = data;
	mutexData.lock();
	FIFOSpecial.append(newFIFO);
	mutexData.unlock();
}





void remotethread::addToFIFOSpecial(int Request_ID)
{
	if (FIFOSpecial.count() > 100) return;
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = Request_ID;
	newFIFO->Request = "";
	mutexData.lock();
	FIFOSpecial.append(newFIFO);
	mutexData.unlock();
}




void remotethread::getFifoString(QString &str)
{
	str = "*" + str + "#";
}




void remotethread::get(QTcpSocket &Socket, QString &Request, tcpData &Data)
{
    trace.clear();
    QString tr = Request;
    tr.chop(1);
    trace.append(tr.remove(0, 1));
    Data.clear();
	QString req = "<" + Request + ">";
	log = addTimeTag " SEND : " + req;
	logFile(log);
        if (Socket.isValid()) Socket.write(req.toLatin1()); else return;
	if (!Socket.waitForBytesWritten(10000))
	{
		if (logEnabled) log += addTimeTag "Error writing data : " + req;
	}
	while(!Data.isComplete())
	{
		if (!Socket.waitForReadyRead(30000))
		{
			Socket.disconnectFromHost();
			if (Socket.state() != QAbstractSocket::UnconnectedState) Socket.waitForDisconnected();
			tcpStatus = Socket.state();
			emit(tcpStatusChange());
			Socket.connectToHost(moduleipaddress, port);
			Socket.waitForConnected();
			log = addTimeTag "Socket was disconnected";
			logFile(log);
			tcpStatus = Socket.state();
			emit(tcpStatusChange());
		}
		else
		{
			QByteArray B;
			B = Socket.readAll();
			Data.append(B);
			log.clear();
			log.append(B);
			logFile(log);
		}
	}
    if (FIFOSpecial.isEmpty())
    {
        trace.clear();
    }
	emit(traceUpdate(trace));
}
