/****************************************************************************
**
** Copyright (C) 2006-2011 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "net1wire.h"

#ifdef MultiGest_No_Thread

#include "errlog.h"
#include "alarmwarn.h"
#include "onewire.h"
#include "formula.h"
#include "configwindow.h"
#include "messagebox.h"
#include "ecogest_no_thread.h"


ecogest::ecogest(logisdom *Parent) : net1wire(Parent)
{
	uiw.setupUi(ui.frame);
	parent = Parent;
	picuploader = NULL;
	unkownCommandRetry = 0;
	request = None;
	busyRetryCount = 0;
	type = NetType(MultiGest);
	setport(default_port_EZL50);
	ui.EditType->setText("MultiGest");
	for (int n=0; n<LastDetector; n++) devices[n] = NULL;
	Interlock = logisdom::NA;
	SolarVirtualStart = false;
	uploadStarted = false;

	uiw.ConvertDelay->setPrefix(tr("Convert Delay  "));
	uiw.ConvertDelay->setValue(1);
	uiw.ConvertDelay->setRange(0.1, 10);
	uiw.ConvertDelay->setDecimals(1);
	uiw.ConvertDelay->setSuffix(tr(" Seconds"));
	uiw.ConvertDelay->setSingleStep(0.1);

	modeProcess = 0;
	setGUImode(modeProcess);

	fluidicGroup.addButton(uiw.pushButtonModeOff);
	fluidicGroup.setId(uiw.pushButtonModeOff, ButtonIDOff);
	fluidicGroup.addButton(uiw.pushButtonModeFill);
	fluidicGroup.setId(uiw.pushButtonModeFill, ButtonIDFill);
	fluidicGroup.addButton(uiw.pushButtonModeKeepFilled);
	fluidicGroup.setId(uiw.pushButtonModeKeepFilled, ButtonIDKeepFiled);
	fluidicGroup.addButton(uiw.pushButtonModeRun);
	fluidicGroup.setId(uiw.pushButtonModeRun, ButtonIDRun);
	fluidicGroup.addButton(uiw.pushButtonModeDegaz);
	fluidicGroup.setId(uiw.pushButtonModeDegaz, ButtonIDDegaz);

	SolarGroup.addButton(uiw.pushButtonModeECS);
	SolarGroup.setId(uiw.pushButtonModeECS, ButtonIDECS);
	SolarGroup.addButton(uiw.pushButtonModeMSD);
	SolarGroup.setId(uiw.pushButtonModeMSD, ButtonIDMSD);
	SolarGroup.addButton(uiw.pushButtonModeNoSun);
	SolarGroup.setId(uiw.pushButtonModeNoSun, ButtonIDNoSun);

	connect(&SolarGroup, SIGNAL(buttonClicked(int)), this, SLOT(solarModeClicked(int)));
	connect(&fluidicGroup, SIGNAL(buttonClicked(int)), this, SLOT(fluidicModeClicked(int)));

	connect(uiw.pushButtonSwitchA, SIGNAL(clicked()), this, SLOT(setHeaterPump()));
	connect(uiw.pushButtonSwitchB, SIGNAL(clicked()), this, SLOT(setSolarPump()));
	connect(uiw.pushButtonSwitchC, SIGNAL(clicked()), this, SLOT(setCirculator()));
	connect(uiw.pushButtonSwitchD, SIGNAL(clicked()), this, SLOT(setFill()));
	connect(uiw.pushButtonSwitchE, SIGNAL(clicked()), this, SLOT(setKeepFilled()));
	connect(uiw.pushButtonMSDEnable, SIGNAL(clicked()), this, SLOT(MSDEnableChanged()));
	//uiw.pushButtonKeepFilled->setStyleSheet("color: yellow");

	connect(uiw.spinBoxServoA, SIGNAL(valueChanged(int)), this, SLOT(ServoAValueChanged(int)));
	connect(uiw.spinBoxServoB, SIGNAL(valueChanged(int)), this, SLOT(ServoBValueChanged(int)));
	connect(uiw.spinBoxServoC, SIGNAL(valueChanged(int)), this, SLOT(ServoCValueChanged(int)));
	connect(uiw.spinBoxServoD, SIGNAL(valueChanged(int)), this, SLOT(ServoDValueChanged(int)));
	connect(uiw.spinBoxServoE, SIGNAL(valueChanged(int)), this, SLOT(ServoEValueChanged(int)));

	connect(uiw.spinBoxDelayON, SIGNAL(valueChanged(int)), this, SLOT(SolarDelayONChanged(int)));
	connect(uiw.spinBoxFillDelay, SIGNAL(valueChanged(int)), this, SLOT(FillDelayValueChanged(int)));
	connect(uiw.spinBoxDegazDelay, SIGNAL(valueChanged(int)), this, SLOT(DegazDelayValueChanged(int)));
	connect(uiw.spinBoxKeepFilledDelay , SIGNAL(valueChanged(int)), this, SLOT(KeepFilledDelayValueChanged(int)));
	connect(uiw.spinBoxTRC , SIGNAL(valueChanged(int)), this, SLOT(TRCValueChanged(int)));
	connect(uiw.spinBoxTRP , SIGNAL(valueChanged(int)), this, SLOT(TRPValueChanged(int)));

	connect(uiw.pushButtonSolarVirtual, SIGNAL(clicked()), this, SLOT(virtualStartClicked()));

	htmlBind->setParameter("Fluidic Mode", cstr::toStr(cstr::NA));
	htmlBind->addParameterCommand("Fluidic Mode", uiw.pushButtonModeFill->text(), "modefill");
	htmlBind->addParameterCommand("Fluidic Mode", uiw.pushButtonModeRun->text(), "moderun");
	htmlBind->addParameterCommand("Fluidic Mode", uiw.pushButtonModeDegaz->text(), "modedegaz");
	htmlBind->addParameterCommand("Fluidic Mode", uiw.pushButtonModeKeepFilled->text(), "modekeepfilled");
	htmlBind->addParameterCommand("Fluidic Mode", uiw.pushButtonModeOff->text(), "modeoff");

	htmlBind->setParameter("Solar Mode", cstr::toStr(cstr::NA));
	htmlBind->addParameterCommand("Solar Mode", uiw.pushButtonModeECS->text(), "modeecs");
	htmlBind->addParameterCommand("Solar Mode", uiw.pushButtonModeMSD->text(), "modemds");
	htmlBind->addParameterCommand("Solar Mode", uiw.pushButtonModeNoSun->text(), "modenosun");

	addtofifo(GetFirmwareVersion);
	connectAll();
}





void ecogest::catchLoaclStrings()
{
	buttonstext[HeaterIn] = uiw.pushButtonT1->text();
	buttonstext[HeaterOut] = uiw.pushButtonT2->text();
	buttonstext[HeatingOut] = uiw.pushButtonT3->text();
	buttonstext[SolarIn] = uiw.pushButtonT4->text();
	buttonstext[SolarOut] = uiw.pushButtonT5->text();
	buttonstext[TankHigh] = uiw.pushButtonT6->text();
	buttonstext[TankLow] = uiw.pushButtonT7->text();
	buttonFlowAText = uiw.pushButtonFlowA->text();
	buttonFlowBText = uiw.pushButtonFlowB->text();
	buttonHeaterPumpText = uiw.pushButtonSwitchA->text();
	buttonSolarPumpText = uiw.pushButtonSwitchB->text();
	buttonCirculatorText = uiw.pushButtonSwitchC->text();
	buttonSolarVirtualText = uiw.pushButtonSolarVirtual->text();
	buttonFillText = uiw.pushButtonSwitchD->text();
	buttonKeepFilledText = uiw.pushButtonSwitchE->text();
}




void ecogest::setGUImode(int mode)
{
	modeProcess = mode;
	switch (mode)
	{
		case 0 : setGUIdefaultMode(); break;
		case 255 : setGUIAutoVidangeMode_A(); break;
		default: modeProcess = 0; setGUIdefaultMode(); break;
	}
	catchLoaclStrings();
}






bool ecogest::isGlobalConvert()
{
	return uiw.Global_Convert->isChecked();
}




void ecogest::setGUIdefaultMode()
{
	uiw.pushButtonT1->setText(tr("T°1"));
	uiw.pushButtonT2->setText(tr("T°2"));
	uiw.pushButtonT3->setText(tr("T°3"));
	uiw.pushButtonT4->setText(tr("T°4"));
	uiw.pushButtonT5->setText(tr("T°5"));
	uiw.pushButtonT6->setText(tr("T°6"));
	uiw.pushButtonT7->setText(tr("T°7"));
	uiw.pushButtonFlowA->setText(tr("Flow A"));
	uiw.pushButtonFlowB->setText(tr("Flow B"));
	uiw.pushButtonFlowC->setText(tr("Flow C"));
	uiw.pushButtonSwitchA->setText(tr("Switch A"));
	uiw.pushButtonSwitchB->setText(tr("Switch B"));
	uiw.pushButtonSwitchC->setText(tr("Switch C"));
	uiw.pushButtonSwitchD->setText(tr("Switch D"));
	uiw.pushButtonSwitchE->setText(tr("Switch E"));
	uiw.pushButtonSwitchF->setText(tr("Switch F"));
	uiw.pushButtonSolarVirtual->setText(tr("Virtual Start"));
}





void ecogest::setGUIAutoVidangeMode_A()
{
	uiw.pushButtonT1->setText(tr("Heater In"));
	uiw.pushButtonT2->setText(tr("Heater Out"));
	uiw.pushButtonT3->setText(tr("Heating Out"));
	uiw.pushButtonT4->setText(tr("Solar In"));
	uiw.pushButtonT5->setText(tr("Solar Out"));
	uiw.pushButtonT6->setText(tr("Tank High"));
	uiw.pushButtonT7->setText(tr("Tank Low"));
	uiw.pushButtonFlowA->setText(tr("Flow Heater"));
	uiw.pushButtonFlowB->setText(tr("Flow Solar"));
	uiw.pushButtonFlowC->setText(tr("Flow C"));
	uiw.pushButtonSwitchA->setText(tr("Heater Pump"));
	uiw.pushButtonSwitchB->setText(tr("Solar Pump"));
	uiw.pushButtonSwitchC->setText(tr("Heating cirulation"));
	uiw.pushButtonSwitchD->setText(tr("Degaz Valve"));
	uiw.pushButtonSwitchE->setText(tr("Switch E"));
	uiw.pushButtonSwitchF->setText(tr("Switch F"));
	uiw.pushButtonSolarVirtual->setText(tr("Démarrage Virtuel"));
}




void ecogest::remoteCommand(const QString &command)
{
	if (command == "modefill") uiw.pushButtonModeFill->click();
	if (command == "moderun") uiw.pushButtonModeRun->click();
	if (command == "modedegaz") uiw.pushButtonModeDegaz->click();
	if (command == "modekeepfilled") uiw.pushButtonModeKeepFilled->click();
	if (command == "modeoff") uiw.pushButtonModeOff->click();

	if (command == "modeecs") uiw.pushButtonModeECS->click();
	if (command == "modemds") uiw.pushButtonModeMSD->click();
	if (command == "modenosun") uiw.pushButtonModeNoSun->click();
}





ecogest::~ecogest()
{
}





void ecogest::getMainValue()
{
	addtofifo("GSP");
}




void ecogest::virtualStartClicked()
{
	if (SolarVirtualStart)
	{
		uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::OFF));
		SolarVirtualStart = false;
		addtofifo("SVT==0");
	}
	else
	{
		uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::ON));
		SolarVirtualStart = true;
		addtofifo("SVT==1");
	}
}





void ecogest::MSDEnableChanged()
{
	if (MSDEnable)
	{
		addtofifo("MSDOFF");
		MSDEnable = false;
	}
	else
	{
		addtofifo("MSDON");
		MSDEnable = true;
	}
}




void ecogest::FillValueChanged(int SliderValue)
{
	devices[devServo4]->setMainValue(SliderValue, false);
}




void ecogest::SolarValueChanged(int SliderValue)
{
	devices[devServo3]->setMainValue(SliderValue, false);
}



void ecogest::TankValueChanged(int SliderValue)
{
	devices[devServo1]->setMainValue(SliderValue, false);
}



void ecogest::HeatingValueChanged(int SliderValue)
{
	devices[devServo2]->setMainValue(SliderValue, false);
}


void ecogest::SolarDelayONChanged(int value)
{
	QString str = "SO==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}


void ecogest::FillDelayValueChanged(int value)
{
	QString str = "SF==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}


void ecogest::DegazDelayValueChanged(int value)
{
	QString str = "SD==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}




void ecogest::KeepFilledDelayValueChanged(int value)
{
	QString str = "SK==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}




void ecogest::TRCValueChanged(int value)
{
	QString str = "TRC==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}




void ecogest::TRPValueChanged(int value)
{
	QString str = "TRP==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}




void ecogest::fluidicModeClicked(int id)
{
	QString str = setFluidicStr "==";
	QString empty = "";
	QString data;
	switch (id)
	{
		case ButtonIDOff : 		data = str + QString("%1").arg(Mode_Solar_Off);
							addtofifo(SetMode, empty, data); break;
		case ButtonIDFill : 		data = str + QString("%1").arg(Mode_Solar_Fill);
							addtofifo(SetMode, empty, data); break;
		case ButtonIDRun : 		data = str + QString("%1").arg(Mode_Solar_Run);
							addtofifo(SetMode, empty, data); break;
		case ButtonIDDegaz : 		data = str + QString("%1").arg(Mode_Solar_Run_Degaz);
							addtofifo(SetMode, empty, data); break;
		case ButtonIDKeepFiled : 	data = str + QString("%1").arg(Mode_Solar_KeepFilled);
							addtofifo(SetMode, empty, data); break;
	}
}




void ecogest::solarModeClicked(int id)
{
	QString str = setSolarStr "==";
	QString empty = "";
	QString data;
	switch (id)
	{
		case ButtonIDECS : 	data = str + QString("%1").arg(Mode_ECS);
						addtofifo(SetMode, empty, data); break;
		case ButtonIDMSD : 	data = str + QString("%1").arg(Mode_MSD);
						addtofifo(SetMode, empty, data); break;
		case ButtonIDNoSun : 	data = str + QString("%1").arg(Mode_NoSun);
						addtofifo(SetMode, empty, data); break;
	}
}




void ecogest::initialsearchreq()
{
	addtofifo(LocalSearch);
	addtofifo(GlobalSearch);
}






QString ecogest::getStr(int index)
{
	QString str;
	switch (index)
	{
		case TankLow : str = "TankLow"; break;
		case TankHigh : str = "TankHigh"; break;
		case HeaterIn : str = "HeaterIn"; break;
		case HeaterOut : str = "HeaterOut"; break;
		case HeatingOut : str = "HeatingOut"; break;
		case SolarIn : str = "SolarIn"; break;
		case SolarOut : str = "SolarOut"; break;
	}
	return str;
}




void ecogest::init()
{
	retry = 0;
	converttimer.setSingleShot(true);
	connect(&converttimer, SIGNAL(timeout()), this, SLOT(timerconvertout()));
	createonewiredevices();
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(this, SIGNAL(requestdone()), this, SLOT(fifonext()));
	TimerPause.setSingleShot(true);
	connect(&TimerPause, SIGNAL(timeout()), this, SLOT(FIFOnext()));
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readExtrabuffer()));
	connect(uiw.listRomIDs, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklList(QPoint)));
	connecttcp();
	search();
	status();
}




void ecogest::getConfig(QString &str)
{
	if (uiw.Global_Convert->isChecked()) str += logisdom::saveformat("GlobalConvert", "1"); else str += logisdom::saveformat("GlobalConvert", "0");
	str += logisdom::saveformat("ConvertDelay", QString("%1").arg(uiw.ConvertDelay->value()));
}



void ecogest::setConfig(const QString &strsearch)
{
	bool GlobalConvert = false;
	if (logisdom::getvalue("GlobalConvert", strsearch) == "1") GlobalConvert = true;
	uiw.Global_Convert->setChecked(GlobalConvert);
	bool ok;
	double CD = logisdom::getvalue("ConvertDelay", strsearch).toDouble(&ok);
	if ((ok) and (CD > 0)) uiw.ConvertDelay->setValue(CD);
}



void ecogest::connectAll()
{
/*	connect(&servo1Range, SIGNAL(valueChanged(int)), this, SLOT(setServo1(int)));
	connect(uiw.SliderServo1, SIGNAL(valueChanged(int)), this, SLOT(setServo1Prc(int)));
	connect(uiw.spinBoxServo1, SIGNAL(valueChanged(int)), this, SLOT(setServo1Prc(int)));
	connect(&servo2Range, SIGNAL(valueChanged(int)), this, SLOT(setServo2(int)));
	connect(uiw.SliderServo2, SIGNAL(valueChanged(int)), this, SLOT(setServo2Prc(int)));
	connect(uiw.spinBoxServo2, SIGNAL(valueChanged(int)), this, SLOT(setServo2Prc(int)));
	connect(uiw.SliderValve1, SIGNAL(valueChanged(int)), this, SLOT(setValve1(int)));
	connect(uiw.spinBoxValve1, SIGNAL(valueChanged(int)), this, SLOT(setValve1(int)));
	connect(uiw.SliderValve2, SIGNAL(valueChanged(int)), this, SLOT(setValve2(int)));
	connect(uiw.spinBoxValve2, SIGNAL(valueChanged(int)), this, SLOT(setValve2(int)));*/
}




