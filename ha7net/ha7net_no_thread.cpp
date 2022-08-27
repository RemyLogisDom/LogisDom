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


#include "net1wire.h"

#ifdef HA7Net_No_Thread

#include "globalvar.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "onewire.h"
#include "messagebox.h"
#include "errlog.h"
#include "ha7net_no_thread.h"

ha7net::ha7net(logisdom *Parent) : net1wire(Parent)
{
	type = NetType(Ha7Net);
    setport(default_port);
	LockID = "";
	busy = false;
	initialsearch = true;
    reply = nullptr;
}
void ha7net::init()
{
	converttimer.setSingleShot(true);
	connect(this, SIGNAL(requestdone()), this, SLOT(fifonext()));
	connect(&converttimer, SIGNAL(timeout()), this, SLOT(timerconvertout()));
	TimerPause.setSingleShot(true);
	connect(&TimerPause, SIGNAL(timeout()), this, SLOT(fifonext()));
        framelayout = new QGridLayout(ui.frameguinet);
	int i = 0;

	Bouton1.setText(tr("Show"));
	connect(&Bouton1, SIGNAL(clicked()), this, SLOT(ShowDevice()));
	framelayout->addWidget(&Bouton1, i++, 0, 1, 1);
	
	//Bouton2.setText(tr("Show All"));
	//connect(&Bouton2, SIGNAL(clicked()), this, SLOT(voiralldevice()));
	//framelayout->addWidget(&Bouton2, i++, 0, 1, 1);
	
	//Bouton3.setText(tr("Initialise"));
	//connect(&Bouton3, SIGNAL(clicked()), this, SLOT(initialsearchreq()));
	//framelayout->addWidget(&Bouton3, i++, 0, 1, 1);
	
	Bouton4.setText(tr("Search"));
	connect(&Bouton4, SIGNAL(clicked()), this, SLOT(searchbutton()));
	framelayout->addWidget(&Bouton4, i++, 0, 1, 1);
	
	Bouton5.setText(tr("Conversion"));
	connect(&Bouton5, SIGNAL(clicked()), this, SLOT(convertSlot()));
	framelayout->addWidget(&Bouton5, i++, 0, 1, 1);
	
	Bouton6.setText(tr("Ha7Net config"));
	connect(&Bouton6, SIGNAL(clicked()), this, SLOT(Ha7Netconfig()));
	framelayout->addWidget(&Bouton6, i++, 0, 1, 1);
	
//	httpState.setText(tr("http : "));
//	connect(&httpState, SIGNAL(clicked()), this, SLOT(Ha7Netconfig()));
//	framelayout->addWidget(&httpState, i++, 0, 1, 1);

	LockEnable.setText(tr("Use 1 wire bus lock"));
	framelayout->addWidget(&LockEnable, i++, 0, 1, 1);
	
	Global_Convert.setText(tr("Global Convert Command"));
	framelayout->addWidget(&Global_Convert, i++, 0, 1, 1);
	
	ui.EditType->setText("HA7Net");

    ConvertDelay.setRange(0.1, 10);
    ConvertDelay.setDecimals(1);
    ConvertDelay.setValue(1);
    ConvertDelay.setSuffix(tr(" Seconds"));
    ConvertDelay.setSingleStep(0.1);
    framelayout->addWidget(&ConvertDelay, i++, 0, 1, 1);

	if (moduleipaddress == simulator) LockEnable.setChecked(false);
	ui.toolButtonTcpState->hide();
	addtofifo(Search);
}




