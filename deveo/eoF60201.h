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



#ifndef devF60201_H
#define devF60201_H
#include "onewire.h"
#include "ui_eoF60201.h"

class eoF60201 : public onewiredevice
{
	Q_OBJECT
public:
    eoF60201(net1wire *Master, logisdom *Parent, QString RomID);
    bool isVanneFamily();
	void setconfig(const QString &strsearch);
    void GetConfigStr(QString &str);
    QString MainValueToStrLocal(const QString &str);
    void assignMainValueLocal(double value);
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    QString getSecondaryValue();
    QSpinBox setPoint;
private:
        Ui::F60201ui ui;
        QLabel Temperature;          // DB1
        QLabel Energy_Input_Enabled ;// DB2.6
        QLabel Energy_Storage_OK;    // DB2.5
        QLabel Battery_Capacity_OK;  // DB2.4
        QLabel Temperatue_Sensor_Error;  // DB2.2
        QLabel Motor_Failure;  // DB2.0
        QCheckBox Summer_Bit;  // DB1.3
        QCheckBox Set_Point_Selection;  // DB1.2
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots:
    void setPointChanged(int);
    void updateDB1();
};


#endif