void ecogest::disconnectAll()
{
/*	disconnect(&servo1Range, SIGNAL(valueChanged(int)), this, SLOT(setServo1(int)));
	disconnect(uiw.SliderServo1, SIGNAL(valueChanged(int)), this, SLOT(setServo1Prc(int)));
	disconnect(uiw.spinBoxServo1, SIGNAL(valueChanged(int)), this, SLOT(setServo1Prc(int)));
	disconnect(&servo2Range, SIGNAL(valueChanged(int)), this, SLOT(setServo2(int)));
	disconnect(uiw.SliderServo2, SIGNAL(valueChanged(int)), this, SLOT(setServo2Prc(int)));
	disconnect(uiw.spinBoxServo2, SIGNAL(valueChanged(int)), this, SLOT(setServo2Prc(int)));
	disconnect(uiw.SliderValve1, SIGNAL(valueChanged(int)), this, SLOT(setValve1(int)));
	disconnect(uiw.spinBoxValve1, SIGNAL(valueChanged(int)), this, SLOT(setValve1(int)));
	disconnect(uiw.SliderValve2, SIGNAL(valueChanged(int)), this, SLOT(setValve2(int)));
	disconnect(uiw.spinBoxValve2, SIGNAL(valueChanged(int)), this, SLOT(setValve2(int)));*/
}




