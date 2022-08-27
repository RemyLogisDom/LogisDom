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




#include <QtWidgets/QMessageBox>

#include <QtCore>
#include "alarmwarn.h"
#include "configwindow.h"
#include "calcthread.h"
#include "logisdom.h"
#include "onewire.h"
#include "server.h"
#include "net1wire.h"
#include "iconf.h"
#include "remote.h"
#include "connection.h"
#include "interval.h"
#include "htmlbinder.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "dataloader.h"
#include "quazip.h"
#include "quazipfile.h"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "curve.h"
#include "graph.h"
#include "graphconfig.h"

#include "globalvar.h"


onewiredevice::onewiredevice(net1wire *Master, logisdom *Parent, QString RomID)
{
	qRegisterMetaType<onewiredevice*>();
	QDateTime Now = QDateTime::currentDateTime();
    romid = RomID;
	lastFreeMem = Now.date();
	lastMainValue = logisdom::NA;
	MainValue = logisdom::NA;
	lastsavevalue = logisdom::NA;
    valid = dataNotValid;
	master = Master;
    //if (master == nullptr) { ui.setupUi(this);
    //    ui.labelRomID->setText(RomID);
    //}
    //writeBox.hide();
    plugin_interface = nullptr;
	parent = Parent;
    readCounter = 0;
    UsetextValuesChanged = false;
	dataLoader = new dataloader(Parent);
	commonReg = nullptr;
	lastSaveIndex = -1;
	VD_Rank = -1;
	RetryWarning = 0;
	ReadRetry = 0;
	firstsave = true;
	logisdomReading = true;
	htmlBind = new htmlBinder(parent, RomID, parent->configwin->htmlBindDeviceMenu->treeItem);
	htmlBind->setName(RomID);
	htmlBind->setParameter("RomID", RomID);
	htmlBindControler = nullptr;
	connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));
    connect(dataLoader, SIGNAL(finished()), this, SLOT(finishedLoading()), Qt::QueuedConnection);
	connect(dataLoader, SIGNAL(beginChanged()), this, SLOT(dataLoaderBeginChanged()), Qt::QueuedConnection);
    connect(this, SIGNAL(LoadRequest()), this, SLOT(setDataLoading()), Qt::QueuedConnection);
    connect(this, SIGNAL(finishedDataLoading(onewiredevice*)), parent, SLOT(finishedDataLoading(onewiredevice*)), Qt::QueuedConnection);
    connect(this, SIGNAL(trafficsig(int)), this, SLOT(setTraffic(int)), Qt::QueuedConnection);
    Unit.setText("");
	setname("");
	ClearWarning();
	setup.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	setupLayout.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setup.setLayout(&setupLayout);
	layoutIndex = 0;
	RenameButton.setText(tr("Name"));
	setupLayout.addWidget(&RenameButton, layoutIndex, 0, 1, 1);
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(changename()));
    setupLayout.addWidget(&traffic, layoutIndex++, 1, 1, 1);
	RomIDButton.setStatusTip(tr("Right click to copy RomID or name"));
	RomIDButton.setContextMenuPolicy(Qt::CustomContextMenu);
	RomIDButton.setText(RomID);
	connect(&RomIDButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(RomID_rigthClick(QPoint)));
	setupLayout.addWidget(&RomIDButton, layoutIndex, 0, 1, 1);
	ValueButton.setText(tr("Value"));
	setupLayout.addWidget(&ValueButton, layoutIndex++, 1, 1, 1);
	UnitText.setText(tr("Unit : "));
	UnitText.setAlignment(Qt::AlignRight);
	setupLayout.addWidget(&UnitText, layoutIndex, 0, 1, 1 );
	setupLayout.addWidget(&Unit, layoutIndex++, 1, 1, 1);
    connect(&Unit, SIGNAL(textChanged(QString)), this, SLOT(unitChanged(QString)));
	ShowButton.setText(tr("Afficher"));
	setupLayout.addWidget(&ShowButton, layoutIndex, 0, 1, 1);
	connect(&ShowButton, SIGNAL(clicked()), this, SLOT(show()));
	Decimal.setRange(0, 10);
	Decimal.setPrefix(tr("Decimal : "));
	setupLayout.addWidget(&Decimal, layoutIndex++, 1, 1, 1);
	connect(&Decimal, SIGNAL(valueChanged(int)), this, SLOT(DecimalChanged(int)));
	saveInterval.setName(tr("Save"));
	setupLayout.addWidget(&saveInterval, layoutIndex++, 0, 1, 1);
	GraphButton.setText(tr("Graphic"));
	setupLayout.addWidget(&GraphButton, layoutIndex, 0, 1, 1);
	connect(&GraphButton, SIGNAL(clicked()), this, SLOT(openGraph()));
	setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
	ScratchPad_Show.setReadOnly(true);
    ScratchPad_Show.setToolTip("Datagram");
	setupLayout.addWidget(&ScratchPad_Show, layoutIndex++, 1, 1, 1);
    UsetextValues.setText(tr("Text Values"));
    setupLayout.addWidget(&UsetextValues, layoutIndex, 0, 1, 1);
	TextValues.setEditable(true);
	TextValues.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&TextValues, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(UserText_rigthClick(QPoint)));
    connect(&TextValues, SIGNAL(editTextChanged(QString)), this, SLOT(UserText_Changed(QString)));
    setupLayout.addWidget(&TextValues, layoutIndex++, 1, 1, 1);

    SendButton.setText(tr("Send"));
    command.setToolTip("Command to send to device");
    setupLayout.addWidget(&command, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&SendButton, layoutIndex++, 1, 1, 1);

    setupLayout.addWidget(&lastValue, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&timeOut, layoutIndex++, 1, 1, 1);
    timeOut.setPrefix("Delai avant alerte : ");
    timeOut.setSuffix(" mn");
    timeOut.setValue(10);
    timeOut.setRange(0, 9999);
    timeOut.setSpecialValueText(tr("Alert delay disabled"));
    timeOut.hide();
    connect(&SendButton, SIGNAL(clicked()), this, SLOT(sendCommand()));

    // log checkBox
	WarnEnabled.setText(tr("Warning Enabled"));
    setupLayout.addWidget(&WarnEnabled, layoutIndex, 0, 1, 1);
	logEnabled.setText(tr("Log"));
	logEnabled.setLayoutDirection(Qt::RightToLeft);
    setupLayout.addWidget(&logEnabled, layoutIndex++, 1, 1, 1);
	connect(&logEnabled, SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));
	skip85.setText(tr("Check 85°C"));
	connect(&skip85, SIGNAL(stateChanged(int)), this, SLOT(skip85Changed(int)));
    if (parent->isRemoteMode())
	{
		int year = Now.date().year();
		int month = Now.date().month();
		QString filename = logisdom::filenameformat(RomID, month, year);
	//	if (fileInfo.lastModified().secsTo(now) > 300)
		QFile file(filename);
		file.remove();
                parent->RemoteConnection->addGetDatFiletoFifo(filename);
	}
	RenameButton.setToolTip(tr("No data Loaded"));
	dataLoader->romID = RomID;
	dataLoader->RemoteConnection = parent->RemoteConnection;
}



