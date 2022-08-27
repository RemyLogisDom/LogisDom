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
#include "htmlbinder.h"
#include "onewire.h"
#include "configmanager.h"
#include "daily.h"
#include "treehtmlwidget.h"



treeHtmlWidget::treeHtmlWidget(logisdom *Parent)
{
	parent = Parent;
	QStringList labels;
	labels << tr("Description") << tr("Value") << tr("Command");
    header()->setSectionResizeMode(QHeaderView::Interactive);
    // Qt4 header()->setResizeMode(QHeaderView::Interactive);
    setHeaderLabels(labels);
	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon), QIcon::Normal, QIcon::Off);
	folderIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon), QIcon::Normal, QIcon::On);
	bookmarkIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileDialogContentsView));
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
//	connect(configParent, SIGNAL(ProgChanged(ProgramData*)), this, SLOT(ChangePrgEvt(ProgramData*)));
}




void treeHtmlWidget::selectionChanged()
{
	htmlBinder *binder = nullptr;
	QTreeWidgetItem *item = currentItem();
	if (item) binder = parent->getTreeItemBinder(item);
	if (binder) parent->setPalette(binder);
}




QTreeWidgetItem *treeHtmlWidget::addHtmlBinder(htmlBinder*)
{
	return nullptr;
	QTreeWidgetItem *item = nullptr;
//	if (!binder->parentItem)
//	{
		//QList<QTreeWidgetItem *> foundItems = findItems(Type, Qt::MatchExactly);
		//if (foundItems.count() > 0) return foundItems[0];
		//else
		//{
//			item = new QTreeWidgetItem(this, 0);
//			item->setIcon(0, folderIcon);
			//item->setText(0, Type);
//			return item;
		//}
//	}
//	else
//	{
		//QTreeWidgetItem * item = nullptr;
		//for (int n=0; n<parent->childCount(); n++)
		//{
		//	if ((parent->child(n)->type() == 0) and (parent->child(n)->text(0) == Type)) item = parent->child(n);
		//}
//		if (!item)
//		{
//			item = new QTreeWidgetItem(binder->parentItem, 0);
//			item->setIcon(0, folderIcon);
			//item->setText(0, Type);
//		}
//		return item;
//	}
	return item;
}


