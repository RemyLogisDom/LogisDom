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
#include "eoF60201.h"




eoF60201::eoF60201(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
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

    connect(&Summer_Bit, SIGNAL(clicked()), this, SLOT(updateDB1()));
    connect(&Set_Point_Selection, SIGNAL(clicked()), this, SLOT(updateDB1()));
}




void eoF60201::updateDB1()
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



void eoF60201::setPointChanged(int v)
{
    QString data_Str;
    data_Str += QString("%1").arg(v, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    data_Str += QString("%1").arg(0, 2, 16, QLatin1Char('0')).toUpper();
    master->writeScratchPad(romid, data_Str);
}


void eoF60201::contextMenuEvent(QContextMenuEvent * event)
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



bool eoF60201::setscratchpad(const QString &V, bool)
{
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
    setWindowTitle(name + " " + QDateTime::currentDateTime().toString(" HH:mm:ss"));
    return true;
}



bool eoF60201::isVanneFamily()
{
	return true;
}


QString eoF60201::getSecondaryValue()
{
 /*   qint64 t = lastMessage.elapsed() / 1000;
    if (t < 60) secondaryValue = "< 1 mn";
    else if (t < 3600) secondaryValue = QString("%1 mn").arg(t/60);
    else secondaryValue = QString("%1 h").arg(t/3600);
    return secondaryValue;*/
    return "";
}


void eoF60201::GetConfigStr(QString &)
{
//    str += logisdom::saveformat("MasterIP", master->getipaddress());
}



void eoF60201::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Valve ")));
}



QString eoF60201::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}



void eoF60201::assignMainValueLocal(double value)
{
    //qDebug() << QString("Assign %1").arg(value);
    int prc = int(value);
    if (prc < 0) prc = 0;
    else if (prc > 99) prc = 99;
    setPoint.setValue(prc);
}
