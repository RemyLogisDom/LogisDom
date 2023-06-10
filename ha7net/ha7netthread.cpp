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
#include "ha7netthread.h"
#include "globalvar.h"



ha7netthread::ha7netthread()
{
	logEnabled = false;
	GlobalConvert = false;
	endLessLoop = true;
	LockID.clear();
	fileIndex = 0;
	moveToThread(this);
 }




ha7netthread::~ha7netthread()
{
	endLessLoop = false;
}



void ha7netthread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
	QString Data;
	QString reqstr;
    while (endLessLoop)
    {
// Process FIFOSpecial
        if (logEnabled)
        {
            log += addTimeTag "********************     FIFOSpecial contains   *************************************";
            for (int n=0; n<FIFOSpecial.count(); n++) log += addTimeTag "" + FIFOSpecial.at(n)->Request;
            log += addTimeTag "*************************************************************************************";
                            processFIFOSpecial();
        }
// Search sequence
        if (FIFO.isEmpty())
        {
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
                if (LockID.isEmpty()) reqstr = "Reset.html";
                else reqstr = "Reset.html?" + LockID;
                get(reqstr, Data);
                if (LockID.isEmpty()) reqstr = "Search.html";
                else reqstr = "Search.html?" + LockID;
                get(reqstr, Data);
                SearchAnalysis(Data);
            }
// Log device listing
            if (logEnabled)
            {
                log += addTimeTag "********************     End of search / Device Listing   ********************************";
                for (int n=0; n<devices.count(); n++) log += addTimeTag "" + QString ("Device_%1 RomID : ").arg(n) + devices.at(n)->RomID + "   Original Scratchpad : " + devices.at(n)->Scratchpad;
                log += addTimeTag "*************************************************************************************";
            }
// Fill FIFO with device readings
            if (FIFO.isEmpty())	// if FIFO is empty add all request otherwise, finish las bunch
            {
                if (logEnabled) log += addTimeTag "********************    Fill FIFO with device Reading  *******************************";
                addDeviceReadings();
            }
        }
// Log FIFO content
        if (logEnabled) for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
        if (logEnabled) log += addTimeTag "**************************************************************************************";
// Get device reading info
        processFIFO();
        saveLog();
        int minute = QDateTime::currentDateTime().time().minute();
        while ((minute == QDateTime::currentDateTime().time().minute()) && endLessLoop)
        {
            if (!FIFOSpecial.isEmpty())
            {
                processFIFOSpecial();
            }
            if (FIFO.isEmpty())
            {
                if (logEnabled) log += addTimeTag "***************    Fill FIFO with device Reading for reading till next minute  *********************";
                addDeviceReadings(false);
                if (logEnabled)
                {
                    for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
                    log += addTimeTag "**************************************************************************************";
                }
            }
            processFIFO(false);
            saveLog();
            sleep(1);
        }
        // Clear FIFO
        mutexData.lock();
        while (!FIFO.isEmpty())
        {
            delete FIFO.first();
            FIFO.removeFirst();
        }
        mutexData.unlock();
        saveLog();
    }
}




void ha7netthread::processFIFO(bool ALL)
{
	QString Data;
	while (!FIFO.isEmpty() && FIFOSpecial.isEmpty() && endLessLoop)
	{
        if (logEnabled) log += addTimeTag "------------------------------------------------------------------------------";
		QString Req = FIFO.first()->Request;
		get(Req, Data);
		readFIFOBuffer(Data);
		msleep(250);
		mutexData.lock();
		//logThis("processFIFO : " + Req);
		delete FIFO.first();
		FIFO.removeFirst();
		mutexData.unlock();
		if (!ALL) return;
	}
}




void ha7netthread::processFIFOSpecial()
{
	QString Data;
	QString reqstr;
	while (!FIFOSpecial.isEmpty() && endLessLoop)
	{
        if (logEnabled) log += addTimeTag "------------------------------------------------------------------------------";
		reqstr = FIFOSpecial.first()->Request;
		get(reqstr, Data);
		readFIFOSpecialBuffer(Data);
		mutexData.lock();
		//logThis("processFIFOSpecial : " + reqstr);
		delete FIFOSpecial.first();
		FIFOSpecial.removeFirst();
		mutexData.unlock();
	}
}



