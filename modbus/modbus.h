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



#ifndef MODBUS_H
#define MODBUS_H
#include "onewire.h"
#include "net1wire.h"

#include "devmodbus.h"
#include "ui_modbus.h"
#include "modbusthread.h"

class modbus : public net1wire
{
	Q_OBJECT
public:
const static int default_port = 502;
    modbus(logisdom *Parent);
	~modbus();
    void startTCP();
    void init();
    void setipaddress();
    void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	void addtofifo(const QString &order);
	void addtofifo(int order);
private:
    Ui::modbus uiw;
    void readConfigFile(QString &configdata);
    QMutex mutexFifonext;
	QString Buffer;
	QMutex mutexreadbuffer;
	modbusthread TcpThread;
	void switchOnOff(bool state);
    onewiredevice *createNewDevice(QString);
	onewiredevice *addDevice(QString name, bool show = false);
private slots:
	void NewDevice();
    void LoadSetup();
    void SaveSetup();
    void clearLogText();
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
	void logEnabledChanged(int);
    void ModbusTCPChanged(bool);
    void LogWriteChanged(int state);
    void MuxIndChanged(int);
    void setDeviceScratchpad(devmodbus*, QString);
};

#endif	// MODBUS_H