void ecogest::search()
{
	addtofifo(GlobalSearch);
	addtofifo(LocalSearch);
}




void ecogest::clearRomIDEEprom()
{
	if (messageBox::questionHide(this, tr("Clear EEProm ?"), tr("Are you sure you want to clear ALL RomIDs ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
	{
		for (int n=0; n<LastDetector; n++)
		{
			addtofifo("Set" + getStr(n) + "==" + "FFFFFFFFFFFFFFFF");
			onewiredevice *device = parent->configwin->DeviceExist(RomIDstr[n]);
			if (device) device->logisdomReading = true;
		}
		addtofifo(SearchReset);
	}
}




void ecogest::getScratchPads()
{
	addtofifo(GetScratchPads);
}




void ecogest::saveScratchPads()
{
	addtofifo(SaveScratchPads);
}




void ecogest::status()
{
	addtofifo(GetStatus);
}




void ecogest::removeDuplicates()
{
	QString dup, last;
	int count = fifoListCount();
	last = fifoListLast();
	if (count < 2) return;
	if (last.contains(familyMultiGestServo1 "Val==")) dup = familyMultiGestServo1 "Val==";
	if (last.contains(familyMultiGestServo1 "Max==")) dup = familyMultiGestServo1 "Max==";
	if (last.contains(familyMultiGestServo1 "Min==")) dup = familyMultiGestServo1 "Min==";
	if (last.contains(familyMultiGestServo2 "Val==")) dup = familyMultiGestServo2 "Val==";
	if (last.contains(familyMultiGestServo2 "Max==")) dup = familyMultiGestServo2 "Max==";
	if (last.contains(familyMultiGestServo2 "Min==")) dup = familyMultiGestServo2 "Min==";
	if (last.contains(familyMultiGestServo3 "Val==")) dup = familyMultiGestServo3 "Val==";
	if (last.contains(familyMultiGestServo3 "Max==")) dup = familyMultiGestServo3 "Max==";
	if (last.contains(familyMultiGestServo3 "Min==")) dup = familyMultiGestServo3 "Min==";
	if (last.contains(familyMultiGestServo4 "Val==")) dup = familyMultiGestServo4 "Val==";
	if (last.contains(familyMultiGestServo4 "Max==")) dup = familyMultiGestServo4 "Max==";
	if (last.contains(familyMultiGestServo4 "Min==")) dup = familyMultiGestServo4 "Min==";
	if (last.contains("SO==")) dup = "SO==";
	if (last.contains("SF==")) dup = "SF==";
	if (last.contains("SD==")) dup = "SD==";
	if (last.contains("SK==")) dup = "SK==";
	bool found = false;
	if (!dup.isEmpty())
	do
	{
		found = false;
		count = fifoListCount();
		for (int n=0; n<(count-1); n++)
		{
			QListWidgetItem *item = ui.fifolist->item(n);
			if (item->text().contains(dup))
			{
				ui.fifolist->takeItem(n);
				delete item;
				found = true;
				break;
			}
		}
	} while (found);
}





void ecogest::timerconvertout()
{
	fifoListRemoveFirst();
	busy = false;
	emit requestdone();
}





void ecogest::OpenUpload()
{
	disconnect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	disconnect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	TimeOut.stop();
	settraffic(Disabled);
	tcp.disconnectFromHost();
	switchOnOff(false);
	picuploader = new uploaderwindow(this);
	picuploader->fileName = QString("MultiGest.hex");
	picuploader->setipaddress(moduleipaddress);
	picuploader->setport(port);
	picuploader->setAttribute(Qt::WA_DeleteOnClose);
	picuploader->show();
	uploadStarted = false;
	connect(picuploader, SIGNAL(destroyed()), this, SLOT(CloseUpload()));
}




bool ecogest::isUploading()
{
	if (picuploader) return true;
	return uploadStarted;
}




void ecogest::CloseUpload()
{
	switchOnOff(true);
//	connect(tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	picuploader = NULL;
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connecttcp();
	uploadStarted = false;
}





void ecogest::readExtrabuffer()
{
	QMutexLocker locker(&mutexReadBuffer);
	readbuffer();
	updatebuttonsvalues();
	htmlBind->setParameter("Status", ui.traffic->toolTip());

	uiw.listRomIDs->clear();
	for (int n=0; n<RomIDs.count(); n++)
	{
		QString romID = RomIDs.at(n);
		QString txt = "Error device not created : + " + romID;
		onewiredevice *device = parent->configwin->DeviceExist(romID);
		if (device)
		{
			if (!device->logisdomReading) txt = romID + " " + device->getname();
			else txt = romID;
		}
		QListWidgetItem *widget = new QListWidgetItem(uiw.listRomIDs);
		widget->setText(txt);
	}
}







void ecogest::LocalSearchAnalysis(QString &dat)
{
	QString RomID, ID;
	QStringList list = dat.split("\n");
	for (int i=0; i<list.count(); i++)
	{
		QString data = list[i];
		for (int n=0; n<LastDetector; n++)
		{
	// Create Devices identifed
			RomID = logisdom::getvalue(getStr(n), data);
			if (!RomID.isEmpty())
			{
				onewiredevice *device = parent->configwin->DeviceExist(RomID);
				if (!device) device = parent->configwin->NewDevice(RomID, this);
				if (device)
				{
					device->setname(getStr(n));
					UpdateLocalDeviceList();
					devices[n] = device;
					device->logisdomReading = false;
					RomIDstr[n] = RomID;
					if (!localdevice.contains(device))
					{
						localdevice.append(device);
						device->setname(getStr(n));
						UpdateLocalDeviceList();
					}
				}
			}
		}
		bool ok;
		int MP = logisdom::getvalue("modeProcess", data).toInt(&ok);
		if ((ok) && (MP != modeProcess)) setGUImode(MP);
	}
	uiw.listRomIDs->clear();
	for (int n=0; n<RomIDs.count(); n++)
	{
		QString romID = RomIDs.at(n);
		QString txt = "Error device not created : + " + romID;
		onewiredevice *device = parent->configwin->DeviceExist(romID);
		if (device)
		{
			if (!device->logisdomReading) txt = romID + " " + device->getname();
			else txt = romID;
		}
		QListWidgetItem *widget = new QListWidgetItem(uiw.listRomIDs);
		widget->setText(txt);
	}
}





void ecogest::updatebuttonsvalues()
{
	if (devices[HeaterIn]) uiw.pushButtonT1->setText(buttonstext[HeaterIn] +  " = " + devices[HeaterIn]->MainValueToStr()); else uiw.pushButtonT1->setText(buttonstext[HeaterIn]);
	if (devices[HeaterOut]) uiw.pushButtonT2->setText(buttonstext[HeaterOut] +  " = " + devices[HeaterOut]->MainValueToStr()); else uiw.pushButtonT2->setText(buttonstext[HeaterOut]);
	if (devices[HeatingOut]) uiw.pushButtonT3->setText(buttonstext[HeatingOut] +  " = " + devices[HeatingOut]->MainValueToStr()); else uiw.pushButtonT3->setText(buttonstext[HeatingOut]);
	if (devices[SolarIn]) uiw.pushButtonT4->setText(buttonstext[SolarIn] +  " = " + devices[SolarIn]->MainValueToStr()); else uiw.pushButtonT4->setText(buttonstext[SolarIn]);
	if (devices[SolarOut]) uiw.pushButtonT5->setText(buttonstext[SolarOut] +  " = " + devices[SolarOut]->MainValueToStr()); else uiw.pushButtonT5->setText(buttonstext[SolarOut]);
	if (devices[TankHigh]) uiw.pushButtonT6->setText(buttonstext[TankHigh] +  " = " + devices[TankHigh]->MainValueToStr()); else uiw.pushButtonT6->setText(buttonstext[TankHigh]);
	if (devices[TankLow]) uiw.pushButtonT7->setText(buttonstext[TankLow] +  " = " + devices[TankLow]->MainValueToStr()); else uiw.pushButtonT7->setText(buttonstext[TankLow]);
}





void ecogest::timeout()
{
	busy = false;
	TimeOut.stop();
	wait += default_wait;
	retry ++;
	if (retry > 3)
	{
		retry = 0;
		fifoListRemoveFirst();
		request = None;
		GenError(85, name);
		reconnecttcp();
	}
	fifonext();
}





void ecogest::rightclicklList(const QPoint &pos)
{
	QMenu contextualmenu;
	QModelIndex index = uiw.listRomIDs->currentIndex();
	QListWidgetItem *item = uiw.listRomIDs->currentItem();
	if (!item) return;
	if (index.row() == -1) return;
	QString romID = item->text().left(16);
	QString family = romID.right(2);
	bool valid = false;
	if (family == family1822) valid = true;
	if (family == family1820) valid = true;
	if (family == family18B20) valid = true;
	if (!valid) return;
	QAction *contextualaction[LastDetector + 1];
	for (int n=0; n<LastDetector; n++)
	{
		contextualaction[n] = new QAction(getStr(n), this);
		contextualmenu.addAction(contextualaction[n]);
	}
	QAction *selection = contextualmenu.exec(uiw.listRomIDs->mapToGlobal(pos));
	for (int n=0; n<LastDetector; n++)
		if (selection == contextualaction[n])
		{
			addtofifo("Set" + getStr(n) + "==" + item->text().left(16));
			addtofifo(SearchReset);
		}
	for (int n=0; n<LastDetector; n++) delete contextualaction[n];
}





void ecogest::saveMainValue()
{
	addtofifo(SaveStatus);
	addtofifo(SaveScratchPads);
}





void ecogest::setDefaultNames()
{
	if ((messageBox::questionHide(this, tr("Set Default Names ?"), tr("Do you want to set default names ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
	{
	// Servo 1
		if (devices[devServo1]) devices[devServo1]->setname("Servo Tank");
	// Servo 2
		if (devices[devServo2]) devices[devServo2]->setname("Servo Heating");
	// Servo3
		if (devices[devServo3]) devices[devServo3]->setname("Servo Solar");
	// Servo4
		if (devices[devServo4]) devices[devServo4]->setname("Servo Fill");
	// Interlock
		if (devices[devInterlock]) devices[devInterlock]->setname("Interlock Loop");
	// FlowA
		if (devices[devFlowA]) devices[devFlowA]->setname("FlowA");
	// FlowB
		if (devices[devFlowB]) devices[devFlowB]->setname("FlowB");
	// FlowC
		if (devices[devFlowC]) devices[devFlowB]->setname("FlowB");
	// Switch A / Heater Pump
		if (devices[devSwitchA]) devices[devSwitchA]->setname("Heater Pump");
	// Switch B / Solar Pump
		if (devices[devSwitchB]) devices[devSwitchB]->setname("Solar Pump");
	// Switch C / Circulator
		if (devices[devSwitchC]) devices[devSwitchC]->setname("Circulator");
	// Switch D / Fill
		if (devices[devSwitchD]) devices[devSwitchD]->setname("Fill Valve");
	// Switch E / KeepFilled
		if (devices[devSwitchE]) devices[devSwitchE]->setname("Keep Fill Valve");
	// Switch F
		if (devices[devSwitchF]) devices[devSwitchF]->setname("Keep Fill Valve");
	// Fluidic mode
		if (devices[devFluidicMode]) devices[devFluidicMode]->setname("Fluidic Mode");
	// Solar mode
		if (devices[devSolarMode]) devices[devSolarMode]->setname("Solar Mode");
		UpdateLocalDeviceList();
	}
}



void ecogest::createonewiredevices()
{
	QString romid;
	onewiredevice *device;
// Servo 1
	RomIDstr[devServo1] = ip2Hex(moduleipaddress) + familyMultiGestServo1 + familyMultiGestValve;
	device = parent->configwin->NewDevice(RomIDstr[devServo1], this);
	if (device)
	{
		devices[devServo1] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Servo 2
	RomIDstr[devServo2] = ip2Hex(moduleipaddress) + familyMultiGestServo2 + familyMultiGestValve;
	device = parent->configwin->NewDevice(RomIDstr[devServo2], this);
	if (device)
	{
		devices[devServo2] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Servo3
	RomIDstr[devServo3] = ip2Hex(moduleipaddress) + familyMultiGestServo3 + familyMultiGestValve;
	device = parent->configwin->NewDevice(RomIDstr[devServo3], this);
	if (device)
	{
		devices[devServo3] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Servo4
	RomIDstr[devServo4] = ip2Hex(moduleipaddress) + familyMultiGestServo4 + familyMultiGestValve;
	device = parent->configwin->NewDevice(RomIDstr[devServo4], this);
	if (device)
	{
		devices[devServo4] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Interlock
	romid = ip2Hex(moduleipaddress) + "IL" + familyMultiGestWarn;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devInterlock] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowA
	romid = ip2Hex(moduleipaddress) + "FA" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devFlowA] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowB
	romid = ip2Hex(moduleipaddress) + "FB" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devFlowB] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowC
	romid = ip2Hex(moduleipaddress) + "FC" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devFlowC] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch A / Heater Pump
	romid = ip2Hex(moduleipaddress) + "HP" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchA] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch B / Solar Pump
	romid = ip2Hex(moduleipaddress) + "SP" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchB] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch C / Circulator
	romid = ip2Hex(moduleipaddress) + "CR" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchC] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch D / Fill
	romid = ip2Hex(moduleipaddress) + "SD" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchD] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch E / KeepFilled
	romid = ip2Hex(moduleipaddress) + "SE" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchE] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch F
	romid = ip2Hex(moduleipaddress) + "SF" + familyMultiGestSwitch;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSwitchF] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Fluidic mode
	romid = ip2Hex(moduleipaddress) + familyMultiGestFluidicMode + familyMultiGestMode;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devFluidicMode] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Solar mode
	romid = ip2Hex(moduleipaddress) + familyMultiGestSolarMode + familyMultiGestMode;
	device = parent->configwin->NewDevice(romid, this);
	if (device)
	{
		devices[devSolarMode] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
	UpdateLocalDeviceList();
}






void ecogest::SetStatusValues(QString &data, bool enregistremode)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::SetStatusValues";
bool ok;
	QString V;
	QByteArray configdata;
	QString order;
	disconnectAll();
	QString next = fifoListNext();
	if (!fifoListEmpty()) order = getOrder(next);
// FlowA
	V = logisdom::getvalue("FLA", data);
	if (!V.isEmpty())
		if (devices[devFlowA])
		{
			devices[devFlowA]->setscratchpad(V, enregistremode);
			uiw.pushButtonFlowA->setText(buttonFlowAText + " : " + V);		
		}
// FlowB
	V = logisdom::getvalue("FLB", data);
	if (!V.isEmpty())
		if (devices[devFlowB])
		{
			devices[devFlowB]->setscratchpad(V, enregistremode);
			uiw.pushButtonFlowB->setText(buttonFlowBText + " : " + V);		
		}
// Interlock
	V = logisdom::getvalue("Interlock", data);
	if (V == "Closed")
	{
		Interlock = false; 
//		uiw.pushButtonInterlock->setText(buttonInterlockText + tr(" Closed"));		
		QPalette pal = palette();
		pal.setColor(QPalette::Normal,QPalette::Button,Qt::red);
//		uiw.pushButtonInterlock->setPalette(pal);
	}
	else
	{
		Interlock = true;
//		uiw.pushButtonInterlock->setText(buttonInterlockText + tr(" Opened"));		
		QPalette pal = palette();
		pal.setColor(QPalette::Normal,QPalette::Button,Qt::blue);
//		uiw.pushButtonInterlock->setPalette(pal);
	}
// Switch A / Heater Pump
	V = logisdom::getvalue("SHP", data);
	if (!V.isEmpty())
		if (devices[devSwitchA])
		{
			devices[devSwitchA]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchA->setText(buttonHeaterPumpText + " : " + devices[devSwitchA]->MainValueToStr());
		}
// Switch B / Solar Pump
	V = logisdom::getvalue("SSP", data);
	if (!V.isEmpty())
		if (devices[devSwitchB])
		{
			devices[devSwitchB]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchB->setText(buttonSolarPumpText + " : " + devices[devSwitchB]->MainValueToStr());
		}
// Switch C / Circulator
	V = logisdom::getvalue("SCR", data);
	if (!V.isEmpty())
		if (devices[devSwitchC])
		{
			devices[devSwitchC]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchC->setText(buttonCirculatorText + " : " + devices[devSwitchC]->MainValueToStr());
		}
// Switch D / Fill
	V = logisdom::getvalue("SFL", data);
	if (!V.isEmpty())
		if (devices[devSwitchD])
		{
			devices[devSwitchD]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchD->setText(buttonFillText + " : " + devices[devSwitchD]->MainValueToStr());
		}
// Switch E / KeepFilled
	V = logisdom::getvalue("SKF", data);
	if (!V.isEmpty())
		if (devices[devSwitchE])
		{
			devices[devSwitchE]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchE->setText(buttonKeepFilledText + " : " + devices[devSwitchE]->MainValueToStr());
		}
// Switch F
	V = logisdom::getvalue("SWF", data);
	if (!V.isEmpty())
		if (devices[devSwitchF])
		{
			devices[devSwitchF]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchE->setText(buttonKeepFilledText + " : " + devices[devSwitchF]->MainValueToStr());
		}
// Servo1
	V = logisdom::getvalue("S1Val", data);
	if (!V.isEmpty())
		if (devices[devServo1])
		{
			devices[devServo1]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoA, SIGNAL(valueChanged(int)), this, SLOT(TankValueChanged(int)));
			uiw.spinBoxServoA->setValue((int)devices[devServo1]->getMainValue());
			connect(uiw.spinBoxServoA, SIGNAL(valueChanged(int)), this, SLOT(TankValueChanged(int)));
		}
