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




#include <QtWidgets/QHeaderView>


#include "globalvar.h"
#include <QTreeWidget>
#include <QtGui>
#include <QHash>
#include <QIcon>
#include <QTreeWidget>
#include "alarmwarn.h"
#include "configwindow.h"
#include "logisdom.h"
#include "interval.h"
#include "onewire.h"
#include "configmanager.h"
#include "daily.h"
#include "programevent.h"


configManager::configManager(logisdom *Parent, configwindow *cfgParent, bool Trans)
{
	qRegisterMetaType<onewiredevice*>();
	parent = Parent;
	configParent = cfgParent;
	translated = Trans;
	QStringList labels;
	labels << tr("Description") << tr("Value");
    header()->setSectionResizeMode(QHeaderView::Interactive);
    setHeaderLabels(labels);
	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon), QIcon::Normal, QIcon::Off);
	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon), QIcon::Normal, QIcon::On);
	bookmarkIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileDialogContentsView));
	connect(configParent, SIGNAL(DeviceChanged(onewiredevice*)), this, SLOT(ChangeDevice(onewiredevice*)));
	connect(configParent, SIGNAL(ProgChanged(ProgramData*)), this, SLOT(ChangePrgEvt(ProgramData*)));
}





QString configManager::getStr(int index)
{
	QString str;
	switch (index)
	{
		case Empty : str = ""; break;
		case CtDevice : if (translated) str = tr("Device"); else str = "Device"; break;
		case CtNet : if (translated) str = tr("Master"); else str = "Master"; break;
		case CtGraph : if (translated) str = tr("Graphic"); else str = "Graphique"; break;
		case CtVmc : if (translated) str = tr("VMC"); else str = "VMC"; break;
		case CtProgram : if (translated) str = tr("Program"); else str = "Program"; break;
		case CtHeating : if (translated) str = tr("Heating"); else str = "Heating"; break;
		case CtDailyPrg: if (translated) str = tr("Daily Program"); else str = "Daily Program"; break;
		case CtWeeklyPrg: if (translated) str = tr("Weekly Program"); else str = "Weekly Program"; break;

		case Name : if (translated) str = tr("Name"); else str = "Name"; break;
		case MainValue : if (translated) str = tr("Main Value"); else str = "Main Value"; break;
		case Scratchpad : if (translated) str = tr("Scrachpad"); else str = "Scrachpad"; break;
		case WinX : if (translated) str = tr("Window X"); else str = "Window X"; break;
		case WinY : if (translated) str = tr("Window Y"); else str = "Window Y"; break;
		case WinHidden : if (translated) str = tr("Window Hidden"); else str = "Window Hidden"; break;
		case WinH : if (translated) str = tr("Window Heigth"); else str = "Window Heigth"; break;
		case WinW : if (translated) str = tr("Window Width"); else str = "Window Width"; break;
		case TownWeather : if (translated) str = tr("Weather Town"); else str = "Weather Town"; break;
		case DaysWeather : if (translated) str = tr("Weather Days"); else str = "Weather Days"; break;
		case GPSTown : if (translated) str = tr("GPS Town"); else str = "GPS Town"; break;
		case Hour : if (translated) str = tr("Hour"); else str = "Hour"; break;
		case Minute : if (translated) str = tr("Minute"); else str = "Minute"; break;
		case Enabled : if (translated) str = tr("Enabled"); else str = "Enabled"; break;
		case TimeMode : if (translated) str = tr("Time Mode"); else str = "Time Mode"; break;
		case IP : if (translated) str = tr("IP"); else str = "IP"; break;
		case Port : if (translated) str = tr("Port"); else str = "Port"; break;
		case Type : if (translated) str = tr("Type"); else str = "Type"; break;
		case TabName : if (translated) str = tr("Tab Name"); else str = "Tab Name"; break;
		case Log : if (translated) str = tr("Log"); else str = "Log"; break;
		case LogAcivity : if (translated) str = tr("Log Activity"); else str = "Log Activity"; break;
		case MasterIP : if (translated) str = tr("Master IP"); else str = "Master IP"; break;
		case Record : if (translated) str = tr("Record"); else str = "Record"; break;
		case RomID : if (translated) str = tr("RomID"); else str = "RomID"; break;
		case Time : if (translated) str = tr("Time"); else str = "Time"; break;
		case TimeShift : if (translated) str = tr("TimeShift"); else str = "TimeShift"; break;
		case ShiftMode : if (translated) str = tr("ShiftMode"); else str = "ShiftMode"; break;
		default : if (translated) str = tr("Undefined"); else str = "Undefined"; break;
	}
	return str;
}







void configManager::ChangeDevice(onewiredevice* device)
{
	if (!device) return;
	QString romid = device->getromid();
	QTreeWidgetItem *folder = setFolder(getStr(CtDevice));
	QTreeWidgetItem *Datafolder =  setFolder(romid, folder);
	setData(getStr(Scratchpad), device->getscratchpad(), Datafolder);
	setData(getStr(MainValue), device->MainValueToStr(), Datafolder);
	setData(getStr(WinX), QString ("%1").arg(device->geometry().x()), Datafolder);
	setData(getStr(WinY), QString ("%1").arg(device->geometry().y()), Datafolder);
	setData(getStr(WinH), QString ("%1").arg(device->geometry().height()), Datafolder);
	setData(getStr(WinW), QString ("%1").arg(device->geometry().width()), Datafolder);
	setData(getStr(WinHidden), QString ("%1").arg(device->isHidden()), Datafolder);
}





