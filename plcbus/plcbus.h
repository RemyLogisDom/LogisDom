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



// These pocedures are part of net1wire class but specific to EZL50 device connected to a PIC16F688
// for PLC BUS controler RS232
// EZL50 ComPort settings "4800,n,8,1"
#ifndef PLCBUS_H
#define PLCBUS_H
#include "x10.h"
#include "net1wire.h"
#include <QtGui>


class plcbus : public net1wire
{
	Q_OBJECT
enum PCLBUSStatusList
	{ PCLBUS_NOTHING, PCLBUS_CHECK, PCLBUS_ACK, PCLBUS_DATA};
public:
	static const char STX = 0x02;			// Frame Start Bit
	static const char ETX = 0x03;			// Frame End Bit
	static const char PLC_ALL_UNIT_OFF = 0x00;
	static const char PLC_ALL_LIGHTS_ON = 0x01;
	static const char PLC_ON = 0x02;
	static const char PLC_OFF = 0x03;
	static const char PLC_DIM = 0x04;
	static const char PLC_BRIGHT = 0x05;
	static const char PLC_ALL_LIGHTS_OFF = 0x06;
	static const char PLC_USER_LIGHTS_ON = 0x07;
	static const char PLC_USER_UNITS_OFF = 0x08;
	static const char PLC_USER_LIGHTS_OFF = 0x09;
	static const char PLC_BLINK = 0x0A;
	static const char PLC_FADE_STOP = 0x0B;
	static const char PLC_PRESET_DIM = 0x0C;
	static const char PLC_STATUS_ON = 0x0D;
	static const char PLC_STATUS_OFF = 0x0E;
	static const char PLC_REQ_STATUS = 0x0F;
	static const char PLC_SET_MASTER_ADR_REC = 0x10;
	static const char PLC_SET_SCENE_ADR = 0x12;
	static const char PLC_CLR_SCENE_ADR = 0x13;
	static const char PLC_CLR_ALL_SCENE_ADR = 0x14;
	static const char PLC_GET_SIGNAL_STRENGTH = 0x18;
	static const char PLC_GET_NOISE_STRENGTH = 0x19;
	static const char PLC_REP_SIGNAL_STRENGTH = 0x1A;
	static const char PLC_REP_NOISE_STRENGTH = 0x1B;
	static const char PLC_GET_ALL_ID_PULSE = 0x1C;
	static const char PLC_GET_ON_ID_PULSE = 0x1D;
	static const char PLC_REP_ALL_ID_PULSE = 0x1E;
	static const char PLC_REP_ON_ID_PULSE = 0x1F;
	static const char ACK_PULSE = 0x20;
	

	plcbus(logisdom *Parent);
	void init();
	void fifonext();
	void setrequest(const QString &req);
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	void readConfigFile(QString &configdata);
	QTimer TimeOut, TcpHeartBeatTimer;
	int retry;
private slots:
	void readPLCBus();
	void ClickOn();
	void ClickOff();
	void NewModule();
	void FeedBackON();
	void clickUserCode();
	void PCLBUS_HeartBeat();
	void PCLBUS_timeout();
	void delayfifonextPLCBUS();
private:
	QMutex mutexFifonext;
	QMutex mutexreadPLCBus;
	int HouseCodeSeekIndex;
	void setUserCode(int code = -1);
	QPushButton UserCodeButton;
	unsigned char UserCode;
	QCheckBox deviceAck;
	QCheckBox devicePool;
	QComboBox ComboHouseCode;
	QComboBox ComboDeviceCode;
	QStringListModel *modelvannes;
	QByteArray tcpdata;
	unsigned char PCLBUSStatus;
signals:
	void X10Change();
};

#endif