// Servo 2
	V = logisdom::getvalue("S2Val", data);
	if (!V.isEmpty())
		if (devices[devServo2])
		{
			devices[devServo2]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoB, SIGNAL(valueChanged(int)), this, SLOT(HeatingValueChanged(int)));
			uiw.spinBoxServoB->setValue((int)devices[devServo2]->getMainValue());
			connect(uiw.spinBoxServoB, SIGNAL(valueChanged(int)), this, SLOT(HeatingValueChanged(int)));
		}
// Servo 3
	V = logisdom::getvalue("S3Val", data);
	if (!V.isEmpty())
		if (devices[devServo3])
		{
			devices[devServo3]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoC, SIGNAL(valueChanged(int)), this, SLOT(SolarValueChanged(int)));
			uiw.spinBoxServoC->setValue((int)devices[devServo3]->getMainValue());
			connect(uiw.spinBoxServoC, SIGNAL(valueChanged(int)), this, SLOT(SolarValueChanged(int)));
		}
// Servo 4
	V = logisdom::getvalue("S4Val", data);
	if (!V.isEmpty())
		if (devices[devServo4])
		{
			devices[devServo4]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoD, SIGNAL(valueChanged(int)), this, SLOT(FillValueChanged(int)));
			uiw.spinBoxServoD->setValue((int)devices[devServo4]->getMainValue());
			connect(uiw.spinBoxServoD, SIGNAL(valueChanged(int)), this, SLOT(FillValueChanged(int)));
		}
