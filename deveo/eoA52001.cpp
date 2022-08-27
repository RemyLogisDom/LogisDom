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
#include "eoA52001.h"




eoA52001::eoA52001(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    LastRadioMessage.setText("Last message : N/A");
    setupLayout.addWidget(&LastRadioMessage, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    Decimal.hide();
    UsetextValues.hide();
    TextValues.hide();
    lastsavevalue = logisdom::NA;
	romid = RomID;
    family = romid.right(8);
	ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("EnOcean ") + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
    if (master) ui.labelmaster->setText(master->getipaddress());
	MainValue = logisdom::NA;
    setPoint.setRange(0, 100);
    setPoint.setPrefix(tr("Set Point : "));
    setupLayout.addWidget(&setPoint, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&setPoint, SIGNAL(valueChanged(int)), this, SLOT(setPointChanged(int)));
    saveInterval.setToAuto();
    logEnabled.hide();
    WarnEnabled.hide();

    Temperature.setText("Temperature : ...");
    setupLayout.addWidget(&Temperature, layoutIndex++, 0, 1, logisdom::PaletteWidth);            // DB2.7

    Energy_Input_Enabled.setText("Energy Input ...");
    setupLayout.addWidget(&Energy_Input_Enabled, layoutIndex++, 0, 1, logisdom::PaletteWidth);// DB2.6

    Energy_Storage_OK.setText("Energy Storage ...");
    setupLayout.addWidget(&Energy_Storage_OK, layoutIndex++, 0, 1, logisdom::PaletteWidth);// DB2.5

    Battery_Capacity_OK.setText("Battery Capacity ...");
    setupLayout.addWidget(&Battery_Capacity_OK, layoutIndex++, 0, 1, logisdom::PaletteWidth);// DB2.4

    Temperatue_Sensor_Error.setText("Temperature Sensor ...");
    setupLayout.addWidget(&Temperatue_Sensor_Error, layoutIndex++, 0, 1, logisdom::PaletteWidth);// DB2.2

    Motor_Failure.setText("Actuator Obsctructed ...");
    setupLayout.addWidget(&Motor_Failure, layoutIndex++, 0, 1, logisdom::PaletteWidth);// DB2.0

    Summer_Bit.setText("Summer Bit");
    setupLayout.addWidget(&Summer_Bit, layoutIndex++, 0, 1, logisdom::PaletteWidth);  // DB1.3
    Set_Point_Selection.setText("set Point Selection");
    setupLayout.addWidget(&Set_Point_Selection, layoutIndex++, 0, 1, logisdom::PaletteWidth);  // DB1.2

    timeOut.setRange(5, 9999);
    timeOut.setValue(120);
    timeOut.setPrefix(tr("Time Out : "));
    timeOut.setSuffix(tr(" minutes"));
    setupLayout.addWidget(&timeOut, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&checkLastMessage, SIGNAL(timeout()), this, SLOT(checkElaspedTime()));
    checkLastMessage.start(60000);

    connect(&Summer_Bit, SIGNAL(clicked()), this, SLOT(updateDB1()));
    connect(&Set_Point_Selection, SIGNAL(clicked()), this, SLOT(updateDB1()));
    master->writeScratchPad(romid, "");
}




void eoA52001::updateDB1()
{
    int D = 0;
    if (Summer_Bit.isChecked()) D += 0x08;
    if (Set_Point_Selection.isChecked()) D += 0x04;
    QString data_Str;
    data_Str += QString("%1").arg(setPoint.value(), 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(D, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    //qDebug() << data_Str;
    master->writeScratchPad(romid, data_Str);
}



void eoA52001::setPointChanged(int v)
{
    QString data_Str;
    data_Str += QString("%1").arg(v, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0x08, 2, 16, QLatin1Char('0')).toUpper();
    master->writeScratchPad(romid, data_Str);
/*  VALVE POS: Example in HEX "0x05 0x77 0x00 0x08"
 DB3.7...DB3.0 = 0x05 = 5: new valve position is 5%
 DB2.7...DB2.0 = 0x77 = 119: room temperature = 255 - 119 = 136 => 40 * 136 / 255 = 21,3 °C
 DB1.7...DB1.0 = 0x00:
o DB1.3 = 0: regular default radio cycle (no summer mode)
o DB1.2 = 0: DB3.7...DB3.0 is set to valve position %
DB0.7...DB0.0 = 0x08: Data telegram

SET_TEMP: Example in HEX "0x80 0x81 0x04 0x08"
 DB3.7...DB3.0 = 0x80 = 128: New target temperature is 40 * 128 / 255 = 20,1°C
 DB2.7...DB2.0 = 0x81 = 129: room temperature = 255 - 129 = 126 => 40 * 126 / 255 = 19,8 °C
 DB1.7...DB1.0 = 0x04:
o DB1.3 = 0: regular default radio cycle (no summer mode)
o DB1.2 = 1: DB3.7...DB3.0 is set to internal temp.-controller with default duty cycle (Summer
bit not active)
DB0.7...DB0.0 = 0x08: Data telegram */
}


void eoA52001::contextMenuEvent(QContextMenuEvent * event)
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



QString eoA52001::getSecondaryValue()
{
    qint64 t = lastMessage.elapsed() / 1000;
    if (t < 60) secondaryValue = "< 1 mn";
    else if (t < 3600) secondaryValue = QString("%1 mn").arg(t/60);
    else secondaryValue = QString("%1 h").arg(t/3600);
    return secondaryValue;
}



bool eoA52001::setscratchpad(const QString &V, bool)
{
    if ((lastMessage.elapsed() < 1000) && (V == lastScratchpad)) return true;   // in case of radio received from multiple EnOcean master
    ScratchPad_Show.setText(V);
    bool ok;
    logMsg(V);
    int v = V.left(2).toInt(&ok, 16);
    if (!ok) return false;
    int rawT = V.mid(4, 2).toInt(&ok, 16);
    uint32_t scaleMax = 40;
    uint32_t scaleMin = 0;
    uint32_t rangeMax = 255;
    uint32_t rangeMin = 0;
    float q = (scaleMax - scaleMin) / (float(rangeMax) - float(rangeMin));
    float T = q * (float(rawT) - float(rangeMin)) + scaleMin;
    Temperature.setText(QString("Temperature : %1").arg(double(T), 0, 'f', 2));
    int BD2 = V.mid(2, 2).toInt(&ok, 16);
    if (ok)
    {
        if (BD2 & 0x40) Energy_Input_Enabled.setText("Energy input enabled"); else Energy_Input_Enabled.setText("Energy input disabled");
        if (BD2 & 0x20) Energy_Storage_OK.setText("Energy storage good"); else Energy_Storage_OK.setText("Energy storage low");
        if (BD2 & 0x10) Battery_Capacity_OK.setText("Battery capacity low"); else Battery_Capacity_OK.setText("Battery capacity good");
        if (BD2 & 0x04) Temperatue_Sensor_Error.setText("Temperature Sensor in error"); else Temperatue_Sensor_Error.setText("Temperature Sensor valid");
        if (BD2 & 0x01) Motor_Failure.setText("Motor Failure YES"); else Motor_Failure.setText("Motor Failure NO");
    }
    else
    {
        Energy_Input_Enabled.setText("Energy Input ...");
        Energy_Storage_OK.setText("Energy Storage ...");
        Battery_Capacity_OK.setText("Battery Capacity ...");
        Temperatue_Sensor_Error.setText("Temperature Sensor ...");
        Motor_Failure.setText("Motor Failure ...");
    }
    setMainValue(v, saveInterval.isEnabled());
    ClearWarning();
    lastScratchpad = V;
    QString Tstr = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
    setWindowTitle(name + " " +Tstr);
    LastRadioMessage.setText("Last message : " + Tstr);
    lastMessage.restart();
    return true;
}



bool eoA52001::isVanneFamily()
{
	return true;
}

void eoA52001::checkElaspedTime()
{
    if (lastMessage.hasExpired(timeOut.value() * 60000) || (!lastMessage.isValid()))
    {
        RetryWarning = 3;
        setTraffic(dataNotValid);
        setMainValue(logisdom::NA, false);
    }
}


void eoA52001::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("TimeOut", QString("%1").arg(timeOut.value()));
}


void eoA52001::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Valve ")));
    bool ok;
    int T = logisdom::getvalue("TimeOut", strsearch).toInt(&ok);
    if (ok) timeOut.setValue(T);
}



QString eoA52001::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}



void eoA52001::assignMainValueLocal(double value)
{
    //qDebug() << QString("Assign %1").arg(value);
    int prc = int(value);
    if (prc < 0) prc = 0;
    else if (prc > 99) prc = 99;
    setPoint.setValue(prc);
    setPointChanged(prc);
}
