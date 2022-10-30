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





#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QtNetwork>
#include "globalvar.h"
#include "graphconfig.h"
#include "addProgram.h"
#include "server.h"
#include "meteo.h"
#include "vmc.h"
#include "energiesolaire.h"
#include "chauffageunit.h"
#include "addDaily.h"
#include "tableauconfig.h"
#include "configwindow.h"
#include "onewire.h"
#include "programevent.h"
#include "tcpdata.h"
#include "quazip.h"
#include "quazipfile.h"
#include "logisdom.h"
#include "connection.h"



Connection::Connection(QTcpSocket *socket, Server *parent)
{
    Parent = parent;
    tcp = socket;
    webFolder = parent->parent->getrepertoirehtml();
    connect(tcp, SIGNAL(disconnected()), this, SLOT(clientEnd()));
    connect(tcp, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
	Privilege = Server::NoRigths;
	isHttp = false;
}




Connection::~Connection()
{
}





void Connection::clientEnd()
{
	emit(clientend(this));
	deleteLater();
}






QString Connection::getName()
{
	return UserName;
}




void Connection::sendAll()
{
    if (tcp->bytesAvailable() > 0) return;
	sendScratchPads();
	sendMainValue();
}





void Connection::transferCommand(QString order)
{
//GenMsg("Transfert : " + order);
	if (isHttp) return;	// problem : transfertToOthers from server add string to http incoming connection
    //QByteArray header;
    //QByteArray configdata; deprecated update 06-10-21
    QString header;
    QString configdata;
	configdata.append(order);
	header.append(headerStart"\n");
	header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
	header.append(logisdom::saveformat(CompressedData, "0"));
	header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
	header.append(logisdom::saveformat(RequestStr, NetRequestMsg[TransferCommand]));
	header.append(headerEnd"\n");
    if (tcp->isValid()) tcp->write(header.toLatin1());
    tcp->waitForBytesWritten(10000);
    if (tcp->isValid()) tcp->write(configdata.toLatin1());
    tcp->waitForBytesWritten(10000);
}




void Connection::sendScratchPads()
{
	if (Privilege == Server::NoRigths) return;
    // deprecated update 06-10-21
    QString configdata, rawdata;
	QString str;
	maison1wirewindow->configwin->GetDevicesScratchpad(str);
	rawdata.append(str); // to UTF8 ici
	//configdata.append(qCompress(rawdata));
	configdata.append(rawdata);
    QString header;
	header.append(headerStart"\n");
	header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
	//header.append(logisdom::saveformat(CompressedData, "1"));
	header.append(logisdom::saveformat(CompressedData, "0"));
	header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
	header.append(logisdom::saveformat(RequestStr, NetRequestMsg[SendDeviceScratchpad]));
	header.append(headerEnd"\n");
    if (tcp->isValid()) tcp->write(header.toLatin1());
    tcp->waitForBytesWritten(10000);
    if (tcp->isValid()) tcp->write(configdata.toLatin1());
    tcp->waitForBytesWritten(10000);
}


//  http://127.0.0.1:1221/request=(details)webid=(070476b8-a1a8-4fd0-afb7-1a728002bb77)menuid=(b6d51585-efc2-4273-9edf-f161319f58cd)
//  http://127.0.0.1:1221/request=(schema.htm)webid=(070476b8-a1a8-4fd0-afb7-1a728002bb77)menuid=(b6d51585-efc2-4273-9edf-f161319f58cd)

void Connection::sendMainValue()
{
	if (Privilege == Server::NoRigths) return;
    // QByteArray configdata, rawdata;     // deprecated update 06-10-21
    QString configdata, rawdata;
	QString str;
	maison1wirewindow->configwin->GetDevicesScratchpad(str);
	rawdata.append(str);
	//configdata.append(qCompress(rawdata));
	configdata.append(rawdata);
    QString header;
	header.append(headerStart"\n");
	header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
	//header.append(logisdom::saveformat(CompressedData, "1"));
	header.append(logisdom::saveformat(CompressedData, "0"));
	header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
	header.append(logisdom::saveformat(RequestStr, NetRequestMsg[SendMainValue]));
	header.append(headerEnd"\n");
    if (tcp->isValid()) tcp->write(header.toLatin1());
    tcp->waitForBytesWritten(10000);
    if (tcp->isValid()) tcp->write(configdata.toLatin1());
    tcp->waitForBytesWritten(10000);
}

//"index.html"
//"login?name=A1&password=B2"

void Connection::extractrequest(QByteArray &data, QString *str)
{
	str->append("HTTP/1.0 200 OK\r\n");
    str->append("Content-Type: text/html; charset=UTF-8\r\n\r\n");
    QString txt;
	QString Data = QUrl::fromPercentEncoding(data);
    //qDebug() << Data;
    if (Data.startsWith("index.htm"))
    {
       str->append("</style></head>"\
       "<form action=\"login\">\
       <input type=\"name\" name=\"name\" id=\"name\" maxlength=\"30\"><label for=\"name\"> Login Name</label>\
       <input type=\"password\" name=\"password\" id=\"password\" maxlength=\"30\"><label for=\"password\"> Password</label>\
       <br><input type=\"submit\" value=\"Submit\">\
       </form>");
        return;
    }
	QString request = logisdom::getvalue(CRequest, Data);
    QString log;
    QHostAddress adr(tcp->peerAddress().toIPv4Address());
    QDateTime now = QDateTime::currentDateTime();
    log.append(now.toString("HH:mm:ss  :  ") + "open socket : " + adr.toString() + " " + Data);
    emit(newRequest(log));
	QString user = logisdom::getvalue(CUser, Data);
	QString password = logisdom::getvalue(CPsw, Data);
	QString id = logisdom::getvalue(CUId, Data);
	int indexID = -1;
	int indexUser = -1;
	int count = Parent->ConnectionUsers.count();
	if (!user.isEmpty())
	{
		for (int n=0; n<count; n++)
				if (Parent->ConnectionUsers[n].Name == user) indexUser = n;
		if (indexUser != -1)
			if (Parent->ConnectionUsers[indexUser].PassWord == password)
				// Generate new ID when user and password is verifed
				{
					indexID = Parent->GetNewId(indexUser);
					id = Parent->ConnectionIDs[indexID].WebID;
				}
	}
	// if user and password failed check is ID is valabe
	if (indexID == -1) indexID = Parent->isIDValable(id);
	// Direct request without password check
    int PngRotation = 0;
    if (indexID == -1)
	{
        // Rotated icon request check
        QStringList split = Data.split(".");
        if (split.count() >0)
        {
            QString str = split.last();
            if (str.startsWith("R"))
            {
                QString StrRotation = str.remove(".R");
                bool ok;
                PngRotation = StrRotation.toInt(&ok);
                if (!ok) PngRotation = 0;
            }
        }
		bool done = false;
		// check if png request
		if (Data.right(4) == ".png") done = writePng(Data);
		if ((!done) && (Data.right(4) == ".png")) done = writeWebFile(Data, "png");
		if ((!done) && (Data.right(4) == ".gif")) done = writeWebFile(Data, "gif");
		if (!done)
		{
			QFile webFile(webFolder + Data);
			if (webFile.exists())
			{
                if (webFile.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QTextStream in(&webFile);
                    QString text;
                    text = in.readAll();
                    webFile.close();
                    Parent->parent->htmlTranslate(text, id);
                    str->append(text);
                    Parent->setLastPageWeb(id, request);
                }
            }
			else
			{
                str->append(tr("Access denied\n") + Data);
			}
		}
		return;
	}
// PNG
	if (request.right(4) == ".png")
	{
        if (PngRotation == 0)
        {
            writeWebFile(request, "png");
            return;
        }
	}
// web command
	if (Data.contains("command=("))
	{
		int indexstart = Data.indexOf("command=(");
        if (indexstart != -1)
		{
			int indexend = Data.indexOf(")", indexstart);
			if (indexend != -1)
			{
                QString command = Data.remove(0, indexstart + 9);
                command = command.mid(0, indexend - indexstart - 9);
                if (Parent->ConnectionIDs[indexID].Rigths == Server::FullControl) Parent->parent->htmlLinkCommand(command);
                request = Parent->getLastPageWeb(id);
                QFile webFile(webFolder + request);
                int page = maison1wirewindow->htmlPageExist(request);
                if (page != -1)
                {
                    QString html;
                    maison1wirewindow->getTabHtml(page, html);
                    Parent->parent->htmlTranslate(html, id);
                    str->append(html);
                    Parent->setLastPageWeb(id, request);
                }
                else if (webFile.exists())
                {
                    if (webFile.open(QIODevice::ReadOnly))
                    {
                        QTextStream in(&webFile);
                        QString text;
                        text = in.readAll();
                        webFile.close();
                        Parent->parent->htmlTranslate(text, id);
                        str->append(text);
                        Parent->setLastPageWeb(id, request);
                    }
                }
                return;
            }
        }
    }
// web page
	if ((request.right(4) == ".htm") or (request.right(5) == ".html"))
	{
        bool trans = true;
        if (request.startsWith("notrans_"))
        {
            trans = false;
            request = request.remove(0, 8);
        }
		QFile webFile(webFolder + request);
        int page = maison1wirewindow->htmlPageExist(request);
        if (page != -1)
        {
            QString html;
            maison1wirewindow->getTabHtml(page, html);
            Parent->parent->htmlTranslate(html, id);
            str->append(html);
            Parent->setLastPageWeb(id, request);
        }
        else if (webFile.exists())
		{
            if (webFile.open(QIODevice::ReadOnly))
            {
                QTextStream in(&webFile);
                QString text;
                text = in.readAll();
                webFile.close();
                if (trans) Parent->parent->htmlTranslate(text, id);
                str->append(text);
                Parent->setLastPageWeb(id, request);
            }
        }
		else
		{
			str->append(request + "\n");
			str->append(tr("Page not found"));
		}
		return;
	}
// htmlRemote
	str->append(maison1wirewindow->getHtmlRequest(Data, Parent->ConnectionIDs[indexID].WebID, Parent->ConnectionIDs[indexID].Rigths));
// X10 ...
	if (request == NetRequestMsg[SetDevice])
	{
		QString RomID = logisdom::getvalue(RomIDMark, Data);
		onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
		if (device)
		{
			device->SetOrder(logisdom::getvalue("order", Data));
			request = Parent->ConnectionIDs[indexID].LastMenu;
		}
	}
// eco confort ...
	if (request == NetRequestMsg[SwitchProgram])
	{
		maison1wirewindow->ProgEventArea->SwitchPrg(logisdom::getvalue(SetPrgMark, Data));
		request = Parent->ConnectionIDs[indexID].LastMenu;
	}
// ConfirmRestart
	if (request == NetRequestMsg[ConfirmRestart])
	{
		str->append(maison1wirewindow->toHtml(tr("Application will restart in 5 secondes ..."), NetRequestMsg[ConfirmRestart], id, logisdom::htmlStyleMenu));
        QTimer::singleShot(5000, maison1wirewindow, SLOT(restartnoconfirm()));
		return;
	}
// add Main menu from confiwin according request
	maison1wirewindow->configwin->GetMenuHtml(&txt, Parent->ConnectionIDs[indexID].WebID, Parent->ConnectionIDs[indexID].Rigths, request);
	Parent->ConnectionIDs[indexID].LastMenu = request ;
	str->append(txt);
	if (str->isEmpty()) str->append(tr("Error"));
	return;
}




bool Connection::writeWebFile(const QString &Data, const QString &type)
{
	QString str;
    QStringList strSplit = Data.split(".");
    if (strSplit.count() > 2)
    {
        QString strRotation = strSplit.first();
        if (strRotation.startsWith("R="))
        {
            QString strR = strRotation.right(strRotation.length() - 2);
            bool ok;
            int rotation = strR.toInt(&ok);
            QString fileName = Data.right(Data.length() - strRotation.length() - 1);
            QFile picture(fileName);
            if (picture.exists())
            {
                //qDebug() << fileName;
                str.append("HTTP/1.0 200 OK\r\n");
                str.append("Content-type: image/" + type  + "\r\n\r\n");
                QPixmap pixmap;
                pixmap.load(fileName);
                if (pixmap.isNull()) return false;
                QTransform t;
                t.rotate(rotation);
                QPixmap RotatedPixmap = pixmap.transformed(t);
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::ReadWrite);
                RotatedPixmap.save(&buffer, "PNG"); // writes pixmap into bytes in PNG format
                if (tcp->isValid()) tcp->write(str.toUtf8());
                tcp->waitForBytesWritten(10000);
                buffer.reset();
                if (tcp->isValid()) tcp->write(buffer.readAll());
                tcp->waitForBytesWritten(10000);
                return true;
            }
            else return false;
        }
    }
    QFile webFile(webFolder + Data);
    if (!webFile.exists()) webFile.setFileName(Data);
    //qDebug() << Data;
    if (webFile.exists())
	{
		str.append("HTTP/1.0 200 OK\r\n");
		str.append("Content-type: image/" + type  + "\r\n\r\n");
		if (webFile.open(QIODevice::ReadOnly))
		{
            if (tcp->isValid()) tcp->write(str.toUtf8());
            tcp->waitForBytesWritten(10000);
            if (tcp->isValid()) tcp->write(webFile.readAll());
            tcp->waitForBytesWritten(10000);
            webFile.close();
			return true;
		}
	}
    else writePng(Data);
	return false;
}



