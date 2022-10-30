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
#include "daily.h"
#include "onewire.h"
#include "logisdom.h"
#include "configwindow.h"
#include "programevent.h"
#include "devchooser.h"
#include "globalvar.h"
#include "htmlbinder.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "formula.h"
#include "vmc.h"





SwitchScrollArea::SwitchScrollArea(logisdom *Parent)
{
	parent = Parent;
	setLayout(&layout);
    connect(&UpdateTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
	htmlBindSwitchMenu = new htmlBinder(parent);
	htmlBindSwitchMenu->setMainParameter(tr("Switch"), "");
// palette setup
	setup.setLayout(&setupLayout);
	QIcon addIcon(QString::fromUtf8(":/images/images/edit_add.png"));
    AddButton.setIcon(addIcon);
    AddButton.setIconSize(QSize(logisdom::statusIconSize/2, logisdom::statusIconSize/2));
	AddButton.setToolTip(tr("Add"));
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(Add()));
    UpdateTimer.start(10000);
}





SwitchScrollArea::~SwitchScrollArea()
{
}



void SwitchScrollArea::updateButton()
{
    layout.removeWidget(&AddButton);
    AddButton.hide();
    for (int n=0; n<Switches.count(); n++) layout.removeWidget(Switches.at(n));
    for (int n=0; n<Switches.count(); n++) layout.addWidget(Switches.at(n), n, 0, 1, 1);
    foreach (SwitchControl *s, Switches)
        if (s->lockStatus != lockStatus) s->Lock(lockStatus);
    if (lockStatus) return;
    layout.addWidget(&AddButton, Switches.count(), 0, 1, 1);
    AddButton.show();
}




void SwitchScrollArea::Lock(bool state)
{
    lockStatus = state;
    foreach (SwitchControl *s, Switches)
        s->Lock(state);
    updateButton();
}




void SwitchScrollArea::Del()
{
}


