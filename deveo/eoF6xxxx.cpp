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
#include "server.h"
#include "net1wire.h"
#include "eoF6xxxx.h"




eoF6XXXX::eoF6XXXX(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    Decimal.hide();
    LastRadioMessage.setText("Last message : N/A");
    setupLayout.addWidget(&LastRadioMessage, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&SwitchType, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&delai, layoutIndex++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth);
    //delai.setEnabled(false);
    delai.setRange(1, 65535);
    delai.setSingleStep(10);
    delai.setValue(60);
    delai.setSuffix(" Sec");
    delaiTimer.setSingleShot(true);
    connect(&delaiTimer, SIGNAL(timeout()), this, SLOT(TimerFinished()));
    SwitchType.addItem(tr("Switch"));
    SwitchType.addItem(tr("Delay On"));
    SwitchType.addItem(tr("Delay Off"));
    SwitchType.addItem(tr("Toggle"));
    SwitchType.addItem(tr("Value"));
    connect(&SwitchType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeType(int)));
    ButtonOn.setText(cstr::toStr(cstr::ON));
    ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&ButtonOff, layoutIndex, 1, 1, 1);
    OnValue.setPrefix("On : ");
    OnValue.setRange(1, 99999);
    OnValue.setValue(100);
    OnValue.setToolTip(tr("On Value"));
    setupLayout.addWidget(&OnValue, layoutIndex++, 2, 1, 1);
    StringValue.setText(tr("Show result as string"));
    //setupLayout.addWidget(&StringValue, layoutIndex++, 0, 1, 1);
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(set_On()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(set_Off()));
    lastsavevalue = logisdom::NA;
	romid = RomID;
    family = romid.right(8);
	ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("EnOcean ") + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
	if (master) ui.labelmaster->setText(master->getipaddress());
    htmlBind->addCommand(cstr::toStr(cstr::ON), SwitchON);
    htmlBind->addCommand(cstr::toStr(cstr::OFF), SwitchOFF);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), SwitchON);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), SwitchOFF);
    MainValue = logisdom::NA;
    saveInterval.setToAuto();
    logEnabled.hide();
    WarnEnabled.hide();
    UsetextValues.hide();
    TextValues.hide();
    Unit.setText("");
    if (master)
    {
        if (!parent->isRemoteMode())
        {
            QString order = master->getFifoString("D20300", romid.left(8), "");
            master->addtofifo(order);
        }
    }
}



void eoF6XXXX::changeType(int index)
{
    switch (index)
    {
        case Switch :
        {
        } break;
        case DelaiOn :
        {
            MainValue = 0;
        } break;
        case DelaiOff :
        {
            MainValue = OnValue.value();
        } break;
        case Toggle :
        {
        } break;
        case Value :
        {
        } break;
    }
    htmlBind->setValue(MainValueToStr());
    htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
    savevalue(QDateTime::currentDateTime(), MainValue);
    ClearWarning();
    MainValueToStr();
    setValid(dataValid);
    emitDeviceValueChanged();
    QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
    setWindowTitle(name + " " + T);
    LastRadioMessage.setText("Last message : " + T);
}


void eoF6XXXX::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Nom(tr("&Name"), this);
    QAction ActionON(cstr::toStr(cstr::ON), this);
    QAction ActionOFF(cstr::toStr(cstr::OFF), this);
    if (!parent->isRemoteMode())
    {
        contextualmenu.addAction(&Nom);
        contextualmenu.addAction(&ActionON);
        contextualmenu.addAction(&ActionOFF);
    }
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Nom) changename();
    if (selection == &ActionON) set_On();
    if (selection == &ActionOFF) set_Off();
}


QString eoF6XXXX::getSecondaryValue()
{
/*    qint64 t = lastMessage.elapsed() / 1000;
    if (t < 60) secondaryValue = "< 1 mn";
    else if (t < 3600) secondaryValue = QString("%1 mn").arg(t/60);
    else secondaryValue = QString("%1 h").arg(t/3600);
    return secondaryValue;*/
    return "";
}


