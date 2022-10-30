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





#ifndef ALARMWARN_H
#define ALARMWARN_H

#include <ui_alarmwarn.h>


class errlog;
class logisdom;
class QSound;

class alarmwarn : public QWidget
{
	Q_OBJECT
public:
	alarmwarn(logisdom *Parent);
	~alarmwarn();
	Ui::alarmwarn ui;
	QTimer *soundtimer;
//	QSound *alarmwav;
	bool ismuet;
    void addtoalarm(const QString &text);
    errlog *newTab(const QString &name);
    void setTabName(errlog *ErrorLog, const QString &name);
	QList <errlog*> logTabList;
	errlog *GeneralErrorLog;
	void getCfgStr(QString &str);
	void setCfgStr(QString strsearch);
private:
	logisdom *parent;
	bool newAlarm;
private slots:
	void config();
	void silence();
	void playsound(QString soundPath = "");
	void alarmChanged(errlog* errorTab);
public slots:
signals:
	void alarmOn(bool);
};

#endif