// complete page html converted to png
bool Connection::writePng(const QString &Data)
{
	QString str;
	int l = Data.length();
	QString name = Data.mid(0, l - 4);
	QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    maison1wirewindow->getTabPix(name, buffer);
    if (buffer.size())
	{
		str.append("HTTP/1.0 200 OK\r\n");
		str.append("Content-type: image/png\r\n\r\n");
		buffer.reset();
        if (tcp->isValid()) tcp->write(str.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(buffer.readAll());
        tcp->waitForBytesWritten(10000);
        buffer.close();
		return true;
	}
	return false;
}




void Connection::processReadyRead()
{
    QString Msg, extract, order;
	QByteArray data;
more:
    data = tcp->readAll();
	QString str;
	str.append(data);
	maison1wirewindow->GenMsg(str);
    maison1wirewindow->GenMsg("Local address : " + tcp->localAddress().toString());
    maison1wirewindow->GenMsg("Peer address : " + tcp->peerAddress().toString());
	extract = extractBuffer(data);
	if (extract.isEmpty())
	{
// http request
		int begin = data.indexOf("GET /");
		if (begin != -1)
		{
			isHttp = true;
			maison1wirewindow->GenMsg("http request detected = ");
            QString linkStr;
            linkStr.append(data);
			int end = data.indexOf(" ", begin + 5);
			if (end != -1)
			{
				QByteArray link = data.mid(begin + 5, end - begin - 5);
				QString strHtml;
                                //GenMsg("HTTP Request = " + link);
                                extractrequest(link, &strHtml);
				data.clear();
                //data.append(strHtml.toLocal8Bit());
                data.append(strHtml.toUtf8());
                                try				\
                                {
                                    if (tcp->isValid()) tcp->write(data);
                                    tcp->waitForBytesWritten(10000);
                                    tcp->disconnectFromHost();
				}
				catch(...)
				{
					QDateTime now = QDateTime::currentDateTime();
					QFile file("exception.log");
					if (file.open(QIODevice::Append | QIODevice::Text))
					{
						QTextStream out(&file);
						out << now.toString() + "  Exception :  " + data + "\n";
						file.close();
					}
                }
			}
		} 
		else maison1wirewindow->GenMsg("http request fail");
		return;
	}
	order = getOrder(extract);
	maison1wirewindow->GenMsg("Order : " + order);
	maison1wirewindow->GenMsg("extract : " + extract);
// SetUserName
	if (order.contains(NetRequestMsg[SetUserName]))
	{
		UserName = logisdom::getvalue(NetRequestMsg[SetUserName], order);
		if (UserName.isEmpty())
		{
			Msg = tr("login aborted");
			maison1wirewindow->GenError(46, Msg);
            tcp->disconnectFromHost();
			return;
		}
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, LogginTypeStr));
		header.append(logisdom::saveformat(CompressedData, "0"));
		header.append(logisdom::saveformat(DataSize, "0"));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
	}
	else if (UserName.isEmpty())
	{
		Msg = tr("login aborted");
		maison1wirewindow->GenError(46, Msg);
        tcp->disconnectFromHost();
		return;
	}
