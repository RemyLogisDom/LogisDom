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





#include <QtGui>
#include "globalvar.h"
#include "configwindow.h"
#include "addProgram.h"
#include "commonstring.h"
#include "logisdom.h"
#include "onewire.h"
#include "programevent.h"
#include "daily.h"
#include "formula.h"
#include "addDaily.h"
#include "devchooser.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "htmlbinder.h"
#include "chauffageunit.h"


ChauffageUnit::ChauffageUnit(logisdom *Parent, QString name)
{
	parent = Parent;
	htmlBind = new htmlBinder(parent, name, parent->ChauffageArea->htmlBind->treeItem);
    RemoteDisplay = new devchooser(parent);
    VanneDevice = new devchooser(parent);
    Integral = 0;
	I_Counter = 0;
	Result = 0;
    master = false;
	mode = modeAuto;
	htmlBind->setParameter("Mode", cstr::toStr(cstr::Auto));
	setMode(modeAuto);
	htmlBind->setParameter("Mode Solaire", cstr::toStr(cstr::OFF));
    int i = 0;
	valveModeTxt = tr("Valve mode");
	targetModeTxt = tr("Target mode");
	autoTxt = tr("Auto");
	manualTxt = tr("Manual");
	targetTxt = tr("Target");
	valveTxt = tr("Valve");
    remoteDisTxt = tr("Remote");
    htmlBind->addParameterCommand(targetModeTxt, autoTxt, modeTargetAuto);
	htmlBind->addParameterCommand(targetModeTxt, manualTxt, modeTargetManual);
	htmlBind->addParameterCommand(valveModeTxt, autoTxt, modeValveAuto);
	htmlBind->addParameterCommand(valveModeTxt, manualTxt, modeValveManual);
	htmlBind->addParameterCommand(targetTxt, "-", targetMinus);
	htmlBind->addParameterCommand(targetTxt, "+", targetPlus);
	htmlBind->addParameterCommand(valveTxt, "-", valveMinus);
	htmlBind->addParameterCommand(valveTxt, "+", valvePlus);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::Auto), setmodeAuto);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), setmodeOn);
	htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), setmodeOff);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::Confort), setmodeConfort);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::Nuit), setmodeNuit);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::Eco), setmodeEco);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::Manuel), setmodeManuel);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::HorsGel), setmodeHorsGel);
    htmlBind->addParameterCommand("Mode Solaire", cstr::toStr(cstr::ON), setmodeSolaireOn);
	htmlBind->addParameterCommand("Mode Solaire", cstr::toStr(cstr::OFF), setmodeSolaireOff);
	connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));
	connect(htmlBind, SIGNAL(sendConfigStr(QString)), this, SLOT(applyConfigStr(QString)));

	int line = 0;
	setLayout(&displayLayout);
	Name = new QPushButton(this);
	Name->setMinimumSize(ProgMinSize, minH);
	Name->setText(name);

    ActualDevValue.setText("...");

    Consigne.setMinimumSize(40, minH);
    Consigne.setDecimals(1);
    Consigne.setValue(19);
    Consigne.setSingleStep(0.5);

    Status.setMinimumSize(20, minH - 5);
    Status.setSuffix("%");
    Status.setRange(-1, 100);
    Status.setSingleStep(1);
    Status.setSpecialValueText(cstr::toStr(cstr::NA));
	
    vanneManual.setMinimumSize(20, minH - 5);
    vanneManual.setText(manualTxt);

	solarEnable.setText(tr("Solar"));

	displayLayout.addWidget(Name, line, i++);
    displayLayout.addWidget(&ActualDevValue, line, i++);
    displayLayout.addWidget(&Consigne, line, i++);
    displayLayout.addWidget(&Status, line, i++);
    displayLayout.addWidget(&vanneManual, line, i++);
	displayLayout.addWidget(&solarEnable, line, i++);
    ModeList.addItem(modeToStr(modeAuto));
    ModeList.addItem(modeToStr(modeOn));
    ModeList.addItem(modeToStr(modeOff));
    ModeList.addItem(modeToStr(modeConfort));
    ModeList.addItem(modeToStr(modeNuit));
    ModeList.addItem(modeToStr(modeEco));
    ModeList.addItem(modeToStr(modeManuel));
    ModeList.addItem(modeToStr(modeHorsGel));
    ModeList.setCurrentIndex(0);
    displayLayout.addWidget(&ModeList, line, i++);

	Formula = new formula(parent);
	Formula->setName(name);
	Formula->removeButtons();
    Formula->calcth->setLinkedEnabled();
	i = 0;