ha7net::~ha7net()
{
	if (!LockID.isEmpty() and (moduleipaddress != simulator))
	{
		request = ReleaseLock;
		setrequest("ReleaseLock.html?" + LockID);
		return;
	}
}
void ha7net::Ha7Netconfig()
{
	  messageBox::aboutHide(this, tr("Ha7net Configuration"),
                           tr("Admin login :  user 'admin' password 'eds'\n"
                           "Default IP adress 192.168.0.250\n"
                           "Change Lock idle time out int the Miscellaneous tab to 5 seconds\n"
                           "\n"
                           "\n\n"));
}
void ha7net::getConfig(QString &str)
{
	if (LockEnable.isChecked()) str += logisdom::saveformat("LockID", "1"); else str += logisdom::saveformat("LockID", "0");
	if (Global_Convert.isChecked()) str += logisdom::saveformat("GlobalConvert", "1"); else str += logisdom::saveformat("GlobalConvert", "0");
	str += logisdom::saveformat("ConvertDelay", QString("%1").arg(ConvertDelay.value()));
}
void ha7net::setConfig(const QString &strsearch)
{
	bool Lock = false, GlobalConvert = false;
	if (logisdom::getvalue("LockID", strsearch) == "1") Lock = true;
	if (logisdom::getvalue("GlobalConvert", strsearch) == "1") GlobalConvert = true;
	LockEnable.setChecked(Lock);
	Global_Convert.setChecked(GlobalConvert);
	bool ok;
	double CD = logisdom::getvalue("ConvertDelay", strsearch).toDouble(&ok);
	if ((ok) and (CD > 0)) ConvertDelay.setValue(CD);
}
void ha7net::fifonext()
{
//	QMutexLocker locker(&mutexFifonext);
    QString order;
	if (moduleipaddress == simulator) LockEnable.setChecked(false);
	if (busy) return;
    if (reply)
    {
        QTimer::singleShot(5000, this, SLOT(fifonext()));
        return;
    }
next:
	busy = true;
	LockEnable.setEnabled(!busy);
	if (fifoListEmpty())
	{
		// if locked unlock 1 wire bus
		if (!LockID.isEmpty() and (moduleipaddress != simulator))
		{
			request = ReleaseLock;
			QString reqstr = "ReleaseLock.html?" + LockID;
			setrequest(reqstr);
			return;
		}
		busy = false;
		LockEnable.setEnabled(!busy);
		emit(fifoEmpty());
        //QTimer::singleShot(1000, this, SLOT(DeviceRealTime()));
		return;		// si le fifo est vide, on quitte
	}
	// if not locked lock 1 wire bus
	if (LockID.isEmpty() and (moduleipaddress != simulator) and (LockEnable.isChecked()))
	{
		request = GetLock;
		setrequest("GetLock.html");
		return;
	}
	QString next = fifoListNext();
        order = getOrder(next);
	if (order.isEmpty())
	{
		GenMsg(next + " returns order empty");
		fifoListRemoveFirst();
		goto next;
	}
	request = getorder(order);
	if (request == 0)
	{
		fifoListRemoveFirst();
		goto next;
	}
	QString reqRomID = getRomID(next);
	QString Data = getData(next);
	Prequest = parent->configwin->DeviceExist(reqRomID);
	if (Prequest) Prequest->setValid(onewiredevice::dataWaiting);
	QString reqstr;
	switch(request)
	{
		case None :
			simpleend();
			break;

		case Reset :			// commande reste 1 wire bus
			reqRomID = "";
			if (LockID.isEmpty()) reqstr = "Reset.html";
			else reqstr = "Reset.html?" + LockID;
			setrequest(reqstr);
			break;

		case Search :			// commande search 1 wire
			 reqRomID = "";

			if (LockID.isEmpty()) reqstr = "Search.html";
			else reqstr = "Search.html?" + LockID;
			setrequest(reqstr);
			break;

		case SkipROM :			// complement commande conversion, skip ROM
			reqRomID = "";
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Data=CC";
			else reqstr = "WriteBlock.html?" + LockID + "&Data=CC";
			setrequest(reqstr);
			break;

		case ReadPIO :
		case ReadRecPIO :
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=F08800FFFFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=F08800FFFFFFFFFFFFFFFFFFFF";
			setrequest(reqstr);
			break;

		case setChannelOn :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO  /PIO FF
			if (Prequest)
			{
				QString data = Prequest->getOnCommand();
				if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=5A" + data + "FF";
				else reqstr ="WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=5A" + data + "FF";
				setrequest(reqstr);
			}
			else GenError(28, "setChannelOn command aborted");
			break;

		case setChannelOff :
            // http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO /PIO FF
			if (Prequest)
			{
				QString data = Prequest->getOffCommand();
				if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=5A" + data + "FF";
				else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=5A" + data + "FF";
				setrequest(reqstr);
			}
			else GenError(28, "setChannelOn command aborted");
			break;

		case ReadDualSwitch :
		case ReadDualSwitchRec :
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=F5FF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=F5FF";
			setrequest(reqstr);
			break;

        case WritePIO :
            // http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO /PIO FF
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=5A" + Data + "FF";
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=5A" + Data + "FF";
                setrequest(reqstr);
            break;
        case ChannelAccessRead :
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=F545FFFFFFFFFF";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=F545FFFFFFFFFF";
            setrequest(reqstr);
        break;
        case ChannelAccessWrite :
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=F5" + Data + "FFFF";
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=F5" + Data + "FFFF";
                setrequest(reqstr);
        break;
        case ConvertTemp :			// commande conversion
			reqRomID = "";
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Data=CC44";
			else reqstr = "WriteBlock.html?" + LockID + "&Data=CC44";
			setrequest(reqstr);
			break;

		case ConvertTempRomID :			// commande conversion
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=44";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=44";
			setrequest(reqstr);
			break;

		case RecallMemPage00h :
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=B800";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=B800";
			setrequest(reqstr);
			break;

        case RecallMemPage01h :
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=B801";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=B801";
            setrequest(reqstr);
            break;

        case ConvertV :
			 if (reqRomID.isEmpty())
			 {
				if (LockID.isEmpty()) reqstr = "WriteBlock.html?Data=CCB4";
				else reqstr = "WriteBlock.html?" + LockID + "&Data=CCB4";
				setrequest(reqstr);
			}
			else
			{
				if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=B4";
				else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=B4";
				setrequest(reqstr);
			}
			 break;

		case ConvertADC :			// commande conversion
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=3C0F00FFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=3C0F00FFFF";
			setrequest(reqstr);
			break;
	
		case ReadADC :
		case ReadADCRec :
			// http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=AA0000FFFFFFFFFFFFFFFFFFFF
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=AA0000FFFFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=AA0000FFFFFFFFFFFFFFFFFFFF";
			setrequest(reqstr);
			break;

        case ReadADCPage01h :
            // http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=AA0008FFFFFFFFFFFFFFFFFFFF
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=AA0800FFFFFFFFFFFFFFFFFFFF";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=AA0800FFFFFFFFFFFFFFFFFFFF";
            setrequest(reqstr);
            break;

        case ReadPage :
        case ReadPage00h :
        case ReadPageRec :
        case ReadPageRec00h :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=BE00FFFFFFFFFFFFFFFFFF
			if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=BE00FFFFFFFFFFFFFFFFFF";
			else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=BE00FFFFFFFFFFFFFFFFFF";
			setrequest(reqstr);
			break;

        case ReadPage01h :
        case ReadPageRec01h :
            // http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=BE00FFFFFFFFFFFFFFFFFF
            if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=BE01FFFFFFFFFFFFFFFFFF";
            else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=BE01FFFFFFFFFFFFFFFFFF";
            setrequest(reqstr);
            break;

            case WriteMemory :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=55' + DataToWrite);
                // http://192.168.0.250/1Wire/WriteBlock.html?Address=6E00000003925E20&Data=55080000FFFFFF
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=55" + Data;
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=55" + Data;
                setrequest(reqstr);
                break;

            case WriteScratchpad00 :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data4E00=' + DataToWrite);
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=4E00" + Data;
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=4E00" + Data;
                setrequest(reqstr);
                break;

            case CopyScratchpad00 :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data4E00=';
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=4800";
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=4800";
                setrequest(reqstr);
                break;

            case WriteValText :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=' + DataToWrite);
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=" + Data;
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=" + Data;
                setrequest(reqstr);
                break;

            case WriteLed :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=' + DataToWrite);
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=" + Data;
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=" + Data;
                setrequest(reqstr);
                break;

            case ReadMemoryLCD :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=' + DataToWrite);
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=" + Data;
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=" + Data;
                setrequest(reqstr);
                break;

            case ReadTemp :
            case ReadTempRec :
                // http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=BEFFFFFFFFFFFFFFFFFF'
                if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=BEFFFFFFFFFFFFFFFFFF";
                else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=BEFFFFFFFFFFFFFFFFFF";
                setrequest(reqstr);
                break;

            case ReadCounter :
            case ReadCounterRec :
                // http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=A5DF01FFFFFFFFFFFFFFFFFFFFFF'
                 if (LockID.isEmpty())
                {
                    if ((reqRomID.right(4) == family2423_A) or (reqRomID.right(4).left(2) == familyLCDOW))
                    {
                        reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=A5DF01FFFFFFFFFFFFFFFFFFFFFF";
                        setrequest(reqstr);
                    }
                    else if (reqRomID.right(4) == family2423_B)
                    {
                        reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=A5FF01FFFFFFFFFFFFFFFFFFFFFF";
                        setrequest(reqstr);
                    }
                    else	simpleend();
                }
                else
                {
                     if ((reqRomID.right(4) == family2423_A) or (reqRomID.right(4).left(2) == familyLCDOW))
                    {
                        reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=A5DF01FFFFFFFFFFFFFFFFFFFFFF";
                        setrequest(reqstr);
                    }
                    else if (reqRomID.right(4) == family2423_B)
                    {
                        reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=A5FF01FFFFFFFFFFFFFFFFFFFFFF";
                        setrequest(reqstr);
                    }
                    else	simpleend();
                }
                break;

            case WriteScratchpad :
                // http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=4E' + DataToWrite);
                    if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=4E" + Data;
                    else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=4E" + Data;
                    setrequest(reqstr);
                break;

            case WriteEEprom :
                // http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=48'
                    if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=48";
                    else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=48";
                    setrequest(reqstr);
                break;

            case RecallEEprom :
                // http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=B8'
                    if (LockID.isEmpty()) reqstr = "WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=B8";
                    else reqstr = "WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=B8";
                    setrequest(reqstr);
                break;

            default : simpleend();
    }
}
bool ha7net::checkbusshorted(const QString &data)
{
// search buffer for    "Exception_String_0" TYPE="text" VALUE="Short detected on 1-Wire Bus"
	int i;
	QByteArray strsearch;
	strsearch = "\"Exception_String_0\" TYPE=\"text\" VALUE=\"Short detected on 1-Wire Bus\"";
	i = data.indexOf(strsearch);
	if (i != -1) return true; else return false;
}
bool ha7net::checkLockIDCompliant(const QString &data)
{
// search buffer for   "A BLOCK of data may not be empty"
	int j, k;
	QByteArray strsearch;
	j = data.indexOf("Lock ID");
	k = data.indexOf("does not exist");
	if ((j != -1) and (k != -1)) return true; 
	return false;
}
void ha7net::convertSlot()
{
	convert();
}
void ha7net::convert()
{
#ifdef HA7Net_No_Thread
	if (Global_Convert.isChecked())
	{
		addtofifo(Reset);
		addtofifo(ConvertTemp);
	}
	else
	{
		for (int n=0; n<localdevice.count(); n++)
		{
			QString family = localdevice.at(n)->getfamily();
            if ((family == family1820) || (family == family18B20) || (family == family1822) || (family == family1825))
			{
				QString RomIDLeft = localdevice.at(n)->getromid().left(16);
				addtofifo(ConvertTempRomID, RomIDLeft);
			}
		}
	}
#endif
}
void ha7net::setrequest(const QString &req)
{
	if (moduleipaddress == simulator)
	{
		settraffic(Disabled);
		settraffic(Connecting);
		settraffic(Disconnected);
		settraffic(Connected);
		settraffic(Paused);
		settraffic(Simulated);
		settraffic(Waitingforanswer);
		QString filename = QString(repertoiresimulator) + QDir::separator() + req;
		filename.remove(QChar('?'));
		filename.remove(QChar('&'));
		GenMsg("SetRequest : " + filename);
		QFile file(filename);
		if (!file.exists())
		{
			GenMsg(tr("cannot find simulated file  ") + filename);
			busy = false;
			LockEnable.setEnabled(!busy);
			return;
		}
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			GenMsg(tr("cannot open simulated file  ") + filename);
			busy = false;
			LockEnable.setEnabled(!busy);
			return;
		}
		busy = false;
		LockEnable.setEnabled(!busy);
		QString data_rec = file.readAll();
		httpRequestAnalysis(data_rec);
		file.close();
	}
	else
	{
		settraffic(Waitingforanswer);
		QString reqstr = "http://" + moduleipaddress + QString(":%1").arg(port) + "/1Wire/" + req;
		QUrl url(reqstr);
		startRequest(url);
		GenMsg(reqstr);
	}
}