void onewiredevice::setPluginInterface(LogisDomInterface *i)
{
    plugin_interface = i;
    stdui.setupUi(this);
    stdui.labelRomID->setText(romid);
    timeOut.show();
    setupLayout.addWidget(&AXplusB, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&checkLastMessage, SIGNAL(timeout()), this, SLOT(checkElaspedTime()));
    checkLastMessage.start(60000);
}


void onewiredevice::checkElaspedTime()
{
    if (timeOut.value() == 0) return;
    if (lastMessage.hasExpired(timeOut.value() * 60000) || (!lastMessage.isValid()))
    {
        RetryWarning = 3;
        setTraffic(dataNotValid);
        setMainValue(logisdom::NA, false);
    }
}


onewiredevice::~onewiredevice()
{
}



void onewiredevice::openGraph()
{
	parent->graphconfigwin->createGraph(this);
}



void onewiredevice::sendCommand()       // send manual command
{
    if (!plugin_interface) return;
    bool ok;
    double Value = command.text().toDouble(&ok);
    if ((!ok) || !AXplusB.isEnabled()) { plugin_interface->setStatus(romid + "=" + command.text()); return; }
    double r = AXplusB.fromResult(Value);
    plugin_interface->setStatus(romid + "=" + QString("%1").arg(r));
}




void onewiredevice::remoteCommand(const QString &command)
{
	remoteCommandExtra(command);
}



void onewiredevice::remoteCommandExtra(const QString &)
{
}




void onewiredevice::logMsg(QString msg)
{
    if (master) master->GenMsg(romid + " : " + msg);
}




void onewiredevice::SetOrder(const QString &)
{
}



void onewiredevice::stopAll()
{
}





void onewiredevice::contextMenuEvent(QContextMenuEvent *event)
{
    if (!plugin_interface) return;
    QString listStr = plugin_interface->getDeviceCommands(romid);
    QStringList list = listStr.split("|");
    QList <QAction*> actionList;
    QMenu contextualmenu;
    foreach (QString str, list) {
        QStringList split = str.split("=");
        QAction *a = new QAction(split.last());
        actionList.append(a);
        contextualmenu.addAction(a);
    }
    QAction *selection;
    selection = contextualmenu.exec(event->globalPos());
    int index = actionList.indexOf(selection);
    if (index != -1) {
        QStringList split = list.at(index).split("=");
        plugin_interface->setStatus(romid + "=" + split.first());
    }
    foreach (QAction *a, actionList) { delete a; }
}


bool onewiredevice::isTempFamily()
{
	return false;
}


bool onewiredevice::isVanneFamily()
{
	return false;
}


bool onewiredevice::isX10Family()
{
	return false;
}



bool onewiredevice::isSwitchFamily()
{
    if (plugin_interface)
    return plugin_interface->isDimmable(romid);
    return false;
}



bool onewiredevice::isDimmmable()
{
    bool dim = false;
    if (plugin_interface) dim = plugin_interface->isDimmable(romid);
    return dim;
}


bool onewiredevice::isManual()
{
    bool m = false;
    if (plugin_interface) m = plugin_interface->isManual(romid);
    return m;
}


bool onewiredevice::isVirtualFamily()
{
	return false;
}



void onewiredevice::writescratchpad()
{
}




QString onewiredevice::NbitsStr(int nbit)
{
	switch (nbit)
	{
		case R9bits : return tr("9 bits");
		case R10bits : return tr("10 bits");
		case R11bits : return tr("11 bits");
		case R12bits : return tr("12 bits");
		default : return "";
	}
}





void onewiredevice::setname(const QString &NameStr)
{
	QString Name = NameStr;
	if (Name.isEmpty())
	{
        if ((Name == "") && (family.isEmpty())) name = tr("Unknown_Family_1" );
		else if (Name == "") name = family + "_1" ; 
		else name = Name;
		if (parent->configwin->devicenameexist(name))
		{
			int index = 1;
			int lastunderscore = name.lastIndexOf("_");
			if (lastunderscore == -1) name += QString("_1");
			while (parent->configwin->devicenameexist(Name))
			{
				lastunderscore = Name.lastIndexOf("_");
				Name = Name.mid(0, lastunderscore) + QString("_%1").arg(index);
			 	index ++;
	 		}
 		}
	}
	// check duplicate name & rename if already exist
	name = assignname(Name);
	setWindowTitle(name);
	RenameButton.setText(name);
	htmlBind->setName(name);
	emit(DeviceConfigChanged(this));
}



void onewiredevice::removeme()
{
    //if (messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to me ?")) setTobeDeleted();
    //messageBox::warningHide(this, "Application restart", "You must save and restart the application\nto remove definitely the controler", parent, QMessageBox::Ok);
}


void onewiredevice::changename()
{
	bool ok;
retry:
    QString Name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, name, &ok, parent);
	if ((ok) and !Name.isEmpty())
	{
		if (parent->configwin->devicenameexist(Name) and (Name != name))
		{
            messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto retry;
		}
		name = Name;
		setWindowTitle(Name);
		RenameButton.setText(Name);
		emit(DeviceConfigChanged(this));
	}	
}



void onewiredevice::mousePressEvent(QMouseEvent*)
{
	emit(setupClick(&setup));
}


QString onewiredevice::getname()
{
	return name;
}


void onewiredevice::send_Value(double)
{
}


void onewiredevice::setHouseCode(int, int)
{
}


void onewiredevice::setHouseCode(QString, QString)
{
}


void onewiredevice::set_On()
{
	if (parent) On(!parent->isRemoteMode());
	else On(true);
}



void onewiredevice::set_Off()
{
	if (parent) Off(!parent->isRemoteMode());
	else Off(true);
}


void onewiredevice::On(bool)
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + "=on");
    }
}

void onewiredevice::Off(bool)
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + "=off");
    }
}

void onewiredevice::Dim(bool)
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + "=dim");
    }
}

void onewiredevice::Bright(bool)
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + "=bright");
    }
}

void onewiredevice::Bright(int v, bool)
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + QString("=bright=%1").arg(v));
    }
}

void onewiredevice::Sleep()
{
    if (plugin_interface) {
        plugin_interface->setStatus(romid + "=sleep");
    }
}

void onewiredevice::SleepStep()
{
}


void onewiredevice::changealarmebasse()
{
}


void onewiredevice::changealarmehaute()
{
}



void onewiredevice::changresoultion()
{
}


void onewiredevice::saveDeviceConfig()
{
}