// Palette setup
	setup.setLayout(&setupLayout);
	RenameButton.setText(name);
	setupLayout.addWidget(&RenameButton, i, 0, 1, 1);
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(changename()));
	setupLayout.addWidget(&CalculButton, i++, 1, 1, 1);
    connect(&CalculButton, SIGNAL(clicked()), parent->ChauffageArea, SLOT(process()));
	labelVanne.setText(valveTxt);
	setupLayout.addWidget(&labelVanne, i, 0, 1, 1);
	VanneDevice->addFamily(familySTA800);
	VanneDevice->addFamily(familyMultiGestValve);
    VanneDevice->addFamily(familyeoA52001);
    //VanneDevice->addFamily(familyEOcean);
    setupLayout.addWidget(VanneDevice, i++, 1, 1, 1);
    labelRemoteDisplay.setText(remoteDisTxt);
    setupLayout.addWidget(&labelRemoteDisplay, i, 0, 1, 1);
    RemoteDisplay->addFamily(familyLCDOW_A);
    setupLayout.addWidget(RemoteDisplay, i++, 1, 1, 1);
    labelProgram.setText("Program");
	setupLayout.addWidget(&labelProgram, i, 0, 1, 1);
	setupLayout.addWidget(&Program, i++, 1, 1, 1);
	TConfort.setDecimals(1);
	TConfort.setValue(19);
	TConfort.setSingleStep(0.5);
	TConfort.setPrefix(tr("Confort : "));
	TConfort.setSuffix(" °C");
	setupLayout.addWidget(&TConfort, i, 0, 1, 1);
	TEco.setDecimals(1);
	TEco.setValue(16);
	TEco.setSingleStep(0.5);
	TEco.setPrefix(tr("Eco : "));
	TEco.setSuffix(" °C");
    setupLayout.addWidget(&TEco, i++, 1, 1, 1);
    TNuit.setDecimals(1);
    TNuit.setValue(18);
    TNuit.setSingleStep(0.5);
    TNuit.setPrefix(tr("Nuit : "));
    TNuit.setSuffix(" °C");
    setupLayout.addWidget(&TNuit, i++, 0, 1, 1);
	minEnabled.setText(tr("Min"));
	minEnabled.setChecked(false);
	setupLayout.addWidget(&minEnabled, i, 0, 1, 1);
	connect(&minEnabled, SIGNAL(stateChanged(int)), this, SLOT(MinEnChanged(int)));
	Vmin.setSuffix("%");
	Vmin.setRange(0, 100);
	Vmin.setValue(0);
	Vmin.setEnabled(false);
	setupLayout.addWidget(&Vmin, i++, 1, 1, 1);
	connect(&Vmin, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
	maxEnabled.setText(tr("Max"));
	maxEnabled.setChecked(false);
	setupLayout.addWidget(&maxEnabled, i, 0, 1, 1);
	connect(&maxEnabled, SIGNAL(stateChanged(int)), this, SLOT(MaxEnChanged(int)));
	Vmax.setSuffix("%");
	Vmax.setRange(0, 100);
	Vmax.setValue(100);
	Vmax.setEnabled(false);
	setupLayout.addWidget(&Vmax, i++, 1, 1, 1);
	connect(&Vmax, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
	passiveMode.setText(tr("Passive Mode"));
	setupLayout.addWidget(&passiveMode, i, 0, 1, 1);
	proportionalMinMax.setText(tr("Proportional"));
	proportionalMinMax.setEnabled(false);
	setupLayout.addWidget(&proportionalMinMax, i++, 1, 1, 1);
	setupLayout.addWidget(Formula, i++, 0, 1, 2);
	connect(Name, SIGNAL(clicked()), this, SLOT(emitSetupClick()));
	connect(this, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
	ComboIndex = line - 1;
	connectcombo();
	consigneManualChange(0);
	vanneManualChange(0);
    update.setSingleShot(true);
    update.setInterval(2000);
    connect(&update, SIGNAL(timeout()), this, SLOT(process()));
    connectcombo();
}
	




ChauffageUnit::~ChauffageUnit()
{
	delete Name;
	delete Formula;
}






bool ChauffageUnit::solarEnabled()
{
	return solarEnable.isChecked();
}





QString ChauffageUnit::modeToStr(int Mode)
{
	switch (Mode)
	{
        case modeAuto : return cstr::toStr(cstr::Auto);
        case modeOn : return cstr::toStr(cstr::ON);
        case modeOff : return cstr::toStr(cstr::OFF);
        case modeConfort : return cstr::toStr(cstr::Confort);
        case modeNuit : return cstr::toStr(cstr::Nuit);
        case modeEco : return cstr::toStr(cstr::Eco);
        case modeManuel : return cstr::toStr(cstr::Manuel);
        case modeHorsGel : return cstr::toStr(cstr::HorsGel);
	}
	return cstr::toStr(cstr::Auto);
}





void ChauffageUnit::clickMode(int Mode)
{
    master = true;
    setMode(Mode);
}



void ChauffageUnit::setMode(int Mode, bool update)
{
	switch (Mode)
	{
		case modeAuto : mode = modeAuto;
			htmlBind->setParameter("Mode", cstr::toStr(cstr::Auto));
            Consigne.setEnabled(false);
            break;
		case modeOn :
			htmlBind->setParameter("Mode", cstr::toStr(cstr::ON));
            Status.setValue(100);
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
			mode = modeOn;
            Consigne.setEnabled(false);
			break;
		case modeOff : mode = modeOff;
			htmlBind->setParameter("Mode", cstr::toStr(cstr::OFF));
            Status.setValue(0);
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(false);
			break;
		case modeConfort : mode = modeConfort;
			htmlBind->setParameter("Mode", cstr::toStr(cstr::Confort));
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(false);
            Consigne.setValue(TConfort.value());
			break;
        case modeNuit : mode = modeNuit;
            htmlBind->setParameter("Mode", cstr::toStr(cstr::Nuit));
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(false);
            Consigne.setValue(TNuit.value());
            break;
        case modeEco : mode = modeEco;
			htmlBind->setParameter("Mode", cstr::toStr(cstr::Eco));
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(false);
            Consigne.setValue(TEco.value());
			break;
        case modeManuel : mode = modeManuel;
            htmlBind->setParameter("Mode", cstr::toStr(cstr::Manuel));
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(true);
            break;
        case modeHorsGel : mode = modeHorsGel;
			htmlBind->setParameter("Mode", cstr::toStr(cstr::HorsGel));
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
            Consigne.setEnabled(false);
            Consigne.setValue(7.5);
			break;
		default : mode = modeAuto; 
			htmlBind->setParameter("Mode", cstr::toStr(cstr::Auto));
            Consigne.setEnabled(false);
            break;
	}
    if (update) startupdate();
}




void ChauffageUnit::applyConfigStr(QString command)
{
	bool ok;
// Valve Mode
	if (command.contains(modeValveCommand))
	{
        //disconnect(vanneManual, SIGNAL(stateChanged(int)), this, SLOT(vanneManualChange(int)));
		int value = logisdom::getvalue(modeValveCommand, command).toInt(&ok);
		if (ok && value)
		{
            Status.setEnabled(true);
            vanneManual.setCheckState(Qt::Checked);
		}
		else
		{
            Status.setEnabled(false);
            vanneManual.setCheckState(Qt::Unchecked);
		}
        //connect(vanneManual, SIGNAL(stateChanged(int)), this, SLOT(vanneManualChange(int)));
	}
// Valve Command
	if (command.contains(setValveCommand))
	{
        //disconnect(Status, SIGNAL(valueChanged(int)), this, SLOT(vanneChange(int)));
		int value = logisdom::getvalue(setValveCommand, command).toInt(&ok);
        if (ok) Status.setValue(value);
        //connect(Status, SIGNAL(valueChanged(int)), this, SLOT(vanneChange(int)));
	}
// Consigne Mode
	if (command.contains(modeTargetCommand))
	{
        //disconnect(consigneManual, SIGNAL(stateChanged(int)), this, SLOT(consigneManualChange(int)));
		int value = logisdom::getvalue(modeTargetCommand, command).toInt(&ok);
		if (ok && value)
		{
            Consigne.setEnabled(true);
		}
		else
		{
            Consigne.setEnabled(false);
		}
        //connect(consigneManual, SIGNAL(stateChanged(int)), this, SLOT(consigneManualChange(int)));
	}
// Consigne Command
	if (command.contains(setTargetCommand))
	{
        //disconnect(Consigne, SIGNAL(valueChanged(double)), this, SLOT(consigneChange(double)));
		double value = logisdom::getvalue(setTargetCommand, command).toDouble(&ok);
        if (ok) Consigne.setValue(value);
        //connect(Consigne, SIGNAL(valueChanged(double)), this, SLOT(consigneChange(double)));
	}
}





void ChauffageUnit::remoteCommand(const QString &command)
{
//	GenMsg("Remote Command = " + command);
    if (command == modeValveAuto) vanneManual.setCheckState(Qt::Unchecked);
    if (command == modeValveManual) vanneManual.setCheckState(Qt::Checked);
    double v = Consigne.value();
    if (command == targetPlus) { ModeList.setCurrentIndex(modeManuel); Consigne.setValue(v + 0.5); }
    if (command == targetMinus){ ModeList.setCurrentIndex(modeManuel); Consigne.setValue(v - 0.5); }
    int s = Status.value();
	if (((command == valvePlus) or (command == valveMinus)) && (s < 0)) s = 0;
    if ((command == valvePlus) && (vanneManual.isChecked())) Status.setValue(s + 10);
    if ((command == valveMinus) && (vanneManual.isChecked())) Status.setValue(s - 10);
	if (command == setmodeAuto) ModeList.setCurrentIndex(modeAuto);// setMode(modeAuto);
	if (command == setmodeOn) ModeList.setCurrentIndex(modeOn);// setMode(modeOn);
	if (command == setmodeOff) ModeList.setCurrentIndex(modeOff);// setMode(modeOff);
	if (command == setmodeConfort) ModeList.setCurrentIndex(modeConfort);// setMode(modeConfort);
    if (command == setmodeNuit) ModeList.setCurrentIndex(modeNuit);// setMode(modeNuit);
    if (command == setmodeEco) ModeList.setCurrentIndex(modeEco);// setMode(modeEco);
	if (command == setmodeHorsGel) ModeList.setCurrentIndex(modeHorsGel);// setMode(modeHorsGel);
	if (command == setmodeSolaireOn) solarEnable.setChecked(true);
	if (command == setmodeSolaireOff) solarEnable.setChecked(false);
    if (command == setmodeSolaireToggle)
    {
        if (solarEnable.isChecked()) solarEnable.setChecked(false);
        else solarEnable.setChecked(true);
    }
}




void ChauffageUnit::changename()
{
	bool ok;
//retry:
    QString name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, RenameButton.text(), &ok, maison1wirewindow);
	if ((ok) and !name.isEmpty())
	{
		/*if (parent->configwin->devicenameexist(Name) and (Name != name))
		{
			messagebox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto retry;
		}*/
		RenameButton.setText(name);
		htmlBind->setName(name);
		Name->setText(name);
		//emit(DeviceConfigChanged(this));
	}	
}




void ChauffageUnit::emitSetupClick()
{
	emit(setupClick(&setup));
}



void ChauffageUnit::consigneChange(double value)
{
	QString V = QString("%1").arg(value);
    QString C = QString("%1 °C").arg(value, 0, 'f', 1);
    htmlBind->setParameter(targetTxt, C);
	htmlBind->sendParameter(setTargetCommand, V);
    master = true;
    update.start();
}




void ChauffageUnit::startupdate()
{
    update.start();
}


void ChauffageUnit::consigneManualChange(int state)
{
	if (state)
	{
        Consigne.setEnabled(true);
		htmlBind->setParameter(targetModeTxt, manualTxt);
		htmlBind->sendParameter(modeTargetCommand, "1");
	}
	else 
	{
        Consigne.setEnabled(false);
		process();
		htmlBind->setParameter(targetModeTxt, autoTxt);
		htmlBind->sendParameter(modeTargetCommand, "0");
	}
    master = true;
    update.start();
}




void ChauffageUnit::vanneChange(int value)
{
	QString V = QString("%1").arg(value);
	htmlBind->setParameter(valveTxt, V + "%");
	htmlBind->sendParameter(setValveCommand, V);
    master = true;
}




void ChauffageUnit::vanneManualChange(int state)
{
	if (state)
	{
        Status.setEnabled(true);
		htmlBind->setParameter(valveModeTxt, manualTxt);
		htmlBind->sendParameter(modeValveCommand, "1");
	}
	else
	{
        Status.setEnabled(false);
		process();
		htmlBind->setParameter(valveModeTxt, autoTxt);
		htmlBind->sendParameter(modeValveCommand, "0");
	}
}




void ChauffageUnit::MinEnChanged(int state)
{
	if (state)
	{
		Vmin.setEnabled(true);
	}
	else
	{
		Vmin.setEnabled(false);
	}
	if ((Vmin.isEnabled()) or Vmax.isEnabled()) proportionalMinMax.setEnabled(true);
	else proportionalMinMax.setEnabled(false);
	if ((Vmin.value() > 0) && minEnabled.isChecked())
	{
		passiveMode.setChecked(true);
		passiveMode.setEnabled(false);
	}
	else
	{
		passiveMode.setChecked(false);
		passiveMode.setEnabled(true);
	}
}




void ChauffageUnit::MaxEnChanged(int state)
{
	if (state)
	{
		Vmax.setEnabled(true);
	}
	else
	{
		Vmax.setEnabled(false);
	}
	if ((Vmin.isEnabled()) or Vmax.isEnabled()) proportionalMinMax.setEnabled(true);
	else proportionalMinMax.setEnabled(false);
}




void ChauffageUnit::minChanged(int value)
{
	Vmax.setMinimum(value);
	if ((Vmin.value() > 0) && minEnabled.isChecked())
	{
		passiveMode.setChecked(true);
		passiveMode.setEnabled(false);
	}
	else
	{
		passiveMode.setChecked(false);
		passiveMode.setEnabled(true);
	}
}




void ChauffageUnit::maxChanged(int value)
{
	Vmin.setMaximum(value);
}




void ChauffageUnit::solarChanged(int)
{
    if (solarEnable.isChecked()) htmlBind->setParameter("Mode Solaire", cstr::toStr(cstr::ON));
    else htmlBind->setParameter("Mode Solaire", cstr::toStr(cstr::OFF));
}





void ChauffageUnit::connectcombo()
{
    connect(&Consigne, SIGNAL(valueChanged(double)), this, SLOT(consigneChange(double)));
    connect(&ModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(clickMode(int)));
    connect(&vanneManual, SIGNAL(stateChanged(int)), this, SLOT(vanneManualChange(int)));
    connect(&solarEnable, SIGNAL(stateChanged(int)), this, SLOT(solarChanged(int)));
}





void ChauffageUnit::disconnectcombo()
{
    disconnect(&Consigne, SIGNAL(valueChanged(double)), this, SLOT(consigneChange(double)));
    disconnect(&ModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(clickMode(int)));
    disconnect(&vanneManual, SIGNAL(stateChanged(int)), this, SLOT(vanneManualChange(int)));
    disconnect(&solarEnable, SIGNAL(stateChanged(int)), this, SLOT(solarChanged(int)));
}



void ChauffageUnit::setValue(int value)
{
    int status = -1;
	if (passiveMode.isChecked())
	{
        if (logisdom::isNotNA(Result)) status = Result;
	}
	else
	{
        if ((value == -1) && (logisdom::isNotNA(Result))) status = Result;
		if (value == 0) status = 0;
		if (value == 100) status = 100;
        if ((value > 100) && (logisdom::isNotNA(Result))) status = Result * value / 100;
		if (minEnabled.isChecked() && (value < Vmin.value())) status = Vmin.value();
		if (maxEnabled.isChecked() && (value > Vmax.value())) status = Vmax.value();
	}
    double result = logisdom::NA;
    if (vanneManual.isChecked())
	{
        if (Status.value() >= 0) result = Status.value();
	}
    else if (status >= 0)
	{
		if (status > 100) status = 100;
        result = status;
	}
    if (parent->isRemoteMode()) return;
    if (int(result) == logisdom::NA) return;
    if (int(result) < 0) result = 0;
    if (int(result) > 100) result = 100;
    if (ModeList.currentIndex() != modeManuel) Status.setValue(Result);
    QString RomID = RemoteDisplay->getRomID();
    onewiredevice *deviceD = parent->configwin->DeviceExist(RomID.left(17) + "D");
    if (deviceD)
        if (int(deviceD->getMainValue()) != int(result)) deviceD->assignMainValue(result);
    onewiredevice *device = VanneDevice->device();
    if (device) device->assignMainValue(int(result));
    Status.setValue(int(result));
    QString V = QString("%1%").arg(result);
    htmlBind->setParameter(valveTxt, V);
    htmlBind->setParameter(valveTxt, V);
    CalculButton.setText(V);
}




double ChauffageUnit::checkRemoteTarget(double Target)
{
    //parent->GenMsg(QString("checkRemoteTarget = %1").arg(Target));
    //onewiredevice *deviceA = RemoteDisplay->device();
    //QString RomID = RemoteDisplay->getRomID();
    //onewiredevice *deviceC = parent->configwin->DeviceExist(RomID.left(17) + "C");
    //if (!deviceA) return Target;
    //if (!deviceC) return Target;
    double remoteTarget = Target;
    //int mode = deviceC->getMainValue();
    //if (logisdom::isNotNA(mode) && (mode != ModeList.currentIndex()))
    //{
    //    disconnect(&ModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(modeClick(int)));
    //    ModeList.setCurrentIndex(mode);
    //    setMode(mode);
    //    connect(&ModeList, SIGNAL(currentIndexChanged(int)), this, SLOT(modeClick(int)));
    //}
    //else    // LogisDom drives remote
    //{
    //    if (Target == logisdom::NA) return Target;
     //   if (deviceA->getMainValue() != Target)
     //   {
      //      deviceA->assignMainValue(Target);
            //parent->GenMsg(QString("remoteValueAssigned = %1   /   assignMainValue %2").arg(remoteValueAssigned).arg(Target));
      //  }
    //}
    return remoteTarget;
}



int ChauffageUnit::process()
{
	if (parent->isRemoteMode()) return logisdom::NA;
// Get target Temp
	double Target = logisdom::NA;
	int indexNow = parent->AddProgwin->getActualProgram(Program.currentIndex());
	int indexPrevious = parent->AddProgwin->getPreviousProgram(Program.currentIndex());
//	GenMsg(QString("indexNow = %1").arg(indexNow));
//	GenMsg(QString("indexPrevious = %1").arg(indexPrevious));
// Check remote status
    QString RomID = RemoteDisplay->getRomID();
    onewiredevice *deviceA = parent->configwin->DeviceExist(RomID.left(17) + "A");
    onewiredevice *deviceC = parent->configwin->DeviceExist(RomID.left(17) + "C");
    if (deviceA && deviceC)
    {
        int remotemode = int(deviceC->getMainValue());
        if (master)
        {
            if (logisdom::isNotNA(remotemode) && logisdom::AreNotSame(remotemode, ModeList.currentIndex()))
            {
                deviceC->assignMainValue(mode);
            }
        }
        else
        {
            if (logisdom::isNotNA(remotemode) && logisdom::AreNotSame(remotemode, ModeList.currentIndex()))
            {
                disconnectcombo();
                ModeList.setCurrentIndex(remotemode);
                setMode(remotemode, false);
                connectcombo();
            }
        }
        if (mode == modeManuel)
        {
            double v = deviceA->getMainValue();
            if (master)
            {
                if (logisdom::isNotNA(remotemode) && (remotemode != ModeList.currentIndex()))
                {
                    deviceC->assignMainValue(mode);
                }
                if (logisdom::isNotNA(v) && logisdom::AreNotSame(v, Consigne.value()))
                {
                    deviceA->assignMainValue(Consigne.value());
                }
            }
            else
            {
                if (logisdom::isNotNA(v) && logisdom::AreNotSame(v, Consigne.value()))
                {
                    disconnectcombo();
                    Consigne.setValue(v);
                    connectcombo();
                }
            }
        }
    }
// Process
    switch (mode)
    {
        case modeAuto :
                    if (parent->getProgEventArea()->isAutomatic())
                    {
                        Target = checkRemoteTarget(parent->AddDaily->getActualValue(indexNow, indexPrevious));
                    }
                    else if (parent->getProgEventArea()->isConfort())
                    {
                        Target = checkRemoteTarget(TConfort.value());
                    }
                    else if (parent->getProgEventArea()->isNuit())
                    {
                        Target = checkRemoteTarget(TNuit.value());
                    }
                    else if (parent->getProgEventArea()->isEco())
                    {
                        Target = checkRemoteTarget(TEco.value());
                    }
                    else if (parent->getProgEventArea()->isUnfreeze())
                    {
                        Target = checkRemoteTarget(7.5);
                    }
                    else
                    {
                        QString PrgName = parent->getProgEventArea()->getButtonName();
                        if (!PrgName.isEmpty())
                        {
                            QTime T = parent->getProgEventArea()->getButtonTime();
                            Target = checkRemoteTarget(parent->AddDaily->getValueAt(indexNow, indexPrevious, T));
                        }						}
                    Result = int(Formula->Calculate(Target)); break;
        case modeConfort : Target = checkRemoteTarget(TConfort.value()); Result = int(Formula->Calculate(Target)); break;
        case modeNuit : Target = checkRemoteTarget(TNuit.value()); Result = int(Formula->Calculate(Target)); break;
        case modeEco : Target = checkRemoteTarget(TEco.value()); Result = int(Formula->Calculate(Target)); break;
        case modeManuel :  Target = checkRemoteTarget(Consigne.value()); Result = int(Formula->Calculate(Target)); break;
        case modeHorsGel : Target = checkRemoteTarget(7.5); Result = int(Formula->Calculate(Target)); break;
        case modeOn : Result = 100; break;
        case modeOff : Result = 0; break;
    }

    if (vanneManual.isChecked()) Result = Status.value();
	if (Result == logisdom::NA) return 0;
	if (proportionalMinMax.isChecked())
	{
		int min = 0;
		if (minEnabled.isChecked()) min = Vmin.value();
		int max = 100;
		if (maxEnabled.isChecked()) max = Vmax.value();
		Result = (((max - min) * Result) / 100) + min;
	}
	else
	{
		if ((minEnabled.isChecked()) && Result <= Vmin.value()) Result = Vmin.value();
		if ((maxEnabled.isChecked()) && Result >= Vmax.value()) Result = Vmax.value();
	}
	QString C = QString("%1 °C").arg(Target, 0, 'f', 1);
    if (logisdom::isNA(Target))
	{
		htmlBind->setParameter(targetTxt, cstr::toStr(cstr::NA));
		htmlBind->setValue(cstr::toStr(cstr::NA));
        if (deviceA)
        {
            double v = deviceA->getMainValue();
            if (logisdom::isNotNA(v))
            {
                disconnectcombo();
                Consigne.setValue(v);
                connectcombo();
            }
        }
    }
	else
	{
		htmlBind->setParameter(targetTxt, C);
		htmlBind->setValue(C);
        disconnectcombo();
        Consigne.setValue(Target);
        connectcombo();
    }
    if (deviceA)
    {
        if (logisdom::AreNotSame(deviceA->getMainValue(), Target) && (logisdom::isNotNA(Target))) deviceA->assignMainValue(Target);
        if (Formula->calcth->deviceList.count() > 0) ActualDevValue.setText(Formula->calcth->deviceList.at(0)->MainValueToStr());
    }
    master = false;
	return Result;
}






void ChauffageUnit::AddProgram(QString Name)
{
		Program.addItem(Name);
}





void ChauffageUnit::RemoveProgram(QString Name)
{
	int indextoRemove = Program.findText(Name);
	if (indextoRemove  != -1) Program.setCurrentIndex(-1);
	Program.removeItem(indextoRemove);
}






ChauffageBlockHeader::ChauffageBlockHeader(QWidget*)
{
	gridLayout = new QGridLayout(this);
	gridLayout->setSpacing(1);
	//gridLayout->setMargin(9);
	gridLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

	setMinimumSize(100, 50);
	int i = 0;
	
	NameDesignation = new QLineEdit(this);
	NameDesignation->setMinimumSize(100, minH);
	NameDesignation->setAlignment(Qt::AlignHCenter);
	NameDesignation->setReadOnly(true);
	NameDesignation->setText(tr("Zone"));
	
	ConsigneDesignation = new QLineEdit(this);
	ConsigneDesignation->setMinimumSize(20, minH);
	ConsigneDesignation->setAlignment(Qt::AlignHCenter);
	ConsigneDesignation->setReadOnly(true);
	ConsigneDesignation->setText("Target");
	
	StatusDesignation = new QLineEdit(this);
	StatusDesignation->setMinimumSize(20, minH);
	StatusDesignation->setAlignment(Qt::AlignHCenter);
	StatusDesignation->setReadOnly(true);
	StatusDesignation->setText(tr("Manual"));
	
	ProgDesignation = new QLineEdit(this);
	ProgDesignation->setMinimumSize(20, minH);
	ProgDesignation->setAlignment(Qt::AlignHCenter);
	ProgDesignation->setReadOnly(true);
	ProgDesignation->setText(tr("Status"));
	
	VanneDesignation = new QLineEdit(this);
	VanneDesignation->setMinimumSize(20, minH);
	VanneDesignation->setAlignment(Qt::AlignHCenter);
	VanneDesignation->setReadOnly(true);
	VanneDesignation->setText("Manual");

	gridLayout->addWidget(NameDesignation, 0, i++);
	gridLayout->addWidget(ConsigneDesignation, 0, i++);
	gridLayout->addWidget(StatusDesignation, 0, i++);
	gridLayout->addWidget(ProgDesignation, 0, i++);
	gridLayout->addWidget(VanneDesignation, 0, i++);
}




ChauffageBlockHeader::~ChauffageBlockHeader()
{
	delete gridLayout;
	delete NameDesignation;
	delete VanneDesignation;
	delete StatusDesignation;
	delete ConsigneDesignation;
	delete ProgDesignation;
}





ChauffageScrollArea::ChauffageScrollArea(logisdom *Parent)
{
	parent = Parent;
	setLayout(&displayLayout);
    displayLayoutIndex = 1;
	htmlBind = new htmlBinder(parent, tr("Heating"));
	//connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));
// Setup palette
	int index = 0;

	QIcon chauffageIcon(QString::fromUtf8(":/images/images/cheminee.png"));
	ChauffageTool.setIcon(chauffageIcon);
	ChauffageTool.setIconSize(QSize(logisdom::iconSize, logisdom::iconSize));
	setupLayout.addWidget(&ChauffageTool, 0, index++, 1, 1);

	QIcon addIcon(QString::fromUtf8(":/images/images/edit_add.png"));
	AddButton.setIcon(addIcon);
	AddButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
	AddButton.setToolTip(tr("Add"));
	setupLayout.addWidget(&AddButton, 0, index++, 1, 1);

	QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
	RemoveButton.setIcon(removeIcon);
	RemoveButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
	RemoveButton.setToolTip(tr("Remove"));
	setupLayout.addWidget(&RemoveButton, 0, index++, 1, 1);

	labelCirculator.setText(tr("Circulator"));
	CirculatorDevice = new devchooser(parent);
	setup.setLayout(&setupLayout);
	CirculatorDevice->addFamily(familyMultiGestSwitch);
	CirculatorDevice->addFamily(familyAM12);
	CirculatorDevice->addFamily(family2413_A);
	CirculatorDevice->addFamily(family2413_B);
    CirculatorDevice->addFamily(family3A2100H_B);
    CirculatorDevice->addFamily(family3A2100H_B);
    CirculatorDevice->addFamily(familyVirtual);
	setupLayout.addWidget(&labelCirculator, index, 0, 1, 1);
	setupLayout.addWidget(CirculatorDevice, index, 1, 1, 1);
	setupLayout.addWidget(&labelCirculatorStatus, index++, 2, 1, 1);

	labelTankEnabler.setText(tr("Tank Enabler"));
	TankEnablerDevice = new devchooser(parent);
	TankEnablerThreshold.setRange(20, 60);
	TankEnablerThreshold.setValue(40);
	TankEnablerThreshold.setSuffix(" °C");
	setupLayout.addWidget(&labelTankEnabler, index, 0, 1, 1);
	setupLayout.addWidget(TankEnablerDevice, index, 1, 1, 1);
	setupLayout.addWidget(&TankEnablerThreshold, index++, 2, 1, 1);

	labelSolarEnabler.setText(tr("Solar Enabler"));
	SolarEnablerDevice = new devchooser(parent);
	SolarEnablerThreshold.setRange(20, 60);
	SolarEnablerThreshold.setValue(40);
	SolarEnablerThreshold.setSuffix(" °C");
	setupLayout.addWidget(&labelSolarEnabler, index, 0, 1, 1);
	setupLayout.addWidget(SolarEnablerDevice, index, 1, 1, 1);
	setupLayout.addWidget(&SolarEnablerThreshold, index++, 2, 1, 1);

	labelSolarDischarge.setText(tr("Solar Discharge"));
	SolarDischargeDevice = new devchooser(parent);
	SolarDischargeThreshold.setRange(20, 90);
	SolarDischargeThreshold.setValue(60);
	SolarDischargeThreshold.setSuffix(" °C");
	setupLayout.addWidget(&labelSolarDischarge, index, 0, 1, 1);
	setupLayout.addWidget(SolarDischargeDevice, index, 1, 1, 1);
	setupLayout.addWidget(&SolarDischargeThreshold, index++, 2, 1, 1);

	labelTankHigh.setText(tr("Tank High"));
	TankHighDevice = new devchooser(parent);
	setupLayout.addWidget(&labelTankHigh, index, 0, 1, 1);
	setupLayout.addWidget(TankHighDevice, index++, 1, 1, 1);

	Process.setText(tr("Process"));
	setupLayout.addWidget(&Process, index++, 0, 1, 1);

	AllOpened.setText(tr("All Opened"));
	AllOpened.setCheckable(true);
	AllClosed.setText(tr("All Closed"));
	AllClosed.setCheckable(true);
	setupLayout.addWidget(&AllOpened, index, 0, 1, 1);
	setupLayout.addWidget(&AllClosed, index++, 1, 1, 1);
//	Header = new ChauffageBlockHeader(this);
//	displayLayout.addWidget(Header, 0, 0, 1, 1);
	Locked = false;
	connect(&AllOpened, SIGNAL(clicked(bool)), this, SLOT(AllOpenedClick(bool)));
	connect(&AllClosed, SIGNAL(clicked(bool)), this, SLOT(AllClosedClick(bool)));
	connect(&Process, SIGNAL(clicked()), this, SLOT(process()));
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(Add()));
	connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(supprimer()));
}