void ha7net::startRequest(QUrl url)
{
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
}


void ha7net::httpFinished()
{
    QNetworkReply* newreply = reply;
    reply = nullptr;
    busy = false;
	LockEnable.setEnabled(!busy);
    if (newreply->error() != QNetworkReply::NoError)
	{
        GenError(51, newreply->errorString());
		if (httperrorretry < 3)
		{
			httperrorretry ++;
			LockID = "";
			QTimer::singleShot(5000, this, SLOT(fifonext()));
		}
		else
		{
			httperrorretry = 0;
			if (initialsearch == false) fifoListRemoveFirst();
			request = None;
			emit requestdone();
		}
        newreply->deleteLater();
		return;
	}
	else
	{
		httperrorretry = 0;
		settraffic(Connected);
        QString data_rec = newreply->readAll();
		httpRequestAnalysis(data_rec);
        newreply->deleteLater();
    }
}


void ha7net::httpRequestAnalysis(const QString &data)
{
	if (data.isEmpty())
	{
		settraffic(Disconnected);
		GenMsg(tr("httpRequestAnalysis : No Data"));
		fifoListRemoveFirst();
		emit requestdone();
		return;
	}
	else if (checkbusshorted(data)) 
	{
		GenError(56, data);
		TimerPause.start(2000);
	}
	else if (checkLockIDCompliant(data)) 
	{
		int i = data.indexOf("Lock ID");
		QString str = "Actual Lock ID = " + LockID + "   Refused Lock ID : " + data.mid(i, 33);
		GenError(87, str);
		LockID = "";
		TimerPause.start(5000);
	}
	else
	{
		switch(request)
		{
			case None : simpleend(); break;
			case Reset : simpleend(); break;
			case SkipROM : simpleend(); break;
			case WriteEEprom : simpleend(); break;
			case RecallEEprom : simpleend(); break;
			case WriteScratchpad : simpleend(); break;
			case RecallMemPage00h : simpleend(); break;
			case WriteScratchpad00 : simpleend(); break;
			case CopyScratchpad00 : simpleend(); break;
			case ConvertTemp : convertendtemp(); break;
            case WriteValText	: simpleend(); emit requestdone(); break;
            case WriteLed : endofwriteled(data); emit requestdone(); break;
            case ReadMemoryLCD	: endofreadmemorylcd(data); emit requestdone(); break;
            case ConvertTempRomID : convertendtemp(); break;
			case ConvertADC : convertendadc(data); break;
			case WriteMemory : endofwritememory(data); emit requestdone(); break;
			case GetLock : endofgetlock(data); emit requestdone(); break;
			case Search : endofsearchrequest(data); emit requestdone(); break;
			case ReleaseLock : endofreleaselock(data); emit requestdone(); break;
			case ReadPIO : endofreadpoirequest(data, false); emit requestdone(); break;
			case ReadRecPIO : endofreadpoirequest(data, true); emit requestdone(); break;
			case setChannelOn : endofsetchannel(data, false); emit requestdone(); break;
			case setChannelOff : endofsetchannel(data, false); emit requestdone(); break;
            case WritePIO : endofsetchannel(data, false); emit requestdone(); break;
            case ChannelAccessRead : endofchannelaccessread(data, false); emit requestdone(); break;
            case ChannelAccessWrite : endofchannelaccesswrite(data, false); emit requestdone(); break;
            case ReadTemp : endofreadtemprequest(data, false); emit requestdone();  break;
			case ReadTempRec : endofreadtemprequest(data, true); emit requestdone(); break;
			case ReadCounter : endofreadcounter(data, false); emit requestdone(); break;
			case ReadCounterRec : endofreadcounter(data, true); emit requestdone(); break;
			case ReadDualSwitch : endofreaddualswitch(data, false); emit requestdone(); break;
			case ReadDualSwitchRec : endofreaddualswitch(data, true); emit requestdone(); break;
			case ReadADC : endofreadadcrequest(data, false); emit requestdone();  break;
			case ReadADCRec : endofreadadcrequest(data, true); emit requestdone();  break;
            case ReadADCPage01h : endofreadadcpage01h(data, false); emit requestdone();  break;
            case ReadPage : endofreadpage(data, false); emit requestdone();  break;
            case ReadPage00h : endofreadpage(data, false); emit requestdone();  break;
            case ReadPage01h : endofreadpage01h(data, false); emit requestdone();  break;
            case ReadPageRec : endofreadpage(data, true); emit requestdone();  break;
            case ReadPageRec00h : endofreadpage(data, true); emit requestdone();  break;
            case ReadPageRec01h : endofreadpage01h(data, true); emit requestdone();  break;
            default : simpleend();
		}
	}
}