void ha7netthread::saveLog()
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




int ha7netthread::checkDevice(const QString RomID)
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
        if (logEnabled) log += addTimeTag " Found new device : " + RomID + QString("  Index %1").arg(ID);
		devices.append(dev);
        emit(newDevice(devices.last()->RomID));
	}
	return ID;
}





void ha7netthread::SearchAnalysis(const QString &data)
{
// search buffer for CLASS="HA7Value" NAME="Address_0" ID="ADDRESS_0" TYPE="text" VALUE=
QString RomID;
int pos, indexstringsearch = 0;
QByteArray strsearch;
	strsearch = "TYPE=\"text\" VALUE=\"";
	pos = data.indexOf(strsearch);
	do
	{
		if ((pos != -1) and (data.mid(pos + 35, 2)) == "\">")		// has found item
		{
			RomID = data.mid(pos + 19, 16);		// extract interesting data : RomID
			bool found = false;
			for (int n=0; n<devices.count(); n++)
			{
				if (devices.at(n)->RomID == RomID) found = true;
			}
			if (!found)
			{
				device *dev = new device;
				dev->RomID = RomID;
				devices.append(dev);
                emit(newDevice(devices.last()->RomID));
			}
			indexstringsearch ++;
			strsearch = "TYPE=\"text\" VALUE=\"";
		}
		pos = data.indexOf(strsearch, pos + 1);
	} while(pos > 0);
}




void ha7netthread::get(QString &Request, QString &Data)
{
/*	QEventLoop loop;
    QHtp http;
	Data.clear();
	QString reqstr = "http://" + moduleipaddress + QString(":%1").arg(port) + "/1Wire/" + Request;
    if (logEnabled) log += addTimeTag " SEND : " + reqstr;
	logThis(QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") + "SEND : " + reqstr);
	http.setHost(moduleipaddress, port);
	http.get(reqstr);
	connect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));
	loop.exec();
	disconnect(&http, SIGNAL(done(bool)), &loop, SLOT(quit()));
	loop.quit();
	int error = http.error();
    if (error == QHtp::NoError)
	{
		Data.append(http.readAll());
        if (logEnabled) log += addTimeTag " GET : " + Data;
	}
    else if (logEnabled) log += addTimeTag " Http error " + http.errorString() ;*/

    QNetworkAccessManager manager;
    QString reqstr = "http://" + moduleipaddress + QString(":%1").arg(port) + "/1Wire/" + Request;
    if (logEnabled) log += addTimeTag " SEND : " + reqstr;
    logThis(QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") + "SEND : " + reqstr);
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(reqstr)));
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    Data.clear();
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
#if QT_VERSION < 0x060000
        QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
        Data = Utf8Codec->toUnicode(data);
#else
        Data.append(data);
#endif
        if (logEnabled) log += addTimeTag tr("Data received without error");
        if (logEnabled) log += addTimeTag " GET : " + Data;
    }
    else if (logEnabled) log += addTimeTag "Http error " + reply->errorString();
}






void ha7netthread::readFIFOBuffer(const QString &htmlData)
{
	QString strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = htmlData.indexOf(strsearch);
	if (i < 0) return;
	int deviceIndex = FIFO.first()->device_ID;
	switch (FIFO.first()->Request_ID)
	{
		case ConvertTemp	:
                    if (logEnabled) log += addTimeTag QString(" Wait for convesion : 1 second");
					sleep(1); break;
		case ConvertTempRomID	:
					if ((deviceIndex >= 0) && (deviceIndex < devices.count())) {
					int convTime = getConvertTime(devices.at(deviceIndex));
                    if (logEnabled) log += addTimeTag QString(" Wait for convesion : %1ms").arg(convTime);
					msleep(convTime + 250); }
					break;
		case ReadTemp		: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 54, 18); } break;
		case ReadPIO		: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 52, 26); } break;
		case ReadCounter	: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) {
				if (FIFO.first()->scratchpad_ID == 0) devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 52, 28);
				else if (FIFO.first()->scratchpad_ID == 1) devices.at(deviceIndex)->Scratchpad_1 = htmlData.mid(i + 52, 28); } break;
		case ReadDualSwitch	: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 52, 4); } break;
		case ReadADC		: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 52, 26); } break;
		case ReadPage		: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = htmlData.mid(i + 56, 18); } break;
	}
	if ((deviceIndex >= 0) && (deviceIndex < devices.count())) logThis(QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") + "Device ScratchPad : " + devices.at(deviceIndex)->Scratchpad);
}