// SetPassWord
	else if (order.contains(NetRequestMsg[SetPassWord]))
	{
		PassWord = logisdom::getvalue(NetRequestMsg[SetPassWord], order);
		if (PassWord.isEmpty())
		{
			Msg = tr("login aborted");
			maison1wirewindow->GenError(46, Msg);
            tcp->disconnectFromHost();
			return;
		}
		int indexUser = -1;
		int count = Parent->ConnectionUsers.count();
		for (int n=0; n<count; n++)
					if (Parent->ConnectionUsers[n].Name == UserName) indexUser = n;
			if (indexUser != -1)
				if (Parent->ConnectionUsers[indexUser].PassWord == PassWord)
					Privilege = Parent->ConnectionUsers[indexUser].Rigths;
		Msg = UserName + tr(" is connected");
        //emit(clientbegin(this));
		maison1wirewindow->GenError(49, Msg);
		QString header;
		header.append(headerStart"\n");
		if (Privilege == Server::FullControl) header.append(logisdom::saveformat(RigthsTypeStr, RigthsAdminStr));
			else header.append(logisdom::saveformat(RigthsTypeStr, RigthsLimitedStr));
		header.append(logisdom::saveformat(DataTypeStr, LogginTypeStr));
		header.append(logisdom::saveformat(CompressedData, "0"));
		header.append(logisdom::saveformat(DataSize, "0"));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
	}
	else if (PassWord.isEmpty())
	{
		Msg = tr("login aborted");
		maison1wirewindow->GenError(46, Msg);
        tcp->disconnectFromHost();
		return;
	}
	else if (Privilege == Server::NoRigths)
	{
		int indexUser = -1;
		int count = Parent->ConnectionUsers.count();
		if (!UserName.isEmpty())
		{
			for (int n=0; n<count; n++)
						if (Parent->ConnectionUsers[n].Name == UserName) indexUser = n;
				if (indexUser != -1)
					if (Parent->ConnectionUsers[indexUser].PassWord == PassWord)
						Privilege = Parent->ConnectionUsers[indexUser].Rigths;
			}
		if (Privilege == Server::NoRigths) close();
		else
		{
			Msg = UserName + tr(" is connected");
            //emit(clientbegin(this));
			maison1wirewindow->GenError(49, Msg);
			QString header;
			header.append(headerStart"\n");
			header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
			header.append(logisdom::saveformat(CompressedData, "0"));
			header.append(logisdom::saveformat(DataSize, "0"));
			header.append(logisdom::saveformat(RequestStr, order));
			header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
		}
	}