// Servo 5
		V = logisdom::getvalue("S5Val", data);
		if (!V.isEmpty())
			if (devices[devServo5])
			{
				devices[devServo5]->setscratchpad(V, enregistremode);
				disconnect(uiw.spinBoxServoE, SIGNAL(valueChanged(int)), this, SLOT(ServoEValueChanged(int)));
				uiw.spinBoxServoE->setValue((int)devices[devServo5]->getMainValue());
				connect(uiw.spinBoxServoE, SIGNAL(valueChanged(int)), this, SLOT(ServoEValueChanged(int)));
			}
// Fluidic Mode
	QString Fluidic = logisdom::getvalue("FluidicMode", data);
	int fluidic = Fluidic.toInt(&ok);
	if (ok)
	{
		disconnect(&fluidicGroup, SIGNAL(buttonClicked(int)), this, SLOT(fluidicModeClicked(int)));
		switch (fluidic)
		{
			case Mode_Solar_Fill : 
				uiw.pushButtonModeFill->setChecked(true); 
				htmlBind->setParameter("Fluidic Mode", uiw.pushButtonModeFill->text());
				break;
			case Mode_Solar_Run : 
				uiw.pushButtonModeRun->setChecked(true);
				htmlBind->setParameter("Fluidic Mode", uiw.pushButtonModeRun->text());
				break;
			case Mode_Solar_Run_Degaz : 
				uiw.pushButtonModeDegaz->setChecked(true);
				htmlBind->setParameter("Fluidic Mode", uiw.pushButtonModeDegaz->text());
				break;
			case Mode_Solar_KeepFilled : 
				uiw.pushButtonModeKeepFilled->setChecked(true);
				htmlBind->setParameter("Fluidic Mode", uiw.pushButtonModeKeepFilled->text());
				break;
			case Mode_Solar_Off : 
				uiw.pushButtonModeOff->setChecked(true);
				htmlBind->setParameter("Fluidic Mode", uiw.pushButtonModeOff->text());
				break;
		}
		if (devices[devFluidicMode]) devices[devFluidicMode]->setscratchpad(Fluidic, enregistremode);
		connect(&fluidicGroup, SIGNAL(buttonClicked(int)), this, SLOT(fluidicModeClicked(int)));
	}
// Solar Mode
	QString Solar = logisdom::getvalue("SolarMode", data);
	int solar = Solar.toInt(&ok);
	if (ok)
	{
		disconnect(&SolarGroup, SIGNAL(buttonClicked(int)), this, SLOT(fluidicModeClicked(int)));
		switch (solar)
		{
			case Mode_ECS :
				uiw.pushButtonModeECS->setChecked(true);
				htmlBind->setParameter("Solar Mode", uiw.pushButtonModeECS->text());
				break;
			case Mode_MSD :
				uiw.pushButtonModeMSD->setChecked(true);
				htmlBind->setParameter("Solar Mode", uiw.pushButtonModeMSD->text());
				break;
			case Mode_NoSun :
				uiw.pushButtonModeNoSun->setChecked(true);
				htmlBind->setParameter("Solar Mode", uiw.pushButtonModeNoSun->text());
				break;
		}
		if (devices[devSolarMode]) devices[devSolarMode]->setscratchpad(Solar, enregistremode);
		connect(&SolarGroup, SIGNAL(buttonClicked(int)), this, SLOT(fluidicModeClicked(int)));
	}
// Solar Virtual Start
	int solarVirtual = logisdom::getvalue("SolarVirtual", data).toInt(&ok);
	if (ok)
	{
		if (solarVirtual)
		{
			uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::ON));
			SolarVirtualStart = true;
		}
		else
		{
			uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::OFF));
			SolarVirtualStart = false;
		}
	}
// Solar DOVar
	int solarDOVar = logisdom::getvalue("SDV", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxDelayON, SIGNAL(valueChanged(int)), this, SLOT(SolarDelayONChanged(int)));
		uiw.spinBoxDelayON->setValue(solarDOVar);
		connect(uiw.spinBoxDelayON, SIGNAL(valueChanged(int)), this, SLOT(SolarDelayONChanged(int)));
	}
// Solar FlllDelayVar
	int solarFillDelayVar = logisdom::getvalue("SFV", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxFillDelay, SIGNAL(valueChanged(int)), this, SLOT(FillDelayValueChanged(int)));
		uiw.spinBoxFillDelay->setValue(solarFillDelayVar);
		connect(uiw.spinBoxFillDelay, SIGNAL(valueChanged(int)), this, SLOT(FillDelayValueChanged(int)));
	}