ChauffageScrollArea::~ChauffageScrollArea()
{
}





void ChauffageScrollArea::remoteCommand(QString)
{
}




void ChauffageScrollArea::AllOpenedClick(bool)
{
	AllClosed.setChecked(false);
	process();
}




void ChauffageScrollArea::AllClosedClick(bool)
{
	AllOpened.setChecked(false);	
	process();
}




void ChauffageScrollArea::setProgramName(int index, QString programName)
{
	for (int n=0; n<chauffage.count(); n++)
		chauffage[n]->Program.setItemText(index, programName);
}
 




void ChauffageScrollArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
	{
		QMenu contextualmenu;
		QAction Add(cstr::toStr(cstr::Add), this);
		QAction Remove(cstr::toStr(cstr::Remove), this);
		QAction Lock(cstr::toStr(cstr::Lock), this);
		QAction Unlock(cstr::toStr(cstr::UnLock), this);
		contextualmenu.addAction(&Add);
		contextualmenu.addAction(&Remove);
		if (Locked == false) contextualmenu.addAction(&Lock);
		else contextualmenu.addAction(&Unlock);
		QAction *selection;
		selection = contextualmenu.exec(event->globalPos());
		if (!parent->isRemoteMode())
		{
            if (selection == &Add) AddEvent("");
			if (selection == &Remove) supprimer();
			if (selection == &Lock) verrouiller();
			if (selection == &Unlock) deverrouiller();
		}
    }
	if (event->button() == Qt::LeftButton)  parent->setPalette(&setup);
}