// Config
	else if (order.contains(NetRequestMsg[GetConfigFile]))
	{
		QByteArray configdata;
		QString str;
	// Graphiques
		if (maison1wirewindow->graphconfigwin) maison1wirewindow->graphconfigwin->SaveConfigStr(str);
	// Program Events
		maison1wirewindow->getProgEventArea()->SaveConfigStr(str);
	// Net 1 Wire Devices
		maison1wirewindow->configwin->SaveConfigStr(str);
	// One Wire Devices
		//maison1wirewindow->configwin->GetDevicesStr(str);
	// Weekly Programs
		maison1wirewindow->AddProgwin->SaveConfigStr(str);
	// Main config
	 	maison1wirewindow->SaveConfigStr(str);
	// Icon Config
		maison1wirewindow->SaveIconConfigStr(str);
	// Meteo
		maison1wirewindow->MeteoArea->SaveConfigStr(str);
	// Switch
		maison1wirewindow->SwitchArea->SaveConfigStr(str);
	// Chauffage
		maison1wirewindow->ChauffageArea->SaveConfigStr(str);
	// Solaire
		maison1wirewindow->EnergieSolaire->SaveConfigStr(str);
	// Daliy Programs
		maison1wirewindow->AddDaily->SaveConfigStr(str);
	// Tableau
		maison1wirewindow->tableauConfig->SaveConfigStr(str);
        //configdata.append(qCompress(str.toUtf8()));
		configdata.append(qCompress(str.toUtf8()));
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(CompressedData, "1"));
		header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(configdata);
        tcp->waitForBytesWritten(10000);
	}