void ha7netthread::readFIFOSpecialBuffer(const QString &htmlData)
{
	QString strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = htmlData.indexOf(strsearch);
	if (i < 0) return;
	int deviceIndex = FIFOSpecial.first()->device_ID;
	switch (FIFOSpecial.first()->Request_ID)
	{
		case setChannelOn	:
		case setChannelOff	:	if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
						{
							QString scratchpad = htmlData.mid(i + 52, 8);
							devices.at(deviceIndex)->Return = scratchpad.right(2);
							devices.at(deviceIndex)->Scratchpad = ""; //scratchpad.mid(2, 4);
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Return));
						}
		break;
	}
}




QString ha7netthread::getScratchPad(const QString &RomID, int scratchpad_ID)
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




int ha7netthread::getConvertTime(device *dev)
{
	QString family = dev->RomID.right(2);
    if (family == family1820) return 750;
	if ((family == family1822) or (family == family18B20))
	{
		return 750;
		if (dev->Scratchpad.isEmpty()) return 750;
		bool ok;
		int resolution = dev->Scratchpad.mid(8, 2).toInt(&ok, 16);
		if (!ok) return 750;
		int P = resolution & 0x60;
		P /= 32;
		P = (3 - P);
		resolution = 750 / (2 ^ P);
		return resolution;
	}
	return 0;
}



void ha7netthread::addDeviceReadings(bool ALL)
{
	QString reqstr;
	if (GlobalConvert)
	{
		if (LockID.isEmpty()) reqstr = "Reset.html";
		else reqstr = "Reset.html?" + LockID;
		FIFOAppend(Reset, reqstr);
		if (LockID.isEmpty()) reqstr = "WriteBlock.html?Data=CC44";
		else reqstr = "WriteBlock.html?" + LockID + "&Data=CC44";
		FIFOAppend(ConvertTemp, reqstr);
	}
	for(int index=0; index<devices.count(); index++)
	{
		QString RomID = devices.at(index)->RomID;
		QString family = RomID.right(2);
		if (!GlobalConvert)
		{
			if (getConvertTime(devices.at(index)))
			{
				if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=44";
				else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=44";
				FIFOAppend(ConvertTempRomID, reqstr, index);
			}
		}
		if ((family == family1822) || (family == family1820)  || (family == family18B20))
		{
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=BEFFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=BEFFFFFFFFFFFFFFFFFF";
			FIFOAppend(ReadTemp, reqstr, index);
		}
		else if (family == family2408)
		{
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=F08800FFFFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=F08800FFFFFFFFFFFFFFFFFFFF";
			FIFOAppend(ReadPIO, reqstr, index);
		}
        else if (isFamily2413)
		{
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=F5FF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=F5FF";
			FIFOAppend(ReadDualSwitch, reqstr, index);
		}
		else if ((family == family2423) && ALL)
		{

			if (LockID.isEmpty())
		       {
				       reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=A5DF01FFFFFFFFFFFFFFFFFFFFFF";
				       FIFOAppend(ReadCounter, reqstr, index);
				       reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=A5FF01FFFFFFFFFFFFFFFFFFFFFF";
				       FIFOAppend(ReadCounter, reqstr, index, 1);
		       }
		       else
		       {
				       reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=A5DF01FFFFFFFFFFFFFFFFFFFFFF";
				       FIFOAppend(ReadCounter, reqstr, index);
				       reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=A5FF01FFFFFFFFFFFFFFFFFFFFFF";
				       FIFOAppend(ReadCounter, reqstr, index, 1);
		       }
		}
		else if (family == family2438)
		{
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=44";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=44";
			FIFOAppend(ConvertTempRomID, reqstr, index);

			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=B4";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=B4";
			FIFOAppend(ConvertV, reqstr, index);

			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=B800";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=B800";
			FIFOAppend(RecallMemPage00h, reqstr, index);

			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=BE00FFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=BE00FFFFFFFFFFFFFFFFFF";
            FIFOAppend(ReadPage00h, reqstr, index);

            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=B801";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=B801";
            FIFOAppend(RecallMemPage01h, reqstr, index);

            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=BE01FFFFFFFFFFFFFFFFFF";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=BE01FFFFFFFFFFFFFFFFFF";
            FIFOAppend(ReadPage01h, reqstr, index);
        }
		else if (family == family2450)
		{
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=3C0F00FFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=3C0F00FFFF";
			FIFOAppend(ConvertADC, reqstr, index);

			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=AA0000FFFFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=AA0000FFFFFFFFFFFFFFFFFFFF";
			FIFOAppend(ReadADC, reqstr, index);
		}
	}
}