QString onewiredevice::getSecondaryValue()
{
    //if (plugin_interface) {
    if (lastMessage.isValid())
    {
        qint64 t = lastMessage.elapsed() / 1000;
        if (t < 60) secondaryValue = "< 1 mn";
        else if (t < 3600) secondaryValue = QString("%1 mn").arg(t/60 + 1);
        else secondaryValue = QString("%1 h").arg(t/3600);
    }
    else secondaryValue = cstr::toStr(logisdom::NA);
    return secondaryValue;
}


QString onewiredevice::getromid()
{
	return romid;
}


QString onewiredevice::getfamily()
{
	return family;
}



QString onewiredevice::getunit()
{
	return Unit.text();
}



int onewiredevice::getHouseCode()
{
	if ((HouseCode > 0) and (HouseCode < 16)) return HouseCode;
	else return 0;
}



int onewiredevice::getModuleCode()
{
	if ((ModuleCode > 0) and (ModuleCode < 16)) return ModuleCode;
	else return 0;
}




QString onewiredevice::getscratchpad()
{
	return scratchpad;
}


void onewiredevice::setListTreeItem(QTreeWidgetItem *item)
{
    listTreeItem = item;
}

void onewiredevice::clearListTreeItem()
{
    listTreeItem = nullptr;
}


double onewiredevice::getMainValue()
{
    return MainValue;
}




bool onewiredevice::getValues(qint64 begin, qint64 end, bool &loading, dataloader::s_Data &data, formula *)
{
	QMutexLocker locker(&mutexGet);
	dataLoader->logGetValue = logEnabled.isChecked();
    qint64 check = 0;
	if (begin < end) check = begin;
	if (begin >= end) check = end;
    if (!dataLoader->isDataReady(check))
	{
		loading = true;
        emit(LoadRequest());
		return false;
	}
	if (dataLoader->getValues(begin, end, data))
	{
        loading = false;
        return true;
	}
	return false;
}





double onewiredevice::getMainValue(qint64 t, bool &loading, formula *)
{
	QMutexLocker locker(&mutexGet);
	dataLoader->logGetValue = logEnabled.isChecked();
	if (!dataLoader->isDataReady(t))
	{
        loading = true;
        emit(LoadRequest());
        // ici traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
        // ici trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
		//connect(this, SIGNAL(finishedDataLoading()), sender, SLOT(CalculateThread()), Qt::UniqueConnection);
		//if (sender)
		//	if (!senderList.contains(sender))
		//		senderList.append(sender);
		return logisdom::NA;
	}
	return dataLoader->getValue(t);
}




double onewiredevice::getMainValue(const QDateTime &T, bool &loading, formula *)
{
	QMutexLocker locker(&mutexGet);
	dataLoader->logGetValue = logEnabled.isChecked();
	if (!dataLoader->isDataReady(T))
	{
		loading = true;
        emit(LoadRequest());
        // ici traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
        // ici trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
		//connect(this, SIGNAL(finishedDataLoading()), sender, SLOT(CalculateThread()), Qt::UniqueConnection);
		//if (sender)
		//	if (!senderList.contains(sender))
		//		senderList.append(sender);
		return logisdom::NA;
	}
	return dataLoader->getValue(T);
}





qint64 onewiredevice::getNextIndex(const QDateTime &T, bool &loading)
{
	QMutexLocker locker(&mutexGet);
	dataLoader->logGetValue = logEnabled.isChecked();
	if (!dataLoader->isDataReady(T))
	{
		loading = true;
        emit(LoadRequest());
		return logisdom::NA;
	}
	return dataLoader->getNextIndex(T);
}




void onewiredevice::setMasternullptr()
{
	master = nullptr;
}





bool onewiredevice::isDataLoading()
{
	return dataLoader->busy;
}



void onewiredevice::finishedLoading()
{
    setValid(finishedLoadingData);
    emit(finishedDataLoading(this));
    //for (int n=0; n<senderList.count(); n++) senderList.at(n)->CalculateThread();
    //disconnect(this, SIGNAL(finishedDataLoading()), 0, 0);
    //senderList.clear();
}



dataloader *onewiredevice::getdataloader()
{
	return dataLoader;
}



void onewiredevice::clearData()
{
	dataLoader->clearData();
}






net1wire *onewiredevice::getMaster()
{
	return master;
}


QString onewiredevice::getMasterName()
{
    if (master) return master->getname();
    if (plugin_interface) return plugin_interface->getName();
    return "";
}


bool onewiredevice::setscratchpad(const QString&, bool)
{
	return false;
}




bool onewiredevice::hasPreviousDatFile()
{
	QDateTime now = QDateTime::currentDateTime();
	QDateTime lastmonth = now.addMonths(-1);
	QString lastfilename = QString(parent->getrepertoiredat()) + romid + "_" + lastmonth.toString("MM-yyyy") + dat_ext;
	QFile lastfile(lastfilename);
	return lastfile.exists();
}




bool onewiredevice::fileAlreadyInsideZip(QString datfileName, QString zipFileName)
{
	QMutexLocker locker(&mutexGet);
	parent->GenMsg("Check zip File " + zipFileName + " if " + datfileName + " already inside zip");
	QuaZip zipFile(zipFileName);
	if (zipFile.open(QuaZip::mdUnzip))
	{
                QTextCodec *codec = zipFile.getFileNameCodec();
                zipFile.setFileNameCodec(codec);
                //zipFile.setFileNameCodec("IBM866");
		QuaZipFileInfo info;
		QuaZipFile datFile(&zipFile);
		QString filename;
		for(bool more=zipFile.goToFirstFile(); more; more=zipFile.goToNextFile())
		{
			if (zipFile.getCurrentFileInfo(&info))
			{
				if (datFile.getZipError() == UNZ_OK)
				{
					filename = datFile.getActualFileName();
					//parent->GenMsg("next file " + filename + " in zipfile ");
					if (filename == datfileName)
					{
						//parent->GenMsg("file " + datfileName + " found");
						zipFile.close();
						return true;
					}
				}
			}
		}
		zipFile.close();
	}
	//parent->GenMsg("file " + datfileName + " not found");
	return false;
}




