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




#ifndef HA7NENOTHREADT_H
#define HA7NENOTHREADT_H
#include <QCheckBox>
#include <QDoubleSpinBox>
#include "onewire.h"
#include "net1wire.h"

#ifdef HA7Net_No_Thread


class ha7net : public net1wire
{
	Q_OBJECT
public:
const static int default_port = 80;
    ha7net(logisdom *Parent);
	void init();
	~ha7net();
	void setrequest(const QString &req);
	bool checkbusshorted(const QString &data);
	bool checkLockIDCompliant(const QString &data);
	void httpRequestAnalysis(const QString &data);
	void endofsearchrequest(const QString &data);
	void endofreadtemprequest(const QString &data, bool enregistre);
	void endofreadpoirequest(const QString &data, bool enregistre);
    void endofwriteled(const QString &data);
    void endofreadmemorylcd(const QString &data);
    void endofsetchannel(const QString &data, bool enregistre);
    void endofchannelaccessread(const QString &data, bool enregistre);
    void endofchannelaccesswrite(const QString &data, bool enregistre);
    void endofreaddualswitch(const QString &data, bool enregistre);
	void endofreadcounter(const QString &data, bool enregistre);
	void endofreadadcrequest(const QString &data, bool enregistre);
    void endofreadadcpage01h(const QString &data, bool enregistre);
    void endofwritememory(const QString &data);
    void endofreadpage(const QString &data, bool enregistre);
    void endofreadpage01h(const QString &data, bool enregistre);
    void endofgetlock(const QString &data);
	void endofreleaselock(const QString &data);
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	void convert();
private:
	bool busy;
	onewiredevice *Prequest;
	int httperrorretry;
    QTimer converttimer, TimerPause;
	void simpleend();
	void convertendtemp();
	int getConvertTime(onewiredevice *dev);
	void convertendadc(const QString &data);
	QString LockID;
	int request;
	bool initialsearch;
	QCheckBox LockEnable;
	QCheckBox Global_Convert;
	QDoubleSpinBox ConvertDelay;
	QMutex mutexFifonext;
    int httpGetId;
	bool httpRequestAborted;
	void startRequest(QUrl url);
    QNetworkReply* reply;
    QNetworkAccessManager qnam;
private slots:
	void convertSlot();
    void httpFinished();
    void fifonext();
    void timerconvertout();
	void Ha7Netconfig();
signals:
	void fifoEmpty();
};

#endif // HA7Net_No_Thread
#endif // HA7NENOTHREADT_H