// Solar Degaz DelayVar
	int solarDegazDelayVar = logisdom::getvalue("SGV", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxDegazDelay, SIGNAL(valueChanged(int)), this, SLOT(DegazDelayValueChanged(int)));
		uiw.spinBoxDegazDelay->setValue(solarDegazDelayVar);
		connect(uiw.spinBoxDegazDelay, SIGNAL(valueChanged(int)), this, SLOT(DegazDelayValueChanged(int)));
	}
// Keep Filled DelayVar
	int solarKeepFilledDelayVar = logisdom::getvalue("SKV", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxKeepFilledDelay , SIGNAL(valueChanged(int)), this, SLOT(KeepFilledDelayValueChanged(int)));
		uiw.spinBoxKeepFilledDelay->setValue(solarKeepFilledDelayVar);
		connect(uiw.spinBoxKeepFilledDelay , SIGNAL(valueChanged(int)), this, SLOT(KeepFilledDelayValueChanged(int)));
	}
// MSD Enable
	int MSDEnabled = logisdom::getvalue("MSDEnable", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.pushButtonMSDEnable, SIGNAL(clicked()), this, SLOT(MSDEnableChanged()));
		if (MSDEnabled)
		{
			uiw.pushButtonMSDEnable->setChecked(true); 
			MSDEnable = true;
		}
		else
		{
			uiw.pushButtonMSDEnable->setChecked(false);
			MSDEnable = false;
		}
		connect(uiw.pushButtonMSDEnable, SIGNAL(clicked()), this, SLOT(MSDEnableChanged()));
	}
// Solar DO
	int solarDO = logisdom::getvalue("SON", data).toInt(&ok);
	if (ok) uiw.pushButtonDODelay->setText(QString("%1").arg(solarDO));
// Solar FlllDelay
	int FlllDelay = logisdom::getvalue("SFD", data).toInt(&ok);
	if (ok) uiw.pushButtonFillDelay->setText(QString("%1").arg(FlllDelay));
// Solar solarDegazDelay
	int solarDegazDelay = logisdom::getvalue("SDD", data).toInt(&ok);
	if (ok) uiw.pushButtonDegazDelay->setText(QString("%1").arg(solarDegazDelay));
// Solar solarKeepFilledDelay
	int solarKeepFilledDelay = logisdom::getvalue("SKD", data).toInt(&ok);
	if (ok) uiw.pushButtonKeepFilledDelay->setText(QString("%1").arg(solarKeepFilledDelay));
// TRC
	int TRC = logisdom::getvalue("TRC", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxTRC, SIGNAL(valueChanged(int)), this, SLOT(TRCValueChanged(int)));
		uiw.spinBoxTRC->setValue(TRC);
		connect(uiw.spinBoxTRC, SIGNAL(valueChanged(int)), this, SLOT(TRCValueChanged(int)));
	}
// TRP
	int TRP = logisdom::getvalue("TRP", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxTRP, SIGNAL(valueChanged(int)), this, SLOT(TRPValueChanged(int)));
		uiw.spinBoxTRP->setValue(TRP);
		connect(uiw.spinBoxTRP, SIGNAL(valueChanged(int)), this, SLOT(TRPValueChanged(int)));
	}
//	GenMsg(str);
	connectAll();
	catchException_logstr
}





void ecogest::appendConfigStr(QString *str, QString RomID, QString Value)
{
	*str += "\n" One_Wire_Device "\n";
	*str += logisdom::saveformat(RomIDTag, RomID);
	*str += logisdom::saveformat(Device_Value_Tag, Value);
	*str += EndMark;
	*str += "\n";
}





void ecogest::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		{
		 QMenu contextualmenu;
		 QAction Search(tr("&Search"), this);
		 QAction SearchReset(tr("&SearchReset"), this);
		 QAction ClearRomIDEEprom(tr("&Clear EEprom RomID"), this);
		 QAction Upload(tr("&Upload"), this);
		 QAction GetSP(tr("&Get ScratchPads"), this);
		 QAction RStart(tr("&Restart"), this);
		 QAction GetST(tr("&Get Status"), this);
		 QAction SetNames(tr("&Set Defaul Names"), this);
		 contextualmenu.addAction(&Search);
		 contextualmenu.addAction(&SearchReset);
		 contextualmenu.addAction(&RStart);
		 contextualmenu.addAction(&ClearRomIDEEprom);
		 contextualmenu.addAction(&Upload);
		 contextualmenu.addAction(&GetSP);
		 contextualmenu.addAction(&GetST);
		 contextualmenu.addAction(&SetNames);
		 QAction *selection;
		 selection = contextualmenu.exec(event->globalPos());
		 if (selection == &Search) search();
		 if (selection == &ClearRomIDEEprom) clearRomIDEEprom();
		 if (selection == &RStart) restart();
		 if (selection == &Upload) OpenUpload();
		 if (selection == &GetSP) addtofifo(GetScratchPads);
		 if (selection == &GetST) addtofifo(GetStatus);
		 if (selection == &SetNames) setDefaultNames();
		}
	if (event->button() != Qt::LeftButton) return;
}





void ecogest::restart()
{
	addtofifo(Restart);
}




void ecogest::setHeaterPump()
{
	if (devices[devSwitchA]->getMainValue() != 0) addtofifo("HPOFF"); else addtofifo("HPON");
}


void ecogest::setSolarPump()
{
	if (devices[devSwitchB]->getMainValue() != 0) addtofifo("SPOFF"); else addtofifo("SPON");
}


void ecogest::setCirculator()
{
	if (devices[devSwitchC]->getMainValue() != 0) addtofifo("CROFF"); else addtofifo("CRON");
}



void ecogest::setFill()
{
	if (devices[devSwitchD]->getMainValue() != 0) addtofifo("FLOFF"); else addtofifo("FLON");
}



void ecogest::setKeepFilled()
{
	if (devices[devSwitchE]->getMainValue() != 0) addtofifo("KFOFF"); else addtofifo("KFON");
}



void ecogest::setSwitchF()
{
	if (devices[devSwitchF]->getMainValue() != 0) addtofifo("KFOFF"); else addtofifo("KFON");
}


void ecogest::convert()
{
//	addtofifo(ConvertTemp);
//	addtofifo(GetScratchPads);
}





void ecogest::simpleend()
{
	TimeOut.stop();
	request = None;
	fifoListRemoveFirst();
	if ((request != ConvertTemp) and (request != ConvertADC) and (request != ConvertV) and (request != ConvertTempRomID) and (request != NetError)) emit requestdone();
}




void ecogest::busyRetry()
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::busyRetry";
	TimeOut.stop();
	request = None;
	busyRetryCount ++;
	if (busyRetryCount > 5)
	{
		busyRetryCount = 0;
		fifoListRemoveFirst();
		if ((request != ConvertTemp) and (request != ConvertADC) and (request != ConvertV) and (request != ConvertTempRomID) and (request != NetError)) emit requestdone();
	}
	else pausefifo(1500);
	catchException_logstr
}




void ecogest::commandRetry()
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::commandRetry";
	TimeOut.stop();
	request = None;
	unkownCommandRetry ++;
	if (unkownCommandRetry > 3)
	{
		unkownCommandRetry = 0;
		fifoListRemoveFirst();
		if ((request != ConvertTemp) and (request != ConvertADC) and (request != ConvertV) and (request != ConvertTempRomID) and (request != NetError)) emit requestdone();
	}
	else emit requestdone();
	catchException_logstr
}








void ecogest::GlobalSearchAnalysis(QString &data)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::GlobalSearchAnalysis";
	QString RomID, channel , channel0, channel1, channel2;
	RomIDs.clear();
	Channels.clear();
	int Ch0, Ch1, Ch2;
	int index = 0;
//	GenMsg(data);
	int busShorted = channel.indexOf("Bus Shorted");
	if (busShorted != -1) GenError(56, "Bus shorted");
	Ch0 = data.indexOf("Channel = (0)");
	Ch1 = data.indexOf("Channel = (1)");
	Ch2 = data.indexOf("Channel = (2)");
	if ((Ch0 != -1) and (Ch1 != -1))
	{
		channel = data.mid(Ch0, Ch1 - Ch0);
		RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		while (!RomID.isEmpty())
		{
			checkFamily(RomID, "0");
			index ++;
			RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		}
	}
	if ((Ch1 != -1) and (Ch2 != -1))
	{
		channel = data.mid(Ch1, Ch2 - Ch1);
		RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		while (!RomID.isEmpty())
		{
			checkFamily(RomID, "1");
			index ++;
			RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		}
	}
	if (Ch2 != -1)
	{
		channel = data.mid(Ch2, channel.length() - Ch2);
		RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		while (!RomID.isEmpty())
		{
			checkFamily(RomID, "2");
			index ++;
			RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
		}
	}
	UpdateLocalDeviceList();
	busy = false;
	catchException_logstr
}