// Devices
	else if (order.contains(NetRequestMsg[GetDevicesConfig]))
	{
		QByteArray configdata;
		QString str;
		maison1wirewindow->configwin->GetDevicesStr(str);
        //configdata.append(qCompress(str.toUtf8()));
		configdata.append(qCompress(str.toUtf8()));
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(CompressedData, "1"));
		header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(configdata);
        tcp->waitForBytesWritten(10000);
	}
// File
	else if (order.contains(NetRequestMsg[GetFile]))
	{
		QByteArray configdata;
		QString folder = logisdom::getvalue(NetRequestMsg[setFolder], order);
		QString filename;
		if (folder.isEmpty()) filename = logisdom::getvalue(NetRequestMsg[GetFile], order);
			else filename = folder + QDir::separator() + logisdom::getvalue(NetRequestMsg[GetFile], order);
		QFile file(filename);
		if (file.exists())
		{
			if (!file.open(QIODevice::ReadOnly))
			{
				maison1wirewindow->logfile("remote connection cannot read file  " + file.fileName());
                if (tcp->isValid()) tcp->write("<" File_not_found ">");
                tcp->waitForBytesWritten(10000);
			}
			else
			{
				configdata.append(qCompress(file.readAll()));
				file.close();
			}
			QString header;
			header.append(headerStart"\n");
			header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
			header.append(logisdom::saveformat(CompressedData, "1"));
			header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
			header.append(logisdom::saveformat(RequestStr, order));
			header.append(headerEnd"\n");
            if (tcp->isValid()) tcp->write(header.toUtf8());
            tcp->waitForBytesWritten(10000);
            if (tcp->isValid()) tcp->write(configdata);
            tcp->waitForBytesWritten(10000);
		}
		else
		{
			QByteArray header;
			header.append(headerStart"\n");
			header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
			header.append(logisdom::saveformat(DataSize, "0"));
			header.append(logisdom::saveformat(RequestStr, order));
			header.append(File_not_found);
			header.append(headerEnd"\n");
            if (tcp->isValid()) tcp->write(header);
            tcp->waitForBytesWritten(10000);
		}
	}
