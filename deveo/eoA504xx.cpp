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
#include "eoA504xx.h"




eoA504xx::eoA504xx(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    LastRadioMessage.setText("Last message : N/A");
    setupLayout.addWidget(&LastRadioMessage, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    lastsavevalue = logisdom::NA;
    romid = RomID;
    family = romid.right(10);
    ui.labelromid->setText("RomID : " + romid);
    if (family.right(4) == QString(familyeoA504XX_T).right(4)) ui.labelfamily->setText(tr("EnOcean Temperature") + " " + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
    else if (family.right(4) == QString(familyeoA504XX_H).right(4)) ui.labelfamily->setText(tr("EnOcean Humidity") + " " + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
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



void eoA504xx::checkElaspedTime()
{
    if (lastMessage.hasExpired(timeOut.value() * 60000) || (!lastMessage.isValid()))
    {
        RetryWarning = 3;
        setTraffic(dataNotValid);
        setMainValue(logisdom::NA, false);
    }
}


void eoA504xx::contextMenuEvent(QContextMenuEvent * event)
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





bool eoA504xx::setscratchpad(const QString &V, bool)
{
    ScratchPad_Show.setText(V);
    //qDebug() << family;
    bool ok;
    logMsg(V);
    float outValue = logisdom::NA;
    //qDebug() << romid + " " + QString("%1").arg(EEP);
    if ((family.left(4) == QString(familyeoA504XX_T).left(4)) && (family.right(4) == QString(familyeoA504XX_T).right(4)))
    {
        int rawValue = V.mid(4, 2).toInt(&ok, 16);
        float rangeMin = 0;
        float rangeMax = 0;
        float scaleMin = 0;
        float scaleMax = 0;
        int EEP = romid.mid(12,2).toInt(&ok, 16);
        if (!ok) { setTraffic(dataNotValid); return false; }
        switch (EEP)
        {
            case 0x00 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 40; break;
            case 0x01 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 40; break;
            case 0x02 : rangeMin = 0; rangeMax = 250; scaleMin = -20; scaleMax = 60; break;
            case 0x03 : rangeMin = 0; rangeMax = 250; scaleMin = -20; scaleMax = 60; rawValue = V.mid(2, 4).toInt(&ok, 16); break;
        default: setTraffic(dataNotValid); return false;
        };
        float q = (scaleMax - scaleMin) / (rangeMax - rangeMin);
        outValue = q * (float(rawValue) - rangeMin) + scaleMin;
    }
    if ((family.left(4) == QString(familyeoA504XX_H).left(4)) && (family.right(4) == QString(familyeoA504XX_H).right(4)))
    {
        int rawValue = V.mid(2, 2).toInt(&ok, 16);
        float rangeMin = 0;
        float rangeMax = 0;
        float scaleMin = 0;
        float scaleMax = 0;
        int EEP = romid.mid(12,2).toInt(&ok, 16);
        if (!ok) { setTraffic(dataNotValid); return false; }
        switch (EEP)
        {
            case 0x00 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 100; break;
            case 0x01 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 100; break;
            case 0x02 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 100; break;
            case 0x03 : rangeMin = 0; rangeMax = 250; scaleMin = 0; scaleMax = 100; rawValue = V.mid(0, 2).toInt(&ok, 16); break;
        default: setTraffic(dataNotValid); return false;
        };
        float q = (scaleMax - scaleMin) / (rangeMax - rangeMin);
        outValue = q * (float(rawValue) - rangeMin) + scaleMin;
    }
    setMainValue(double(outValue), saveInterval.isEnabled());
    ClearWarning();
    lastScratchpad = V;
    QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
    setWindowTitle(name + " " + T);
    LastRadioMessage.setText("Last message : " + T);
    lastMessage.restart();
    return true;
}


QString eoA504xx::getSecondaryValue()
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


void eoA504xx::setconfig(const QString &strsearch)
{
    QString Name = logisdom::getvalue("Name", strsearch);
    if (!Name.isEmpty()) setname(assignname(Name));
    else if (family.right(4) == QString(familyeoA504XX_T).right(4) && family.left(4) == QString(familyeoA504XX_T).left(4)) setname(assignname(tr("Temperature_EO")));
    else if (family.right(4) == QString(familyeoA504XX_H).right(4) && family.left(4) == QString(familyeoA504XX_H).left(4)) setname(assignname(tr("Humidity_EO")));
    bool ok;
    int T = logisdom::getvalue("TimeOut", strsearch).toInt(&ok);
    if (ok) timeOut.setValue(T);
}



void eoA504xx::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("TimeOut", QString("%1").arg(timeOut.value()));
}



QString eoA504xx::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}
