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



#ifndef DAILY_H
#define DAILY_H

#include <QtGui>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include "ui_dailyui.h"


class ProgramEvent;
class ProgramData;
class logisdom;



class dailyUnit : public QWidget
{
	Q_OBJECT
public :
	dailyUnit(logisdom *Parent);
	~dailyUnit();
	logisdom *parent;
	QGridLayout setupLayout;
	QComboBox Type;
	QTimeEdit Time;
	QCheckBox Before, After;
	QDoubleSpinBox Value;
	QComboBox State;
	void getCfgStr(QString &str);
	void setCfgStr(QString &strsearch);
	QString getStr();
	void setMode(int mode);
	QTime time();
    QString autoTxt;
	void RemovePrgEvt(ProgramData *prog);
	void AddPrgEvt(ProgramData *prog);
	double getValue();
private:
	int Mode;
private slots:
	void typechange(int index);
	void ClickBefore(int state);
	void ClickAfter(int state);
	void timeChange(QTime);
	void emitChange(int);
	void emitChange(double);
signals:
	void changed(dailyUnit*);
};




class daily : public QWidget
{
	Q_OBJECT
public:
enum DailyStates
    {	StateOFF, StateON, StateAuto, StateDisabled	};
enum DailyMode
    {	Temp, Variable, ONOFF, OpenClose, Analog, VariableOpenClose };
enum DailyShift
	{	None, Before, After};
struct dailyelement
{
	QTime time, timeShift;
	double value;
	int shift;
	QString eventType;
};
	daily(QWidget *parentWidget, logisdom *Parent, const QString &name, bool userChange = true);
    Ui::dailyui ui;
    logisdom *parent;
	~daily();
	QWidget *previousSetup;
	double getValue(const QTime &T);
	double getLastValue();
	void sort();
	QString Name;
	void getStrConfig(QString &str);
    void setStrConfig(const QString &strsearch);
	QList <dailyUnit *> dailyList;
    QGridLayout *timeLayout;
	void setMode(QString mode);
	void setMode(int mode);
	int getMode();
	dailyUnit *addpoint();
    void updateList();
private slots:
	void newOne();
	void removepoint();
	void removepoint(int n);
	void changeMode(int mode);
	void RowChanged(int dailyUnit);
	void updateList(dailyUnit *);
public slots:
	void changePrgEvt(ProgramData *prog);
	void RemovePrgEvt(ProgramData *prog);
	void AddPrgEvt(ProgramData *prog);
signals:
	void ValueChange();
	void change(daily *);
};

#endif