void ha7net::simpleend()
{
	request = None;
	fifoListRemoveFirst();
	if (((request != ConvertTemp) or (request != ConvertADC) or (request != ConvertV) or (request != ConvertTempRomID)) and (request != NetError)) emit requestdone();
}


void ha7net::convertendtemp()
{
	int convTime = getConvertTime(Prequest);
	converttimer.start(convTime);
}


int ha7net::getConvertTime(onewiredevice *dev)
{
    if (!dev) return ConvertDelay.value() * 1000.0;
    QString family = dev->getromid().right(2);
    if (family == family1820) return 200;
    if ((family == family1822) or (family == family18B20) or (family == family1825))
    {
        //return ConvertDelay.value() * 1000.0;
        if (dev->getscratchpad().isEmpty()) return 750;
        bool ok;
        int resolution = dev->getscratchpad().mid(8, 2).toInt(&ok, 16);
        if (!ok) return 750;
        int P = resolution & 0x60;
        P /= 32;
        P = (3 - P);
        resolution = 750 / (2 ^ P);
        return resolution;
    }
    return 0;
}


void ha7net::convertendadc(const QString &data)
{
QString match;
QByteArray str, strsearch;
	strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 10);
		GenMsg(tr("End Convert ADC : ") + match);
	}
	// CRC Check
	int convTime = getConvertTime(Prequest);
	converttimer.start(convTime);
}