bool onewiredevice::copyZipWithoutFile(QString datfileName, QString zipFileName)
{
    QMutexLocker locker(&mutexGet);
    QString tempFileName = QString(parent->getrepertoiredat()) + romid + ".tmp";
    QuaZip zipFile(zipFileName);
    QuaZip tempZipFile(tempFileName);
    bool success = true;
	if (zipFile.open(QuaZip::mdUnzip) && tempZipFile.open(QuaZip::mdCreate))
	{
                QTextCodec *codec = zipFile.getFileNameCodec();
                zipFile.setFileNameCodec(codec);
                //zipFile.setFileNameCodec("IBM866");
                QuaZipFileInfo info;
		QuaZipFile datFile(&zipFile);
		QuaZipFile tempDatFile(&tempZipFile);
		QString currentfilename;
		for(bool more=zipFile.goToFirstFile(); more; more=zipFile.goToNextFile())
		{
			if (zipFile.getCurrentFileInfo(&info))
			{
				if (datFile.getZipError() == UNZ_OK)
				{
					currentfilename = datFile.getActualFileName();
					//parent->GenMsg("Found " + currentfilename + " in " + zipFileName);
					if (currentfilename != datfileName)
					{
						if (datFile.open(QIODevice::ReadOnly) && tempDatFile.open(QIODevice::WriteOnly, QuaZipNewInfo(currentfilename, currentfilename)))
						{
							//parent->GenMsg("Copy " + currentfilename + " in new zip file");
							QByteArray data = datFile.readAll();
							datFile.close();
							tempDatFile.write(data);
							if (tempDatFile.getZipError() != UNZ_OK)
							{
								success = false;
								parent->GenMsg("Cannot write Zip file " + tempFileName);
							}
							tempDatFile.close();
							if (tempDatFile.getZipError() != UNZ_OK)
							{
								success = false;
								parent->GenMsg("Cannot write Zip file " + tempFileName);
							}
						}
						else
						{
							parent->GenMsg("Cannot open file into zip");
							zipFile.close();
							tempZipFile.close();
							QFile tmpZipFile(tempFileName);
							tmpZipFile.remove();
							return false;
						}
					}
					else parent->GenMsg("Don't copy " + currentfilename + " in new zip file");
				}
				else success = false;
			}
		}
                zipFile.close();
                tempZipFile.close();
		if (success)
		{
			QFile oldZipFile(zipFileName);
                        if (oldZipFile.remove())
                        {
                            QFile newZipFile(tempFileName);
                            if (newZipFile.rename(zipFileName))
                            {
                                parent->GenMsg("File " + zipFileName + " was successfully copied without " + datfileName);
                                return true;
                            }
                        }
                        else
                        {
                            parent->GenMsg("File " + zipFileName + " cannot be removed");
                        }
		}
	}
	return false;
}




bool onewiredevice::zipPreviousDatFile()
{
	if (isReprocessing())
	{
		//parent->GenMsg("Zipping aborted " + romid + " reprocessing running");
		return false;
	}
	//parent->GenMsg("Start zipping " + romid);
	QString datfilename;
	QString shortFileName;
	QString zipFileName;
// File search
	QDateTime now = QDateTime::currentDateTime();
	QDir fileSearch(parent->getrepertoiredat());
	QStringList filters;
	filters << romid + "*.dat";
	QStringList fileList = fileSearch.entryList(filters);	// Get file list with same RomID
	if (fileList.count() < 2) return false;
	//parent->GenMsg(QString("found %1 files ").arg(fileList.count()));
	for (int n=0; n<fileList.count(); n++)		// For each file
	{
		//parent->GenMsg(QString(fileList.at(n)));
		QString YM = fileList.at(n);	// get current file
		YM.chop(4);		// remove extension
		YM = YM.right(7);	// take year and month
		int idL = romid.length();
		QString id = fileList.at(n).left(idL);
		bool okY, okM;
		int Y = YM.right(4).toInt(&okY);
		int M = YM.left(2).toInt(&okM);
		if (okY && okM && (id == romid))  // if romid is ok and to Int succeed
		{
			if (!((Y == now.date().year()) && (M == now.date().month())))	// if file is not actual month
			{
				shortFileName = fileList.at(n);
				parent->GenMsg("Add " + shortFileName + " to zip file");
				datfilename = QString(parent->getrepertoiredat()) + fileList.at(n);
                zipFileName = QString(parent->getrepertoirezip()) + romid + "_" + QString("%1").arg(Y, 4, 10, QChar('0')) + ".zip";
				break;
			}
		}
	}
	if (datfilename.isEmpty())
	{
		//parent->GenMsg("No file found " + romid);
		return false;		// not dat file found
	}
	QFile datfile(datfilename);
	if (datfile.open(QIODevice::ReadWrite))		// Open file to take ownership
	{
		parent->GenMsg("open " + datfilename);
		QuaZip zipFile(zipFileName);
// Check if file already exist inside then remove it
		if (fileAlreadyInsideZip(shortFileName, zipFileName))
		{
			parent->GenMsg("File " + shortFileName + " already inside zip");
			if (copyZipWithoutFile(shortFileName, zipFileName))
			{
				parent->GenMsg("File " + shortFileName + " removed from zip");
			}
			else
			{
				parent->GenMsg("File " + shortFileName + " could not be removed from zip");
				return false;
			}
		}
// if zip file cannot open, create it
    if (!zipFile.open(QuaZip::mdAdd)) zipFile.open(QuaZip::mdCreate);
// open zip file
		if (zipFile.getZipError() == UNZ_OK)
		{
			QuaZipFile outZipFile(&zipFile);
// open dat file to add to zip file
			if (datfile.isOpen())
			{
				bool zipped = true;
				QByteArray data = datfile.readAll();
				if (outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(shortFileName, shortFileName)))
				{
					outZipFile.write(data);
					datfile.close();
					if (outZipFile.getZipError() != UNZ_OK)
					{
						zipped = false;
						parent->GenMsg("Cannot create Zip file " + zipFileName);
						return false;
					}
					else parent->GenMsg("File : " + datfilename + " was zipped to " + zipFileName);
					outZipFile.close();
					if (zipped)
					{
						datfile.remove();
						parent->GenMsg("File : " + datfilename + " was removed");
					}
				}
				else
				{
					parent->GenMsg("Cannot open " + zipFileName);
					return false;
				}
			}
			else
			{
				parent->GenMsg("Cannot open " + datfilename);
				return false;
			}
		}
	}
	else
	{
		parent->GenMsg("Cannot open " + datfilename);
		return false;
	}
	return true;
}





void onewiredevice::convertDatToV2(QString)
{
}


QString onewiredevice::getOffCommand()
{
	return "not defined";
}



QString onewiredevice::getOnCommand()
{
	return "not defined";
}



bool onewiredevice::hastoread()
{
    if (saveInterval.isAutoSave()) return false;
    else return saveInterval.isitnow();
}


bool onewiredevice::isAutoSave()
{
	return saveInterval.isAutoSave();
}



