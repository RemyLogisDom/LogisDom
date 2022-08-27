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
#include <QtCore>
#include "errlog.h"
#include "alarmwarn.h"
#include "onewire.h"
#include "formula.h"
#include "masterthread.h"
#include "configwindow.h"
#include "messagebox.h"
#include "ecogest.h"



ecogest::ecogest(logisdom *Parent) : net1wire(Parent)
{
    uiw.setupUi(ui.frameguinet);
    ui.fifolist->hide();
    picuploader = nullptr;
    TcpThread.endLessLoop = true;
    TcpThread.moduleipaddress = moduleipaddress;
    TcpThread.port = port;
    connect(&TcpThread, SIGNAL(newDevice(QString, QString)), this, SLOT(newDeviceSlot(QString, QString)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(deviceReturn(QString, QString)), this, SLOT(deviceReturn(QString, QString)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(newStatus(QString)), this, SLOT(updatebuttonsvalues(QString)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(modeChanged(int)), this, SLOT(modeChanged(int)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(tcpStatusUpdate(QString)), this, SLOT(tcpStatusUpdate(QString)), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(tcpStatusChange()), this, SLOT(tcpStatusChange()), Qt::QueuedConnection);
    connect(&TcpThread, SIGNAL(searchFinished()), this, SLOT(searchFinished()), Qt::QueuedConnection);
    type = NetType(MultiGest);
    setport(default_port_EZL50);
	ui.EditType->setText("MultiGest");
	for (int n=0; n<LastDetector; n++) devices[n] = nullptr;
    //Interlock = logisdom::NA;
	SolarVirtualStart = false;
	uploadStarted = false;

	uiw.searchInterval->addItem(tr("Search Once"));
	uiw.searchInterval->addItem(tr("Continus Search"));
	uiw.searchInterval->addItem(tr("Search 5 minutes"));
	uiw.searchInterval->addItem(tr("Search 10 minutes"));
	uiw.searchInterval->addItem(tr("Search 30 minutes"));
	uiw.searchInterval->addItem(tr("Search 1 hours"));
	uiw.searchInterval->addItem(tr("Search 6 hours"));
	uiw.searchInterval->addItem(tr("Search 1 day"));

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

	connect(uiw.pushButtonSwitchA, SIGNAL(clicked()), this, SLOT(setSwitchA()));
	connect(uiw.pushButtonSwitchB, SIGNAL(clicked()), this, SLOT(setSwitchB()));
	connect(uiw.pushButtonSwitchC, SIGNAL(clicked()), this, SLOT(setSwitchC()));
	connect(uiw.pushButtonSwitchD, SIGNAL(clicked()), this, SLOT(setSwitchD()));
	connect(uiw.pushButtonSwitchE, SIGNAL(clicked()), this, SLOT(setSwitchE()));
	connect(uiw.pushButtonSwitchF, SIGNAL(clicked()), this, SLOT(setSwitchF()));
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
	connect(uiw.spinBoxHTM , SIGNAL(valueChanged(int)), this, SLOT(HTMValueChanged(int)));
	connect(uiw.spinBoxSOT , SIGNAL(valueChanged(int)), this, SLOT(SOTValueChanged(int)));
	connect(uiw.checkBoxServoSolo , SIGNAL(stateChanged(int)), this, SLOT(SOLOStateChanged(int)));
	connect(uiw.Global_Convert, SIGNAL(toggled(bool)), this, SLOT(Global_ConvertChanged(bool)));
	connect(ui.checkBoxLog , SIGNAL(stateChanged(int)), this, SLOT(logEnabledChanged(int)));

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
	htmlBind->addParameterCommand("MSD Mode", uiw.pushButtonMSDEnable->text(), "modemsdon");

}


ecogest::~ecogest()
{
    TcpThread.endLessLoop = false;
    TcpThread.quit();
}




void ecogest::startTCP()
{
    TcpThread.start(QThread::LowestPriority);
}



void ecogest::switchOnOff(bool state)
{
	if (state)
	{
        TcpThread.endLessLoop = true;
		OnOff = true;
        TcpThread.start(QThread::LowestPriority);
	}
	else
	{
        TcpThread.endLessLoop = false;
		OnOff = false;
	}
}



void ecogest::catchLocalStrings()
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
	buttonFlowCText = uiw.pushButtonFlowC->text();
	buttonSwitchAText = uiw.pushButtonSwitchA->text();
	buttonSwitchBText = uiw.pushButtonSwitchB->text();
	buttonSwitchCText = uiw.pushButtonSwitchC->text();
	buttonSwitchDText = uiw.pushButtonSwitchD->text();
	buttonSwitchEText = uiw.pushButtonSwitchE->text();
	buttonSwitchFText = uiw.pushButtonSwitchF->text();
	buttonSolarVirtualText = uiw.pushButtonSolarVirtual->text();
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
	catchLocalStrings();
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
	uiw.spinBoxServoA->setPrefix(tr("Tank = "));
	uiw.spinBoxServoB->setPrefix(tr("Heating = "));
	uiw.spinBoxServoC->setPrefix(tr("Solar = "));
	uiw.spinBoxServoD->setPrefix(tr("Fill = "));
	//uiw.spinBoxServoE->setPrefix(tr(""));
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

	if (command == "modemsdon") uiw.pushButtonModeNoSun->click();
}




QString ecogest::getScratchPad(const QString &RomID, int scratchpad_ID)
{
    if (!TcpThread.isRunning()) return "";
    return TcpThread.getScratchPad(RomID.left(16), scratchpad_ID);
}




void ecogest::addtofifo(const QString &order)
{
    TcpThread.addToFIFOSpecial(order);
}




void ecogest::addtofifo(int order)
{
    TcpThread.addToFIFOSpecial(NetRequestMsg[order]);
}




void ecogest::addtofifo(int order, const QString &RomID, bool)
{
	QString Data = "";
    TcpThread.addToFIFOSpecial(RomID, Data, order);
}




void ecogest::addtofifo(int order, const QString &RomID, const QString &Data, bool)
{
    TcpThread.addToFIFOSpecial(RomID, Data, order);
}




void ecogest::virtualStartClicked()
{
	if (SolarVirtualStart)
	{
		uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::OFF));
		SolarVirtualStart = false;
        addtofifo("SNC==0");
	}
	else
	{
		uiw.pushButtonSolarVirtual->setText(buttonSolarVirtualText + " : " + cstr::toStr(cstr::ON));
		SolarVirtualStart = true;
        addtofifo("SNC==1");
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




void ecogest::ServoAValueChanged(int SliderValue)
{
	if (devices[devServo1])
		devices[devServo1]->setMainValue(SliderValue, false);
}



void ecogest::ServoBValueChanged(int SliderValue)
{
	if (devices[devServo2])
		devices[devServo2]->setMainValue(SliderValue, false);
}



void ecogest::ServoCValueChanged(int SliderValue)
{
	if (devices[devServo3])
		devices[devServo3]->setMainValue(SliderValue, false);
}



void ecogest::ServoDValueChanged(int SliderValue)
{
	if (devices[devServo4])
		devices[devServo4]->setMainValue(SliderValue, false);
}


void ecogest::ServoEValueChanged(int SliderValue)
{
	if (devices[devServo5])
		devices[devServo5]->setMainValue(SliderValue, false);
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




void ecogest::HTMValueChanged(int value)
{
	QString str = "HTM==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}



void ecogest::SOTValueChanged(int value)
{
	QString str = "SOT==";
	QString empty = "";
	QString data;
	data = str + QString("%1").arg(value);
	addtofifo(SetValue, empty, data);
}




void ecogest::SOLOStateChanged(int state)
{
	QString str = "SLO==";
	QString empty = "";
	QString data;
	if (state == Qt::Unchecked) data = str + QString("%1").arg(0);
	else data = str + QString("%1").arg(1);
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



void ecogest::setipaddress()
{
    moduleipaddress = ui.ipaddressedit->text();
    TcpThread.moduleipaddress = moduleipaddress;
}



void ecogest::init()
{
	connect(uiw.searchInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(searchIntervalChanged(int)));
	ui.toolButtonClear->hide();
	createdevices();
	connect(uiw.listRomIDs, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklList(QPoint)));
    TcpThread.endLessLoop = true;
    TcpThread.moduleipaddress = moduleipaddress;
    TcpThread.port = port;
    TcpThread.start(QThread::LowestPriority);
    ui.traffic->hide();
}




void ecogest::getConfig(QString &str)
{
	if (uiw.Global_Convert->isChecked()) str += logisdom::saveformat("GlobalConvert", "1"); else str += logisdom::saveformat("GlobalConvert", "0");
	str += logisdom::saveformat("searchInterval", QString("%1").arg(uiw.searchInterval->currentIndex()));
}



void ecogest::setConfig(const QString &strsearch)
{
    bool GlobalConvert = false;
    if (logisdom::getvalue("GlobalConvert", strsearch) == "1") GlobalConvert = true;
    uiw.Global_Convert->setChecked(GlobalConvert);
    if (GlobalConvert) TcpThread.GlobalConvert = true;
    bool ok;
    int SI = logisdom::getvalue("searchInterval", strsearch).toInt(&ok);
    if (!ok) SI = 1;
    if (SI < 0) SI = 1;
    uiw.searchInterval->setCurrentIndex(SI);
    searchIntervalChanged(SI);
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







void ecogest::OpenUpload()
{
    settraffic(Disabled);
    TcpThread.endLessLoop = false;
    QElapsedTimer timer;
    timer.start();
    while(TcpThread.isRunning() && (timer.hasExpired(10000)));
    if (!timer.hasExpired(10000))
    {
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
}




bool ecogest::isUploading()
{
    if (picuploader) return true;
    return uploadStarted;
}




void ecogest::CloseUpload()
{
	picuploader = nullptr;
	switchOnOff(true);
	uploadStarted = false;
}






void ecogest::newDeviceSlot(const QString &romid, const QString &name)
{
	onewiredevice *device = parent->configwin->DeviceExist(romid);
	if (!device)
	{
		newDevice(romid);
		device = parent->configwin->DeviceExist(romid);
		if (device)
            if (!name.isEmpty())
                device->logisdomReading = false;
	}
}




void ecogest::searchFinished()
{
    //qDebug() << "searchFinished";
    uiw.listRomIDs->clear();
    QString txt;
    for (int n=0; n<TcpThread.devices.count(); n++)
    {
        QString romid = TcpThread.devices.at(n)->RomID;
        QString localName = TcpThread.devices.at(n)->localName;
        onewiredevice *device = parent->configwin->DeviceExist(romid);
        //qDebug() << romid + "  " + localName;
        if (device)
        {
            QString name = device->getname();
            if (localName.isEmpty()) txt = romid + " " + name;
            else txt = romid + " " + name + "  (" + localName + ")";
            QListWidgetItem *widget = new QListWidgetItem(uiw.listRomIDs);
            widget->setText(txt);
        }
    }
}




void ecogest::logEnabledChanged(int state)
{
    if (state) TcpThread.logEnabled = true;
    else TcpThread.logEnabled = false;
}




void ecogest::modeChanged(int newMode)
{
	setGUImode(newMode);
}






void ecogest::tcpStatusUpdate(const QString &str)
{
    QString txt = ui.textBrowser->toPlainText();
    if(txt.length() > 10000)
    {
        txt = txt.right(9000);
        ui.textBrowser->setText(txt);
    }
    ui.textBrowser->append(str);
    ui.textBrowser->moveCursor(QTextCursor::End);
}



void ecogest::searchIntervalChanged(int index)
{
	int delay = -1;
	switch (index)
	{
		case searchOnce : delay = -1; break;
		case searchConstant : delay = 0; break;
		case search5mn : delay = 300; break;
		case search10mn : delay = 600; break;
		case search30mn : delay = 1800; break;
		case search1hr : delay = 3600; break;
		case search6hr : delay = 21600; break;
		case search1day : delay = 86400; break;
	}
    TcpThread.searchDelay = delay;
}




void ecogest::Global_ConvertChanged(bool state)
{
    TcpThread.GlobalConvert = state;
}



void ecogest::tcpStatusChange()
{
    TcpStateChanged(TcpThread.tcpStatus);
}




void ecogest::updatebuttonsvalues(const QString& data)
{
    if (data.startsWith("Done")) SetDone(data);
    else SetStatusValues(data, true);
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







void ecogest::setDefaultNames()
{
	if ((messageBox::questionHide(this, tr("Set Default Names ?"), tr("Do you want to set default names ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
	{
	// Servo1
		if (devices[devServo1]) devices[devServo1]->setname("Servo Tank");
	// Servo2
		if (devices[devServo2]) devices[devServo2]->setname("Servo Heating");
	// Servo3
		if (devices[devServo3]) devices[devServo3]->setname("Servo Solar");
	// Servo4
		if (devices[devServo4]) devices[devServo4]->setname("Servo Fill");
	// Servo5
		if (devices[devServo5]) devices[devServo5]->setname("Servo 5");
	// Interlock
		if (devices[devInterlock]) devices[devInterlock]->setname("Interlock Loop");
	// FlowA
		if (devices[devFlowA]) devices[devFlowA]->setname("FlowA");
	// FlowB
		if (devices[devFlowB]) devices[devFlowB]->setname("FlowB");
	// FlowC
		if (devices[devFlowC]) devices[devFlowC]->setname("FlowC");
	// Switch A / Heater Pump
		if (devices[devSwitchA]) devices[devSwitchA]->setname("Heater Pump");
	// Switch B / Solar Pump
		if (devices[devSwitchB]) devices[devSwitchB]->setname("Solar Pump");
	// Switch C / Circulator
		if (devices[devSwitchC]) devices[devSwitchC]->setname("Circulator");
	// Switch D / Fill
		if (devices[devSwitchD]) devices[devSwitchD]->setname("Fill Valve");
	// Switch E / KeepFilled
		if (devices[devSwitchE]) devices[devSwitchE]->setname("Switch E");
	// Switch F
		if (devices[devSwitchF]) devices[devSwitchF]->setname("Switch F");
    // Switch N
        if (devices[devSwitchN]) devices[devSwitchN]->setname("Switch Night Cooling");
    // Fluidic mode
		if (devices[devFluidicMode]) devices[devFluidicMode]->setname("Fluidic Mode");
	// Solar mode
		if (devices[devSolarMode]) devices[devSolarMode]->setname("Solar Mode");
		UpdateLocalDeviceList();
	}
}



void ecogest::createdevices()
{
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
// Servo5
	RomIDstr[devServo5] = ip2Hex(moduleipaddress) + familyMultiGestServo5 + familyMultiGestValve;
	device = parent->configwin->NewDevice(RomIDstr[devServo5], this);
	if (device)
	{
		devices[devServo5] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Interlock
	RomIDstr[devInterlock] = ip2Hex(moduleipaddress) + "IL" + familyMultiGestWarn;
	device = parent->configwin->NewDevice(RomIDstr[devInterlock], this);
	if (device)
	{
		devices[devInterlock] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowA
	RomIDstr[devFlowA] = ip2Hex(moduleipaddress) + "FA" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(RomIDstr[devFlowA], this);
	if (device)
	{
		devices[devFlowA] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowB
	RomIDstr[devFlowB] = ip2Hex(moduleipaddress) + "FB" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(RomIDstr[devFlowB], this);
	if (device)
	{
		devices[devFlowB] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// FlowC
	RomIDstr[devFlowC] = ip2Hex(moduleipaddress) + "FC" + familyMultiGestFlow;
	device = parent->configwin->NewDevice(RomIDstr[devFlowC], this);
	if (device)
	{
		devices[devFlowC] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch A / Heater Pump
	RomIDstr[devSwitchA] = ip2Hex(moduleipaddress) + familyMultiGestSwitchA;
	device = parent->configwin->NewDevice(RomIDstr[devSwitchA], this);
	if (device)
	{
		devices[devSwitchA] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch B / Solar Pump
	RomIDstr[devSwitchB] = ip2Hex(moduleipaddress) + familyMultiGestSwitchB;
	device = parent->configwin->NewDevice(RomIDstr[devSwitchB], this);
	if (device)
	{
		devices[devSwitchB] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch C / Circulator
	RomIDstr[devSwitchC] = ip2Hex(moduleipaddress) + familyMultiGestSwitchC;
	device = parent->configwin->NewDevice(RomIDstr[devSwitchC], this);
	if (device)
	{
		devices[devSwitchC] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch D / Fill
	RomIDstr[devSwitchD] = ip2Hex(moduleipaddress) + familyMultiGestSwitchD;
	device = parent->configwin->NewDevice(RomIDstr[devSwitchD], this);
	if (device)
	{
		devices[devSwitchD] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch E / KeepFilled
	RomIDstr[devSwitchE] = ip2Hex(moduleipaddress) + familyMultiGestSwitchE;
	device = parent->configwin->NewDevice(RomIDstr[devSwitchE], this);
	if (device)
	{
		devices[devSwitchE] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch F
    RomIDstr[devSwitchF] = ip2Hex(moduleipaddress) + familyMultiGestSwitchF;
    device = parent->configwin->NewDevice(RomIDstr[devSwitchF], this);
	if (device)
	{
		devices[devSwitchF] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Switch N
    RomIDstr[devSwitchN] = ip2Hex(moduleipaddress) + familyMultiGestSwitchN;
    device = parent->configwin->NewDevice(RomIDstr[devSwitchN], this);
    if (device)
    {
        devices[devSwitchN] = device;
        if (!localdevice.contains(device)) localdevice.append(device);
        UpdateLocalDeviceList();
    }
// Fluidic mode
	RomIDstr[devFluidicMode] = ip2Hex(moduleipaddress) + familyMultiGestFluidicMode + familyMultiGestMode;
	device = parent->configwin->NewDevice(RomIDstr[devFluidicMode], this);
	if (device)
	{
		devices[devFluidicMode] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
// Solar mode
	RomIDstr[devSolarMode] = ip2Hex(moduleipaddress) + familyMultiGestSolarMode + familyMultiGestMode;
	device = parent->configwin->NewDevice(RomIDstr[devSolarMode], this);
	if (device)
	{
		devices[devSolarMode] = device;
		if (!localdevice.contains(device)) localdevice.append(device);
		UpdateLocalDeviceList();
	}
	UpdateLocalDeviceList();
}




void ecogest::SetDone(const QString &data)
{
    if (data.endsWith("SWAON")) { if (devices[devSwitchA]) devices[devSwitchA]->setscratchpad("1", true); }
    else if (data.endsWith("SWAOFF")) { if (devices[devSwitchA]) devices[devSwitchA]->setscratchpad("0", true); }
    else if (data.endsWith("SWBON")) { if (devices[devSwitchB]) devices[devSwitchB]->setscratchpad("1", true); }
    else if (data.endsWith("SWBOFF")) { if (devices[devSwitchB]) devices[devSwitchB]->setscratchpad("0", true); }
    else if (data.endsWith("SWCON")) { if (devices[devSwitchC]) devices[devSwitchC]->setscratchpad("1", true); }
    else if (data.endsWith("SWCOFF")) { if (devices[devSwitchC]) devices[devSwitchC]->setscratchpad("0", true); }
    else if (data.endsWith("SWDON")) { if (devices[devSwitchD]) devices[devSwitchD]->setscratchpad("1", true); }
    else if (data.endsWith("SWDOFF")) { if (devices[devSwitchD]) devices[devSwitchD]->setscratchpad("0", true); }
    else if (data.endsWith("SWEON")) { if (devices[devSwitchE]) devices[devSwitchE]->setscratchpad("1", true); }
    else if (data.endsWith("SWEOFF")) { if (devices[devSwitchE]) devices[devSwitchE]->setscratchpad("0", true); }
    else if (data.endsWith("SWFON")) { if (devices[devSwitchF]) devices[devSwitchF]->setscratchpad("1", true); }
    else if (data.endsWith("SWFOFF")) { if (devices[devSwitchF]) devices[devSwitchF]->setscratchpad("0", true); }
    else if (data.endsWith("SWNON")) { if (devices[devSwitchN]) devices[devSwitchN]->setscratchpad("1", true); }
    else if (data.endsWith("SWNOFF")) { if (devices[devSwitchN]) devices[devSwitchN]->setscratchpad("0", true); }
}


void ecogest::SetStatusValues(const QString &data, bool enregistremode)
{
	bool ok;
	QString V;
	if (devices[HeaterIn]) uiw.pushButtonT1->setText(buttonstext[HeaterIn] +  " = " + devices[HeaterIn]->MainValueToStr()); else uiw.pushButtonT1->setText(buttonstext[HeaterIn]);
	if (devices[HeaterOut]) uiw.pushButtonT2->setText(buttonstext[HeaterOut] +  " = " + devices[HeaterOut]->MainValueToStr()); else uiw.pushButtonT2->setText(buttonstext[HeaterOut]);
	if (devices[HeatingOut]) uiw.pushButtonT3->setText(buttonstext[HeatingOut] +  " = " + devices[HeatingOut]->MainValueToStr()); else uiw.pushButtonT3->setText(buttonstext[HeatingOut]);
	if (devices[SolarIn]) uiw.pushButtonT4->setText(buttonstext[SolarIn] +  " = " + devices[SolarIn]->MainValueToStr()); else uiw.pushButtonT4->setText(buttonstext[SolarIn]);
	if (devices[SolarOut]) uiw.pushButtonT5->setText(buttonstext[SolarOut] +  " = " + devices[SolarOut]->MainValueToStr()); else uiw.pushButtonT5->setText(buttonstext[SolarOut]);
	if (devices[TankHigh]) uiw.pushButtonT6->setText(buttonstext[TankHigh] +  " = " + devices[TankHigh]->MainValueToStr()); else uiw.pushButtonT6->setText(buttonstext[TankHigh]);
	if (devices[TankLow]) uiw.pushButtonT7->setText(buttonstext[TankLow] +  " = " + devices[TankLow]->MainValueToStr()); else uiw.pushButtonT7->setText(buttonstext[TankLow]);
// FlowA
	V = logisdom::getvalue("MeanFlowA", data);
	if (!V.isEmpty())
		if (devices[devFlowA])
		{
			devices[devFlowA]->setscratchpad(V, enregistremode);
			QString MFlow = logisdom::getvalue("FLA", data);
			QString str = buttonFlowAText + " : " + V;
			if (!MFlow.isEmpty()) str += "   (Counter " + MFlow + ")";
			uiw.pushButtonFlowA->setText(str);
		}
// FlowB
	V = logisdom::getvalue("MeanFlowB", data);
	if (!V.isEmpty())
		if (devices[devFlowB])
		{
			devices[devFlowB]->setscratchpad(V, enregistremode);
			QString MFlow = logisdom::getvalue("FLB", data);
			QString str = buttonFlowBText + " : " + V;
			if (!MFlow.isEmpty()) str += "   (Counter " + MFlow + ")";
			uiw.pushButtonFlowB->setText(str);
		}
// FlowC
	V = logisdom::getvalue("MeanFlowC", data);
	if (!V.isEmpty())
		if (devices[devFlowC])
		{
			devices[devFlowC]->setscratchpad(V, enregistremode);
			QString MFlow = logisdom::getvalue("FLC", data);
			QString str = buttonFlowCText + " : " + V;
			if (!MFlow.isEmpty()) str += "   (Counter " + MFlow + ")";
			uiw.pushButtonFlowC->setText(str);
		}
// Interlock
	V = logisdom::getvalue("Interlock", data);
        if (devices[devInterlock])
        {
            devices[devInterlock]->setscratchpad(V, enregistremode);
            //QString str = buttonInterlockText + " : " + V;
        }
// Switch A / Heater Pump
	V = logisdom::getvalue("SWA", data);
	if (!V.isEmpty())
		if (devices[devSwitchA])
		{
			devices[devSwitchA]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchA->setText(buttonSwitchAText + " : " + devices[devSwitchA]->MainValueToStr());
		}
// Switch B / Solar Pump
	V = logisdom::getvalue("SWB", data);
	if (!V.isEmpty())
		if (devices[devSwitchB])
		{
			devices[devSwitchB]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchB->setText(buttonSwitchBText + " : " + devices[devSwitchB]->MainValueToStr());
		}
// Switch C / Circulator
	V = logisdom::getvalue("SWC", data);
	if (!V.isEmpty())
		if (devices[devSwitchC])
		{
			devices[devSwitchC]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchC->setText(buttonSwitchCText + " : " + devices[devSwitchC]->MainValueToStr());
		}
// Switch D / Fill
	V = logisdom::getvalue("SWD", data);
	if (!V.isEmpty())
		if (devices[devSwitchD])
		{
			devices[devSwitchD]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchD->setText(buttonSwitchDText + " : " + devices[devSwitchD]->MainValueToStr());
		}
// Switch E / KeepFilled
	V = logisdom::getvalue("SWE", data);
	if (!V.isEmpty())
		if (devices[devSwitchE])
		{
			devices[devSwitchE]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchE->setText(buttonSwitchEText + " : " + devices[devSwitchE]->MainValueToStr());
		}
// Switch F
	V = logisdom::getvalue("SWF", data);
	if (!V.isEmpty())
		if (devices[devSwitchF])
		{
			devices[devSwitchF]->setscratchpad(V, enregistremode);
			uiw.pushButtonSwitchF->setText(buttonSwitchFText + " : " + devices[devSwitchF]->MainValueToStr());
		}
// Switch N
    V = logisdom::getvalue("SWN", data);
    if (!V.isEmpty())
        if (devices[devSwitchN])
        {
            devices[devSwitchN]->setscratchpad(V, enregistremode);
            // ici uiw.pushButtonSwitchN->setText(buttonSwitchFText + " : " + devices[devSwitchN]->MainValueToStr());
        }
// Servo1
	V = logisdom::getvalue("S1Val", data);
	if (!V.isEmpty())
		if (devices[devServo1])
		{
			devices[devServo1]->setscratchpad(V, enregistremode);
            disconnect(uiw.spinBoxServoA, SIGNAL(valueChanged(int)), this, SLOT(ServoAValueChanged(int)));
            uiw.spinBoxServoA->setValue(int(devices[devServo1]->getMainValue()));
			connect(uiw.spinBoxServoA, SIGNAL(valueChanged(int)), this, SLOT(ServoAValueChanged(int)));
		}
// Servo 2
	V = logisdom::getvalue("S2Val", data);
	if (!V.isEmpty())
		if (devices[devServo2])
		{
			devices[devServo2]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoB, SIGNAL(valueChanged(int)), this, SLOT(ServoBValueChanged(int)));
            uiw.spinBoxServoB->setValue(int(devices[devServo2]->getMainValue()));
			connect(uiw.spinBoxServoB, SIGNAL(valueChanged(int)), this, SLOT(ServoBValueChanged(int)));
		}
// Servo 3
	V = logisdom::getvalue("S3Val", data);
	if (!V.isEmpty())
        if (devices[devServo3])
		{
			devices[devServo3]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoC, SIGNAL(valueChanged(int)), this, SLOT(ServoCValueChanged(int)));
            uiw.spinBoxServoC->setValue(int(devices[devServo3]->getMainValue()));
			connect(uiw.spinBoxServoC, SIGNAL(valueChanged(int)), this, SLOT(ServoCValueChanged(int)));
		}
// Servo 4
	V = logisdom::getvalue("S4Val", data);
	if (!V.isEmpty())
		if (devices[devServo4])
		{
			devices[devServo4]->setscratchpad(V, enregistremode);
			disconnect(uiw.spinBoxServoD, SIGNAL(valueChanged(int)), this, SLOT(ServoDValueChanged(int)));
            uiw.spinBoxServoD->setValue(int(devices[devServo4]->getMainValue()));
			connect(uiw.spinBoxServoD, SIGNAL(valueChanged(int)), this, SLOT(ServoDValueChanged(int)));
		}
// Servo 5
		V = logisdom::getvalue("S5Val", data);
		if (!V.isEmpty())
			if (devices[devServo5])
			{
				devices[devServo5]->setscratchpad(V, enregistremode);
				disconnect(uiw.spinBoxServoE, SIGNAL(valueChanged(int)), this, SLOT(ServoEValueChanged(int)));
                uiw.spinBoxServoE->setValue(int(devices[devServo5]->getMainValue()));
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
// Solar FillDelayVar
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
// HTM
	int HTM = logisdom::getvalue("HTM", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxHTM, SIGNAL(valueChanged(int)), this, SLOT(HTMValueChanged(int)));
		uiw.spinBoxHTM->setValue(HTM);
		connect(uiw.spinBoxHTM, SIGNAL(valueChanged(int)), this, SLOT(HTMValueChanged(int)));
	}
// SOT
	int SOT = logisdom::getvalue("SOT", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.spinBoxSOT, SIGNAL(valueChanged(int)), this, SLOT(SOTValueChanged(int)));
		uiw.spinBoxSOT->setValue(SOT);
		connect(uiw.spinBoxSOT, SIGNAL(valueChanged(int)), this, SLOT(SOTValueChanged(int)));
	}
// SLO
	int SLO = logisdom::getvalue("SLO", data).toInt(&ok);
	if (ok)
	{
		disconnect(uiw.checkBoxServoSolo, SIGNAL(stateChanged(int)), this, SLOT(SOLOStateChanged(int)));
		if (SLO) uiw.checkBoxServoSolo->setChecked(true);
		else uiw.checkBoxServoSolo->setChecked(false);
		connect(uiw.checkBoxServoSolo, SIGNAL(stateChanged(int)), this, SLOT(SOLOStateChanged(int)));
	}
//	GenMsg(str);
}






void ecogest::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
		{
		 QMenu contextualmenu;
		 QAction Search(tr("&Search"), this);
         QAction SearchRst(tr("&SearchReset"), this);
		 QAction ClearRomIDEEprom(tr("&Clear EEprom RomID"), this);
		 QAction Upload(tr("&Upload"), this);
		 QAction GetSP(tr("&Get ScratchPads"), this);
		 QAction RStart(tr("&Restart"), this);
		 QAction GetST(tr("&Get Status"), this);
		 QAction SetNames(tr("&Set Defaul Names"), this);
		 contextualmenu.addAction(&Search);
         contextualmenu.addAction(&SearchRst);
		 contextualmenu.addAction(&RStart);
		 contextualmenu.addAction(&ClearRomIDEEprom);
		 contextualmenu.addAction(&Upload);
		 contextualmenu.addAction(&GetSP);
		 contextualmenu.addAction(&GetST);
		 contextualmenu.addAction(&SetNames);
		 QAction *selection;
		 selection = contextualmenu.exec(event->globalPos());
		 if (selection == &ClearRomIDEEprom) clearRomIDEEprom();
		 if (selection == &RStart) restart();
		 if (selection == &Upload) OpenUpload();
		 if (selection == &GetSP) addtofifo(GetScratchPads);
		 if (selection == &GetST) addtofifo(GetStatus);
         //if (selection == &Search) addtofifo(GlobalSearch);
         if (selection == &Search) TcpThread.addToFIFO(LocalSearch);
         if (selection == &SearchRst) addtofifo(LocalSearch);
         if (selection == &SetNames) setDefaultNames();
		}
	if (event->button() != Qt::LeftButton) return;
}




void ecogest::restart()
{
	addtofifo(Restart);
}



void ecogest::setSwitchA()
{
	if (devices[devSwitchA]) {
        if (int(devices[devSwitchA]->getMainValue()) != 0) addtofifo("SWAOFF"); else addtofifo("SWAON"); }
}


void ecogest::setSwitchB()
{
	if (devices[devSwitchB]) {
        if (int(devices[devSwitchB]->getMainValue()) != 0) addtofifo("SWBOFF"); else addtofifo("SWBON");  }
}


void ecogest::setSwitchC()
{
	if (devices[devSwitchC]) {
        if (int(devices[devSwitchC]->getMainValue()) != 0) addtofifo("SWCOFF"); else addtofifo("SWCON"); }
}



void ecogest::setSwitchD()
{
	if (devices[devSwitchD]) {
        if (int(devices[devSwitchD]->getMainValue()) != 0) addtofifo("SWDOFF"); else addtofifo("SWDON"); }
}



void ecogest::setSwitchE()
{
	if (devices[devSwitchE]) {
        if (int(devices[devSwitchE]->getMainValue()) != 0) addtofifo("SWEOFF"); else addtofifo("SWEON"); }
}



void ecogest::setSwitchF()
{
	if (devices[devSwitchF]) {
        if (int(devices[devSwitchF]->getMainValue()) != 0) addtofifo("SWFOFF"); else addtofifo("SWFON"); }
}


void ecogest::setSwitchN()
{
    if (devices[devSwitchN]) {
        if (int(devices[devSwitchN]->getMainValue()) != 0) addtofifo("SWNOFF"); else addtofifo("SWNON"); }
}






void ecogest::foundDevice(onewiredevice *device)
{
	if (!localdevice.contains(device)) localdevice.append(device);
	RomIDs.append(device->getromid());
}





