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



#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QTreeWidget>
class configwindow;
class onewiredevice;
class logisdom;
class ProgramEvent;
class ProgramData;
class interval;
class daily;






class configManager : public QTreeWidget
{
	Q_OBJECT
public:
	configManager(logisdom *Parent, configwindow *cfgParent, bool Trans);
	enum ConfigDataStr { 
		Empty, CtDevice, CtNet, CtGraph, CtVmc, CtProgram, CtHeating, CtDailyPrg, CtWeeklyPrg,
		Name, MainValue, Scratchpad, WinX, WinY, WinHidden, WinH, WinW,
		TownWeather, DaysWeather,
		GPSTown,
		Hour, Minute, Enabled, TimeMode, TimeShift, ShiftMode,
		IP, Port, Type, TabName, Log, LogAcivity, MasterIP, Record, RomID, Time};
	QString getStr(int index);
	QTreeWidgetItem *setFolder(QString Type, QTreeWidgetItem *parent = nullptr);
	QTreeWidgetItem *setData(QString Type, QString Value, QTreeWidgetItem *parent = nullptr);
	void removeData(int Type, QString ID);
	void removeData(QTreeWidgetItem *item);
private:
	bool translated;
	enum col_description {ColGenName = 0, ColTrName, ColValue};
	configwindow *configParent;
	logisdom *parent;
	QIcon folderIcon;
	QIcon bookmarkIcon;
public slots:
	void ChangeDevice(onewiredevice* device);
	void ChangePrgEvt(ProgramData* prog);
	void ChangeDaily(daily *);
	void RemoveDaily(daily *Daily);
	void RemovePrgEvt(ProgramData* prog);
};


#endif