void onewiredevice::savevalue(const QDateTime &now, const double &V, bool intervalCheck)
{
    if (isReprocessing()) return;
	QMutexLocker locker(&mutexGet);
    if (int(V) == logisdom::NA) return;
    if (!isAutoSave())
	{
		if (intervalCheck)
			if (!saveInterval.isitnow()) return;
	}
    if (logisdom::isNotNA(V) and ((skip85.isChecked() && logisdom::AreNotSame(V, 85)) or (!skip85.isChecked())))
	{
		if (parent->graphconfigwin) parent->graphconfigwin->AddData(V, now, romid);
		dataLoader->appendData(now, V);
	}
    if (!QDir().exists(parent->getrepertoiredat()))
	{
		parent->logfile("Try to create Dir " + parent->getrepertoiredat());
		if (!QDir().mkdir(parent->getrepertoiredat()))
		{
			parent->logfile("Cannot create Dir " + parent->getrepertoiredat());
			return;
		}
		parent->logfile("Dir " + parent->getrepertoiredat() + " created");
	}
    QString filename = QString(parent->getrepertoiredat()) + romid + "_" + now.toString("MM-yyyy") + dat_ext;
    QString filenameThread = romid + "_" + now.toString("MM-yyyy") + dat_ext;
    /*QFile file(filename);
    if (!file.exists())
	{
		newFile = true;
		// create new file
		if(file.open(QIODevice::Append | QIODevice::Text))
		{
			QTextStream out(&file);
                        out << "// Version 1\n";
                        datFileVersion = 1;
                        //out << "// Version 2\n";
                        //datFileVersion = 2;
                        file.close();
                }
		else
		{
			parent->logfile("cannot open file " + filename);
			return;
		}
    }
    else if (datFileVersion < 0)
    {
        // open file to check version
        if (!file.open(QIODevice::ReadOnly))
        {
                parent->logfile("cannot read " + filename + " to check version, set to version 2");
                datFileVersion = 1;
                //datFileVersion = 2;
        }
        else
        {
            QString line = file.readLine();
            if (line.contains("Version 1")) datFileVersion = 1;
            else datFileVersion = 1;
            //else datFileVersion = 2;
            file.close();
        }
    }
	if (!file.open(QIODevice::Append | QIODevice::Text))
	{
		parent->logfile("cannot write to " + filename);
		return;
    }*/
    //QTextStream out(&file);
	QString strOut;
	bool checkLastsave = false;
	if (lastSaveIndex < 0) checkLastsave = true;
	int currentIndex = (now.date().month() * 31 * 24) + (now.date().day() * 24) + now.time().hour();
	if (currentIndex != lastSaveIndex) checkLastsave = true;
    if (firstsave || checkLastsave || (now.time().minute() == 0))
    {
        strOut = now.toString("(dd)[HH:mm:ss]") + now.toUTC().toString("{dd:HH:mm:ss}") + "'" + QString("%1").arg(V, 0, 'f', Decimal.value()) + "'" + "\n";
        // UTC strOut = now.toUTC().toString("(dd)[HH:mm:ss]") + "'" + QString("%1").arg(V, 0, 'f', Decimal.value()) + "'" + "\n";
    }
	else
	{
		strOut = now.toString("[mm:ss]");
        if (logisdom::AreSame(lastsavevalue, V)) strOut += "=";
		else strOut += "'" + QString("%1").arg(V, 0, 'f', Decimal.value()) + "'";
		strOut += "\n";
	}
	if (!strOut.isEmpty())
	{
        //out << strOut;
        emit(saveDat(filenameThread, strOut));
		firstsave = false;
		lastSaveIndex = currentIndex;
	}
    //file.close();
	lastsavevalue = V;
}






double onewiredevice::calcultemperature(const QString &)
{
	return logisdom::NA;
}






int onewiredevice::getresolution(QString scratch)
{
	bool ok;
	if (family == family1820) return 9;
    int Config = scratch.mid(8,2).toInt(&ok,16) & 0x60 >> 5;
    if ((Config == 3) and ok) return 12;
    if ((Config == 2) and ok) return 11;
    if ((Config == 1) and ok) return 10;
    if ((Config == 0) and ok) return 9;
	return 0;
}




void onewiredevice::setTobeDeleted()
{
    tobeDeleted = true;
}



double onewiredevice::calcultemperaturealarmeH(const QString &scratch)
{
double T;
bool ok;

	T = scratch.mid(4,2).toInt(&ok,16);
	if (T > 128) T = 128 - T;
	return T;
}





double onewiredevice::calcultemperaturealarmeB(const QString &scratch)
{
double T;
bool ok;

	T = scratch.mid(6,2).toInt(&ok,16);
	if (T > 128) T = 128 - T;
	return T;
}





QString onewiredevice::checkValid(const QString str)
{
    if ((valid == dataNotValid) and (logisdom::isNotNA(MainValue))) return cstr::toStr(cstr::NotValid) + " (" + str + ")";
	return str;
}



void onewiredevice::setdualchannelAOn()
{
}





void onewiredevice::setdualchannelAOff()
{
}




void onewiredevice::lecture()
{
    if (!plugin_interface) return;
    if (saveInterval.checkOnly()) plugin_interface->readDevice(romid);
}




void onewiredevice::lecturerec()
{
    if (!plugin_interface) return;
    if (saveInterval.checkOnly()) plugin_interface->readDevice(romid);
}



void onewiredevice::masterlecturerec()
{
}


void onewiredevice::skip85Changed(int state)
{
	if (state) dataLoader->check85 = true;
	else dataLoader->check85 = false;
}



bool onewiredevice::check85(double V)
{
	if (!skip85.isChecked()) return false;
	check85Values.append(V);
	if (check85Values.count() > 10) check85Values.removeAt(0);
	double mean = 0;
	for (int n=0; n<check85Values.count(); n++) mean += check85Values[n];
	mean /= check85Values.count();
	double dif = qAbs(mean - V);
    if ((logisdom::AreSame(V, 85)) && (dif > 5)) return true;
	return false;
}




QString onewiredevice::MainValueToStrLocal(const QString &str)
{
	return str;
}




QString onewiredevice::MainValueToStr()
{
	QString str;
    str = ValueToStr(MainValue);
    str = checkValid(str);
	ValueButton.setText(str);
	return str;
}




QString onewiredevice::ValueToStr(double Value, bool noText)
{
    if (UsetextValuesChanged)
    {
        textValList.clear();
        textStrList.clear();
        for (int n=0; n<TextValues.count(); n++)
        {
            QStringList split = TextValues.itemText(n).split("=");
            if (split.count() == 2)
            {
                bool ok;
                double v = split.at(0).toDouble(&ok);
                if (ok && !split.at(1).isEmpty())
                {
                    textValList.append(v);
                    textStrList.append(split.at(1));
                }
            }
        }
        UsetextValuesChanged = false;
    }
    QString str;
    if (!UsetextValues.isChecked() && (logisdom::isNotNA(Value)))  str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
    else if ((noText) && (logisdom::isNotNA(Value)))               str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
    else if (UsetextValues.isChecked())
	{
        int index = textValList.indexOf(MainValue);
        if (index != -1) str = textStrList.at(index);
    }
    if (str.isEmpty()) str = MainValueToStrLocal(str); else MainValueToStrLocal(str);
    if (!isVirtualFamily() && (logisdom::isNA(Value))) str = cstr::toStr(cstr::NA);
    if (isVirtualFamily() && str.isEmpty() && (logisdom::isNA(Value))) str = cstr::toStr(cstr::NA);
    if (str.isEmpty()) str = "\"" + parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text() + "\"";
    if (!isVirtualFamily()) MainValueToStrLocal(str);
    return str;
}




