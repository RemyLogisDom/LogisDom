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



#ifndef TELEINFO_H
#define TELEINFO_H
#include "onewire.h"
#include "net1wire.h"


class teleinfo : public net1wire
{
	Q_OBJECT
public:
static const char STX = 0x02;			// Frame Start
static const char ETX = 0x03;			// Frame End
static const char EOT = 0x04;			// Frame Interrupted
static const char LF = 0x0A;			// Line Feed
static const char SP = 0x20;			// Space
static const char CR = 0x0D;			// Carriage Return
static const int DataTimeOut = 60000;
enum TeleInfoValeur
	{
        ADCO = 0, OPTARIF, ISOUSC, BASE, HCHC, HCHP, EJPHN, EJPPM, EJPHPM, BBRHCJB, \
    BBRHPJB, BBRHCJW, BBRHPJW, BBRHCJR, BBRHPJR, PEJP, PTEC, DEMAIN, IINST, ADPS, \
    IMAX, PAPP, HHPHC, MOTDETAT, IINST1, IINST2, IINST3, IMAX1, IMAX2, IMAX3, \
    ADSC, VTIC, DATE, NGTF, LTARF, EAST, EASF01, EASF02, EASF03, EASF04, EASF05, \
    EASF06, EASF07, EASF08, EASF09, EASF10, EASD01, EASD02, EASD03, EASD04, EAIT, \
    ERQ1, ERQ2, ERQ3, ERQ4, IRMS1, IRMS2, IRMS3, URMS1, URMS2, URMS3, PREF, PCOUP, SINSTS, \
    SINSTS1, SINSTS2, SINSTS3, SMAXSN, SMAXSN1, SMAXSN2, SMAXSN3, \
    SMAXSN_1, SMAXSN1_1, SMAXSN2_1, SMAXSN3_1, SINSTI, SMAXIN, SMAXIN_1, \
    CCASN, CCASN_1, CCAIN, CCAIN_1, UMOY1, UMOY2, UMOY3, STGE, DPM1, FPM1, \
    DPM2, FPM2, DPM3, FPM3, MSG1, MSG2, PRM, RELAIS, NTARF, NJOURF, \
    NJOUR_1, PJOUR_1, PPOINTE, UserDefined, TeleInfoValMax
	};
	teleinfo(logisdom *Parent);
	void init();
	void fifonext();
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	static QString TeleInfoValeurtoStr(int index);
	static QString TeleInfoParamtoStr(int index);
	static int TeleInfoValeurLength(int index);
	static QString TeleInfoUnit(int index);
    static bool horodatage(int index);
    QTimer TimeOut;
	int retry;
    bool receivedData = false;
    bool receiveData();
private:
	void readConfigFile(QString &configdata);
	QString Buffer;
	QMutex mutexreadbuffer;
	QString extractBuffer(const QString &data);
    onewiredevice *addDevice(QString name, bool show = false);
private slots:
	void timeout();
	void readbuffer();
	void NewDevice();
};

#endif