void SwitchScrollArea::Add()
{
	SwitchControl *NewSwitch = new SwitchControl(parent);
	Switches.append(NewSwitch);
    updateButton();
	connect(NewSwitch, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
    connect(NewSwitch, SIGNAL(saveConfig()), parent, SLOT(saveconfig()));
    connect(NewSwitch, SIGNAL(deleteRequest(SwitchControl*)), this, SLOT(deleteMe(SwitchControl*)));
    NewSwitch->makeConnections();
    NewSwitch->ui.comboBoxType->setCurrentIndex(1);
    NewSwitch->modeChanged(1);
    emit(setupClick(&NewSwitch->setup));
}




void SwitchScrollArea::deleteMe(SwitchControl *me)
{
    if (lockStatus)
    {
        messageBox::warningHide(this, tr("Locked"), tr("Please unlock first"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }
    bool paletteHidden = parent->isPaletteHidden();
    parent->PaletteHide(true);
    parent->PaletteHide(paletteHidden);
    if ((messageBox::questionHide(this, tr("Remove Switch"),
            tr("Are you sure you want to remove this switch") , parent,
            QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
    {
        int index = Switches.indexOf(me);
        if (index != -1) Switches.removeAt(index);
        me->hide();
        me->Clean();
        //me->deleteLater();
        updateButton();
    }
    parent->PaletteHide(paletteHidden);
}




void SwitchScrollArea::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
	}
	if (event->button() == Qt::LeftButton)
	{
		emit(setupClick(&setup));
	}
}





void SwitchScrollArea::updateStatus()
{
    UpdateTimer.stop();
    if (!mutexUpdate.tryLock()) return;
    for (int n=0; n<Switches.count(); n++)
		Switches[n]->updateStatus();
    UpdateTimer.start(1000);
    mutexUpdate.unlock();
}




void SwitchScrollArea::updateJourneyBreaks(ProgramData *prog)
{
	for (int n=0; n<Switches.count(); n++)
		Switches[n]->updateJourneyBreaks(prog);
}




void SwitchScrollArea::readconfigfile(const QString &configdata)
{
	QString ReadName;
	QString TAG_Begin = Switch_Config_Begin;
	QString TAG_End = Switch_Config_End;
	SearchLoopBegin
		if (!strsearch.isEmpty()) readSwitchConfig(strsearch);
	SearchLoopEnd
}





void SwitchScrollArea::readSwitchConfig(QString &data)
{
	QByteArray configdata;
	configdata.append(data);
	QString TAG_Begin = Switch_Begin_Mark;
	QString TAG_End = Switch_End_Mark;
	SearchLoopBegin
	if (!strsearch.isEmpty())
	{
		SwitchControl *NewSwitch = new SwitchControl(parent);
		NewSwitch->readconfigfile(strsearch);
		Switches.append(NewSwitch);
		connect(NewSwitch, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
        connect(NewSwitch, SIGNAL(saveConfig()), parent, SLOT(saveconfig()));
        connect(NewSwitch, SIGNAL(deleteRequest(SwitchControl*)), this, SLOT(deleteMe(SwitchControl*)));
    }
	SearchLoopEnd
    updateButton();
}






void  SwitchScrollArea::SaveConfigStr(QString &str)
{
	str += "\n" Switch_Config_Begin "\n";
	for (int n=0; n<Switches.count(); n++)
		Switches[n]->SaveConfigStr(str);
	str += Switch_Config_End;
}




void SwitchScrollArea::setPalette()
{
	parent->setPalette(&setup);
}





SwitchThreshold::SwitchThreshold(logisdom *Parent, QString formulaName)
{
	parent = Parent;
	setLayout(&layout);
	if (formulaName.isEmpty())
	{
	    switchThreshold = new devchooser(parent);
        layout.addWidget(switchThreshold, 0, 1, 1, 1);
	}
	else
	{
	    Switchformula = new formula(parent);
	    Switchformula->removeButtons();
	    ButtonFormula.setText(formulaName);
        layout.addWidget(&ButtonFormula, 0, 1, 1, 1);
	    name = formulaName;
	    Switchformula->setWindowTitle(name);
	    connect(&ButtonFormula, SIGNAL(clicked()), this, SLOT(showFormula()));
	}

    QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
    ButtonDel.setIcon(removeIcon);
    ButtonDel.setIconSize(QSize(logisdom::statusIconSize/2, logisdom::statusIconSize/2));
    ButtonDel.setToolTip(tr("Remove"));
    const QSize BUTTON_SIZE = QSize(22, 22);
    ButtonDel.setMaximumSize(BUTTON_SIZE);
    connect(&ButtonDel, SIGNAL(clicked()), this, SLOT(delClick()));

    mode.addItem("if ON");
    mode.addItem("<");
    mode.addItem("=");
    mode.addItem("<>");
    mode.addItem(">");
    mode.addItem("value");
    layout.addWidget(&mode, 0, 2, 1, 1);
    thresholdValue.setRange(-9999999, 9999999);
    layout.addWidget(&thresholdValue, 0, 3, 1, 1);
	thresholdValue.setEnabled(false);
    //turnOnDelay.setPrefix(tr("Turn On Delay : "));
    turnOnDelay.setToolTip(tr("Turn On Delay"));
	turnOnDelay.setSuffix(tr(" mn"));
	turnOnDelay.setRange(0, 999);
	turnOnDelay.setValue(2);
    layout.addWidget(&turnOnDelay, 0, 4, 1, 1);
    //delay.setPrefix(tr("Delay : "));
    delay.setToolTip(tr("Delay to turn OFF"));
	delay.setSuffix(tr(" mn"));
	delay.setRange(0, 999);
	delay.setValue(10);
    layout.addWidget(&delay, 0, 5, 1, 1);
	status.setText("...");
    layout.addWidget(&status, 0, 6, 1, 1);
	lastMainValue = logisdom::NA;
    currentValue = logisdom::NA;
    lastStatus = false;
}




SwitchThreshold::~SwitchThreshold()
{
/*    switchThreshold->deleteLater();
    Switchformula->deleteLater();*/
}




QString SwitchThreshold::getName()
{
    if (switchThreshold) return switchThreshold->getName();
    else return name;
}



void SwitchThreshold::delClick()
{
    emit(deleteRequest(this));
}


void SwitchThreshold::configChange(int)
{
    switch (mode.currentIndex())
	{
        case ifON :
        case formula_Value : thresholdValue.setEnabled(false);
            //delay.setValue(0);
            delay.setEnabled(false);
            //turnOnDelay.setValue(0);
            turnOnDelay.setEnabled(true);
        break;
		case isLower :
		case isEqual :
		case isDifferent :
		case isHigher : thresholdValue.setEnabled(true);
            delay.setEnabled(true);
            turnOnDelay.setEnabled(true);
        break;
		default: break;
	}
    emit(configChanged());
}




void SwitchThreshold::showFormula()
{
    if (Switchformula) Switchformula->show();
}




void SwitchThreshold::CheckStatus(double &DeviceON)
{
	bool state = false;
	QDateTime now = QDateTime::currentDateTime();
	double v = logisdom::NA;
	if (switchThreshold)
	{
	    onewiredevice *device = switchThreshold->device();
	    if (device) v = device->getMainValue();
	    else v = logisdom::NA;
	}
	if (Switchformula)
	{
	    v = Switchformula->getValue();
	}
    if (logisdom::isNA(v))
	{
		status.setText("Device not ready");
		return;
	}
    if (mode.currentIndex() == formula_Value)
    {
        if (logisdom::isNA(currentValue) || logisdom::AreSame(v, lastMainValue))
        {
            currentValue = v;
            lastMainValue = v;
            PushOn = PushOn.addSecs((-turnOnDelay.value() * 60)+1);
        }
        if (turnOnDelay.value() == 0)   // No delay
        {
            DeviceON = v;
            status.setText(QString("%1").arg(int(v)));
            ButtonFormula.setText(name + " (" + QString("%1").arg(v) + ")");
        }
        else if (logisdom::AreNotSame(currentValue,v))    // restart delay because value changed
        {
            PushOn.setDate(now.date());
            PushOn.setTime(now.time());
            currentValue = v;
            DeviceON = lastMainValue;
        }
        else    // check delay
        {
            QDateTime check = PushOn.addSecs(turnOnDelay.value() * 60);
            if (now.secsTo(check) >= 0)// delay not reached
            {
                DeviceON = lastMainValue;
                status.setText(QString("Value %1 Wait %2 s before setting to %3").arg(lastMainValue).arg(now.secsTo(check)).arg(currentValue));
                ButtonFormula.setText(name + " (" + QString("%1").arg(lastMainValue) + ")");
            }
            else
            {
                DeviceON = v;
                lastMainValue = v;
                status.setText(QString("%1").arg(int(v)));
                ButtonFormula.setText(name + " (" + QString("%1").arg(v) + ")");
            }
        }
    }
    else
    {
        switch (mode.currentIndex())
        {
            case ifON : if (logisdom::isNotZero(v)) state = true; break;
            case isLower : if (v < thresholdValue.value()) state = true; break;
            case isEqual : if (logisdom::AreSame(v, thresholdValue.value())) state = true; break;
            case isDifferent : if (logisdom::AreNotSame(v, thresholdValue.value())) state = true; break;
            case isHigher : if (v > thresholdValue.value()) state = true; break;
            default: break;
        }
        if (state != lastStatus)
        {
            lastStatus = state;
            if (state)
            {
                 PushOn.setDate(now.date());
                 PushOn.setTime(now.time());
            }
        }
        qint64 difSecs = PushOn.secsTo(now);
        QTime T;
        T.setHMS(0, 0, 0);
        qint64 dif = difSecs / 60;
        if (((state) and (dif >= turnOnDelay.value())) or (((state) and (turnOnDelay.value() == 0))))
        {
            DeviceON = 100;
            status.setText("Active");
            if (switchThreshold) switchThreshold->setStyleSheet("color: green");
            if (Switchformula) ButtonFormula.setStyleSheet("color: green");
        }
        else if ((state) and (dif < turnOnDelay.value()))
        {
            QString s = tr("Not active");
            s += T.addSecs(int(turnOnDelay.value() * 60 - difSecs)).toString(" mm:ss ");
            s += tr("minutes to go");
            if (switchThreshold) switchThreshold->setStyleSheet("color: yellow");
            if (Switchformula) ButtonFormula.setStyleSheet("color: yellow");
            status.setText(s);
        }
        else
        {
            status.setText("Not active");
            if (switchThreshold) switchThreshold->setStyleSheet("color: red");
            if (Switchformula) ButtonFormula.setStyleSheet("color: red");
        }
    }
/* background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f6f7fa, stop: 1 #dadbde) */
}



void SwitchThreshold::SaveConfigStr(QString &str)
{
	str += New_Trigger_Begin "\n";
	if (switchThreshold) str += logisdom::saveformat("TriggerRomID", switchThreshold->getRomID());
	if (Switchformula)
	{
	    QString F = Switchformula->getFormula();
        str += logisdom::saveformat("FormulaStr", F, true);
        str += logisdom::saveformat("FormulaName", name, true);
        str += logisdom::saveformat("ValueOnErrorEnabled", QString("%1").arg(Switchformula->ui.ValueOnErrorEnable->isChecked()));
        str += logisdom::saveformat("ValueOnErrorTxt", Switchformula->ui.ValueOnError->text());
    }
	str += logisdom::saveformat("Delay", QString("%1").arg(delay.value()));
	str += logisdom::saveformat("TurnOnLag", QString("%1").arg(turnOnDelay.value()));
	str += logisdom::saveformat("Level", QString("%1").arg(thresholdValue.value()));
    int Mode = mode.currentIndex();
	if (Mode != -1) str += logisdom::saveformat("Mode", QString("%1").arg(Mode));
	str += New_Trigger_End "\n";
}



void SwitchThreshold::readconfigfile(QString &strsearch)
{
	bool ok;
	QString RomID;
	int Delay, TurnOnDelay, Level, Mode;
	if (Switchformula)
	{
        QString str = strsearch;
        QString result;
        QString search = "Formula";
        int l = str.length();
        if (l != 0)
        {
            int i = str.indexOf(search);
            if (i != -1)
            {
                int coma = str.indexOf("(", i);
                if (coma != -1)
                {
                    int nextcoma = str.indexOf(")", coma + 1);
                    if (nextcoma == -1) nextcoma = str.indexOf("(", coma + 1);
                    if (nextcoma != -1)
                    {
                        result = str.mid(coma + 1, nextcoma - coma - 1);
                        if (!result.isEmpty())
                        {
                            if (result.mid(0, 4) == "HEX:")
                            {
                               QByteArray Hex;
                                Hex.append(result.remove(0, 4));
                                QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
                                QByteArray F = QByteArray::fromHex(Hex);
                                result = Utf8Codec->toUnicode(F);
                            }
                            else
                            {
                                QString Fo = result.replace('[', "(");
                                result = Fo.replace(']', ")");
                            }
                        }
                    }
                }
            }
        }
        Switchformula->setFormula(result);
        int errsaveen = logisdom::getvalue("ValueOnErrorEnabled", strsearch).toInt(&ok);
        if (ok && errsaveen) Switchformula->ui.ValueOnErrorEnable->setChecked(true);
        Switchformula->ui.ValueOnError->setText(logisdom::getvalue("ValueOnErrorTxt", strsearch));
        QString N = logisdom::getvalue("FormulaName", strsearch);
	    name = N;
    }
	if (switchThreshold)
	{
	    RomID = logisdom::getvalue("TriggerRomID", strsearch);
	    if (!RomID.isEmpty()) switchThreshold->setRomID(RomID);
	}
	Delay = logisdom::getvalue("Delay", strsearch).toInt(&ok, 10);
	if (ok) delay.setValue(Delay);
	TurnOnDelay = logisdom::getvalue("TurnOnLag", strsearch).toInt(&ok, 10);
	if (ok) turnOnDelay.setValue(TurnOnDelay);
	Level = logisdom::getvalue("Level", strsearch).toInt(&ok, 10);
	if (ok) thresholdValue.setValue(Level);
	Mode = logisdom::getvalue("Mode", strsearch).toInt(&ok, 10);
    if (ok) mode.setCurrentIndex(Mode);
    init();
}



void SwitchThreshold::init()
{
    //connect(&thresholdValue, SIGNAL(valueChanged(int)), this, SLOT(configChange(int)));
    connect(&mode, SIGNAL(currentIndexChanged(int)), this, SLOT(configChange(int)));
}



void SwitchThreshold::Lock(bool state)
{
    lockStatus = state;
    updateLock();
}



void SwitchThreshold::updateLock()
{
    if (lockStatus)
    {
        layout.removeWidget(&ButtonDel);
        ButtonDel.hide();
    }
    else
    {
        layout.addWidget(&ButtonDel, 0, 0, 1, 1);
        ButtonDel.show();
    }
}



SwitchControl::SwitchControl(logisdom *Parent)
{
	parent = Parent;
    w = new QWidget(this);
    ui.setupUi(w);
    volet_status = 0;
    sliderHasMoved = false;
    moving = false;
    Button_borderwidth = "border-width: 1px;";
    Button_borderradius = "border-radius: 4px;";
    Button_margin = "margin: 0 1px 0 1px;";
    Button_minheight = "min-height: 1.4em;";
    Button_font = "font: 1em;";
    Button_bckrgd_1 = "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #2198c0, stop: 1 #0d5ca6);";
    //Button_bckrgd_2 = "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #0098c0, stop: 1 #005ca6);";
    Button_bckrgd_2 = "color: black; background-color: orange;";

    styleNormal = "QPushButton {" + Button_bckrgd_1 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";
//    styleDisabled = "QPushButton:disabled {" + Button_bckrgd_1 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";
//    stylePressed = "QPushButton:focus:pressed {" + Button_bckrgd_2 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";
    styleChecked = "QPushButton:checked {" + Button_bckrgd_2 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";
//    styleFocus = "QPushButton:focus {" + Button_bckrgd_1 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";
//    styleFocusPressed = "QPushButton:focus:pressed {" + Button_bckrgd_1 + Button_borderwidth + Button_borderradius + Button_margin + Button_minheight + Button_font + "} ";

    ButtonStyle = styleNormal + styleDisabled + styleChecked + stylePressed + styleFocus + styleFocusPressed;

    SwitchControlLayout = new QGridLayout(this);

	ButtonName.setText(tr("not assigned"));
    //ButtonName.setToolTip("Press control and click to remove");
    SwitchControlLayout->addWidget(&ButtonName, 0, 0, 1, 1);

	ButtonAuto.setText(tr("Automatic"));
	ButtonAuto.setCheckable(true);
    ButtonAuto.setStyleSheet(ButtonStyle);
    ButtonGroup.addButton(&ButtonAuto);
    SwitchControlLayout->addWidget(&ButtonAuto, 0, 1, 1, 1);

	ButtonOn.setText(cstr::toStr(cstr::ON));
	ButtonOn.setCheckable(true);
    ButtonOn.setStyleSheet(ButtonStyle);
    ButtonGroup.addButton(&ButtonOn);
    SwitchControlLayout->addWidget(&ButtonOn, 0, 2, 1, 1);

	ButtonOff.setText(cstr::toStr(cstr::OFF));
	ButtonOff.setCheckable(true);
    ButtonOff.setStyleSheet(ButtonStyle);
    ButtonGroup.addButton(&ButtonOff);
    SwitchControlLayout->addWidget(&ButtonOff, 0, 3, 1, 1);

    SliderStatus.setOrientation(Qt::Horizontal);
    SwitchControlLayout->addWidget(&SliderStatus, 0, 4, 1, 1);
    connect(&SliderStatus, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    SliderStatus.hide();

    ButtonDisabled.setText(cstr::toStr(cstr::Disabled));
	ButtonDisabled.setCheckable(true);
    ButtonDisabled.setStyleSheet(ButtonStyle);
    ButtonGroup.addButton(&ButtonDisabled);
    SwitchControlLayout->addWidget(&ButtonDisabled, 0, 5, 1, 1);

	ButtonGroup.setId(&ButtonAuto, ID_Auto);
	ButtonGroup.setId(&ButtonOn, ID_On);
	ButtonGroup.setId(&ButtonOff, ID_Off);
	ButtonGroup.setId(&ButtonDisabled, ID_Disabled);

    QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
    ButtonDel.setIcon(removeIcon);
    ButtonDel.setIconSize(QSize(logisdom::statusIconSize/2, logisdom::statusIconSize/2));
    ButtonDel.setToolTip(tr("Remove"));
    const QSize BUTTON_SIZE = QSize(22, 22);
    ButtonDel.setMaximumSize(BUTTON_SIZE);
    connect(&ButtonDel, SIGNAL(clicked()), this, SLOT(Del()));

    ButtonAuto.setChecked(true);
	StatusText = tr("Status");
    SwitchStatus.setText(StatusText);
    ui.labelStatus->setText(StatusText);
    SwitchControlLayout->addWidget(&SwitchStatus, 0, 6, 1, 1);

// Palette
	setup.setLayout(&setupLayout);
    setupLayout.addWidget(ui.tabWidget, 0, 0, 1, logisdom::PaletteWidth);
    QIcon addIcon(QString::fromUtf8(":/images/images/edit_add.png"));
    ui.AddDeviceButton->setText("");
    ui.AddDeviceButton->setIcon(addIcon);
    ui.AddDeviceButton->setIconSize(QSize(logisdom::statusIconSize/2, logisdom::statusIconSize/2));
    ui.AddDeviceButton->setMaximumSize(BUTTON_SIZE);
    ui.AddDeviceButton->setToolTip(tr("Add device to control"));
    connect(ui.AddDeviceButton, SIGNAL(clicked()), this, SLOT(addDevice()));
    ui.comboBoxType->addItem(tr("Variable"));
    ui.comboBoxType->addItem(tr("On/Off"));
    ui.comboBoxType->addItem(tr("Open/Close"));
    ui.comboBoxType->addItem(tr("Open/Close Variable"));
    ui.comboBoxType->setCurrentIndex(1);
	ModeTxt = tr("Mode");
	OnModeTxt = tr("On");
	OffModeTxt = tr("Off");
    AutoModeTxt = tr("Auto");
    OpenModeTxt = tr("Opened");
    CloseModeTxt = tr("Closed");
    ManualModeTxt = tr("Manual");
    DisabledModeTxt = tr("Disabled");

    ui.labelMaxTimeOn->setText(tr("Max Time ON") + " : ");
    ui.spinBoxMaxTimeOn->setSuffix(" " + tr("mn"));
    ui.spinBoxMaxTimeOn->setRange(1, 999);
    ui.spinBoxMaxTimeOn->setSpecialValueText(tr("Max Time ON Disabled"));

    htmlBind = new htmlBinder(parent, ButtonName.text(), parent->SwitchArea->htmlBindSwitchMenu->treeItem);
    htmlBind->addParameterCommand(ModeTxt, AutoModeTxt, setModeAuto);
    htmlBind->addParameterCommand(ModeTxt, OnModeTxt, setModeOn);
    htmlBind->addParameterCommand(ModeTxt, OffModeTxt, setModeOff);
    htmlBind->addParameterCommand(ModeTxt, OpenModeTxt, setModeOpen);
    htmlBind->addParameterCommand(ModeTxt, CloseModeTxt, setModeClose);
    htmlBind->addParameterCommand(ModeTxt, DisabledModeTxt, setModeDisabled);
	connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));

    newSwitchDevice();
    newCloseDevice();
    dailyprog = new daily(this, parent, "VMC", false);
    timeLayout = new QGridLayout(ui.tabWidget->widget(1));
    timeLayout->addWidget(dailyprog);
    thresholdLayout = new QGridLayout(ui.frameThresholdList);
    connect(ui.AddTriggerButton, SIGNAL(clicked()), this, SLOT(addTrigger()));
    connect(ui.AddFormula, SIGNAL(clicked()), this, SLOT(addFormula()));

	QDateTime now = QDateTime::currentDateTime();
	lastminute = now.time().minute();
    LastTurnOn = now;
    ui.spinBoxMaxTimeOn->setValue(60);
    ButtonOff.click();
    SwitchDelay = 0;
    waitMove = 0;
}



void SwitchControl::addFamilies(devchooser *SwitchDevice)
{
    if (SwitchDevice)
    {
        SwitchDevice->addFamily(familyMultiGestSwitch);
        SwitchDevice->addFamily(familyAM12);
        SwitchDevice->addFamily(familyPLCBUS);
        SwitchDevice->addFamily(family2413_A);
        SwitchDevice->addFamily(family2413_B);
        SwitchDevice->addFamily(family3A2100H_A);
        SwitchDevice->addFamily(family3A2100H_B);
        SwitchDevice->addFamily(family2406_A);
        SwitchDevice->addFamily(family2406_B);
        SwitchDevice->addFamily(family2408_A);
        SwitchDevice->addFamily(family2408_B);
        SwitchDevice->addFamily(family2408_C);
        SwitchDevice->addFamily(family2408_D);
        SwitchDevice->addFamily(family2408_E);
        SwitchDevice->addFamily(family2408_F);
        SwitchDevice->addFamily(family2408_G);
        SwitchDevice->addFamily(family2408_H);
        SwitchDevice->addFamily(familyModBus);
        SwitchDevice->addFamily(familyLedOW);
        SwitchDevice->addFamily(familyLCDOW_A);
        SwitchDevice->addFamily(familyLCDOW_B);
        SwitchDevice->addFamily(familyLCDOW_C);
        SwitchDevice->addFamily(familyLCDOW_D);
        SwitchDevice->addFamily(familyeoD2010F);
        SwitchDevice->addFamily(familyeoD20112_A);
        SwitchDevice->addFamily(familyeoD20112_B);
    }
}


SwitchControl::~SwitchControl()
{
    CloseDeviceList.clear();
    thresholdLayout->deleteLater();
    htmlBind->deleteLater();
    SwitchControlLayout->deleteLater();
    dailyprog->deleteLater();
    timeLayout->deleteLater();
    delete w;
}



void SwitchControl::Lock(bool state)
{
    lockStatus = state;
    if (state)
    {
        ButtonDel.hide();
        SwitchControlLayout->removeWidget(&ButtonDel);
        ui.AddDeviceButton->hide();
    }
    else
    {
        SwitchControlLayout->addWidget(&ButtonDel, 0, 7, 1, 1);
        ButtonDel.show();
        ui.AddDeviceButton->show();
    }
    foreach (devchooser *s, SwitchDeviceList)
    {
        s->Lock(state);
    }
    foreach (SwitchThreshold *t, triggerList)
    {
        t->Lock(state);
    }
}




void SwitchControl::Clean()
{
    foreach (devchooser *s, SwitchDeviceList)
    {
        if (s) s->freeDevice();
        s->deleteLater();
    }
    SwitchDeviceList.clear();
    foreach (devchooser *c, CloseDeviceList)
    {
        if (c) c->freeDevice();
        c->deleteLater();
    }
}




SwitchThreshold *SwitchControl::addTrigger(QString Name)
{
	SwitchThreshold *newTrigger = new SwitchThreshold(parent, Name);
    newTrigger->init();
    thresholdLayout->addWidget(newTrigger, triggerList.count(), 0, 1, 1);
    triggerList.append(newTrigger);
	connect(newTrigger, SIGNAL(configChanged()), this, SLOT(updateStatus()));
    connect(newTrigger, SIGNAL(deleteRequest(SwitchThreshold *)), this, SLOT(deleteThreshold(SwitchThreshold *)));
    newTrigger->Lock(lockStatus);
    return newTrigger;
}




void SwitchControl::remoteCommand(const QString &command)
{
	if (command == setModeAuto) ButtonAuto.click();
	if (command == setModeOn) ButtonOn.click();
    if (command == setModeOpen) ButtonOn.click();
    if (command == setModeClose) ButtonOff.click();
    if (command == setModeOff) ButtonOff.click();
    if (command == setModeDisabled) ButtonDisabled.click();
}




SwitchThreshold *SwitchControl::addFormula()
{
    bool ok;
Retry :
    QString name = inputDialog::getTextPalette(this, tr("Formula name"), "", QLineEdit::Normal, "", &ok, parent);
    if (name.isEmpty()) return nullptr;
    for (int n=0; n<triggerList.count(); n++)
    {
	if (name == triggerList.at(n)->getName())
	{
	    messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
	    goto Retry;
	}
    }
    if (ok)
    {
        removeLayoutThreshold();
        SwitchThreshold *newTrigger = new SwitchThreshold(parent, name);
        triggerList.append(newTrigger);
        connect(newTrigger, SIGNAL(configChanged()), this, SLOT(updateStatus()));
        connect(newTrigger, SIGNAL(deleteRequest(SwitchThreshold *)), this, SLOT(deleteThreshold(SwitchThreshold *)));
        newTrigger->Lock(lockStatus);
        addLayoutThreshold();
        return newTrigger;
    }
    return nullptr;
}





void SwitchControl::deviceSelectionChanged()
{
    QString Name = tr("not assigned");
    ButtonName.setText(Name);
    if (SwitchDeviceList.isEmpty()) return;
    if (!SwitchDeviceList.first()) return;
    onewiredevice *device = SwitchDeviceList.first()->device();
    if (!device) return;
    Name = device->getname();
    ButtonName.setText(Name);
}



void SwitchControl::deleteMe(devchooser *me)
{
    if ((messageBox::questionHide(this, tr("Remove Switch"),
            tr("Are you sure you want to remove this switch") , parent,
            QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
            {
                removeLayoutDev();
                for (int n=0; n<SwitchDeviceList.count(); n++)
                {
                    if (SwitchDeviceList.at(n) == me)
                    {
                        QMutexLocker locker(&mutexConfig);
                        SwitchDeviceList.at(n)->freeDevice();
                        CloseDeviceList.at(n)->freeDevice();
                        SwitchDeviceList.at(n)->deleteLater();
                        CloseDeviceList.at(n)->deleteLater();
                        SwitchDeviceList.removeAt(n);
                        CloseDeviceList.removeAt(n);
                        break;
                    }
                }
                addLayoutDev();
            }
}





void SwitchControl::deleteThreshold(SwitchThreshold *me)
{
    if ((messageBox::questionHide(this, tr("Remove"),
            tr("Are you sure you want to remove this threshold") , parent,
            QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
            {
                removeLayoutThreshold();
                for (int n=0; n<triggerList.count(); n++)
                {
                    if (triggerList.at(n) == me)
                    {
                        QMutexLocker locker(&mutexConfig);
                        delete triggerList.at(n);
                        triggerList.removeAt(n);
                        break;
                    }
                }
                addLayoutThreshold();
            }
}




void SwitchControl::addLayoutThreshold()
{
    for (int n=0; n<triggerList.count(); n++)
    {
        thresholdLayout->addWidget(triggerList.at(n), n, 0, 1, logisdom::PaletteWidth);
    }
}

void SwitchControl::removeLayoutThreshold()
{
    for (int n=0; n<triggerList.count(); n++)
    {
        thresholdLayout->removeWidget(triggerList.at(n));
    }
}



void SwitchControl::removeLayoutDev()
{
    for (int n=0; n<SwitchDeviceList.count(); n++)
    {
        ui.gridLayoutDevChooser->removeWidget(SwitchDeviceList.at(n));
        ui.gridLayoutDevChooser->removeWidget(CloseDeviceList.at(n));
    }
}



void SwitchControl::addLayoutDev()
{
    for (int n=0; n<SwitchDeviceList.count(); n++)
    {
        ui.gridLayoutDevChooser->addWidget(SwitchDeviceList.at(n), n+1, 0, 1, logisdom::PaletteWidth/2);
    }
    for (int n=0; n<CloseDeviceList.count(); n++)
        ui.gridLayoutDevChooser->addWidget(CloseDeviceList.at(n), n+1, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
}




void SwitchControl::updateStatus()
{
    QMutexLocker locker(&mutexConfig);
    QDateTime now = QDateTime::currentDateTime();
    int DeviceON = logisdom::NA;
    bool maxTimeReached = false;
    double schedulerValue = logisdom::NA;
    bool Disabled = false;
    int actualminute = now.time().minute();
    if (SliderStatus.maximum() != ui.spinBoxMaxTimeOn->value()) SliderStatus.setMaximum(ui.spinBoxMaxTimeOn->value());
    onewiredevice *switchdevice = nullptr;
    if (!SwitchDeviceList.isEmpty()) switchdevice = SwitchDeviceList.first()->device();
    onewiredevice *opendevice = switchdevice;
    onewiredevice *closedevice = nullptr;
    if (!CloseDeviceList.isEmpty()) closedevice = CloseDeviceList.first()->device();
// check if switch is not defined
    Status = "";
    if (!SwitchDeviceList.isEmpty())
    {
        if (!SwitchDeviceList.first())
        {
            if (SwitchDeviceList.first()->getRomID().isEmpty()) Status = "  " + tr("not defined") + "   ";
            else Status = SwitchDeviceList.first()->getRomID() + "  " + tr("not found") + "   ";
            SwitchStatus.setText(Status + " " + StatusText + " : " + cstr::toStr(cstr::NA));
            ui.labelStatus->setText(Status + " " + StatusText + " : " + cstr::toStr(cstr::NA));
        }
    }
// Get scheduler daily result if Program not user defined
// pas de calcul si pas de daily prog defini à supprimer 03-2017
//	if ((parent->ProgEventArea->isAutomatic()) or (parent->ProgEventArea->isOutside()) or (parent->ProgEventArea->isConfort()) or (parent->ProgEventArea->isEco()) or (parent->ProgEventArea->isUnfreeze()))
//	{
        schedulerValue = dailyprog->getValue(now.time());
        if (logisdom::isNA(schedulerValue)) schedulerValue = dailyprog->getLastValue(); // check on the previous day
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case VariableOPENCLOSE :
             if (logisdom::isNA(schedulerValue)) schedulerValue = -1;	// if not schedule is defined leave threshold to decode states
            break;
            case ONOFF :
            case OPENCLOSE :
                if (logisdom::isNA(schedulerValue))	schedulerValue = daily::StateAuto;	// if not schedule is defined leave threshold to decode states
            break;
        }
//	}
//	else // if user defined set corresponding value
//	{
//		QString PrgName = parent->ProgEventArea->getButtonName();
//		if (!PrgName.isEmpty()) schedulerValue = dailyprog->getValue(parent->getProgEventArea()->getButtonTime());
//	}
    if (switchdevice)
    {
        if (switchdevice->isManual())  // switch to manual
        {
            SliderStatus.setValue(int(switchdevice->getMainValue()));
            sliderActive();
            sliderHasMoved = false;
        }
    }
// process mode
    bool getTrigger = true;
    if (ButtonAuto.isChecked())
    {   // check scheduler only if Automatic is selected
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case VariableOPENCLOSE :
                if (logisdom::AreSame(schedulerValue, -1))
                {
                    getTrigger = true;
                }
                else
                {
                    DeviceON = int(schedulerValue);
                    getTrigger = false;
                }
            break;
            case ONOFF :
                if (logisdom::AreSame(schedulerValue,daily::StateOFF))
                {
                    DeviceON = 0;
                    SwitchDelay = 0;
                    getTrigger = false;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateON))
                {
                    DeviceON = 100;
                    getTrigger = false;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateAuto))
                {
                    getTrigger = true;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateDisabled))
                {
                    getTrigger = false;
                    Disabled = true;
                }
            break;
            case OPENCLOSE :
                if (logisdom::AreSame(schedulerValue, daily::StateOFF))
                {
                    DeviceON = 0;
                    SwitchDelay = 0;
                    getTrigger = false;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateON))
                {
                    DeviceON = ui.spinBoxMaxTimeOn->value();
                    getTrigger = false;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateAuto))
                {
                    getTrigger = true;
                }
                if (logisdom::AreSame(schedulerValue, daily::StateDisabled))
                {
                    getTrigger = false;
                    Disabled = true;
                }
            break;
        }
    }
    else
    {
        getTrigger = false;
        SwitchDelay = 0;
    }
// Check all Triggers
    int triggerCount = triggerList.count();
    int triggerCounter = 0;
// Get number of trigger, to prepare for AND/OR decision
    for (int n=0; n<triggerCount; n++)
    {
        double isDeviceON = 0;
        triggerList[n]->CheckStatus(isDeviceON);
        if (int(isDeviceON) && getTrigger) triggerCounter ++;
    }
// Apply only if trigger is required
    if (getTrigger)
    {
        if (triggerCounter)
        {
            double value = 0;
            double total = 0;
            bool zero = false;
            switch (ui.comboBoxType->currentIndex())
            {
                case Variable :
                case VariableOPENCLOSE :
                    for (int n=0; n<triggerCount; n++)
                    {
                        triggerList[n]->CheckStatus(value);
                        if (logisdom::isNotNA(value))
                        {
                            if (ui.logicalAND->isChecked())
                            {
                                if (logisdom::isZero(value)) zero = true;
                                else if (value > total) total = value;
                            }
                            else total += value;
                        }
                    }
                    if (zero) DeviceON = 0; else DeviceON = int(total);
                break;
                case ONOFF :
                    if (ui.logicalAND->isChecked())
                    {
                        if (triggerCounter == triggerCount) DeviceON = 100; else DeviceON = 0;
                    }
                    else DeviceON = 100;
                break;
                case OPENCLOSE :
                    for (int n=0; n<triggerCount; n++)
                    {
                        triggerList[n]->CheckStatus(value);
                        if (logisdom::isNotNA(value))
                        {
                            if (ui.logicalAND->isChecked())
                            {
                                if (int(value))
                                {
                                    if (value > total) total = value;
                                }
                                else zero = true;
                            }
                            else total += value;
                        }
                    }
                    if (zero) DeviceON = 0; else DeviceON = int(total);
                    if (DeviceON > ui.spinBoxMaxTimeOn->value()) DeviceON = ui.spinBoxMaxTimeOn->value();
                break;
            }
        }
        else DeviceON = 0;
    }
// if DeviceON Set VMC Delay
    if (DeviceON > 0)
    {
        for (int n=0; n<triggerCount; n++)
        {
            double isDeviceON = 0;
            triggerList[n]->CheckStatus(isDeviceON);
            if (int(isDeviceON) && getTrigger)		// Apply only if trigger is required
            {
                if (triggerList[n]->delay.value() > SwitchDelay) SwitchDelay = triggerList[n]->delay.value();
            }
        }
    }
// minute timer
    if (lastminute != actualminute)
    {
        int dif;
        if (actualminute > lastminute)
            dif = actualminute - lastminute;
        else
            dif = 60 + actualminute - lastminute;
        lastminute = actualminute;
        if ((SwitchDelay > 0) and (DeviceON == 0)) SwitchDelay -= dif;
        if (SwitchDelay < 0) SwitchDelay = 0;
    }
// Max Time ON only if Button Auto is activated
    if (ButtonAuto.isChecked())
    {
        htmlBind->setParameter(ModeTxt, AutoModeTxt);
        qint64 timeON;
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case ONOFF :
                if (DeviceON == 0) LastTurnOn = now;
                timeON = LastTurnOn.secsTo(now) / 60;
                if (ui.spinBoxMaxTimeOn->value() != 1)
                    if (timeON >= ui.spinBoxMaxTimeOn->value())
                    {
                        DeviceON = 0;
                        SwitchDelay = 0;
                        maxTimeReached = true;
                    }
            break;
            case OPENCLOSE :
            case VariableOPENCLOSE :
                SliderStatus.setValue(DeviceON);
            break;
        }
    }
    else if (ButtonOn.isChecked())
    {

        Disabled = false;
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
               DeviceON = ui.spinBoxMaxTimeOn->value();
               htmlBind->setParameter(ModeTxt, OnModeTxt);
            break;
            case ONOFF :
                DeviceON = 100;
                htmlBind->setParameter(ModeTxt, OnModeTxt);
            break;
            case OPENCLOSE :
            case VariableOPENCLOSE :
                DeviceON = ui.spinBoxMaxTimeOn->value();
                htmlBind->setParameter(ModeTxt, OpenModeTxt);
            break;
        }
    }
    else if (ButtonOff.isChecked())
    {
        DeviceON = 0;
        Disabled = false;
        SwitchDelay = 0;
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case ONOFF :
                htmlBind->setParameter(ModeTxt, OffModeTxt);
            break;
            case OPENCLOSE :
            case VariableOPENCLOSE :
                htmlBind->setParameter(ModeTxt, CloseModeTxt);
            break;
        }
    }
    else if (ButtonDisabled.isChecked())
    {
        Disabled = true;
        htmlBind->setParameter(ModeTxt, DisabledModeTxt);
    }
    else
    {
        switch (ui.comboBoxType->currentIndex())
        {
            case ONOFF :
            break;
            case Variable :
            case VariableOPENCLOSE :
            case OPENCLOSE :
            if (sliderHasMoved) DeviceON = SliderStatus.value();
            else
            {
                if (switchdevice)
                {
                    DeviceON = int(switchdevice->getMainValue());
                    SliderStatus.setValue(DeviceON);
                }
            }
            sliderHasMoved = false;
            break;
        }
        htmlBind->setParameter(ModeTxt, ManualModeTxt);
    }

    int SwitchValue = logisdom::NA;
    if (switchdevice)
    {
        SwitchValue = int(switchdevice->getMainValue());
        if (logisdom::isNotNA(SwitchValue))
        {
            switch (ui.comboBoxType->currentIndex())
            {
                case Variable :
                case VariableOPENCLOSE :
                break;
                case ONOFF :
                case OPENCLOSE : if (SwitchValue) SwitchValue = 100; break;
            }
        }
    }

    double maxValue = logisdom::NA;
    if (switchdevice) maxValue = switchdevice->getMaxValue();
    if (switchdevice && (!parent->isRemoteMode()) && (!Disabled) && logisdom::isNotNA(SwitchValue))
    {
        if (switchdevice->isDimmmable())
        {
            if (logisdom::isNotNA(maxValue))
            {
                switch (ui.comboBoxType->currentIndex())
                {
                    case Variable :
                        if (DeviceON != 0)
                        {
                            if (DeviceON <= maxValue) setDevicestate(switchdevice, DeviceON);
                            else setDevicestate(switchdevice, maxValue);
                        }
                        else setDevicestate(switchdevice, 0);
                    break;
                    case VariableOPENCLOSE :
                        if (DeviceON != 0)
                        {
                            if (DeviceON <= maxValue) setDevicestate(switchdevice, DeviceON);
                            else setDevicestate(switchdevice, maxValue);
                        }
                        else setDevicestate(switchdevice, 0);
                    break;
                    case ONOFF :
                        if (ui.ActiveLow->isChecked())
                        {
                            if ((DeviceON > 0) or (SwitchDelay > 0)) setDevicestate(switchdevice, 0);
                            else setDevicestate(switchdevice, maxValue);
                        }
                        else
                        {
                            if ((DeviceON > 0) or (SwitchDelay > 0)) setDevicestate(switchdevice, maxValue);
                            else setDevicestate(switchdevice, 0);
                        }
                    break;
                    case OPENCLOSE :
                    break;
                }
            }
        }
        else switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case VariableOPENCLOSE :
            case ONOFF :
                if (ui.ActiveLow->isChecked())
                {
                    if ((DeviceON > 0) or (SwitchDelay > 0)) setDevicestate(switchdevice, 0);
                    else if (logisdom::isNotNA(DeviceON)) setDevicestate(switchdevice, 1);
                }
                else
                {
                    if ((DeviceON > 0) or (SwitchDelay > 0)) setDevicestate(switchdevice, 1);
                    else if (logisdom::isNotNA(DeviceON)) setDevicestate(switchdevice, 0);
                }
            break;
            case OPENCLOSE :
                if (volet_status < DeviceON)
                {
                    volet_status++;
                    setDevicestate(closedevice, 0);
                    setDevicestate(opendevice, 1);
                    moving = true;
                }
                else if (volet_status > DeviceON)
                {
                    volet_status--;
                    setDevicestate(closedevice, 1);
                    setDevicestate(opendevice, 0);
                    moving = true;
                }
                else
                {
                    setDevicestate(closedevice, 0);
                    setDevicestate(opendevice, 0);
                    moving = false;
                }
            break;
        }
    }
    else if ((Disabled) && (!parent->isRemoteMode()))
    {
        switch (ui.comboBoxType->currentIndex())
        {
            case Variable :
            case VariableOPENCLOSE :
            break;
            case ONOFF :
            break;
            case OPENCLOSE :
                    setDevicestate(closedevice, 0);
                    setDevicestate(opendevice, 0);
                    moving = false;
            break;
        }
    }

// Set ui text
    QString str;
    switch (ui.comboBoxType->currentIndex())
    {
        case Variable :
            if (Disabled) { str = cstr::toStr(cstr::Disabled); if (logisdom::isNotNA(SwitchValue)) str += QString(" device value %1").arg(SwitchValue); }
            else if ((SwitchDelay > 0) and (DeviceON == 0)) str = cstr::toStr(cstr::ON) + QString(tr("   Time remaining : %1 minutes ").arg(SwitchDelay));
            else if ((DeviceON != 0) && (DeviceON != logisdom::NA)) {
                QString devVal = "device value ";
                if (logisdom::isNotNA(SwitchValue)) devVal += QString("%1").arg(SwitchValue);
                else devVal += cstr::toStr(cstr::NA);
                if (logisdom::isNA(maxValue)) str = QString("assign value : %1, " + devVal + ", no max value").arg(DeviceON);
                    else  if (!Disabled) str = QString("assign value : %1, " + devVal + ", Max value : %2").arg(DeviceON).arg(maxValue);
            }
            else if (DeviceON == logisdom::NA) str = cstr::toStr(cstr::NA);
            else if (maxTimeReached) str = cstr::toStr(cstr::OFF) + "  " + tr("Max time reached");
            else str = cstr::toStr(cstr::OFF);
        break;
        case VariableOPENCLOSE :
            if (Disabled) { str = cstr::toStr(cstr::Disabled); if (logisdom::isNotNA(SwitchValue)) str += QString(" device value %1").arg(SwitchValue); }
            else if ((SwitchDelay > 0) and (DeviceON == 0)) str = cstr::toStr(cstr::Open) + QString(tr("   Time remaining : %1 minutes ").arg(SwitchDelay));
            else if (int(SwitchValue) == int(maxValue)) { str = cstr::toStr(cstr::Open); }
            else if ((DeviceON != 0) && (DeviceON != logisdom::NA))
            {
                QString devVal = "device value ";
                if (logisdom::isNotNA(SwitchValue)) devVal += QString("%1").arg(SwitchValue);
                else devVal += cstr::toStr(cstr::NA);
                if (logisdom::isNA(maxValue)) str = QString("assign value : %1, " + devVal + ", no max value").arg(DeviceON);
                    else if (!Disabled) str = QString("assign value : %1, " + devVal + ", Max value : %2").arg(DeviceON).arg(maxValue);
            }
            else if (DeviceON == logisdom::NA) str = cstr::toStr(cstr::NA);
            else str = cstr::toStr(cstr::Close);
        break;
        case ONOFF :
            if (Disabled) str = cstr::toStr(cstr::Disabled);
            else if ((SwitchDelay > 0) and (DeviceON == 0)) str = cstr::toStr(cstr::ON) + QString(tr("   Time remaining : %1 minutes ").arg(SwitchDelay));
            else if (DeviceON > 0) str = cstr::toStr(cstr::ON);
            else if (maxTimeReached) str = cstr::toStr(cstr::OFF) + "  " + tr("Max time reached");
            else str = cstr::toStr(cstr::OFF);
        break;
        case OPENCLOSE :
            if (volet_status < DeviceON) str = QString(tr("is opening"));
            if (volet_status > DeviceON) str = QString(tr("is closing"));
            if (Disabled) str = cstr::toStr(cstr::Disabled);
            str += QString(" %1").arg(volet_status);
            str += QString(" / %1").arg(DeviceON);
        break;
        }
    displayStatus(str);
}



