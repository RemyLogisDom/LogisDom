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



#ifndef REPROCESSTHREAD_H
#define REPROCESSTHREAD_H
#include "calcthread.h"
#include <QtGui>

class formula;
class onewiredevice;


class reprocessthread : public calcthread
{
#define statusFormat " dd-MM-yyyy  hh:mm"
	Q_OBJECT
public:
	reprocessthread(formula *parent);
	onewiredevice *device;
	QString state;
	QDateTime timeIndex;
    qint64 saveInterval;
	int lastSaveIndex;
	onewiredevice *deviceIndex;
	qint64 progress, progressMax, progressIndex;
private:
	double lastsavevalue;
public slots:
    void runprocess();
signals:
    void finished();
};


#endif