void configManager::ChangePrgEvt(ProgramData* prog)
{
	if (!prog) return;
	QTreeWidgetItem *folder = setFolder(getStr(CtProgram));
	QTreeWidgetItem *Datafolder =  setFolder(prog->Button.text(), folder);
//	setData(getStr(Enabled), QString("%1").arg(prog->Enable.isChecked()), Datafolder);
	setData(getStr(Hour), QString("%1").arg(prog->Time.time().hour()), Datafolder);
	setData(getStr(Minute), QString("%1").arg(prog->Time.time().minute()), Datafolder);
}





void configManager::ChangeDaily(daily *Daily)
{
	QTreeWidgetItem *folder = setFolder(getStr(CtDailyPrg));
	QTreeWidgetItem *Datafolder = setFolder(Daily->Name, folder);
	for (int n=0; n<Daily->dailyList.count(); n++)
	{
		QTreeWidgetItem *Pointfolder =  setFolder(QString("%1").arg(n + 1), Datafolder);
		QTreeWidgetItem *Timefolder = setFolder(getStr(Time), Pointfolder);
		setData(getStr(Type), Daily->dailyList[n]->Type.currentText(), Pointfolder);
		setData(getStr(MainValue), QString("%1").arg(Daily->dailyList[n]->Value.value(), 0, 'f', 2), Pointfolder);
		setData(getStr(Hour), QString("%1").arg(Daily->dailyList[n]->time().hour()), Timefolder);
		setData(getStr(Minute), QString("%1").arg(Daily->dailyList[n]->time().minute()), Timefolder);
	}
}



void configManager::RemoveDaily(daily*)
{
//	QTreeWidgetItem *Datafolder = setFolder(getStr(CtDailyPrg), setFolder(getStr(CtDailyPrg));
//	delete Datafolder;
}




void configManager::RemovePrgEvt(ProgramData* prog)
{
	if (!prog) return;
	removeData(CtProgram, prog->Button.text());
}




void configManager::removeData(int Type, QString ID)
{
	QTreeWidgetItem *itemType = nullptr;
	QString type = getStr(Type);
	QList<QTreeWidgetItem *> foundType = findItems(type, Qt::MatchExactly);
	QList<QTreeWidgetItem *>::iterator indexType = foundType.begin();
	while (indexType != foundType.end())
	{
		if (((*indexType)->type() == 0) and ((*indexType)->text(0) == type))
		{
	        itemType = *indexType;
	        break;
		}
		indexType ++;
	}
	if (!itemType) return;
	int childIDs = itemType->childCount();
	for (int n=0; n<childIDs; n++)
	{
		if ((itemType->child(n)->type() == 1) and (itemType->child(n)->text(0) == ID))
		{
			 delete itemType->child(n);
	        break;
		}
	}
}



QTreeWidgetItem *configManager::setFolder(QString Type, QTreeWidgetItem * parent)
{
	QTreeWidgetItem *item = nullptr;
	if (!parent)
	{
		QList<QTreeWidgetItem *> foundItems = findItems(Type, Qt::MatchExactly);
		if (foundItems.count() > 0) return foundItems[0];
		else
		{
			item = new QTreeWidgetItem(this, 0);
			item->setIcon(0, folderIcon);
			item->setText(0, Type);
			return item;
		}
	}
	else
	{
		QTreeWidgetItem *item = nullptr;
		for (int n=0; n<parent->childCount(); n++)
		{
			if ((parent->child(n)->type() == 0) and (parent->child(n)->text(0) == Type)) item = parent->child(n);
		}
		if (!item)
		{
			item = new QTreeWidgetItem(parent, 0);
			item->setIcon(0, folderIcon);
			item->setText(0, Type);
		}
	}
	return item;
}



QTreeWidgetItem *configManager::setData(QString Type, QString Value, QTreeWidgetItem * parent)
{
	QTreeWidgetItem *item = nullptr;
	if (!parent)
	{
		QList<QTreeWidgetItem *> foundItems = findItems(Type, Qt::MatchExactly);
		if (foundItems.count() > 0)
		{
			foundItems[0]->setText(1, Value);
			return foundItems[0];
		}
		else
		{
			item = new QTreeWidgetItem(this, 1);
			item->setIcon(0, bookmarkIcon);
			item->setText(0, Type);
			item->setText(1, Value);
			return item;
		}
	}
	else
	{
		QTreeWidgetItem * item = nullptr;
		for (int n=0; n<parent->childCount(); n++)
		{
			if ((parent->child(n)->type() == 1) and (parent->child(n)->text(0) == Type))
			{
				item = parent->child(n);
				break;
			}
		}
		if (!item)
		{
			item = new QTreeWidgetItem(parent, 1);
			item->setIcon(0, bookmarkIcon);
			item->setText(0, Type);
		}
		item->setText(1, Value);
	}
	return item;
}




void configManager::removeData(QTreeWidgetItem *item)
{
	delete item;
}