void ecogest::checkFamily(const QString &RomID, const QString &Channel)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::checkFamily";
	QString family = RomID.right(2);
	if (family == family2423)
	{
		createDevice(RomID + "_A", Channel);
		createDevice(RomID + "_B", Channel);
	}
	if (family == family2413)
	{
		createDevice(RomID + "_A", Channel);
		createDevice(RomID + "_B", Channel);
	}
	else if (family == family2438)
	{
		createDevice(RomID + "_T", Channel);
		createDevice(RomID + "_V", Channel);
		createDevice(RomID + "_A", Channel);
	}
	else if (family == family2450)
	{
		createDevice(RomID + "_A", Channel);
		createDevice(RomID + "_B", Channel);
		createDevice(RomID + "_C", Channel);
		createDevice(RomID + "_D", Channel);
	}
	else if (family == family2408)
	{
		createDevice(RomID + "_A", Channel);
		createDevice(RomID + "_B", Channel);
		createDevice(RomID + "_C", Channel);
		createDevice(RomID + "_D", Channel);
		createDevice(RomID + "_E", Channel);
		createDevice(RomID + "_F", Channel);
		createDevice(RomID + "_G", Channel);
		createDevice(RomID + "_H", Channel);
	}
	else createDevice(RomID, Channel);
	catchException_logstr
}





void ecogest::createDevice(const QString &RomID, const QString &Channel)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::createDevice";
	onewiredevice *device = parent->configwin->DeviceExist(RomID);
	if (device) foundDevice(device, Channel);
	else
	{
		onewiredevice *device = parent->configwin->NewDevice(RomID, this);
		if (device) foundDevice(device, Channel);
	}
	catchException_logstr
}




void ecogest::foundDevice(onewiredevice *device, const QString Channel)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::foundDevice";
	if (!localdevice.contains(device)) localdevice.append(device);
	UpdateLocalDeviceList();
	RomIDs.append(device->getromid());
	Channels.append(Channel);
	catchException_logstr
}





void ecogest::fifonext()
{
//	QMutexLocker locker(&mutexFifonext);
	if (busy) return;
//	if (TimerPause->isActive()) return;
//	busy = true;
	pausefifo(100);
}




void ecogest::pausefifo(int delay)
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::pausefifo";
	settraffic(Paused);
	TimerPause.start(delay);
	catchException_logstr
}




void ecogest::FIFOnext()
{
//	QMutexLocker locker(&mutexFifonext);
	if (TimerPause.isActive()) return;
	settraffic(Connected);
	if (fifoListEmpty())
	{
		//emit(fifoEmpty());
		QTimer::singleShot(1000, this, SLOT(DeviceRealTime()));
		return;		// si le fifo est vide, on quitte
	}
	busy = true;
	QString next = fifoListNext();
	QString order = getOrder(next);
	request = getorder(order);
	QString reqRomID = getRomID(next);
	QString Data = getData(next);
	QString Req, Channel;
	int IDindex = RomIDs.indexOf(reqRomID);
	if (IDindex != -1) Channel = Channels[IDindex];
	Prequest = parent->configwin->DeviceExist(reqRomID);
	if (Prequest) Prequest->setValid(onewiredevice::dataWaiting);
	switch(request)
	{
		case None :
			Req = order; break;
			break;

		case Reset :			// commande reset 1 wire bus
			reqRomID = "";
			Req = "ResetBus";
			break;

		case Restart :			// commande Restart
			reqRomID = "";
			Req = NetRequestMsg[Restart];
			break;

		case GetFirmwareVersion :			// commande GetVersion
			reqRomID = "";
			Req = NetRequestMsg[GetFirmwareVersion];
			break;

		case Search :			// commande search 1 wire, only for Ha7Net
			reqRomID = "";
			Req = NetRequestMsg[Search];
			break;

		case LocalSearch :			// commande search 1 wire, will only return required device to operate solar and heater
			reqRomID = "";
			Req =  NetRequestMsg[LocalSearch];
			break;

		case GlobalSearch :			// commande search 1 wire, will return all devices connected and found
			reqRomID = "";
			Req =  NetRequestMsg[GlobalSearch];
			break;

		case SearchReset :			//
			reqRomID = "";
			Req =  NetRequestMsg[SearchReset];
			break;

		case SetMode :			//
			reqRomID = "";
			Req =  Data;
			break;

		case SetValue :			//
			reqRomID = "";
			Req =  Data;
			break;

		case SkipROM :			// complement commande conversion, skip ROM
			reqRomID = "";
			Req = NetRequestMsg[SkipROM];
			break;

		case ConvertTemp :			// commande conversion
			reqRomID = "";
			Req = NetRequestMsg[ConvertTemp];
			break;

		case ConvertTempRomID :			// commande conversion
			Req = "WCS=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=44";
			break;

		case RecallMemPage00h :			// commande conversion
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=B800";
			 break;

		case ConvertV :			// commande conversion
			if (reqRomID.isEmpty()) Req = "ConvertV";
			else Req = "WCS=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=B4";
			break;

		case ConvertADC :			// commande conversion
			if (reqRomID.isEmpty()) Req = "ConvertADC";
			else Req = "WCS=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=3C0F00FFFF";
			break;

		case ReadADC :
		case ReadADCRec :
			// WCh=2&Adr=31000000C71C1928&Dat=BEFFFFFFFFFFFFFFFFFF
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=AA0000FFFFFFFFFFFFFFFFFFFF";
			break;

		case ReadPage :
		case ReadPageRec :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=BE00FFFFFFFFFFFFFFFFFF
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=BE00FFFFFFFFFFFFFFFFFF";
			break;

		case ReadPIO :
		case ReadRecPIO :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=F08800FFFFFFFFFFFFFFFFFFFF
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=F08800FFFFFFFFFFFFFFFFFFFF";
			break;

		case ChannelAccessWrite :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO  /PIO FF
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=5A" + Data + "FF";
			break;

		case setChannelOn :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO  /PIO FF
			if (Prequest)
			{
				QString data = Prequest->getOnCommand();
				Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=5A" + data + "FF";
			}
			else GenError(28, "setChannelOn command aborted");
			break;

		case setChannelOff :
			// http://IP address/1Wire/WriteBlock.html?Address=8D00000085C8C126&Data=5A PIO  /PIO FF
			if (Prequest)
			{
				QString data = Prequest->getOffCommand();
				Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=5A" + data + "FF";
			}
			else GenError(28, "setChannelOn command aborted");
			break;
/*
		case WriteMemory :
			// http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=55' + DataToWrite);
			// http://192.168.0.250/1Wire/WriteBlock.html?Address=6E00000003925E20&Data=55080000FFFFFF
			if (Data.isEmpty()) break;
			request = WriteScratchpad;
			 if (LockID.isEmpty()) setrequest("WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=55" + Data);
				else setrequest("WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=55" + Data);
			break;
*/
		case WriteScratchpad00 :
			// http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data4E00=' + DataToWrite);
			if (Data.isEmpty()) break;
			Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=4E00" + Data;
			break;

		case CopyScratchpad00 :			//
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=4800";
			 break;

		case ReadTemp :
		case ReadTempRec :
			// WCh=2&Adr=31000000C71C1928&Dat=BEFFFFFFFFFFFFFFFFFF
			 Req = "WCh=" + Channel + "&Adr=" + reqRomID + "&Dat=BEFFFFFFFFFFFFFFFFFF";
			break;
		case ReadCounter :
		case ReadCounterRec :
			// WCh=1&Adr=0E0000000DB72B1D_A&Dat=A5DF01FFFFFFFFFFFFFFFFFFFFFF
			if (reqRomID.right(4) == family2423_A) Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=A5DF01FFFFFFFFFFFFFFFFFFFFFF";
			else if (reqRomID.right(4) == family2423_B) Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=A5FF01FFFFFFFFFFFFFFFFFFFFFF";
			break;
		case ReadDualSwitch :
		case ReadDualSwitchRec :
			// WCh=2&Adr=31000000C71C1928&Dat=F5FF
			Req = "WCh=" + Channel + "&Adr=" + reqRomID.left(16) + "&Dat=F5FF";
			break;
/*

		case WriteScratchpad :
			// http://' + IP addres + '/1Wire/WriteBlock.html?Address=' + RomID + '&Data=4E' + DataToWrite);
			if (Data.isEmpty()) break;
			request = WriteScratchpad;
				 if (LockID.isEmpty()) setrequest("WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=4E" + Data);
				else setrequest("WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=4E" + Data);
			break;

		case WriteEEprom :
			// http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=48'
				 if (LockID.isEmpty()) setrequest("WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=48");
				else setrequest("WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=48");
			break;

		case RecallEEprom :
			// http://IP address/1Wire/WriteBlock.html?Address=RomID&Data=B8'
				 if (LockID.isEmpty()) setrequest("WriteBlock.html?Address=" + reqRomID.left(16) + "&Data=B8");
				else setrequest("WriteBlock.html?" + LockID + "&Address=" + reqRomID.left(16) + "&Data=B8");
			break;

*/
		default : 	Req = order; break;
	}
	currentRequest = Req;
	Req += "\r";
	if (tcp.isOpen())
	{
		for (int n = 0 ; n < Req.length(); n ++) writeTcp(Req[n].unicode());
		settraffic(Waitingforanswer);
	}
	else
	{
		settraffic(Disabled);
	}
	GenMsg(tr("Write : ") + Req);
	TimeOut.start(5000);
}