void SwitchControl::setDevicestate(onewiredevice *device, double state)
{
    if (!device) return;
    if (SwitchDeviceList.count() > 0)
        if (device == SwitchDeviceList.first()->device())
            foreach (devchooser *device, SwitchDeviceList)
                if (device)
                    setDevState(device->device(), state);
    if (CloseDeviceList.count() > 0)
        if (device == CloseDeviceList.first()->device())
            foreach (devchooser *device, CloseDeviceList)
                if (device)
                    setDevState(device->device(), state);
}



void SwitchControl::setDevState(onewiredevice *device, double state)
{
    if (!device) return;
    double devState = device->getMainValue();
    if (device->isDimmmable())
    {
        switch (ui.comboBoxType->currentIndex())
        {
            case VariableOPENCLOSE :
                if (logisdom::AreNotSame(devState, state))
                {
                    if (waitMove == 0)
                    {
                        waitMove = qAbs(int(devState - state));
                        if (waitMove < 5) waitMove = 5;
                        device->assignMainValue(state);
                    }
                }
                if (waitMove > 0) waitMove --;
            break;
            case ONOFF :
            case Variable :
            case OPENCLOSE :
                if (logisdom::AreNotSame(devState, state)) device->assignMainValue(state);
            break;
        }


    }
    else
    {
        if (int(state))
        {
            if (!int(devState)) device->set_On();
        }
        else
        {
            if (int(devState)) device->set_Off();
        }
    }
}



