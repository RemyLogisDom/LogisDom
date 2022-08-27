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



#ifndef devmodbus_H
#define devmodbus_H
#include "onewire.h"
#include "linecoef.h"
#include "ui_devmodbus.h"
#include "ui_Pdevmodbus.h"

class devmodbus : public onewiredevice
{
	Q_OBJECT
public:
	devmodbus(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
    void SetOrder(const QString &order);
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    void assignMainValueLocal(double value);
    double getMaxValue();
    bool isDimmmable();
    quint16 address = 0;
    quint16 slave = 0;
    quint16 function = 0;
    quint16 resolution = 0;
    bool ValUnsigned = false;
    bool isM3MuxEnabled = false;
    bool muxHasBeenRead;
    bool autoRead = true;
    bool readNow = false;
    bool initialRead = false;
    void changeRomID(QString, QString);
    lineCoef Coef;
    QString getSetup();
private:
	Ui::devmodbusui ui;
    Ui::Pdevmodbusui Pui;
	QLabel error_Txt;
    bool dataOk = false;
public slots:
    void checkRead();
private slots:
    void MuxEnableChange(int);
    void unsignedChange(int);
    void addressChanged(QString);
    void slaveChanged(QString);
    void functionChanged(int);
    void contextMenuEvent(QContextMenuEvent *event);
    void SlaveID_textChanged(const QString &);
    void hexSlaveID_stateChanged(int);
    void Address_textChanged(const QString &str);
    void hexAddress_stateChanged(int);
    void Read_Mask_Enable_stateChanged(int);
    void Write_Value_textChanged(const QString &str);
    void hexWrite_stateChanged(int);
    void pushButtonWrite_clicked();
    void comboBoxResolution_currentIndexChanged(int index);
    void MaxValue_textChanged(const QString &str);
    void readChanged();
};

#endif
