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



// These pocedures are part of net1wire class but specific to EZL50 device connected to a PIC16F688
// for LM11 X10 devices
// EZL50 ComPort settings "4800,n,8,1"
#ifndef X10_H
#define X10_H

#include <QDoubleSpinBox>
#include "net1wire.h"


class x10 : public net1wire
{
	Q_OBJECT
enum CM11StatusList
	{ CM11_NOTHING, CM11_STATUS_INIT, CM11_WAIT_CHECKSUM, CM11_STATUS_CHECK, CM11_WAIT_READY, CM11_POL_BYTE };
public:
struct X10HM
{
	int H;
	int M;
};
	x10(logisdom *Parent);
	void init();
	void fifonext();
	void setrequest(const QString &req);
	QTimer TimeOut, TcpHeartBeatTimer;
	int retry;
	QString request;
private slots:
	void readConfigFile(QString &configdata);
	void readX10();
	void SendX10On();
	void NewModule();
	void SendX10Dim();
	void SendX10Off();
	void X10HeartBeat();
	void CM11_timeout();
	void SendX10Bright();
	void SendX10Address();
	void delayfifonextX10();
	void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
private:
	QMutex mutexFifonext;
	QComboBox ComboHouseCode;
	QComboBox ComboDeviceCode;
	QStringListModel *modelvannes;
	QByteArray tcpdata;
	int X10Status;
	char X10Checksum;
	QDoubleSpinBox X10Power;
	unsigned char Cm11Status;
	X10HM lastAddressRequest, lastAddress;
	QDateTime lastX10EmitON;
	int remoteSleep;
	QString getFunctionText(int function);
	void polRequest();
	void InitRequest();
	void CM11Ready();
signals:
	void X10Change();
};

#endif