QString onewiredevice::MainValueToStrNoWarning()
{
	QString str = MainValueToStr();
	return str.remove(cstr::toStr(cstr::NotValid));
}


// Plante quand max valuie n'est pas bon

void onewiredevice::assignMainValue(double value)
{
    if (logisdom::isNA(value)) {
        IncWarning();
        emitDeviceValueChanged();
        return; }
        if (plugin_interface) {
            //if (writeBox.isChecked()) {
                //if (value < Min.value()) value = Min.value();
                //if (value > Max.value()) value = Max.value();
                double r = value;
                if (AXplusB.isEnabled()) r = AXplusB.fromResult(value);
                QString command = romid + QString("=%1").arg(r, 0, 'f', 3);
                if ((lastCommand == command) && (lastCommandCount++ < 10)) return;
                lastCommandCount = 0;
                lastCommand = command;
            //qDebug() << QDateTime::currentDateTime().toString() + "  " + romid + QString("=%1").arg(value, 0, 'f', 3);
            plugin_interface->setStatus(command); }
        else {
            assignMainValueLocal(value); }
}




double onewiredevice::getMaxValue()
{
    if (plugin_interface) return plugin_interface->getMaxValue(romid);
    return logisdom::NA;
}


void onewiredevice::assignMainValueLocal(double)
{
}

bool onewiredevice::readNow()
{
    return false;
}



void onewiredevice::setMainValue(QString&)
{
}


void onewiredevice::emitDeviceValueChanged()
{
    lastMainValue = MainValue;
    emit(DeviceValueChanged(this));
}


void onewiredevice::setPluginMainValue(double value)
{
    setMainValue(AXplusB.result(value), true);
    if (plugin_interface) {
        stdui.Value->setText(MainValueToStr()); }
}


void onewiredevice::setMainValue(double value, bool enregistremode)
{
    if (logisdom::isNA(value)) {
        IncWarning();
        emitDeviceValueChanged();
        return; }
	MainValue = value;
	ClearWarning();
    lastTimeSetValue = QDateTime::currentDateTime();
    secondaryValue = lastTimeSetValue.toString("HH:mm:ss");
    lastMessage.restart();
    lastValue.setText("Dernière lecture : " + secondaryValue);
    if (enregistremode) savevalue(lastTimeSetValue, MainValue);
    QString valueStr = MainValueToStr();
    htmlBind->setValue(valueStr);
    htmlBind->setParameter("Value", valueStr.remove(getunit()));
    emitDeviceValueChanged();
    if (listTreeItem) listTreeItem->setText(1,valueStr);
}


void onewiredevice::setHtmlMenulist(QListWidget *List)
{
	htmlBind->setHtmlMenulist(List);
}





void onewiredevice::removeHtmlMenulist(QString name)
{
	htmlBind->removeHtmlMenulist(name);
}




void onewiredevice::setconfig(const QString &strsearch)
{
    setname(romid);
    AXplusB.setconfig(strsearch);
    QString Name = logisdom::getvalue("Name", strsearch);
    if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(romid));
    AXplusB.setconfig(strsearch);
    if ((family == family1820) || (family == family18B20) || (family == family1822) || (family == family1825)) {
        bool ok;
        int sk85 = logisdom::getvalue("Skip85", strsearch).toInt(&ok);
        if (!ok) dataLoader->check85 = false;
        else if (ok && sk85) dataLoader->check85 = true;
        else dataLoader->check85 = false;
    }
    else dataLoader->check85 = false;
}


void onewiredevice::getCfgStr(QString &str, bool All)
{
    if (!isVirtualFamily()) if ((!master) && (!plugin_interface)) return;
    str +="\n" One_Wire_Device "\n";
    if (All)
    {
        str += logisdom::saveformat(RomIDTag, romid);
        str += logisdom::saveformat("Name", name, true);
        str += logisdom::saveformat("Window_X", QString("%1").arg(geometry().x()));
        str += logisdom::saveformat("Window_Y", QString("%1").arg(geometry().y()));
        str += logisdom::saveformat(Window_Width_Mark, QString("%1").arg(width()));
        str += logisdom::saveformat(Window_Height_Mark, QString("%1").arg(height()));
        str += logisdom::saveformat("Window_Hidden", QString("%1").arg(isHidden()));
        if (master) str += logisdom::saveformat("Master", master->getipaddress());
        str += logisdom::saveformat("LogEnabled", QString("%1").arg(logEnabled.isChecked()));
        str += logisdom::saveformat("WarnEnabled", QString("%1").arg(WarnEnabled.isChecked()));
    }

    str += logisdom::saveformat("Unit", Unit.text(), true);
    str += logisdom::saveformat("NextSave", saveInterval.getNext());
	str += logisdom::saveformat("Decimal", QString("%1").arg(Decimal.value()));
	str += logisdom::saveformat("SaveMode", saveInterval.getMode());
	str += logisdom::saveformat("SaveEnabled", saveInterval.getSaveMode());
	str += logisdom::saveformat("UsetextValues", QString("%1").arg(UsetextValues.isChecked()));
    for (int n=0; n<TextValues.count(); n++) str += logisdom::saveformat(QString("textValues_%1").arg(n), TextValues.itemText(n), true);
    UsetextValuesChanged = true;
    str += logisdom::saveformat("timeOut", QString("%1").arg(timeOut.value()));
    //str += logisdom::saveformat("WriteMin", QString("%1").arg(Min.value()));
    //str += logisdom::saveformat("WriteMax", QString("%1").arg(Max.value()));

    htmlBind->getCfgStr(str);
    if (plugin_interface) str += plugin_interface->getDeviceConfig(romid);
    if (All) GetConfigStr(str);
    if (All) str += EndMark;
	str += "\n";
}