ChauffageUnit *ChauffageScrollArea::AddEvent(QString Name)
{
	bool ok;
	QString name;
	if (Name.isEmpty())
	{
Retry :
        name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, name, &ok, maison1wirewindow);
		if ((!ok) || name.isEmpty()) return nullptr;
		for (int i=0; i<chauffage.count(); i++)
			if (chauffage[i]->Name->text() == name)
			{
				messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
				goto Retry;
			}
	}
	else name = Name;
	ChauffageUnit *newchauffage;
	int n = chauffageList.findText(name);
	if (n == -1)
	{
		newchauffage = new ChauffageUnit(parent, name);
        newchauffage->Formula->ui.textEditFormule->setText("' Utiliser le mot clé TARGET comme consigne de chauffage dans la formule");
		chauffage.append(newchauffage);
		chauffageList.addItem(name);
        connect(parent->AddProgwin, SIGNAL(AddProgram(QString)), newchauffage, SLOT(AddProgram(QString)));
        connect(parent->AddProgwin, SIGNAL(RemoveProgram(QString)), newchauffage, SLOT(RemoveProgram(QString)));
        connect(parent->AddDaily, SIGNAL(ValueChange()), newchauffage, SLOT(process()));
		for (int n=0; n<parent->AddProgwin->program.count(); n++)
			newchauffage->Program.addItem(parent->AddProgwin->program[n]->Name);
        displayLayout.addWidget(newchauffage, displayLayoutIndex++, 0, 1, 5);
	}
	else newchauffage = chauffage[n];
	return newchauffage;
}




