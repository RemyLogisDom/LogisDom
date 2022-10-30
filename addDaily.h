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






#ifndef ADDDAILY_H
#define ADDDAILY_H

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtGui>
#include <QDateTime>

class daily;
class ProgramData;
class logisdom;
class configwindow;

class addDaily : public QWidget
{
	Q_OBJECT
public:
	addDaily(logisdom *Parent);
	logisdom *parent;
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton DailyTool;
	QPushButton AddButton;
	QPushButton RenameButton;
	QPushButton RemoveButton;
	QListWidget List;
	QWidget *previousSetup;
	void SaveConfigStr(QString &str);
	daily *NewOne(QString Name);
    double getDailyProgValue(const QString &progName, const QDateTime &T);
	double getActualValue(int indexNow, int indexPrevious);
    double getValue(int indexNow, int indexPrevious, const QDateTime &datetime);
    double getValueAt(int indexNow, int indexPrevious, const QTime &time);
	QList <daily*> dailyarray;
    int dailyProgNameExist(const QString &name);
	void updateList();
private:
	daily * lastwidget;
private slots:
	void Add();
	void Remove();
	void Rename();
	void RowChanged(int daily);
	void updateJourneyBreaks(ProgramData *prog);
public slots:
	void changePrgEvt(ProgramData *prog);
	void AddPrgEvt(ProgramData *prog);
	void RemovePrgEvt(ProgramData *prog);
    void readconfigfile(const QString &configdata);
	void ValueasChanged();
signals:
	void AddNewDaily(QString);
	void RemoveDaily(QString);
	void ValueChange();
};
#endif
