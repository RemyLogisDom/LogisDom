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
#include "server.h"
#include "eoA502xx.h"




eoA502xx::eoA502xx(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    LastRadioMessage.setText("Last message : N/A");
    setupLayout.addWidget(&LastRadioMessage, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    lastsavevalue = logisdom::NA;
	romid = RomID;
    family = romid.right(8);
	ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("EnOcean ") + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
	if (master) ui.labelmaster->setText(master->getipaddress());
	MainValue = logisdom::NA;
    saveInterval.setToAuto();
    logEnabled.hide();
    WarnEnabled.hide();
    UsetextValues.hide();
    TextValues.hide();
    timeOut.setRange(5, 9999);
    timeOut.setValue(60);
    timeOut.setPrefix(tr("Time Out : "));
    timeOut.setSuffix(tr(" minutes"));
    setupLayout.addWidget(&timeOut, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&checkLastMessage, SIGNAL(timeout()), this, SLOT(checkElaspedTime()));
    checkLastMessage.start(60000);
}




void eoA502xx::checkElaspedTime()
{
    if (lastMessage.hasExpired(timeOut.value() * 60000) || (!lastMessage.isValid()))
    {
        RetryWarning = 3;
        setTraffic(dataNotValid);
        setMainValue(logisdom::NA, false);
    }
}


void eoA502xx::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Nom(tr("&Name"), this);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Nom) changename();
}





bool eoA502xx::setscratchpad(const QString &V, bool)
{
    if ((lastMessage.elapsed() < 1000) && (V == lastScratchpad)) return true;   // in case of radio received from multiple EnOcean master
    ScratchPad_Show.setText(V);
    bool ok;
    logMsg(V);
    int rawValue = V.mid(4, 2).toInt(&ok, 16);
    float rangeMin = 0;
    float rangeMax = 0;
    float scaleMin = 0;
    float scaleMax = 0;
    int EEP = romid.mid(12,2).toInt(&ok, 16);
    //qDebug() << romid + " " + QString("%1").arg(EEP);
    if (!ok) { setTraffic(dataNotValid); return false; }
    switch (EEP)
    {
        case 0x00 : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 0; break;
        case 0x01 : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 0; break;
        case 0x02 : rangeMin = 255; rangeMax = 0; scaleMin = -30; scaleMax = 10; break;
        case 0x03 : rangeMin = 255; rangeMax = 0; scaleMin = -20; scaleMax = 20; break;
        case 0x04 : rangeMin = 255; rangeMax = 0; scaleMin = -10; scaleMax = 30; break;
        case 0x05 : rangeMin = 255; rangeMax = 0; scaleMin = 0; scaleMax = 40; break;
        case 0x06 : rangeMin = 255; rangeMax = 0; scaleMin = 10; scaleMax = 50; break;
        case 0x07 : rangeMin = 255; rangeMax = 0; scaleMin = 20; scaleMax = 60; break;
        case 0x08 : rangeMin = 255; rangeMax = 0; scaleMin = 30; scaleMax = 70; break;
        case 0x09 : rangeMin = 255; rangeMax = 0; scaleMin = 40; scaleMax = 80; break;
        case 0x0A : rangeMin = 255; rangeMax = 0; scaleMin = 50; scaleMax = 90; break;
        case 0x0B : rangeMin = 255; rangeMax = 0; scaleMin = 60; scaleMax = 100; break;
        case 0x0C : rangeMin = 255; rangeMax = 0; scaleMin = -60; scaleMax = 20; break;
        case 0x0D : rangeMin = 255; rangeMax = 0; scaleMin = -50; scaleMax = 30; break;
        case 0x0E : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 40; break;
        case 0x0F : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 0; break;
        case 0x10 : rangeMin = 255; rangeMax = 0; scaleMin = -60; scaleMax = 20; break;
        case 0x11 : rangeMin = 255; rangeMax = 0; scaleMin = -50; scaleMax = 30; break;
        case 0x12 : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 40; break;
        case 0x13 : rangeMin = 255; rangeMax = 0; scaleMin = -30; scaleMax = 50; break;
        case 0x14 : rangeMin = 255; rangeMax = 0; scaleMin = -20; scaleMax = 60; break;
        case 0x15 : rangeMin = 255; rangeMax = 0; scaleMin = -10; scaleMax = 70; break;
        case 0x16 : rangeMin = 255; rangeMax = 0; scaleMin = 0; scaleMax = 80; break;
        case 0x17 : rangeMin = 255; rangeMax = 0; scaleMin = 10; scaleMax = 90; break;
        case 0x18 : rangeMin = 255; rangeMax = 0; scaleMin = 20; scaleMax = 100; break;
        case 0x19 : rangeMin = 255; rangeMax = 0; scaleMin = 30; scaleMax = 110; break;
        case 0x1A : rangeMin = 255; rangeMax = 0; scaleMin = 40; scaleMax = 120; break;
        case 0x1B : rangeMin = 255; rangeMax = 0; scaleMin = 50; scaleMax = 130; break;
        case 0x1C :
        case 0x1D :
        case 0x1E :
        case 0x1F :
        case 0x21 :
        case 0x22 :
        case 0x23 :
        case 0x24 :
        case 0x25 :
        case 0x26 :
        case 0x27 :
        case 0x28 :
        case 0x29 :
        case 0x2A :
        case 0x2B :
        case 0x2C :
        case 0x2E :
        case 0x2F : rangeMin = 255; rangeMax = 0; scaleMin = -40; scaleMax = 0; break;
    default: setTraffic(dataNotValid); return false;
    };
    float q = (scaleMax - scaleMin) / (rangeMax - rangeMin);
    float outValue = q * (float(rawValue) - rangeMin) + scaleMin;
    setMainValue(double(outValue), saveInterval.isEnabled());
    ClearWarning();
    lastScratchpad = V;
    QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
    setWindowTitle(name + " " + T);
    LastRadioMessage.setText("Last message : " + T);
    lastMessage.restart();
    return true;
}



QString eoA502xx::getSecondaryValue()
{
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


void eoA502xx::setconfig(const QString &strsearch)
{
    //qDebug() << strsearch;
    QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(tr("T° ")));
    bool ok;
    int T = logisdom::getvalue("TimeOut", strsearch).toInt(&ok);
    if (ok) timeOut.setValue(T);
}



void eoA502xx::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("TimeOut", QString("%1").arg(timeOut.value()));
}




QString eoA502xx::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}


