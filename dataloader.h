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



#ifndef DATALOADER_H
#define DATALOADER_H
#include <QtCore>
#include <QThread>
#include <QMutex>
#include "remote.h"
#include "globalvar.h"


class onewiredevice;


class dataloader : public QThread
{
    Q_OBJECT
public:
struct s_Data
{
	QVector <qreal> data_Y;
    QVector <qint64> offset;	// seconds since 1970/1/1 00:00
};
enum searchMode { searchAround, searchBefore, searchAfter };
	dataloader(logisdom *Parent);
	logisdom *parent;
	QString romID;
	bool check85;
	bool busy;
	bool logGetValue;
	bool logLoadData;
	remote *RemoteConnection;
    qint64 begin;
	QDateTime DateRequest;
	void run();
	bool isDataReady(const QDateTime &T);
    bool isDataReady(qint64 T);
	QVector <qreal> Data_Y;
    QVector <qint64> Data_Time;
	QMutex data_Access;
    bool getValues(qint64 begin, qint64 end, s_Data &data, int minDif = 0);
    double getValue(qint64 t, int minDif = 0);
	double getValue(const QDateTime &T, int minDif = 0);
    qint64 getIndex(qint64 t, int searchMode, int minDif = 0);
    qint64 getNextIndex(const QDateTime &T);
	void appendData(const QDateTime &T, const double &V);
	void clearData();
    bool done;
private:
	QVector <qreal> Extract_Data_Y;
    QVector <qint64> Extract_Data_Time;
	bool loadData(int month, int year);
    void extractdata(QTextStream &in, qint64 offset_Time);
    void addData(qreal Y, qint64 T);
	QStringList getFileList;
	QString logtxt;
signals:
	void beginChanged();
private slots:
};

#endif // DATALOADER_H