QString ecogest::extractBuffer(const QString &data)
{
	QString extract = "";
	int chdeb, chfin, L;
	Buffer += data;
	chdeb = Buffer.indexOf("<");
	chfin = Buffer.indexOf(">");
	L = Buffer.length();
	if ((chdeb == -1) and (chfin == -1)) return "";		//GenMsg("No begin No end found : ");		//GenMsg("Buffer : " + Buffer);
	if ((chdeb != -1) and (chfin == -1)) return "";		//GenMsg("No end found : ");	//GenMsg("Buffer : " + Buffer);
	if (chdeb > chfin)   //GenMsg("No begin found : ");//GenMsg("Buffer : " + Buffer);
	{
		Buffer = Buffer.right(L - chdeb);
	}
	if ((chdeb != -1) and (chfin != -1))
	{
		extract = Buffer.mid(chdeb + 1, chfin - chdeb - 1);
		Buffer = Buffer.right(L - chfin - 1);
	}
	return extract;
}




void ecogest::endconvert()
{
	//converttimer.start((int)(uiw.ConvertDelay->value() * 1000.0));
	int convTime = 0;
	//if (Prequest) convTime = Prequest->getConvertTime();
	//if (convTime == 0) convTime = ConvertDelay.value() * 1000.0;
	if (convTime == 0) convTime = 1000;//ConvertDelay.value() * 1000.0;
	converttimer.start(convTime);
}





void ecogest::endGetVersion(QString &data)
{
	TimeOut.stop();
	request = None;
	fifoListRemoveFirst();
	QString version = logisdom::getvalue("Version", data);
	uiw.labelVersion->setText(version);
	emit requestdone();
}





void ecogest::readbuffer()
{
	tryMacro
	exeptionLogStr = "Exception in ecogest::readbuffer";
	QString R;
	QByteArray data;
	QString extract;
more:
	data = tcp.readAll();
	extract = extractBuffer(data);
	if (extract.isEmpty()) return;
	busy = false;
next:
	if (extract.length() > 10000) GenMsg(tr("Read : ") + extract.left(50) + "....." + extract.right(50));
	else  GenMsg(tr("Read : ") + extract);
	if (fifoListCount() == 0)
	{
// GetStatus
		if (extract.contains("Send : " + NetRequestMsg[GetStatus])) SetStatusValues(extract, false);
		if (extract.contains("LogisDom Controler")) GenError(35, "Interface Restarted");
		retry = 0;
		unkownCommandRetry = 0;
		if (Buffer.isEmpty()) return; else goto more;
	}
	QString next = fifoListNext();
	QString order = getOrder(next);
	data.clear();
	data.append(extract);
	if (extract.contains("Busy"))
	{
		busyRetry();
		return;
	}
	if (extract.contains("Unknown Command : "))
	{
		commandRetry();
		return;
	}
	else if (extract.contains("Done : ") && extract.contains(currentRequest))
	{
		settraffic(Connected);
		TimeOut.stop();
		retry = 0;
		unkownCommandRetry = 0;
		busyRetryCount = 0;
		QString devicescratchpad = logisdom::getvalue("ScratchPad", extract);
		QString R18 = devicescratchpad.right(18);
		QString R26 = devicescratchpad.right(26);
		QString R2 = devicescratchpad.right(2);
		switch(request)
		{
			case None : simpleend(); break;
			case Reset : simpleend(); break;
			case SkipROM : simpleend(); break;
			case Restart : simpleend(); break;
			case GetFirmwareVersion : endGetVersion(extract); break;
//			case WriteEEprom : simpleend(); break;
//			case RecallEEprom : simpleend(); break;
//			case WriteScratchpad : simpleend(); break;
			case WriteScratchpad00 : simpleend(); break;
			case CopyScratchpad00 : simpleend(); break;
			case RecallMemPage00h : simpleend(); break;
			case ConvertTemp : endconvert(); break;
			case ConvertTempRomID : endconvert(); break;
			case GetStatus : SetStatusValues(extract, false); simpleend(); break;
			case SetMode : SetStatusValues(extract, true); simpleend(); break;
			case SetValue : SetStatusValues(extract, true); simpleend(); break;
			case SaveStatus : SetStatusValues(extract, true); simpleend(); break;
			case GetScratchPads : parent->configwin->SetDevicesScratchpad(data, false); simpleend(); break;
			case SaveScratchPads : parent->configwin->SetDevicesScratchpad(data, true); simpleend(); break;
			case ConvertADC : endconvert(); break;
			case ConvertV : endconvert(); break;
//			case WriteMemory : endofwritememory(data); emit requestdone(); break;
			case Search : GlobalSearchAnalysis(extract); simpleend(); break;
			case LocalSearch : LocalSearchAnalysis(extract); simpleend(); break;
			case GlobalSearch : GlobalSearchAnalysis(extract); simpleend(); break;
			case SearchReset : LocalSearchAnalysis(extract); simpleend(); break;
			case ReadPIO : if (Prequest) if (order == NetRequestMsg[ReadPIO]) Prequest->setscratchpad(R26, false); simpleend(); break;
			case ReadRecPIO : if (Prequest) if (order == NetRequestMsg[ReadRecPIO]) Prequest->setscratchpad(R26, true); simpleend(); break;
			case ReadDualSwitch : if (Prequest) if (order == NetRequestMsg[ReadDualSwitch]) Prequest->setscratchpad(R2, false); simpleend(); break;
			case ReadDualSwitchRec : if (Prequest) if (order == NetRequestMsg[ReadDualSwitchRec]) Prequest->setscratchpad(R2, true); simpleend(); break;
			case setChannelOn : if (Prequest) if (order == NetRequestMsg[setChannelOn]) Prequest->setscratchpad(devicescratchpad, true); simpleend(); break;
			case setChannelOff : if (Prequest) if (order == NetRequestMsg[setChannelOff]) Prequest->setscratchpad(devicescratchpad, true); simpleend(); break;
			case ReadTemp : if (Prequest) if (order == NetRequestMsg[ReadTemp]) Prequest->setscratchpad(R18, false); simpleend(); break;
			case ReadTempRec : if (Prequest) if (order == NetRequestMsg[ReadTempRec]) Prequest->setscratchpad(R18, true); simpleend(); break;
			case ReadCounter : if (Prequest) if (order == NetRequestMsg[ReadCounter]) Prequest->setscratchpad(devicescratchpad, false); simpleend(); break;
			case ReadCounterRec : if (Prequest) if (order == NetRequestMsg[ReadCounterRec]) Prequest->setscratchpad(devicescratchpad, true); simpleend(); break;
			case ReadADC : if (Prequest) if (order == NetRequestMsg[ReadADC]) Prequest->setscratchpad(R26, false); simpleend(); break;
			case ReadADCRec : if (Prequest) if (order == NetRequestMsg[ReadADCRec]) Prequest->setscratchpad(R26, true); simpleend(); break;
			case ReadPage :  if (Prequest) if (order == NetRequestMsg[ReadPage]) Prequest->setscratchpad(R18, false); simpleend(); break;
			case ReadPageRec : if (Prequest) if (order == NetRequestMsg[ReadPageRec]) Prequest->setscratchpad(R18, true); simpleend(); break;
			default : simpleend();
		}
		extract = extractBuffer("");
		if (!extract.isEmpty()) goto next;
	}
	catchException_logstr
}


#endif // MultiGest_No_Thread