bool eoF6XXXX::setscratchpad(const QString &V, bool enregistremode)
{
    if (V == "00") return true;
    ScratchPad_Show.setText(V);
    bool ok;
    logMsg(V);
    int cmd = V.toInt(&ok, 16);
    int v = logisdom::NA;
    if (ok) switch (SwitchType.currentIndex())
    {
        case Switch :
        {
            if (cmd == 0x30) v = OnValue.value();
            if (cmd == 0x10) v = 0;
        } break;
        case DelaiOn :
        {
            if (cmd == 0x30)
            {
                v = OnValue.value();
                delaiTimer.start(delai.value() * 1000);
            }
            if (cmd == 0x10)
            {
                v = 0;
                delaiTimer.stop();
            }
        } break;
        case DelaiOff :
        {
            if (cmd == 0x30)
            {
                v = 0;
                delaiTimer.start(delai.value() * 1000);
            }
            if (cmd == 0x10)
            {
                v = OnValue.value();
                delaiTimer.stop();
            }
        } break;
        case Toggle :
        {
            if (logisdom::isNotZero(MainValue)) v = 0; else v = OnValue.value();
        } break;
        case Value :
        {
            if (cmd == 0x10) v = 0;
            if (cmd == 0x30) v = 1 * OnValue.value();
            if (cmd == 0x50) v = 2 * OnValue.value();
            if (cmd == 0x70) v = 3 * OnValue.value();
        } break;
    }
    if (v != logisdom::NA)
    {
        MainValue = v;
        htmlBind->setValue(MainValueToStr());
        htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
        if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
        ClearWarning();
        MainValueToStr();
        setValid(dataValid);
        emitDeviceValueChanged();
        QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
        setWindowTitle(name + " " + T);
        LastRadioMessage.setText("Last message : " + T);
        return true;
    }
    return false;
}



void eoF6XXXX::TimerFinished()
{
    delaiTimer.stop();
    switch (SwitchType.currentIndex())
    {
        case DelaiOn :
        {
            MainValue = 0;
        } break;
        case DelaiOff :
        {
            MainValue = OnValue.value();
        } break;
        case Toggle :
        {
        } break;
        case Value :
        {
        } break;
    }
    htmlBind->setValue(MainValueToStr());
    htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
    savevalue(QDateTime::currentDateTime(), MainValue);
    ClearWarning();
    MainValueToStr();
    setValid(dataValid);
    emitDeviceValueChanged();
    QString T = tr("Delay finished") + " " + QDateTime::currentDateTime().toString("ddd HH:mm:ss");
    setWindowTitle(name + " " + T);
    LastRadioMessage.setText(T);
}

void eoF6XXXX::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("SwitchType", QString("%1").arg(SwitchType.currentIndex()));
    str += logisdom::saveformat("Delay", QString("%1").arg(delai.value()));
    str += logisdom::saveformat("OnValue", QString("%1").arg(OnValue.value()));
}


void eoF6XXXX::setconfig(const QString &strsearch)
{
    //qDebug() << strsearch;
    QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(tr("Switch ")));
    bool ok;
    int type = logisdom::getvalue("SwitchType", strsearch).toInt(&ok);
    if (ok) SwitchType.setCurrentIndex(type);
    int d = logisdom::getvalue("Delay", strsearch).toInt(&ok);
    if (ok) delai.setValue(d);
    int v = logisdom::getvalue("OnValue", strsearch).toInt(&ok);
    if (ok) OnValue.setValue(v);
}



void eoF6XXXX::remoteCommandExtra(const QString &command)
{
    if (command == SwitchON) set_On();
    if (command == SwitchOFF) set_Off();
}


QString eoF6XXXX::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}



void eoF6XXXX::On(bool)
{
    if (!master) return;
    setscratchpad("30", true);
}





void eoF6XXXX::Off(bool)
{
    if (!master) return;
    setscratchpad("10", true);
}



bool eoF6XXXX::isSwitchFamily()
{
    return true;
}