void onewiredevice::setCfgStr(QString &strsearch)
{
    int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0, ok = 0;
	x = logisdom::getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = logisdom::getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if ((ok_h != 0) and (h == 0)) show(); else hide();
    QString Name = logisdom::getvalue("Name", strsearch);
    if (master) setconfig(strsearch); else setname(Name);
    if (plugin_interface) setconfig(strsearch);
    setWindowTitle(name);
    Unit.setText(logisdom::getvalue("Unit", strsearch));
	int dec = 0;
    dec = logisdom::getvalue("Decimal", strsearch).toInt(&ok);
	if (ok) Decimal.setValue(dec);
	QString next = logisdom::getvalue("NextSave", strsearch);
	if (next.isEmpty()) saveInterval.setNext(QDateTime::currentDateTime());
		else saveInterval.setNext(QDateTime::fromString (next, Qt::ISODate));
    int savemode = logisdom::getvalue("SaveMode", strsearch).toInt(&ok);
	if (ok) saveInterval.setMode(savemode);
	int saveen = logisdom::getvalue("SaveEnabled", strsearch).toInt(&ok);
	if (!ok) saveInterval.setSaveMode(true);
		else if (saveen) saveInterval.setSaveMode(true);
			else  saveInterval.setSaveMode(false);
	int waren = logisdom::getvalue("WarnEnabled", strsearch).toInt(&ok);
	if (!ok) WarnEnabled.setCheckState(Qt::Checked);
		else if (waren) WarnEnabled.setCheckState(Qt::Checked);
			else WarnEnabled.setCheckState(Qt::Unchecked);
    htmlBind->setCfgStr(strsearch);
	int txtEn = logisdom::getvalue("UsetextValues", strsearch).toInt(&ok);
	if (!ok) UsetextValues.setCheckState(Qt::Unchecked);
		else if (txtEn) UsetextValues.setCheckState(Qt::Checked);
			else UsetextValues.setCheckState(Qt::Unchecked);

    int t = logisdom::getvalue("timeOut", strsearch).toInt(&ok);
    if (ok) timeOut.setValue(t);

    for (int n=0; n<99999;  n++)
	{
		QString userTxt = logisdom::getvalue(QString("textValues_%1").arg(n), strsearch);
		if (userTxt.isEmpty()) break;
		else TextValues.addItem(userTxt);
	}
	UserText_Changed("");
    if (plugin_interface) plugin_interface->setDeviceConfig(romid, strsearch);
    emit(DeviceConfigChanged(this));
}




void onewiredevice::GetConfigStr(QString &str)
{
    AXplusB.GetConfigStr(str);
}



void onewiredevice::setDataLoading()
{
    setValid(dataLoading);
    parent->loadRequest(this);
}



void onewiredevice::startLoading()
{
    if (!dataLoader->isRunning()) dataLoader->start();
}


void onewiredevice::setValid(int datavalid)
{
    //qDebug() << QString("setValid %1").arg(datavalid);
	bool loading = false;
    if (datavalid == finishedLoadingData)
	{
		loading = false;
        valid = datavalid;
    }
    else if ((datavalid == dataNotValid) && logisdom::isNotNA(MainValue))
	{
		loading = dataLoader->isRunning();
		valid = dataNotValid;
		MainValueToStr();
	}
	else if (((valid == ReadRequest) or (valid == dataWaiting)) && (datavalid == ReadRequest))
	{
		ReadRetry ++;
		if (ReadRetry > 2)
		{
			loading = dataLoader->isRunning();
			ReadRetry = 0;
			valid = dataNotValid;
			MainValueToStr();
		}
	}
	else
	{
		loading = dataLoader->isRunning();
		valid = datavalid;
		ReadRetry = 0;
	}
    if (loading) emit(trafficsig(dataLoading));
    else emit(trafficsig(valid));
}




void onewiredevice::setTraffic(int trafficValue)
{
    switch (trafficValue)
    {
        case dataLoading :
            traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
            traffic.setToolTip(tr("Loading data"));
            trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/reload.png")));
            trafficUi.setToolTip(tr("Loading data"));
        break;
        case dataNotValid :
            traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballred.png")));
            traffic.setToolTip(tr("Not Valid"));
            trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballred.png")));
            trafficUi.setToolTip(tr("Not Valid"));
        break;
        case dataWaiting :
            traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballyellow.png")));
            traffic.setToolTip(tr("Waiting"));
            trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballyellow.png")));
            trafficUi.setToolTip(tr("Waiting"));
        break;
        case ReadRequest :
            traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballgreenlight.png")));
            traffic.setToolTip(tr("Read Request"));
            trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballgreenlight.png")));
            trafficUi.setToolTip(tr("Read Request"));
        break;
        case dataValid :
        case finishedLoadingData :
            traffic.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballgreen.png")));
            traffic.setToolTip(tr("Valid"));
            trafficUi.setPixmap(QPixmap(QString::fromUtf8(":/images/images/ballgreen.png")));
            trafficUi.setToolTip(tr("Valid"));
        break;
    }
}




void onewiredevice::DecimalChanged(int)
{
	emitDeviceValueChanged();
}




void onewiredevice::logEnabledChanged(int state)
{
	if (state == Qt::Checked)
	{
		dataLoader->logGetValue = true;
		dataLoader->logLoadData = true;
	}
	else
	{
		dataLoader->logGetValue = false;
		dataLoader->logLoadData = false;
	}
}




void onewiredevice::dataLoaderBeginChanged()
{
#if QT_VERSION > 0x050603
    if (dataLoader->begin) RenameButton.setToolTip(tr("Loaded data since : ") + QDateTime::fromSecsSinceEpoch(dataLoader->begin).toString("dd-MMM-yyyy"));
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    if (dataLoader->begin) RenameButton.setToolTip(tr("Loaded data since : ") + origin.addSecs(dataLoader->begin).toString("dd-MMM-yyyy"));
#endif
    else RenameButton.setToolTip(tr("No data Loaded"));
}




void onewiredevice::unitChanged(const QString &U)
{
	emit(DeviceConfigChanged(this));
	htmlBind->setParameter("Unit", U);
}




void onewiredevice::UserText_Changed(const QString &)
{
    UsetextValuesChanged = true;
}



void onewiredevice::UserText_rigthClick(const QPoint &pos)
{
    QMenu contextualmenu;
    QAction removeAction(tr("Remove current line"), this);
    contextualmenu.addAction(&removeAction);
    QAction *selection;
    selection = contextualmenu.exec(TextValues.mapToGlobal(pos));
    if (selection == &removeAction)
    {
	int i = TextValues.currentIndex();
	if (i != -1) TextValues.removeItem(i);
    }
}




void onewiredevice::RomID_rigthClick(const QPoint &pos)
{
    QMenu contextualmenu;
    QAction copyRomIDAction(tr("Copy RomID"), this);
    QAction copyNameAction(tr("Copy Name"), this);
    QAction saveVirtualDeviceConfig(tr("Save configuration file"), this);
    contextualmenu.addAction(&copyRomIDAction);
    contextualmenu.addAction(&copyNameAction);
    if (isVirtualFamily()) contextualmenu.addAction(&saveVirtualDeviceConfig);
    QAction *selection;
    selection = contextualmenu.exec(RomIDButton.mapToGlobal(pos));
    if (selection == &copyRomIDAction)
    {
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(romid);
    }
    if (selection == &copyNameAction)
    {
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(name);
    }
    if (selection == &saveVirtualDeviceConfig)
    {
        saveDeviceConfig();
    }
}




bool onewiredevice::IncWarning()
{
	RetryWarning ++;
	if (RetryWarning <= 3) return false;
	if (master && WarnEnabled.isChecked()) master->GenError(45, scratchpad + "   RomID = " + romid + "  " + name);
	setValid(dataNotValid);
	emitDeviceValueChanged();
	parent->mainStatus();
	return true;
}



bool onewiredevice::isReprocessing()
{
	return false;
}


void onewiredevice::ClearWarning()
{
	RetryWarning = 0;
	setValid(dataValid);
	parent->mainStatus();
}





