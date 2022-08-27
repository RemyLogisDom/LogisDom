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





#ifndef HA7NET_H
#define HA7NET_H
#include <QCheckBox>
#include <QDoubleSpinBox>
#include "net1wire.h"

#ifndef HA7Net_No_Thread

#include "ha7netthread.h"

class ha7net : public net1wire
{
	Q_OBJECT
public:
	ha7net(logisdom *Parent);
	~ha7net();
	void init();
	bool checkbusshorted(const QString &data);
	bool checkLockIDCompliant(const QString &data);
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	ha7netthread *HttpThread;
	QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
	void switchOnOff(bool state);
	void addtofifo(const QString &order);
	void addtofifo(int order);
	void addtofifo(int order, const QString &RomID, bool priority = false);
	void addtofifo(int order, const QString &RomID, const QString &Data, bool priority = false);
private:
	void newThread();
	QTimer logUpdate;
	QString LockID;
	QCheckBox LockEnable;
	QCheckBox Global_Convert;
	QComboBox searchInterval;
	void setipaddress(const QString &adr);
	void setport(int Port);
private slots:
	void updateLog();
	void Ha7Netconfig();
    void newDeviceSlot(const QString);
    void tcpStatusUpdate(const QString);
	void logEnabledChanged(int);
	void searchIntervalChanged(int);
	void Global_ConvertChanged(bool);
};


#define try_ha7net			\
	QString exeptionLogStr;	\
	try				\
	{


#define catch_ha7net								\
	}									\
	catch(...)								\
	{									\
		QDateTime now = QDateTime::currentDateTime();			\
		QFile file("exception.log");					\
		if (file.open(QIODevice::Append | QIODevice::Text))		\
		{								\
			QTextStream out(&file);					\
			out << now.toString() + "  " + exeptionLogStr + "\n";	\
			file.close();						\
		}								\
		newThread();							\
	}


#endif	// HA7Net_No_Thread

#endif	//HA7NET_H