// GetDatFile
	else if (order.contains(NetRequestMsg[GetDatFile]))
	{
		QDateTime now = QDateTime::currentDateTime();
		bool compress = false;
		QByteArray configdata;
		QString datname = logisdom::getvalue(NetRequestMsg[GetDatFile], order);
		if (!datname.isEmpty())
		{
			int posUnderScore = datname.indexOf("_");
			if (posUnderScore > 0)
			{
				QString datfolder = Parent->parent->getrepertoiredat();
				QString zipfolder = Parent->parent->getrepertoirezip();
				QString romid = datname.left(posUnderScore);
				QString year = datname.right(4);
				QString month = datname.right(7);
				month = month.left(2);
				bool okm, oky;
				int m = month.toInt(&okm);
				int y = year.toInt(&oky);
				int Month = now.date().month();
				int Year = now.date().year();
				QFile file;
				QString filename;
				// check if actual month
				filename = (romid + "_" + now.toString("MM-yyyy") + dat_ext);
				file.setFileName(datfolder + filename);
				if (okm && oky && (Month == m) && (Year == y) && file.exists())
				{
					compress = true;
				}
				else
				{
					filename = QString(romid + "_" + year + ".zip");
					file.setFileName(zipfolder + QDir::separator() + filename);
					if (file.exists())
					{
						compress = false;
					}
					else
					{
						// check for local zip file
						filename = datname + ".zip";
						file.setFileName(zipfolder + QDir::separator() + filename);
						if (file.exists())
						{
							compress = false;
						}
						else 
						{
							// check for zat file
							filename = datname + ".zat";
							file.setFileName(datfolder + QDir::separator() + filename);
							if (file.exists())
							{
								maison1wirewindow->GenMsg("Read : " + filename);
								compress = false;
							}
							else 
							{
								// check for dat file
								filename = datname + ".dat";
								file.setFileName(datfolder + QDir::separator() + filename);
								compress = true;
							}
						}
					}
				}
				if (!file.open(QIODevice::ReadOnly))
				{
					maison1wirewindow->GenMsg("Cannot find data file for " + datname);
					QString header;
					header.append(headerStart"\n");
					header.append(logisdom::saveformat(RequestStr, File_not_found));
					header.append(logisdom::saveformat(DataSize, "0"));
					header.append(headerEnd"\n");
                    if (tcp->isValid()) tcp->write(header.toUtf8());
                    tcp->waitForBytesWritten(10000);
                    if (tcp->isValid()) tcp->write(configdata);
                    tcp->waitForBytesWritten(10000);
				}
				else
				{
					maison1wirewindow->GenMsg("Read : " + filename);
					if (compress) configdata.append(qCompress(file.readAll()));
					else configdata.append(file.readAll());
					file.close();
					QString header;
					header.append(headerStart"\n");
					header.append(logisdom::saveformat(DataTypeStr, DataFileTypeStr));
					if (compress) header.append(logisdom::saveformat(CompressedData, "1"));
					else header.append(logisdom::saveformat(CompressedData, "0"));
					header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
					header.append(logisdom::saveformat(RequestStr, NetRequestMsg[GetFile]));
					header.append(logisdom::saveformat(NetRequestMsg[GetFile], filename));
					if (compress) header.append(logisdom::saveformat(NetRequestMsg[setFolder], datfolder));
					else header.append(logisdom::saveformat(NetRequestMsg[setFolder], zipfolder));
					header.append(headerEnd"\n");
                    if (tcp->isValid()) tcp->write(header.toUtf8());
                    tcp->waitForBytesWritten(10000);
                    if (tcp->isValid()) tcp->write(configdata);
                    tcp->waitForBytesWritten(10000);
				}
			}
		}
	}