void ha7netthread::addToFIFOSpecial(const QString &RomID, const QString &data, int Request_ID)
{
	if (FIFOSpecial.count() > 100) return;
	QString reqstr;
	if (RomID.isEmpty())
	{
		reqstr.clear();
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
						if (data.isEmpty())
						{
                            if (logEnabled) log += addTimeTag RomID + " setChannelOn empty data";
						}
						else
						{
							if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=5A" + data + "FF";
							else reqstr ="WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=5A" + data + "FF";
							FIFOSpecialAppend(setChannelOn, reqstr, index);
						}
						break;
					case setChannelOff :
						if (data.isEmpty())
						{
                            if (logEnabled) log += addTimeTag RomID + " setChannelOff empty data";
						}
						else
						{
							if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=5A" + data + "FF";
							else reqstr ="WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=5A" + data + "FF";
							FIFOSpecialAppend(setChannelOff, reqstr, index);
						}
						break;
					case WriteScratchpad00	:
						if (data.isEmpty())
						{
                            if (logEnabled) log += addTimeTag RomID + " WriteScratchpad00 empty data";
						}
						else
						{
							if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=4E00" + data;
							else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=4E00" + data;
							FIFOSpecialAppend(WriteScratchpad00, reqstr, index);
						}
						break;
					case CopyScratchpad00	:
						if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + RomID.left(16) + "&Data=4800";
						else reqstr = "WriteBlock.html?" + LockID + "&Address=" + RomID.left(16) + "&Data=4800";
						FIFOSpecialAppend(CopyScratchpad00, reqstr, index);
						break;
				}
			}
		}
	}
}





void ha7netthread::addToFIFOSpecial(const QString &data)
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
	logThis("FIFOSpecialAppend : " + data);
	mutexData.unlock();
}




void ha7netthread::FIFOAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID)
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





void ha7netthread::logThis(const QString &str)
{
    log_List.append(str);
    if (log_List.count() > 50) log_List.removeFirst();
    public_Log.clear();
    for (int n=0; n<log_List.count(); n++) public_Log.append(log_List.at(n) + "\n");
}



void ha7netthread::getLog(QString &str)
{
    QMutexLocker locker(&mutexData);
    str.append(public_Log);
    return ;
}



void ha7netthread::FIFOSpecialAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID)
{
	mutexData.lock();
	FIFOStruc *newFIFO = new FIFOStruc;
	newFIFO->Request_ID = reqID;
	newFIFO->Request = reqStr;
	newFIFO->scratchpad_ID = scratchpad_ID;
	if ((device_ID >= 0) && (device_ID < devices.count())) newFIFO->RomID = devices.at(device_ID)->RomID; else newFIFO->RomID = "";
	newFIFO->device_ID = device_ID;
	FIFOSpecial.append(newFIFO);
	logThis("FIFOSpecialAppend : " + reqStr);
	mutexData.unlock();
}






