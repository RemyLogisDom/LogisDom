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



#ifndef FTS800_H
#define FTS800_H
#include "onewire.h"
#include "net1wire.h"


class fts800 : public net1wire
{
	Q_OBJECT
public:
	fts800(logisdom *Parent);
	void init();
	int retry;
	QTimer TimeOut, TimerPause;
	void fifonext();
	void update();
	void pausefifo(int delay);
	void setCode1(int code);
	void setCode2(int code);
	int getcode1();
	int getcode2();
	int getNbVanne();
	int getVannePourcent(int vanne);
	void createonewiredevices();
	void setVannePourCent(int vanne, int prc);
    void GetConfigStr(QString &str);
    void setconfig(const QString &strsearch);
private:
//	Ui::guinet1wire uiw;
	int wait;
	QString Buffer, request;
	QString extractBuffer(const QString &data);
	void AssignCode(int code);
	bool statusAdded;
	QMutex mutexFifonext, mutexReadBuffer;
#define maxvannesFTS800 14
	onewiredevice *devices[maxvannesFTS800];
private slots:
	void status();
	void timeout();
	void synchro();
	void readbuffer();
	void FTS800fifonext();
	void setNbVannes(int Nb = 0);
	void rightclickvannes(const QPoint & pos);
	void rightclickcode1(const QPoint & pos);
	void rightclickcode2(const QPoint & pos);
	void rightclicklistvannes(const QPoint & pos);
public:
private:
	QStringListModel *modelvannes;
	int Code1, Code2, NbVannes, VannePourcent[maxvannesFTS800];
	QListView listViewVanne;
   	QStringList listvannes;
	QLineEdit lineEditNbVannes, lineEditC1, lineEditC2;
};

#endif