// GetDeviceScratchpad
	else if (order.contains(NetRequestMsg[GetDeviceScratchpad]) or order.contains(NetRequestMsg[SaveDeviceScratchpad]))
	{
		QByteArray configdata;
		QString str;
		maison1wirewindow->configwin->GetDevicesScratchpad(str);
        //configdata.append(qCompress(str.toUtf8()));
		configdata.append(qCompress(str.toUtf8()));
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(CompressedData, "1"));
		header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(configdata);
        tcp->waitForBytesWritten(10000);
	}
// GetMainValue
	else if (order.contains(NetRequestMsg[GetMainValue]) or order.contains(NetRequestMsg[SaveMainValue]))
	{
		QByteArray configdata;
		QString str;
		maison1wirewindow->configwin->GetDevicesMainValue(str);
        //configdata.append(qCompress(str.toUtf8()));
		configdata.append(qCompress(str.toUtf8()));
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(CompressedData, "1"));
		header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(configdata);
        tcp->waitForBytesWritten(10000);
	}
// Command
	else if (order.contains(NetRequestMsg[SendCommand]))
	{
		if (Privilege == Server::FullControl) 
		{
				maison1wirewindow->setBinderCommand(order);
				Parent->transfertToOthers(order, this);
		}			
		QByteArray header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(DataSize, "0"));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header);
        tcp->waitForBytesWritten(10000);
	}
	else if ((!getData(extract).isEmpty()) and (!order.isEmpty()) and (!getRomID(extract).isEmpty()))
	{
		QByteArray configdata;
		QString RomID = getRomID(extract);
		net1wire *master = nullptr;
		onewiredevice *device = maison1wirewindow->configwin->DeviceExist(RomID);
		if (device) master = device->getMaster();
			else master = maison1wirewindow->configwin->MasterExist(RomID);
		if (master) master->addremotefifo(extract);
		QString header;
		header.append(headerStart"\n");
		header.append(logisdom::saveformat(DataTypeStr, DataDeviceTypeStr));
		header.append(logisdom::saveformat(CompressedData, "0"));
		header.append(logisdom::saveformat(DataSize, QString("%1").arg(configdata.size())));
		header.append(logisdom::saveformat(RequestStr, order));
		header.append(headerEnd"\n");
        if (tcp->isValid()) tcp->write(header.toUtf8());
        tcp->waitForBytesWritten(10000);
        if (tcp->isValid()) tcp->write(configdata);
        tcp->waitForBytesWritten(10000);
	}
    while (extractBuffer("") != ""){}
    if (tcp->bytesAvailable() > 0) goto more;
}





