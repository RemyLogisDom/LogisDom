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



#ifndef PROGRAMEVENT_H
#define PROGRAMEVENT_H
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QButtonGroup>
#include <QScrollArea>
#include <QtGui>

class configwindow;
class htmlBinder;
class logisdom;


class ProgramData : public QWidget
{
	Q_OBJECT
public:
	ProgramData();
	QPushButton Button;
	QTimeEdit Time;
	int TimeMode;
	htmlBinder *htmlBind;
private slots:
	void change();
	void change(int);
	void change(QDateTime);
public slots:
	void remoteCommand(const QString &command);
signals:
	void DataChange(ProgramData *);
};




class ProgramEvent : public QScrollArea
{
	Q_OBJECT
#define SetPrgMark "SetProgram"
#define Program_Event_Mark "Program Event"
#define minutePlus5 "mp5"
#define minuteMinus5 "mm5"
#define hourPlus1 "hp1"
#define hourMinus1 "hm1"
#define progAuto "auto"
#define progConfort "confort"
#define progNuit "night"
#define progEco "eco"
#define progUnfrost "unfrost"
#define progOutside "outside"
enum ButtonType
	{
		None, Sunrise, Sunset
	};
public:
	ProgramEvent(logisdom *Parent, configwindow *cfgParent);
	~ProgramEvent();
	htmlBinder *htmlBind;
	htmlBinder *htmlBindChooser;
	void getHtmlMenu(QString *str, QString &ID, int Privilege);
	configwindow *configParent;
	logisdom *parent;
	void SaveConfigStr(QString &str);
    void readconfigfile(const QString &configdata);
    void AddEvent(QString Name, QTime Time, int Enabled, int TimeMode);
	void getComboList(QComboBox * Combo);
	QTimeEdit *GetTime(QString Name);
	bool isAutomatic();
	bool isOutside();
	bool isConfort();
    bool isNuit();
    bool isEco();
	bool isUnfreeze();
	QString getButtonName();
	QTime getButtonTime();
	void checkTimeMatch();
	void trier();
	void SwitchPrg(QString prg);
    QPushButton ButtonAuto, ButtonOutside, ButtonHorsGel, ButtonConfort, ButtonNuit, ButtonEco;
private:
	void supprimer();
	QCheckBox SwitchToAuto;
	QList <ProgramData*> PrgEvntList;
	QButtonGroup ButtonGroup;
	QGridLayout *PrgLayout;
public slots:
private slots:
	void remoteCommand(const QString &command);
	void ProgHasChanged(ProgramData *prog);
	void updateProcess(int);
protected:
	void mousePressEvent(QMouseEvent *event);
signals:
	void removeProgramEvent(ProgramData *);
	void addProgramEvent(ProgramData *);
	void changeProgramEvent(ProgramData *);
};



#endif