void ha7net::timerconvertout()
{
	fifoListRemoveFirst();
	busy = false;
	LockEnable.setEnabled(!busy);
	emit requestdone();
}


void ha7net::endofgetlock(const QString &data)
{
QString strsearch, match;
int i, l, n;
	// NAME="LockID_0" SIZE="10" MAXLENGTH="10" VALUE="668232839"
	strsearch = "NAME=\"LockID_0\" SIZE=\"10\" MAXLENGTH=\"10\" VALUE=\"";
	l = strsearch.length();
	i = data.indexOf(strsearch);
	if (i != -1)
	{
		n = data.indexOf("\"", i + l);
		match =  data.mid(i + l, n - i - l);	
		GenMsg("Lock ID = " + match);
		LockID = "LockID=" + match;
		request = None;
		busy = false;
		LockEnable.setEnabled(!busy);
	}
}


void ha7net::endofreleaselock(const QString&)
{
	GenMsg(LockID + "  Unlocked");
	LockID = "";
	request = None;
	busy = false;
	LockEnable.setEnabled(!busy);
}


void ha7net::endofsearchrequest(const QString &data)
{
// search buffer for CLASS="HA7Value" NAME="Address_0" ID="ADDRESS_0" TYPE="text" VALUE=
QString match;
int pos, indexstringsearch = 0;
QByteArray strsearch;
//qDebug() << QString("end of search");
//	strsearch = "INPUT CLASS=\"HA7Value\" NAME=\"Address_" + str.setNum(indexstringsearch) + "\" ID=\"ADDRESS_" + str.setNum(indexstringsearch) + "\" TYPE=\"text\" VALUE=";
	strsearch = "TYPE=\"text\" VALUE=\"";
	pos = data.indexOf(strsearch);
	do
	{
		if ((pos != -1) and (data.mid(pos + 35, 2)) == "\">")		// has found item
		{
			//match = data.mid(pos + 74, 16);		// extract interesting data : RomID
			match = data.mid(pos + 19, 16);		// extract interesting data : RomID
			newDevice(match);
			indexstringsearch ++;
			//strsearch = "INPUT CLASS=\"HA7Value\" NAME=\"Address_" + str.setNum(indexstringsearch) + "\" ID=\"ADDRESS_" + str.setNum(indexstringsearch) + "\" TYPE=\"text\" VALUE=";
			strsearch = "TYPE=\"text\" VALUE=\"";
		}
		pos = data.indexOf(strsearch, pos + 1);
	} while(pos > 0);
	request = None;
	fifoListRemoveFirst();
	initialsearch = false;
	busy = false;
	LockEnable.setEnabled(!busy);
}


