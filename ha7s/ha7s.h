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



#ifndef HA7S_H
#define HA7S_H
#include "onewire.h"
#include "net1wire.h"


#include "ha7sthread.h"

class ha7s : public net1wire
{
	Q_OBJECT
enum SearchIntervals { searchOnce, searchConstant, search5mn, search10mn, search30mn, search1hr, search6hr, search1day};
public:
	ha7s(logisdom *Parent);
	~ha7s();
    void startTCP();
    void init();
    void closeTCP();
    void setipaddress();
    void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
    void addtofifo(const QString &order);
	void addtofifo(int order);
	void addtofifo(int order, const QString &RomID, bool priority = false);
	void addtofifo(int order, const QString &RomID, const QString &Data, bool priority = false);
private:
	QComboBox searchInterval;
	QCheckBox Global_Convert;
	QMutex mutexFifonext;
	QTimer converttimer;
	QString Buffer;
	QMutex mutexreadbuffer;
	QStringList multitrame;
	bool isGlobalConvert();
    ha7sthread HA7SThread;
	void switchOnOff(bool state);
private slots:
    void newDeviceSlot(const QString&);
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
	void logEnabledChanged(int);
	void searchIntervalChanged(int);
	void Global_ConvertChanged(bool);
};

#endif	// HA7S_H