void SwitchControl::displayStatus(QString Text)
{
    SwitchStatus.setText(Status + " " + StatusText + " : " + Text);
    ui.labelStatus->setText(Status + " " + StatusText + " : " + Text);
	htmlBind->setName(ButtonName.text());
    if (!SwitchDeviceList.isEmpty())
    {
        if (SwitchDeviceList.first()) htmlBind->setParameter("Device RomID", SwitchDeviceList.first()->getRomID());
    }
    htmlBind->setParameter("Status", Text);
    htmlBind->setValue(Text);
}




void SwitchControl::clickSwitchOn()
{
	QDateTime now = QDateTime::currentDateTime();
	LastTurnOn = now;
    SwitchDelay = 0;
    SliderStatus.setValue(ui.spinBoxMaxTimeOn->value());
    ButtonGroup.setExclusive(true);
    if (moving && (volet_status > 2)) volet_status -= 4;
    updateStatus();
    emit(saveConfig());
}



void SwitchControl::clickSwitchOff()
{
	QDateTime now = QDateTime::currentDateTime();
	LastTurnOn = now;
    SwitchDelay = 0;
    SliderStatus.setValue(0);
    ButtonGroup.setExclusive(true);
    if (moving && (volet_status < ui.spinBoxMaxTimeOn->value())) volet_status += 4;
    updateStatus();
    emit(saveConfig());
}



