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





#include "globalvar.h"
#include "weathercom.h"
#include "meteo.h"
#include "graphconfig.h"
#include "configwindow.h"
#include "alarmwarn.h"
#include "programevent.h"
#include "addProgram.h"
#include "chauffageunit.h"
#include "vmc.h"
#include "tableau.h"
#include "tableauconfig.h"
#include "errlog.h"
#include "addDaily.h"
#include "energiesolaire.h"
#include "tcpdata.h"
#include "quazip.h"
#include "quazipfile.h"
#include "remote.h"


remote::remote(logisdom *Parent) : net1wire(Parent)
{
	parent = Parent;
	TcpThread = new remotethread(Parent);
	ui.localdevicecombolist->hide();
	ui.gridLayout->removeWidget(ui.localdevicecombolist);
	ui.pushButtonShow->hide();
	ui.gridLayout->removeWidget(ui.pushButtonShow);
        ui.frameguinet->hide();
        ui.gridLayout->removeWidget(ui.frameguinet);
	type = RemoteType;
	setport(default_port_EZL50);
	Admin = false;
	saveRequest = false;
}



void remote::switchOnOff(bool state)
{
	if (state)
	{
		TcpThread->endLessLoop = true;
		OnOff = true;
		TcpThread->start(QThread::LowestPriority);
	}
	else
	{
		TcpThread->endLessLoop = false;
		OnOff = false;
	}
}




void remote::addtofifo(int order, const QString &Data)
{
	TcpThread->addToFIFOSpecial(order, Data);
}



void remote::add2fifo(int order, bool)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TcpThread->addToFIFOSpecial(order);
}




void remote::add2fifo(const QString &order, bool)
{
	if (!OnOff) return;
	QMutexLocker locker(&mutex);
	TcpThread->addToFIFOSpecial(order);
}




void remote::init(QString userName, QString password)
{
	connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
	ui.EditType->setText("Remote");
	name = userName;
	TcpThread->UserName = userName;
	TcpThread->PassWord = password;
	TcpThread->endLessLoop = true;
	TcpThread->moduleipaddress = moduleipaddress;
	TcpThread->port = port;
	QDir dat = QDir(QString(parent->getrepertoiredat()));
	if (dat.exists())
	{
		QFileInfo properties;
		QFile file;
		QStringList fileslist = dat.entryList(QDir::Files);
		for (int n = 0; n < fileslist.count(); n ++)
		{
			file.setFileName(fileslist[n]);
			properties.setFile(file);
			QString ext = dat_ext;
			if (properties.suffix() == ext.left(3)) addGetFiletoFifo(file.fileName(), parent->getrepertoiredat());
		}
	}
    connect(TcpThread, SIGNAL(configReady(QString)), this, SLOT(configReady(QString)), Qt::BlockingQueuedConnection);
    connect(TcpThread, SIGNAL(deviceConfigReady(QString)), this, SLOT(deviceConfigReady(QString)), Qt::BlockingQueuedConnection);
    connect(TcpThread, SIGNAL(getMainValueReady(QString)), this, SLOT(getMainValueReady(QString)), Qt::BlockingQueuedConnection);
	connect(TcpThread, SIGNAL(reloadGrahp()), parent->graphconfigwin, SLOT(ReloadGraphs()), Qt::BlockingQueuedConnection);
    connect(TcpThread, SIGNAL(traceUpdate(QString)), this, SLOT(traceUpdate(QString)), Qt::BlockingQueuedConnection);
    TcpThread->start(QThread::LowestPriority);
}