bool onewiredevice::isValid()
{
    if (logisdom::isNA(MainValue)) return false;
	if (valid != dataNotValid) return true;
	return false;	
}





QString onewiredevice::assignname(const QString Name)
{
	int index = 1;
	onewiredevice *device = parent->configwin->Devicenameexist(Name);
	if (device == this) return Name;
	if (!device) return Name;
	QString name = Name + QString("%1").arg(index);
	while (parent->configwin->devicenameexist(name)) name = Name + QString("%1").arg(++index);
	return (Name + QString("%1").arg(index));
}




uint8_t onewiredevice::calcCRC8(QVector <uint8_t> s)
{
    uint8_t crc = 0;
    foreach (uint8_t t, s) crc = crc8Table[(crc ^ t) & 0xff];
    return crc;
}


bool onewiredevice::checkCRC8(QString scratchpad)
{
    bool ok;
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    QVector <uint8_t> v;
    for (int n=0; n<scratchpad.length(); n+=2) v.append(uint8_t(scratchpad.mid(n, 2).toUInt(&ok, 16)));
    if (calcCRC8(v))
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        //qDebug() << "bad CRC8";
        return false;
    }
    logMsg("CRC ok");
    return true;
}



bool onewiredevice::checkCRC16(QString scratchpad)
{
    bool ok;
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    QString CrcStr = scratchpad.right(4);
    QString Crc = CrcStr.right(2) + CrcStr.left(2);
    ulong L = ulong((scratchpad.length() / 2) - 2);
    if (L <= 0)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(26, errorMsg);
        return IncWarning();
    }
    uint16_t crc = uint16_t(Crc.toUInt(&ok, 16));
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
        return IncWarning();
    }
    QVector <uint8_t> v;
    for (int n=0; n<scratchpad.length()-4; n+=2) v.append(uint8_t(scratchpad.mid(n, 2).toUInt(&ok, 16)));
    uint16_t crccalc = calcCRC16(v);
    if (crc == crccalc)
    {
        logMsg("CRC ok");
        return true;
    }
    if (master && WarnEnabled.isChecked()) master->GenError(53, errorMsg);
    //qDebug() << "bad CRC16";
    return false;
}



uint16_t onewiredevice::calcCRC16(QVector <uint8_t> s)
{
    uint16_t crc = 0;
    foreach (uint8_t t, s)
        crc = crc16Table[(crc ^ t) & 0xff] ^ (crc>>8);
    crc ^= 0xffff;  // Final Xor Value 0xffff
    return crc;
}


const uint8_t onewiredevice::crc8Table[256] = {
0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53,};


const uint16_t onewiredevice::crc16Table[256] = {
0x0000,  0xc0c1,  0xc181,  0x0140,  0xc301,  0x03c0,  0x0280,  0xc241,
0xc601,  0x06c0,  0x0780,  0xc741,  0x0500,  0xc5c1,  0xc481,  0x0440,
0xcc01,  0x0cc0,  0x0d80,  0xcd41,  0x0f00,  0xcfc1,  0xce81,  0x0e40,
0x0a00,  0xcac1,  0xcb81,  0x0b40,  0xc901,  0x09c0,  0x0880,  0xc841,
0xd801,  0x18c0,  0x1980,  0xd941,  0x1b00,  0xdbc1,  0xda81,  0x1a40,
0x1e00,  0xdec1,  0xdf81,  0x1f40,  0xdd01,  0x1dc0,  0x1c80,  0xdc41,
0x1400,  0xd4c1,  0xd581,  0x1540,  0xd701,  0x17c0,  0x1680,  0xd641,
0xd201,  0x12c0,  0x1380,  0xd341,  0x1100,  0xd1c1,  0xd081,  0x1040,
0xf001,  0x30c0,  0x3180,  0xf141,  0x3300,  0xf3c1,  0xf281,  0x3240,
0x3600,  0xf6c1,  0xf781,  0x3740,  0xf501,  0x35c0,  0x3480,  0xf441,
0x3c00,  0xfcc1,  0xfd81,  0x3d40,  0xff01,  0x3fc0,  0x3e80,  0xfe41,
0xfa01,  0x3ac0,  0x3b80,  0xfb41,  0x3900,  0xf9c1,  0xf881,  0x3840,
0x2800,  0xe8c1,  0xe981,  0x2940,  0xeb01,  0x2bc0,  0x2a80,  0xea41,
0xee01,  0x2ec0,  0x2f80,  0xef41,  0x2d00,  0xedc1,  0xec81,  0x2c40,
0xe401,  0x24c0,  0x2580,  0xe541,  0x2700,  0xe7c1,  0xe681,  0x2640,
0x2200,  0xe2c1,  0xe381,  0x2340,  0xe101,  0x21c0,  0x2080,  0xe041,
0xa001,  0x60c0,  0x6180,  0xa141,  0x6300,  0xa3c1,  0xa281,  0x6240,
0x6600,  0xa6c1,  0xa781,  0x6740,  0xa501,  0x65c0,  0x6480,  0xa441,
0x6c00,  0xacc1,  0xad81,  0x6d40,  0xaf01,  0x6fc0,  0x6e80,  0xae41,
0xaa01,  0x6ac0,  0x6b80,  0xab41,  0x6900,  0xa9c1,  0xa881,  0x6840,
0x7800,  0xb8c1,  0xb981,  0x7940,  0xbb01,  0x7bc0,  0x7a80,  0xba41,
0xbe01,  0x7ec0,  0x7f80,  0xbf41,  0x7d00,  0xbdc1,  0xbc81,  0x7c40,
0xb401,  0x74c0,  0x7580,  0xb541,  0x7700,  0xb7c1,  0xb681,  0x7640,
0x7200,  0xb2c1,  0xb381,  0x7340,  0xb101,  0x71c0,  0x7080,  0xb041,
0x5000,  0x90c1,  0x9181,  0x5140,  0x9301,  0x53c0,  0x5280,  0x9241,
0x9601,  0x56c0,  0x5780,  0x9741,  0x5500,  0x95c1,  0x9481,  0x5440,
0x9c01,  0x5cc0,  0x5d80,  0x9d41,  0x5f00,  0x9fc1,  0x9e81,  0x5e40,
0x5a00,  0x9ac1,  0x9b81,  0x5b40,  0x9901,  0x59c0,  0x5880,  0x9841,
0x8801,  0x48c0,  0x4980,  0x8941,  0x4b00,  0x8bc1,  0x8a81,  0x4a40,
0x4e00,  0x8ec1,  0x8f81,  0x4f40,  0x8d01,  0x4dc0,  0x4c80,  0x8c41,
0x4400,  0x84c1,  0x8581,  0x4540,  0x8701,  0x47c0,  0x4680,  0x8641,
0x8201,  0x42c0,  0x4380,  0x8341,  0x4100,  0x81c1,  0x8081,  0x4040};