void SwitchControl::clickAuto()
{
    ButtonGroup.setExclusive(true);
    updateStatus();
    emit(saveConfig());
}



void SwitchControl::clickDisabled()
{
    ButtonGroup.setExclusive(true);
    emit(saveConfig());
}



void SwitchControl::sliderActive()
{
    ButtonGroup.setExclusive(false);
    ButtonAuto.setChecked(false);
    ButtonOn.setChecked(false);
    ButtonOff.setChecked(false);
    ButtonDisabled.setChecked(false);
}



void SwitchControl::sliderReleased()
{
    sliderHasMoved = true;
    sliderActive();
}



void SwitchControl::updateJourneyBreaks(ProgramData*)
{
    dailyprog->updateList();
}



void SwitchControl::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
/*		QMenu contextualmenu;
		QAction ActionAdd(tr("Add trigger"), this);
		QAction ActionRemove(tr("Remove trigger"), this);
		if (parent->isRemoteMode())
		{
			ActionAdd.setEnabled(false);
			ActionRemove.setEnabled(false);
		}
        //QAction *selection;
		if (!parent->isRemoteMode())
		{
			contextualmenu.addAction(&ActionAdd);
			contextualmenu.addAction(&ActionRemove);
		}
        contextualmenu.exec(event->globalPos());*/