void remote::configReady(const QString &dataString)
{
    QFile Cfgfile(parent->configfilename);
	if (!Cfgfile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
        QString configdata;
        parent->get_ConfigData(configdata);
        parent->readconfigfile(dataString);
        parent->configwin->readPathConfig(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
        parent->readIconTabconfigfile(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
        parent->MeteoArea->readconfigfile(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
        parent->EnergieSolaire->readconfigfile(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
        parent->graphconfigwin->readconfigfile(dataString );
		QCoreApplication::processEvents(QEventLoop::AllEvents);
        parent->setTabs(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}
	else
	{
		QString configdata;
        parent->get_ConfigData(configdata);
        parent->readconfigfile(dataString);
        parent->configwin->readPathConfig(dataString);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
		parent->readIconTabconfigfile(configdata);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
		parent->MeteoArea->readconfigfile(configdata);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
		parent->EnergieSolaire->readconfigfile(configdata);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
		parent->graphconfigwin->readconfigfile(configdata );
		QCoreApplication::processEvents(QEventLoop::AllEvents);
		parent->setTabs(configdata);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}
    parent->AddDaily->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->getProgEventArea()->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->AddDaily->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->AddProgwin->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->ChauffageArea->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->tableauConfig->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
    parent->SwitchArea->readconfigfile(dataString);
	QCoreApplication::processEvents(QEventLoop::AllEvents);
	GenMsg("GetConfigFile DONE");
}





void remote::deviceConfigReady(const QString &dataString)
{
    parent->configwin->UpdateRemoteDevice(dataString);
	GenMsg("GetDevicesConfig DONE");
}



void remote::traceUpdate(const QString &dataString)
{
	QMutexLocker locker(&mutexFifoList);
    QByteArray txt;
    // Qt 6
    txt.append(dataString.toLatin1());
    parent->setTitle(dataString);
	QListWidgetItem *item = new QListWidgetItem(txt);
	ui.fifolist->addItem(item);
	if (ui.fifolist->count() > 100)
	{
		QListWidgetItem *item = ui.fifolist->takeItem(0);
		if (item) delete item;
	}
}


void remote::getMainValueReady(const QString &dataString)
{
    QByteArray txt;
    // Qt 6
    txt.append(dataString.toLatin1());
    parent->configwin->SetDevicesMainValue(txt, saveRequest);
	saveRequest = false;
	GenMsg("getMainValueReady DONE");
}





void remote::saveMainValue()
{
	saveRequest = true;
}




void remote::logEnabledChanged(int state)
{
	if (state) TcpThread->logEnabled = true;
	else TcpThread->logEnabled = false;
}




/*	QString order;
	QByteArray data;
	QMutexLocker locker(&mutexTimer);
	//busy = false;
	data = tcp.readAll();
	Data.append(data);
//	emit(TimerStart());
	if (!Data.isComplete()) return;
more:
	QString next = fifoListNext();
	if (fifoListCount() > 0) order = getOrder(next); else order = "";
	QString Header;
	Header.append(Data.Header);
	QByteArray request = logisdom::getvalue(RequestStr, Data.Header);
	GenMsg("Header Size OK " + Header);
	Data.getData(data);
	QString dataString;
	dataString.append(data);
	if (data.isEmpty())
	{
		GenMsg("Compressed data corrupted : " + Header);
		emit requestdone();
	}
	GenMsg("Returned request = " + QString(request));
//	GenMsg("Data = " + QString(data));
//	GetDeviceScratchpad
	if (request.indexOf(NetRequestMsg[GetDeviceScratchpad]) != -1)
	{
		parent->configwin->SetDevicesScratchpad(data, false);
		if (order.indexOf(NetRequestMsg[GetDeviceScratchpad]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("GetDeviceScratchpad DONE");
		goto processed;
	}
// SaveDeviceScratchpad
	else if (request.indexOf(NetRequestMsg[GetDeviceScratchpad]) != -1)
	{
		parent->configwin->SetDevicesScratchpad(data, true);
		if (order.indexOf(NetRequestMsg[SaveDeviceScratchpad]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("SaveDeviceScratchpad DONE");
		goto processed;
	}
// SendDeviceScratchpad
	else if (request.indexOf(NetRequestMsg[SendDeviceScratchpad]) != -1)
	{
		parent->configwin->SetDevicesScratchpad(data, false);
		if (order.indexOf(NetRequestMsg[SaveDeviceScratchpad]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("SaveDeviceScratchpad DONE");
		goto processed;
	}
// GetMainValue
	else if (request.indexOf(NetRequestMsg[GetMainValue]) != -1)
	{
		parent->configwin->SetDevicesMainValue(data, false);
		if (order.indexOf(NetRequestMsg[GetMainValue]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("GetMainValue DONE");
		goto processed;
	}
// SaveMainValue
	else if (request.indexOf(NetRequestMsg[SaveMainValue]) != -1)
	{
		parent->configwin->SetDevicesMainValue(data, true);
		if (order.indexOf(NetRequestMsg[SaveMainValue]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("SaveMainValue DONE");
		goto processed;
	}
// SendMainValue
	else if (request.indexOf(NetRequestMsg[SendMainValue]) != -1)
	{
		parent->configwin->SetDevicesMainValue(data, false);
		if (order.indexOf(NetRequestMsg[SaveMainValue]) != -1)
		{
			remoteComOK
			remoteFifoRemoveLast
		}
		GenMsg("SendMainValue DONE");
		goto processed;
	}
// TransferCommand
	else if (request.indexOf(NetRequestMsg[TransferCommand]) != -1)
	{
		QString d;
		d.append(data);
		GenMsg("Transfert = " + d);
		parent->setBinderCommand(d);
		GenMsg("TransferCommand DONE");
	}
// UserName
	else if ((order.contains(NetRequestMsg[SetUserName])) && (request.indexOf(NetRequestMsg[SetUserName]) != -1))
	{
		remoteComOK
		remoteFifoRemoveLast
		GenMsg("UserName DONE");
		goto processed;
	}
// SetPassWord
	else if ((order.contains(NetRequestMsg[SetPassWord])) && (request.indexOf(NetRequestMsg[SetPassWord]) != -1))
	{
		QString rigths = logisdom::getvalue(RigthsTypeStr, Header);
		if (rigths.indexOf(RigthsAdminStr) != -1) Admin = true;
		remoteComOK
		remoteFifoRemoveLast
		GenMsg("SetPassWord DONE");
		goto processed;
	}
// GetConfigFile
	else if ((order.contains(NetRequestMsg[GetConfigFile])) && (request.indexOf(NetRequestMsg[GetConfigFile]) != -1))
	{
		remoteComOK
		QFile Cfgfile(configfilename);
		if (!Cfgfile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			OpenConfigFile
			parent->readconfigfile(dataString);
			parent->readIconTabconfigfile(dataString);
			parent->MeteoArea->readconfigfile(dataString);
			parent->EnergieSolaire->readconfigfile(dataString);
			parent->graphconfigwin->readconfigfile(dataString );
			parent->setTabs(dataString);
		}
		else
		{
			OpenConfigFile
			parent->readconfigfile(dataString);
			parent->readIconTabconfigfile(configdata);
			parent->MeteoArea->readconfigfile(configdata);
			parent->EnergieSolaire->readconfigfile(configdata);
			parent->graphconfigwin->readconfigfile(configdata );
			parent->setTabs(configdata);
		}
		parent->AddDaily->readconfigfile(dataString);
		parent->getProgEventArea()->readconfigfile(dataString);
		parent->AddDaily->readconfigfile(dataString);
		parent->AddProgwin->readconfigfile(dataString);
		parent->ChauffageArea->readconfigfile(dataString);
		parent->tableauConfig->readconfigfile(dataString);
		parent->SwitchArea->readconfigfile(dataString);
		parent->saveconfig();
		remoteFifoRemoveLast
		GenMsg("GetConfigFile DONE");
	}
// GetDevicesConfig
	else if ((order.contains(NetRequestMsg[GetDevicesConfig])) && (request.indexOf(NetRequestMsg[GetDevicesConfig]) != -1))
	{
		remoteComOK
		parent->configwin->UpdateRemoteDevice(dataString);
		remoteFifoRemoveLast
		GenMsg("GetDevicesConfig DONE");
	}
// SendCommand
	else if ((order.contains(NetRequestMsg[SendCommand])) && (request.indexOf(NetRequestMsg[SendCommand]) != -1))
	{
		remoteComOK
		remoteFifoRemoveLast
		GenMsg("SendCommand DONE");
	}
// GetFile
	else if (request.indexOf(NetRequestMsg[GetFile]) != -1)
	{
		remoteComOK
		bool isIcon = false;
		QString folder = logisdom::getvalue(NetRequestMsg[setFolder], Header);
		if (!folder.isEmpty())
		{
			if (!QDir().exists(folder))
			{
				parent->logfile("Try to create Dir " + folder);
				if (!QDir().mkdir(folder)) 
				{
					parent->logfile("Cannot create Dir " + folder);
				}
				parent->logfile("Dir " + folder + " created");
				parent->GenMsg("Dir " + folder + " created");
			}
			if (folder == repertoireicon) isIcon = true;
		}
		QString filename;
		if (folder.isEmpty()) filename = logisdom::getvalue(NetRequestMsg[GetFile], Header);
			else filename = folder + QDir::separator() + logisdom::getvalue(NetRequestMsg[GetFile], Header);
		QFile file(filename);
		if(file.open(QIODevice::WriteOnly))
		{
			file.write(data);
			file.close();
		}
		else
		{
			GenMsg("cannot open file " + filename);
		}
		remoteFifoRemoveLast
		// update icons
		//if (isIcon) parent->updateIcons();
		// remove request included in new zip file
		int index = 0;
		while (index < fifoListCount())
		{
			bool found = false;
			QString item = ui.fifolist->item(index)->text();
			QString fifoorder = getOrder(item);
			QString fifofilenamme = logisdom::getvalue(NetRequestMsg[GetDatFile], fifoorder);
			//GenMsg("Try Remove extra : " + fifoorder + "   " + fifofilenamme);
			if (fifoorder.contains(NetRequestMsg[GetDatFile]))
			{
				// check if file is here in a global zip file
				int posUnderScore = fifofilenamme.indexOf("_");
				if (posUnderScore > 0)
				{
					QString romid = fifofilenamme.left(posUnderScore);
					QString year = fifofilenamme.right(4);
					QString zipfilename = QString(logisdom::getrepertoiredat()) + QDir::separator() + romid + "_" + year + ".zip";
					QFile file(zipfilename);
					if (file.exists())
					{
						QuaZip zip(zipfilename);
						if(zip.open(QuaZip::mdUnzip))
						{		
							zip.setFileNameCodec("IBM866");
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
											//GenMsg("Remove extra : " + name);
										}
									}
								}
							}
							zip.close();
							if(zip.getZipError() != UNZ_OK) GenMsg("Zip file close error : " + zipfilename);
						}
					}
				}
			}
			if (found)
			{
				QListWidgetItem *item = ui.fifolist->item(index);
				ui.fifolist->takeItem(index);
				delete item;
				Buffer = "";
			}
			else index ++;
		}
		bool found = false;
		for (int n=0; n<fifoListCount(); n++)
			if (fifoListContains(NetRequestMsg[GetFile]) or fifoListContains(NetRequestMsg[GetDatFile])) found = true;
		if (!found) parent->graphconfigwin->ReloadGraphs();
		GenMsg("Files " + filename + " Done");
	}
// File_not_found
	else if (request.indexOf(File_not_found) != -1)
	{
		remoteComOK
		remoteFifoRemoveLast
		GenMsg("File not found");
	}
	else if ((order.contains(request)))
	{
		remoteFifoRemoveLast
		GenMsg("Command not supported");
	}
processed:
	emit requestdone();
	if (Data.isEmpty()) return;
	goto more;*/




bool remote::isAdmin()
{
	return Admin;
}





void remote::addGetFiletoFifo(QString name, QString folder)
{
	if (folder.isEmpty()) //add2fifo(NetRequestMsg[GetFile] + " = (" + name + ")");
		addtofifo(GetFile, NetRequestMsg[GetFile] + " = (" + name + ")");
		else //add2fifo(NetRequestMsg[GetFile] + " = (" + name + ")" + NetRequestMsg[setFolder] + " = (" + folder + ")");
		addtofifo(GetFile, NetRequestMsg[GetFile] + " = (" + name + ")" + NetRequestMsg[setFolder] + " = (" + folder + ")");
}




void remote::addGetDatFiletoFifo(QString name)
{
//	add2fifo(NetRequestMsg[GetDatFile] + " = (" + name + ")");
    addtofifo(GetDatFile, NetRequestMsg[GetDatFile] + " = (" + name + ")");
}



void remote::addCommandtoFifo(QString command)
{
	addtofifo(SendCommand, NetRequestMsg[SendCommand] + " = (" + command + ")");
}





QString remote::getUserName()
{
	return TcpThread->UserName;
}




QString remote::getPassWord()
{
	return TcpThread->PassWord;
}


/*
void remote::GraphAnalysis(QString &data)
{
	QByteArray bytedata;
	bytedata.clear();
	bytedata.append(data);
	QString next = fifoListNext();
	bytedata.truncate(data.indexOf("Done : " + next));
	bool found = false;
	for (int n=0; n<fifoListCount(); n++)
		if (fifoListContains(NetRequestMsg[GetFile])) found = true;
	if (!found) parent->graphconfigwin->updateGraphs();
	if (!QDir().exists(logisdom::getrepertoiredat()))
	{
		parent->logfile("Try to create Dir " + logisdom::getrepertoiredat());
		if (!QDir().mkdir(logisdom::getrepertoiredat()))
		{
			parent->logfile("Cannot create Dir " + logisdom::getrepertoiredat());
			return;
		}
		parent->logfile("Dir " + logisdom::getrepertoiredat() + " created");
	}
	QString filename = QString(logisdom::getrepertoiredat()) + QDir::separator() + logisdom::getvalue(NetRequestMsg[GetFile], data);
	QFile file(filename);
	if(file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream out(&file);
		out << bytedata;
		file.close();
	}
	else
	{
		parent->logfile("cannot open file " + filename);
	}
}





void remote::ConfigAnalysis(QString &data)
{
	QFile Cfgfile(configfilename);
	if (!Cfgfile.open(QIODevice::ReadOnly | QIODevice::Text))	
	{
		OpenConfigFile
		parent->readconfigfile(data);
		parent->readIconTabconfigfile(data);
		parent->MeteoArea->readconfigfile(data);
		parent->graphconfigwin->readconfigfile(data);
	}
	else
	{
		OpenConfigFile
		parent->readconfigfile(data);
		parent->readIconTabconfigfile(configdata);
		parent->MeteoArea->readconfigfile(configdata);
		parent->graphconfigwin->readconfigfile(configdata);
	}
	parent->AddDaily->readconfigfile(data);
	parent->AddProgwin->readconfigfile(data);
	parent->ChauffageArea->readconfigfile(data);
	parent->saveconfig();
}
*/