void Connection::writeHeader(QByteArray, bool, long, QByteArray, QByteArray)
{
}




void Connection::writeToClient(QByteArray)
{
}



QString Connection::extractBuffer(const QString &data)
{
	QString extract = "";
	int chdeb, chfin, L;
	buffer += data;
	chdeb = buffer.indexOf("<");
	chfin = buffer.indexOf(">");
	L = buffer.length();
	if ((chdeb == -1) and (chfin == -1))
	{
//		GenMsg("No begin No end found : ");
//		GenMsg("Buffer : " + buffer);
		return "";
	}
	if (chdeb > chfin)
	{
		buffer = buffer.right(L - chdeb);
//		GenMsg("No begin found : ");
//		GenMsg("Buffer : " + buffer);
		return "";
	}
	if ((chdeb != -1) and (chfin == -1))
	{
//		GenMsg("begin found but no end found, Buffer : " + buffer);
		extract = buffer.mid(chdeb + 1, chfin - chdeb - 1);
		buffer = buffer.right(L - chfin - 1);
//		maison1wirewindow->GenMsg("extract : " + extract);
//		GenMsg("Buffer remains : " + buffer);
		return "";
	}
	if ((chdeb != -1) and (chfin != -1))
	{
//		GenMsg("begin and end found, Buffer : " + buffer);
		extract = buffer.mid(chdeb + 1, chfin - chdeb - 1);
		buffer = buffer.right(L - chfin - 1);
//		maison1wirewindow->GenMsg("extract : " + extract);
//		GenMsg("Buffer remains : " + buffer);
		return extract;
	}
	if ((extract.isEmpty()) and (buffer.size() >1000)) buffer.clear();
	return extract;
}






QString Connection::getRomID(QString str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("{");
	end = str.indexOf("}");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}






QString Connection::getOrder(QString str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("*");
	end = str.indexOf("#");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}





QString Connection::getData(QString str)
{
	int begin, end;
	QString Str;
	begin = str.indexOf("[");
	end = str.indexOf("]");
	if ((begin != -1) and (end > begin))
		Str = str.mid(begin + 1, end - begin - 1);
	return Str;
}


