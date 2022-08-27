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



#ifndef PRS2_H
#define PRS2_H
#include "onewire.h"
#include "net1wire.h"


class rps2 : public net1wire
{
	Q_OBJECT
public:
static const char ETX1 = 0x0D;		// Frame End
static const char ETX2 = 0x0A;		// Frame End
static const char SEP = 0x3B;			// Frame Interrupted
	rps2(logisdom *Parent);
	void init();
	void fifonext();
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	QTimer TimeOut;
	int retry;
private:
	int parameterNb;
	void readConfigFile(QString &configdata);
	QString Buffer;
	QMutex mutexreadbuffer;
	QString extractBuffer(const QString &data);
	onewiredevice *addDevice(QString name, bool show = false);
private slots:
	void timeout();
	void readbuffer();
	void NewDevice();
public:
private:
};

#endif