void ha7net::endofreadtemprequest(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE
    QString match;
    QByteArray strsearch;
    strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
    int i = data.indexOf(strsearch);
    if (i != -1)
    {
        match = data.mid(i + 54, 18);
        if (!Prequest)
        {
            GenError(38, data);
            return;
        }
        if (!Prequest->setscratchpad(match, enregistre))
        {
            GenError(39, Prequest->getromid() + " endofreadtemprequest Scratchpad = " + match);
            return;
        }
    }
    else GenError(43, data);
    GenMsg(tr("End Reading Temperature : ") + match);
    fifoListRemoveFirst();
}

void ha7net::endofwriteled(const QString &data)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE
    QString match;
    QByteArray strsearch;
    strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
    int i = data.indexOf(strsearch);
    if (i != -1)
    {
        match = data.mid(i + 52, 8);
        if (!Prequest)
        {
            GenError(38, data);
            return;
        }
        if (!Prequest->setscratchpad(match))
        {
            GenError(39, Prequest->getromid() + " endofwriteled Scratchpad = " + match);
            return;
        }
    }
    else GenError(43, data);
    GenMsg(tr("End writing LED : ") + match);
    fifoListRemoveFirst();
}

void ha7net::endofreadmemorylcd(const QString &data)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE
QString match;
QByteArray strsearch;
    strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
    int i = data.indexOf(strsearch);
    if (i != -1)
    {
        match = data.mid(i + 52, 24);
        if (!Prequest)
        {
            GenError(38, data);
            return;
        }
        if (!Prequest->setscratchpad(match))
        {
            GenError(39, Prequest->getromid() + " endofreadmemorylcd Scratchpad = " + match);
            return;
        }
    }
    else GenError(43, data);
    GenMsg(tr("End reading LCD : ") + match);
    fifoListRemoveFirst();
}
void ha7net::endofreadpoirequest(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="
QString match;
QByteArray strsearch;
	strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 26);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofreadpoirequest Scratchpad = " + match);
            return;
		}
	}
	else GenError(43, data);
	GenMsg(tr("End Reading PIO : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofchannelaccessread(const QString &data, bool enregistre)
{
    // search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="
    QString match;
    QByteArray strsearch;
        strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
        int i = data.indexOf(strsearch);
        if (i != -1)
        {
            match = data.mid(i + 52, 14);
            if (!Prequest)
            {
                GenError(38, data);
                return;
            }
            if (!Prequest->setscratchpad(match, enregistre))
            {
                GenError(39, Prequest->getromid() + " endofchannelaccessread Scratchpad = " + match);
                return;
            }
        }
        else GenError(43, data);
        GenMsg(tr("End SetChannel : ") + match);
        fifoListRemoveFirst();
}

void ha7net::endofchannelaccesswrite(const QString &data, bool enregistre)
{
    // search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="
    QString match;
    QByteArray strsearch;
        strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
        int i = data.indexOf(strsearch);
        if (i != -1)
        {
            match = data.mid(i + 52, 16);
            if (!Prequest)
            {
                GenError(38, data);
                return;
            }
            if (!Prequest->setscratchpad(match, enregistre))
            {
                GenError(39, Prequest->getromid() + " endofchannelaccesswrite Scratchpad = " + match);
                return;
            }
        }
        else GenError(43, data);
        GenMsg(tr("End SetChannel : ") + match);
        fifoListRemoveFirst();
}

void ha7net::endofsetchannel(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="
QString match;
QByteArray strsearch;
	strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 8).right(2);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofsetchannel Scratchpad = " + match);
            return;
		}
	}
	else GenError(43, data);
	GenMsg(tr("End SetChannel : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofreadpage(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE

QString match;
QByteArray strsearch;

	strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 56, 18);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofreadpage Scratchpad = " + match);
            return;
		}
	}
	else GenError(43, data);
	GenMsg(tr("End Reading Page : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofreadpage01h(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE

QString match;
QByteArray strsearch;

    strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
    int i = data.indexOf(strsearch);
    if (i != -1)
    {
        match = "page01h=(" + data.mid(i + 56, 18) + ")";
        if (!Prequest)
        {
            GenError(38, data);
            return;
        }
        if (!Prequest->setscratchpad(match, enregistre))
        {
            GenError(39, Prequest->getromid() + " endofreadpage01h Scratchpad = " + match);
            return;
        }
    }
    else GenError(43, data);
    GenMsg(tr("End Reading Page : ") + match);
    fifoListRemoveFirst();
}

void ha7net::endofreadcounter(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE

QString match;
QByteArray strsearch;
	strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 28);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofreadcounter Scratchpad = " + match);
            return;
		}
	}
	GenMsg(tr("End Reading Counter : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofreaddualswitch(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE
QString match;
	QByteArray strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 4);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofreaddualswitch Scratchpad = " + match);
            return;
		}
	}
	GenMsg(tr("End Reading Dual Switch : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofreadadcrequest(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE

QString match;
	QByteArray strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
	int i = data.indexOf(strsearch);
	if (i != -1)
	{
		match = data.mid(i + 52, 26);
		if (!Prequest)
		{
			GenError(38, data);
			return;
		}
		if (!Prequest->setscratchpad(match, enregistre))
		{
            GenError(39, Prequest->getromid() + " endofreadadcrequest Scratchpad = " + match);
            return;
		}
	}
	GenMsg(tr("End Reading ADC : ") + match);
	fifoListRemoveFirst();
}

void ha7net::endofreadadcpage01h(const QString &data, bool enregistre)
{
// search buffer for    NAME="ResultData_0" SIZE="56" MAXLENGTH="56" VALUE="BE

QString match;
    QByteArray strsearch = "NAME=\"ResultData_0\" SIZE=\"56\" MAXLENGTH=\"56\" VALUE=\"";
    int i = data.indexOf(strsearch);
    if (i != -1)
    {
        match = "page01h=(" + data.mid(i + 52, 26) + ")";
        if (!Prequest)
        {
            GenError(38, data);
            return;
        }
        if (!Prequest->setscratchpad(match, enregistre))
        {
            GenError(39, Prequest->getromid() + " endofreadadcpage01h Scratchpad = " + match);
            return;
        }
    }
    GenMsg(tr("End Reading ADC : ") + match);
    fifoListRemoveFirst();
}

void ha7net::endofwritememory(const QString&)
{
	request = None;
	fifoListRemoveFirst();
}

#endif // HA7Net_No_Thread