//qDebug() << QString("now = " + now.toString() + "   lastsave = " + lastsave.toString() + "  Dif = %1").arg(lastsave.secsTo(now));
//qDebug() << Name;






ChauffageUnit *ChauffageScrollArea::AddEvent(QString Name, QString Formula, QString ReadRomIDVanne, QString ReadRomIDRemote, QString ReadProgName, double consigne, double conf, double nuit, double eco)
{

    ChauffageUnit *newchauffage = AddEvent(Name);
    //newchauffage->disconnectcombo();
	newchauffage->VanneDevice->setRomID(ReadRomIDVanne);
    newchauffage->RemoteDisplay->setRomID(ReadRomIDRemote);
    newchauffage->ProgramName = ReadProgName;
    if (int(consigne) != 0) newchauffage->Consigne.setValue(consigne);
    if (int(conf) != 0) newchauffage->TConfort.setValue(conf);
    if (int(nuit) != 0) newchauffage->TNuit.setValue(nuit);
    if (int(eco) != 0) newchauffage->TEco.setValue(eco);
	newchauffage->Program.setCurrentIndex(newchauffage->Program.findText(ReadProgName));
	newchauffage->Formula->setFormula(Formula);
    return newchauffage;
}




void ChauffageScrollArea::Add()
{
    AddEvent("");
}





