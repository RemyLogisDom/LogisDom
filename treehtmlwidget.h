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



#ifndef TREEHTML_H
#define TREEHTML_H

#include <QTreeWidget>
class configwindow;
class onewiredevice;
class logisdom;
class ProgramEvent;
class ProgramData;
class interval;
class htmlBinder;
class daily;






class treeHtmlWidget : public QTreeWidget
{
	Q_OBJECT
public:
	treeHtmlWidget(logisdom *Parent);
	QTreeWidgetItem *addHtmlBinder(htmlBinder *binder);
private:
	logisdom *parent;
	QIcon folderIcon;
	QIcon bookmarkIcon;
private slots:
	void selectionChanged();
};


#endif