//		if (selection == &ActionAdd) addTrigger("");
//		if (selection == &ActionRemove) removeTrigger("");
	}
    if (event->button() == Qt::LeftButton)
	{
		emit(setupClick(&setup));
    }
}


void SwitchControl::Del()
{
    emit(deleteRequest(this));
}


void SwitchControl::showDelMe()
{
    //unsigned int state = QApplication::keyboardModifiers();
    //if (state & Qt::ControlModifier) emit(deleteRequest(this));
    //else
    emit(setupClick(&setup));
}




void SwitchControl::addDevice()
{
    if (lockStatus)
   {
        messageBox::warningHide(this, tr("Locked"), tr("Please unlock first"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
   }
    newSwitchDevice();
    newCloseDevice();
    int mode = ui.comboBoxType->currentIndex();
    if (mode != -1) modeChanged(mode);
}


void SwitchControl::enableCloseDevices()
{
    foreach (devchooser *CloseDevice, CloseDeviceList)
    {
        CloseDevice->show();
        CloseDevice->setEnabled(true);
        CloseDevice->setToolTip(tr("Device for closing"));
    }
}



void SwitchControl::disableCloseDevices()
{
    foreach (devchooser *CloseDevice, CloseDeviceList)
    {
        CloseDevice->setEnabled(false);
        CloseDevice->hide();
        CloseDevice->setToolTip(tr("Device not used"));
    }
}



void SwitchControl::modeChanged(int index)
{
	switch (index)
	{
        case VariableOPENCLOSE : dailyprog->setMode(daily::VariableOpenClose);
            disableCloseDevices();
            ui.ActiveLow->hide();
            foreach (devchooser *SwitchDevice, SwitchDeviceList) SwitchDevice->setToolTip(tr("Device to open and close"));
            ButtonOn.setText(OpenModeTxt);
            ButtonOff.setText(CloseModeTxt);
            ui.labelMaxTimeOn->setText(tr("Travel Time") + " : ");
            ui.spinBoxMaxTimeOn->setSuffix(" " + tr("sec"));
            ui.spinBoxMaxTimeOn->setRange(1, 999);
            ui.spinBoxMaxTimeOn->setSpecialValueText("");
            htmlBind->addParameterCommand(ModeTxt, OpenModeTxt, setModeOpen);
            htmlBind->addParameterCommand(ModeTxt, CloseModeTxt, setModeClose);
            htmlBind->delParameterCommand(ModeTxt, OnModeTxt);
            htmlBind->delParameterCommand(ModeTxt, OffModeTxt);
            SliderStatus.show();
    break;
        case Variable : dailyprog->setMode(daily::Variable);
            disableCloseDevices();
            ui.ActiveLow->hide();
            foreach (devchooser *SwitchDevice, SwitchDeviceList) if (SwitchDevice) SwitchDevice->setToolTip(tr("Device to control"));
            ButtonOn.setText(OnModeTxt);
            ButtonOff.setText(OffModeTxt);
            ui.labelMaxTimeOn->setText(tr("Max Time ON") + " : ");
            ui.spinBoxMaxTimeOn->setSuffix(" " + tr("mn"));
            ui.spinBoxMaxTimeOn->setRange(1, 999);
            ui.spinBoxMaxTimeOn->setSpecialValueText(tr("Max Time ON Disabled"));
            htmlBind->addParameterCommand(ModeTxt, OnModeTxt, setModeOn);
            htmlBind->addParameterCommand(ModeTxt, OffModeTxt, setModeOff);
            htmlBind->delParameterCommand(ModeTxt, OpenModeTxt);
            htmlBind->delParameterCommand(ModeTxt, CloseModeTxt);
            SliderStatus.show();
        break;
        case ONOFF : dailyprog->setMode(daily::ONOFF);
            disableCloseDevices();
            ui.ActiveLow->show();
            foreach (devchooser *SwitchDevice, SwitchDeviceList) if (SwitchDevice) SwitchDevice->setToolTip(tr("Device ON/OFF"));
            ButtonOn.setText(OnModeTxt);
            ButtonOff.setText(OffModeTxt);
            ui.labelMaxTimeOn->setText(tr("Max Time ON") + " : ");
            ui.spinBoxMaxTimeOn->setSuffix(" " + tr("mn"));
            ui.spinBoxMaxTimeOn->setRange(1, 999);
            ui.spinBoxMaxTimeOn->setSpecialValueText(tr("Max Time ON Disabled"));
            htmlBind->addParameterCommand(ModeTxt, OnModeTxt, setModeOn);
            htmlBind->addParameterCommand(ModeTxt, OffModeTxt, setModeOff);
            htmlBind->delParameterCommand(ModeTxt, OpenModeTxt);
            htmlBind->delParameterCommand(ModeTxt, CloseModeTxt);
            SliderStatus.hide();
        break;
        case OPENCLOSE : dailyprog->setMode(daily::OpenClose);
            enableCloseDevices();
            ui.ActiveLow->hide();
            foreach (devchooser *SwitchDevice, SwitchDeviceList) if (SwitchDevice) SwitchDevice->setToolTip(tr("Device for opening"));
            foreach (devchooser *CloseDevice, CloseDeviceList) if (CloseDevice) CloseDevice->setToolTip(tr("Device for closing"));
            ButtonOn.setText(OpenModeTxt);
            ButtonOff.setText(CloseModeTxt);
            ui.labelMaxTimeOn->setText(tr("Travel Time") + " : ");
            ui.spinBoxMaxTimeOn->setSuffix(" " + tr("sec"));
            ui.spinBoxMaxTimeOn->setRange(1, 999);
            ui.spinBoxMaxTimeOn->setSpecialValueText("");
            htmlBind->addParameterCommand(ModeTxt, OpenModeTxt, setModeOpen);
            htmlBind->addParameterCommand(ModeTxt, CloseModeTxt, setModeClose);
            htmlBind->delParameterCommand(ModeTxt, OnModeTxt);
            htmlBind->delParameterCommand(ModeTxt, OffModeTxt);
            SliderStatus.show();
        break;
	}
}




void SwitchControl::SaveConfigStr(QString &str)
{
	str += Switch_Begin_Mark "\n";
    for (int n=0; n<SwitchDeviceList.count(); n++)
    {
        if (SwitchDeviceList.at(n))
        {
            str += logisdom::saveformat("SwitchRomID", SwitchDeviceList.at(n)->getRomID());
            if (CloseDeviceList.at(n)) str += logisdom::saveformat("CloseRomID", CloseDeviceList.at(n)->getRomID());
        }
    }
    str += logisdom::saveformat("Mode", QString("%1").arg(ui.comboBoxType->currentIndex()));
    str += logisdom::saveformat("MaxTimeON", QString("%1").arg(ui.spinBoxMaxTimeOn->value()));
    if (ButtonGroup.exclusive())
    {
        str += logisdom::saveformat("SelectionName", ButtonGroup.checkedButton()->text());
        if (ButtonGroup.exclusive()) str += logisdom::saveformat("SelectionID", QString("%1").arg(ButtonGroup.checkedId()));
    }
    else str += logisdom::saveformat("SliderValue", QString("%1").arg(SliderStatus.value()));
    str += logisdom::saveformat("volet_status", QString("%1").arg(volet_status));
    str += logisdom::saveformat("LogicalAND", QString("%1").arg(ui.logicalAND->isChecked()));
    str += logisdom::saveformat("ActiveLow", QString("%1").arg(ui.ActiveLow->isChecked()));
    for (int n=0; n<triggerList.count(); n++) triggerList[n]->SaveConfigStr(str);
	dailyprog->getStrConfig(str);
	str += Switch_End_Mark "\n";
}


devchooser *SwitchControl::newSwitchDevice()
{
    QMutexLocker locker(&mutexConfig);
    devchooser *SwitchDevice = nullptr;
    SwitchDevice = new devchooser(parent);
    if (SwitchDevice)
    {
        SwitchDeviceList.append(SwitchDevice);
        addFamilies(SwitchDevice);
        ui.gridLayoutDevChooser->addWidget(SwitchDevice, SwitchDeviceList.count(), 0, 1, logisdom::PaletteWidth/2);
        connect(SwitchDevice, SIGNAL(deviceSelectionChanged()), this, SLOT(deviceSelectionChanged()));
        connect(SwitchDevice, SIGNAL(deleteRequest(devchooser*)), this, SLOT(deleteMe(devchooser*)));
        SwitchDevice->setHtmlBindControler(htmlBind);
        SwitchDevice->Lock(lockStatus);
        //qDebug() << SwitchDevice->getRomID();
        //if (!htmlBind) qDebug() << "nullptr";
    }
    return SwitchDevice;
    //QLabel *trash = new QLabel();
    //trash->setPixmap(QPixmap(QString::fromUtf8(":/images/images/deletetrash.png")));
    //ui.gridLayoutDevChooser->addWidget(trash, count+1, 3, 1, logisdom::PaletteWidth/2);
    //connect(trash, SIGNAL(clicked()), this, SLOT(clickSwitchOn()));

}



devchooser *SwitchControl::newCloseDevice()
{
    QMutexLocker locker(&mutexConfig);
    devchooser *CloseDevice = nullptr;
    {
        CloseDevice = new devchooser(parent);
        if (CloseDevice)
        {
            CloseDeviceList.append(CloseDevice);
            addFamilies(CloseDevice);
            ui.gridLayoutDevChooser->addWidget(CloseDevice, CloseDeviceList.count(), logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
            connect(CloseDevice, SIGNAL(deviceSelectionChanged()), this, SLOT(deviceSelectionChanged()));
            CloseDevice->setHtmlBindControler(htmlBind);
        }
    }
    return CloseDevice;
}



void SwitchControl::createSwitchDevices(QString &data)
{
    bool firstSwitch = true;
    bool firstClose = true;
    QStringList lines = data.split("\n");
    foreach (QString line, lines)
    {
        QString SwitchRomID = logisdom::getvalue("SwitchRomID", line);
        if (!SwitchRomID.isEmpty())
        {
            if (SwitchDeviceList.count() > CloseDeviceList.count()) newCloseDevice();
            devchooser *SwitchDevice = nullptr;
            if (firstSwitch) SwitchDevice = SwitchDeviceList.first();
            else SwitchDevice = newSwitchDevice();
            SwitchDevice->setRomID(SwitchRomID);
            firstSwitch = false;
        }

        QString CloseRomID = logisdom::getvalue("CloseRomID", line);
        if (!CloseRomID.isEmpty())
        {
            devchooser *CloseDevice = nullptr;
            if (firstClose) CloseDevice = CloseDeviceList.first();
            else CloseDevice = newCloseDevice();
            CloseDevice->setRomID(CloseRomID);
            firstClose = false;
        }
    }
    if (SwitchDeviceList.count() > CloseDeviceList.count()) newCloseDevice();
}



void SwitchControl::readconfigfile(QString &data)
{
	int TMax, Mode, ID;
	bool ok;
    createSwitchDevices(data);
    TMax = logisdom::getvalue("MaxTimeON", data).toInt(&ok, 10);
    if (ok) ui.spinBoxMaxTimeOn->setValue(TMax);
	Mode = logisdom::getvalue("Mode", data).toInt(&ok, 10);
    if (ok)
    {
        ui.comboBoxType->setCurrentIndex(Mode);
        modeChanged(Mode);
    }
	ID = logisdom::getvalue("SelectionID", data).toInt(&ok, 10);
    if (ok) {
		switch (ID)
		{
            case ID_Auto : ButtonAuto.setChecked(true); clickAuto(); break;
            case ID_On : ButtonOn.setChecked(true); clickSwitchOn(); break;
            case ID_Off : ButtonOff.setChecked(true); clickSwitchOff(); break;
        case ID_Disabled : ButtonDisabled.setChecked(true); clickDisabled(); break;
		}
    }
    else
    {
        sliderReleased();
        int slider = logisdom::getvalue("SliderValue", data).toInt(&ok, 10);
        if (ok) SliderStatus.setValue(slider);
    }
    int volet = logisdom::getvalue("volet_status", data).toInt(&ok, 10);
    if (ok) volet_status = volet;
    int activLow = logisdom::getvalue("ActiveLow", data).toInt(&ok, 10);
    if (activLow) ui.ActiveLow->setChecked(true);
    int Logical = logisdom::getvalue("LogicalAND", data).toInt(&ok, 10);
    if (Logical) ui.logicalAND->setChecked(true); else ui.logicalOR->setChecked(true);
    dailyprog->setStrConfig(data);
	QByteArray configdata;
	configdata.append(data);
	QString TAG_Begin = New_Trigger_Begin;
	QString TAG_End = New_Trigger_End;
	SearchLoopBegin
	if (!strsearch.isEmpty())
	{
	    QString N = logisdom::getvalue("FormulaName", strsearch);
	    SwitchThreshold *newTrigger = addTrigger(N);
	    if (newTrigger) newTrigger->readconfigfile(strsearch);
    }
	SearchLoopEnd
    makeConnections();
}



void SwitchControl::makeConnections()
{
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(clickSwitchOn()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(clickSwitchOff()));
    connect(&ButtonAuto, SIGNAL(clicked()), this, SLOT(clickAuto()));
    connect(&ButtonDisabled, SIGNAL(clicked()), this, SLOT(clickDisabled()));
    connect(&ButtonName, SIGNAL(clicked()), this, SLOT(showDelMe()));
    connect(ui.comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
    connect(ui.ActiveLow, SIGNAL(clicked(bool)), this, SLOT(updateStatus()));
}
