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



#ifndef RESOL_H
#define RESOL_H
#include "onewire.h"
#include "net1wire.h"


class resol : public net1wire
{
	Q_OBJECT
public:
const static int default_port = 7053;
static const char SYNC = 0xAA;		// Frame End
enum resolModel { DeltaSolM, DeltaSolES };
	resol(logisdom *Parent);
	void init();
	void fifonext();
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
    static double getParameter(QByteArray &frameStr, int &frame, int &pos, int&size);
	static int getByte(QByteArray &frame, int position);
	static QByteArray getFrame(QByteArray &frame, int index);
	static QString AddressToName(int address);
	static void getParameterList(int address, QStringList &list);
	QTimer TimeOut;
	int retry;
private:
	void writeTCP(QString Req);
	int parameterNb;
	void readConfigFile(QString &configdata);
	QByteArray Buffer;
	QMutex mutexreadbuffer;
	onewiredevice *addDevice(QString name, bool show = false);
	QLabel label_pw;
	QLineEdit password;
	bool waitingEndOfFrame;
	QString toHex(QByteArray data);
private slots:
	void timeout();
	void readbuffer();
	void NewDevice();
public:
private:
};

#endif
