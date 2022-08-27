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



#ifndef __ADDPROGRAM_H__
#define __ADDPROGRAM_H__
#include "ui_addProgram.h"
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtGui>

class daily;
class logisdom;


class weeklyProgramUnit : public QWidget
{
	Q_OBJECT
#define minH 20
public:
	weeklyProgramUnit(logisdom *Parent, QString name);
	~weeklyProgramUnit();
	logisdom *parent;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QString Name;
	QPushButton NameButton;
	QComboBox *DaysCombo[7];
	QLabel *DaysName[7];
public slots:
	void AddDaily(QString);
	void RemoveDaily(QString);
};





class weeklyProgram : public QWidget
{
	Q_OBJECT
public:
enum eWeekDays
{
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
	Sunday,
	NbDays
};
	weeklyProgram(logisdom *Parent);
	logisdom *parent;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QToolButton ToolButton;
	QPushButton AddButton;
	QPushButton RenameButton;
	QPushButton RemoveButton;
	QListWidget List;
	QWidget *previousSetup;
    QList <weeklyProgramUnit*> program;
	weeklyProgramUnit *NewOne(QString Name);
//	QList <daily*> dailyarray;
    void SaveConfigStr(QString &str);
    void readconfigfile(const QString &configdata);
	int getActualProgram(int indexPrg);
    int getActualProgram(const QString &name, const QDateTime &T);
    int getPreviousProgram(int indexPrg);
    int getPreviousProgram(const QString &name, const QDateTime &T);
    QString dayToString(int D);
    int weeklyProgNameExist(const QString &name);
private slots:
	void Add();
	void Remove();
	void Rename();
	void RowChanged(int row);
	void updateList();
public slots:
    void updateName(const QString &NewName);
protected:
	void mousePressEvent(QMouseEvent *event);
signals:
	void AddProgram(QString);
	void RemoveProgram(QString);
};



#endif