void ChauffageScrollArea::supprimer()
{
	bool ok;
	QStringList items;
	int count = chauffageList.count();
	for (int n=0; n<count; n++) items << chauffageList.itemText(n);
	QString item = inputDialog::getItemPalette(this, cstr::toStr(cstr::Remove), cstr::toStr(cstr::Remove), items, 0, false, &ok, Qt::Dialog, parent);
	if (!ok) return;
	if (item.isEmpty()) return;
	int index = chauffageList.findText(item);
	if (index < 0) return;
	if (messageBox::questionHide(this, tr("Remove Switch"),
			tr("Are you sure you want to remove ") + item , parent,
			QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			{
				chauffageList.removeItem(index);
			// Remove Widgets
				displayLayout.removeWidget(chauffage[index]);
			// Scroll next widgets
				for (int n=index; n<count; n++)
				{
					displayLayout.removeWidget(chauffage[n]);
                    displayLayout.addWidget(chauffage[n], n, 0, 1, 5);
                }
			// delete ChauffageUnit
				delete chauffage[index];
			// Rearrange array
				chauffage.removeAt(index);
                displayLayoutIndex--;
           }
}






void ChauffageScrollArea::verrouiller()
{
	Locked = true;
	for (int index=0; index<chauffageList.count(); index++)
	{
        chauffage[index]->Consigne.setReadOnly(true);
		chauffage[index]->TConfort.setReadOnly(true);
		chauffage[index]->TEco.setReadOnly(true);
		chauffage[index]->Program.setDisabled(true);
	}
}






void ChauffageScrollArea::deverrouiller()
{
	Locked = false;
	for (int index=0; index<chauffageList.count(); index++)
	{
        chauffage[index]->Consigne.setReadOnly(false);
		chauffage[index]->TConfort.setReadOnly(false);
		chauffage[index]->TEco.setReadOnly(false);
		chauffage[index]->Program.setDisabled(false);
	}
}





void ChauffageScrollArea::process()
{
	bool deviceOn = false;
	Process.setEnabled(false);
	int total = 0;
	bool Solar = false;
	QList <int> chauffageProcess;
// get total of valve perccentage in case < 100%
	for (int n=0; n<chauffage.count(); n++)
	{
		int r = chauffage[n]->process();
		if (chauffage[n]->solarEnabled()) Solar = true;
		if (!chauffage[n]->passiveMode.isChecked())
            if (logisdom::isNotNA(r)) total += r;
		chauffageProcess.append(r);
	}
// get device enabler pointers from devicechooser
	onewiredevice *device = CirculatorDevice->device();
	onewiredevice *tankEnabler = TankEnablerDevice->device();
	onewiredevice *solarEnabler = SolarEnablerDevice->device();
	onewiredevice *solarDischarge = SolarDischargeDevice->device();
	onewiredevice *TankHigh = TankHighDevice->device();
// calculate only if not remote mode
	if (!parent->isRemoteMode())
	{
		if (AllOpened.isChecked())
		{
			for (int n=0; n<chauffage.count(); n++) chauffage[n]->setValue(100);
			deviceOn = true;
		}
		else if (AllClosed.isChecked())
		{
			for (int n=0; n<chauffage.count(); n++) chauffage[n]->setValue(0);
			deviceOn = false;
		}
// recalculate if total < 100% to get a total of 100%
		else if (total)
		{
            int coef = 1;
            if (total) coef = 10000 / total;
            if (total < 100) for (int n=0; n<chauffage.count(); n++) chauffage[n]->setValue(coef);
            else
            for (int n=0; n<chauffage.count(); n++) chauffage[n]->setValue(-1);
			deviceOn = true;
		}
		else
		{
			deviceOn = false;
		}
	}
	bool on = false;
// circulator on if tank T° is high enough
	if (tankEnabler)
        if (tankEnabler->getMainValue() >= double(TankEnablerThreshold.value())) on = true;
    //qDebug() << "Solar Check";
    if (solarEnabler)
	{
// circulator on if solar T° is high enough
        if (solarEnabler->getMainValue() >= double(SolarEnablerThreshold.value())) on = true;
		if (TankHigh)
		{
            //qDebug() << "TankHigh";
            if (Solar && (solarEnabler->getMainValue() <= TankHigh->getMainValue())
                && (solarEnabler->getMainValue() >= double(SolarEnablerThreshold.value())))
			{
				for (int n=0; n<chauffage.count(); n++)
					if (chauffage[n]->solarEnabled()) chauffage[n]->setValue(100);
					//else chauffage[n]->setValue(0); // process()
					// if not dedicated to solar, heat as expected normally
					else chauffage[n]->setValue(chauffageProcess[n]);
				deviceOn = true;
				on = true;
                //qDebug() << "Solar ON";
			}
		}
	}
	if (solarDischarge)
	{
                if (solarDischarge->getMainValue() >= double(SolarDischargeThreshold.value()))
		{
			deviceOn = true;
			on = true;
		}
	}
	if (!parent->isRemoteMode())
		if (device)
		{
			if (on && deviceOn)
				device->set_On();
			else
				device->set_Off();
		}
	if (on && deviceOn) labelCirculatorStatus.setText(cstr::toStr(cstr::ON));
	else labelCirculatorStatus.setText(cstr::toStr(cstr::OFF));
	Process.setEnabled(true);
}





void  ChauffageScrollArea::SaveConfigStr(QString &str)
{
	str += "\nChauffage Config\n";
	str += logisdom::saveformat("CircRomID", CirculatorDevice->getRomID());
	str += logisdom::saveformat("TankEnablerRomID", TankEnablerDevice->getRomID());
	str += logisdom::saveformat("TankEnablerValue",  QString("%1").arg(TankEnablerThreshold.value()));
	str += logisdom::saveformat("SolarEnablerRomID", SolarEnablerDevice->getRomID());
	str += logisdom::saveformat("SolarDischargeRomID", SolarDischargeDevice->getRomID());
	str += logisdom::saveformat("TankHighRomID", TankHighDevice->getRomID());
	str += logisdom::saveformat("SolarEnablerValue",  QString("%1").arg(SolarEnablerThreshold.value()));
	str += logisdom::saveformat("SolarDischargeValue",  QString("%1").arg(SolarDischargeThreshold.value()));
	str += EndMark;
	for (int n=0; n<chauffage.count(); n++)
	{
	 	str += "\nChauffage Unit\n";
        str += logisdom::saveformat("Name", chauffage[n]->Name->text(), true);
        str += logisdom::saveformat("Consigne", QString("%1").arg(chauffage[n]->Consigne.value()));
        QString F = chauffage[n]->Formula->getFormula();
        //QString Fo = F.replace('(', "[");
        //QString Fc = Fo.replace(')', "]");
        str += logisdom::saveformat("Formula", F, true);
		str += logisdom::saveformat("VanneRomID", chauffage[n]->VanneDevice->getRomID());
        str += logisdom::saveformat("RemoteRomID", chauffage[n]->RemoteDisplay->getRomID());
        str += logisdom::saveformat("Program", chauffage[n]->Program.currentText());
		str += logisdom::saveformat("TConfort", QString("%1").arg(chauffage[n]->TConfort.value()));
        str += logisdom::saveformat("TNuit", QString("%1").arg(chauffage[n]->TNuit.value()));
        str += logisdom::saveformat("TEco", QString("%1").arg(chauffage[n]->TEco.value()));
		str += logisdom::saveformat("Vmin", QString("%1").arg(chauffage[n]->Vmin.value()));
		str += logisdom::saveformat("minEnabled", QString("%1").arg(chauffage[n]->maxEnabled.isChecked()));
		str += logisdom::saveformat("Vmax", QString("%1").arg(chauffage[n]->Vmax.value()));
		str += logisdom::saveformat("maxEnabled", QString("%1").arg(chauffage[n]->maxEnabled.isChecked()));
		str += logisdom::saveformat("passiveState", QString("%1").arg(chauffage[n]->passiveMode.isChecked()));
		str += logisdom::saveformat("proportionalMinMax", QString("%1").arg(chauffage[n]->proportionalMinMax.isChecked()));
		str += logisdom::saveformat("Mode", QString("%1").arg(chauffage[n]->mode));
		str += logisdom::saveformat("Solar", QString("%1").arg(chauffage[n]->solarEnabled()));
		str += logisdom::saveformat("BinderID", chauffage[n]->htmlBind->ID);
		str += EndMark;
		str +="\n";
	}
}





void ChauffageScrollArea::readconfigfile(const QString &configdata)
{
	bool ok;
    QString ReadRomIDVanne, ReadRomIDRemote, ReadProgName, ReadName;
	readchauffagconfig(configdata);
	QString TAG_Begin = "Chauffage Unit";
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadName = logisdom::getvalue("Name", strsearch);
	ReadRomIDVanne = logisdom::getvalue("VanneRomID", strsearch);
    ReadRomIDRemote = logisdom::getvalue("RemoteRomID", strsearch);
    ReadProgName = logisdom::getvalue("Program", strsearch);
	double consigne = logisdom::getvalue("Consigne", strsearch).toDouble(&ok);
	double conf = logisdom::getvalue("TConfort", strsearch).toDouble(&ok);
    double nuit = logisdom::getvalue("TNuit", strsearch).toDouble(&ok);
    double eco = logisdom::getvalue("TEco", strsearch).toDouble(&ok);
    QString F = logisdom::getvalue("Formula", strsearch);
    QString Fo, Fc;
    if (!F.isEmpty())
    {
        Fo = F.replace('[', "(");
        Fc = Fo.replace(']', ")");
    }
    ChauffageUnit *newUnit;
	if (!ReadName.isEmpty()) 
	{
            newUnit = AddEvent(ReadName, Fc, ReadRomIDVanne, ReadRomIDRemote, ReadProgName, consigne, conf, nuit, eco);
            int Mode = logisdom::getvalue("Mode", strsearch).toInt(&ok);
            if (ok) newUnit->ModeList.setCurrentIndex(Mode);
            int solar = logisdom::getvalue("Solar", strsearch).toInt(&ok);
            if (ok && solar) newUnit->solarEnable.setCheckState(Qt::Checked);
            QString ID = logisdom::getvalue("BinderID", strsearch);
            if (parent->isRemoteMode())
            if (!ID.isEmpty()) newUnit->htmlBind->ID = ID;
            int Vmin = logisdom::getvalue("Vmin", strsearch).toInt(&ok);
            if (ok) newUnit->Vmin.setValue(Vmin);
            int minEnabled = logisdom::getvalue("minEnabled", strsearch).toInt(&ok);
            if (ok && minEnabled) newUnit->minEnabled.setChecked(true);
            int Vmax = logisdom::getvalue("Vmax", strsearch).toInt(&ok);
            if (ok) newUnit->Vmax.setValue(Vmax);
            int maxEnabled = logisdom::getvalue("maxEnabled", strsearch).toInt(&ok);
            if (ok && maxEnabled) newUnit->maxEnabled.setChecked(true);
            int passiveMode = logisdom::getvalue("passiveState", strsearch).toInt(&ok);
            if (ok && passiveMode) newUnit->passiveMode.setChecked(true);
            int proportionalMinMax = logisdom::getvalue("proportionalMinMax", strsearch).toInt(&ok);
            if (ok && proportionalMinMax) newUnit->proportionalMinMax.setChecked(true);
	}
	SearchLoopEnd
}







void ChauffageScrollArea::readchauffagconfig(const QString &configdata)
{
	bool ok;
	QString RomID;
	QString TAG_Begin = "Chauffage Config";
	QString TAG_End = EndMark;
	SearchLoopBegin
	RomID = logisdom::getvalue("CircRomID", strsearch);
	if (!RomID.isEmpty()) CirculatorDevice->setRomID(RomID);
	RomID = logisdom::getvalue("TankEnablerRomID", strsearch);
	if (!RomID.isEmpty()) TankEnablerDevice->setRomID(RomID);
	RomID = logisdom::getvalue("TankHighRomID", strsearch);
	if (!RomID.isEmpty()) TankHighDevice->setRomID(RomID);
	int v = logisdom::getvalue("TankEnablerValue", strsearch).toInt(&ok);
	if (ok) TankEnablerThreshold.setValue(v);
	RomID = logisdom::getvalue("SolarEnablerRomID", strsearch);
	if (!RomID.isEmpty()) SolarEnablerDevice->setRomID(RomID);
	v = logisdom::getvalue("SolarEnablerValue", strsearch).toInt(&ok);
	if (ok) SolarEnablerThreshold.setValue(v);
	v = logisdom::getvalue("SolarDischargeValue", strsearch).toInt(&ok);
	if (ok) SolarDischargeThreshold.setValue(v);
	RomID = logisdom::getvalue("SolarDischargeRomID", strsearch);
	if (!RomID.isEmpty()) SolarDischargeDevice->setRomID(RomID);
	SearchLoopEnd
}




