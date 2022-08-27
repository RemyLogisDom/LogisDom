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




#include "globalvar.h"
#include "logisdom.h"
#include "configmanager.h"
#include "connection.h"
#include "configwindow.h"
#include "server.h"
#include "remote.h"
#include "htmlbinder.h"



htmlBinder::htmlBinder(logisdom *Parent, QTreeWidgetItem *ParentItem)
{
	parent = Parent;
	parentItem = ParentItem;
	treeItem = parent->addHtmlBinder(this);
	widgetSetup();
	htmlMenuList.setSortingEnabled(true);
	//setParameter("ID", ID); for debug only
	connect(&htmlMenuList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(clickHtmlList(QListWidgetItem *)));
}





htmlBinder::htmlBinder(logisdom *Parent, QString MainMenu, QTreeWidgetItem *ParentItem)
{
	parent = Parent;
	parentItem = ParentItem;
	treeItem = parent->addHtmlBinder(this);
	if (treeItem) treeItem->setText(0, MainMenu);
	widgetSetup();
	//setParameter("ID", ID); for debug only
	connect(&htmlMenuList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(clickHtmlList(QListWidgetItem *)));
}





htmlBinder::~htmlBinder()
{
}






void htmlBinder::setHtmlMenulist(QListWidget *List)
{
	// save current selection
	for (int n=0; n<List->count(); n++)
	{
		bool found = false;
		for (int i=0; i<htmlMenuList.count(); i++)
		{
			if (htmlMenuList.item(i)->text() == List->item(n)->text())
			{
				found = true;
				if (htmlMenuList.item(i)->checkState()) htmlMenuList.item(i)->setCheckState(Qt::Checked);
			}
		}
		if (!found)
		{
			QListWidgetItem *item = new QListWidgetItem(List->item(n)->text());
			item->setCheckState(Qt::Unchecked);
			htmlMenuList.addItem(item);
		}
	}
	htmlMenuList.sortItems();
	/*for (int n=0; n<htmlMenuList.count(); n++)
		if (htmlMenuList.item(n)->checkState()) backup.append(htmlMenuList.item(n)->text());
	htmlMenuList.clear();
	for (int n=0; n<List->count(); n++)
	{
		QListWidgetItem *item = new QListWidgetItem(List->item(n)->text());
		htmlMenuList.addItem(item);
		if (backup.contains(List->item(n)->text())) item->setCheckState(Qt::Checked);
		else item->setCheckState(Qt::Unchecked);
	}*/
}





void htmlBinder::removeHtmlMenulist(QString name)
{
	for (int i=0; i<htmlMenuList.count(); i++)
	{
		if (htmlMenuList.item(i)->text() == name)
		{
			QListWidgetItem *widget = htmlMenuList.takeItem(i);
			delete widget;
		}
	}
	htmlMenuList.sortItems();
}




void htmlBinder::clickHtmlList(QListWidgetItem *item)
{
	if (!item) return;
	if (item->checkState()) item->setCheckState(Qt::Unchecked);
		else item->setCheckState(Qt::Checked);
}




void htmlBinder::emitSendConfigStr(const QString &command)
{
	//GenMsg("Binder emit  " + ID + "  " + command);
	QString T = command;
	emit(sendConfigStr(T));
	emit(remoteCommand(T));
}



void htmlBinder::sendParameter(const QString &ParameterName, const QString &value)
{
	QString command = logisdom::saveformat(CMenuId, ID);
	command += logisdom::saveformat(ParameterName, value);
	if (parent->isRemoteMode()) parent->RemoteConnection->addCommandtoFifo(command);
		else parent->configwin->server->transfertToOthers(command);
}



void htmlBinder::widgetSetup()
{
	setLayout(&layout);
	layout.addWidget(&htmlMenuList);
}





bool htmlBinder::menuCheck(QString menu)
{
	for (int n=0; n<htmlMenuList.count(); n++)
	{
		if (htmlMenuList.item(n)->checkState() && (htmlMenuList.item(n)->text() == menu)) return true;
	}
	return false;
}







QTreeWidgetItem *htmlBinder::setParameter(const QString &Name, const QString &Txt)
{
	//QList<QTreeWidgetItem *> foundItems = parent->treeHtml->findItems(Type, Qt::MatchExactly);
	QTreeWidgetItem *item = nullptr;
	for (int n=0; n<treeItem->childCount(); n++)
	{
		//if ((treeItem->child(n)->type() == 0) and (treeItem->child(n)->text(0) == Type)) item = treeItem->child(n);
		if (treeItem->child(n)->text(0) == Name) item = treeItem->child(n);
	}
	if (!item)
	{
		item = new QTreeWidgetItem(treeItem, 1);
	}
	//item->setIcon(0, folderIcon);
    if ((lastName != Name) || (lastTxt != Txt))
    {
        item->setText(0, Name);
        item->setText(1, Txt);
        lastName = Name;
        lastTxt = Txt;
        emit(valueChanged());
    }
	return item;
}








void htmlBinder::getCfgStr(QString &str)
{
	int index = 0;
	for (int n=0; n<htmlMenuList.count(); n++)
		if (htmlMenuList.item(n)->checkState() == Qt::Checked)
			str += logisdom::saveformat(QString("HtmlMenu%1").arg(index++), htmlMenuList.item(n)->text());
}







void htmlBinder::setCfgStr(QString &str)
{
	int index = 1;
	QString newHtmlMenu = logisdom::getvalue("HtmlMenu0", str);
	while (!newHtmlMenu.isEmpty())
	{
		bool found = false;
		for (int n=0; n<htmlMenuList.count(); n++)
		{
			if (htmlMenuList.item(n)->text() == newHtmlMenu)
			{
				htmlMenuList.item(n)->setCheckState(Qt::Checked);
				found = true;
			}
		}
		if (!found)
		{
			QListWidgetItem *item = new QListWidgetItem(newHtmlMenu);
			item->setCheckState(Qt::PartiallyChecked);
			htmlMenuList.addItem(item);
		}
		newHtmlMenu = logisdom::getvalue(QString("HtmlMenu%1").arg(index++), str);
	}
}





void htmlBinder::setMainParameter(const QString &name, const QString &Txt)
{
	treeItem->setText(0, name);
	treeItem->setText(1, Txt);
}






QString htmlBinder::getMainHtml(QString &Request, QString &WebID, int Privilege)
{
	QString request = logisdom::getvalue(CRequest, Request);
	QString link = CRequest "=(" showChilds ")" CUId "=(" + WebID + ")" CMenuId "=(" + ID + ")";
	QString str;
	str += "&nbsp;&nbsp;";
	str += logisdom::toHtml(treeItem->text(0), link, logisdom::htmlStyleMenu);
	QString txt = getHtmlCommand(treeItem, WebID, Privilege);
//	if (!txt.isEmpty())
//	{
//		str += logisdom::spanIt("&nbsp;&nbsp;" + treeItem->text(1), logisdom::htmlStyleValue);
//		str += txt;
//	}
	str += "<br>";
	return str;
}




QString htmlBinder::getChildHtml(QString &Request, QString &WebID, int Privilege, bool displayChilds)
{
	QString request = logisdom::getvalue(CRequest, Request);
	QString link, str, z;
	if (Privilege == Server::FullControl) emit(remoteCommand(request));
	link =  CRequest "=(" showChilds ")" CUId "=(" + WebID + ")" CMenuId "=(" + ID + ")";
	str += logisdom::toHtml(treeItem->text(0), link, logisdom::htmlStyleDetector);
	str += "&nbsp;&nbsp;";
	str += logisdom::spanIt(treeItem->text(1), logisdom::htmlStyleValue);
	str += "&nbsp;";
	str += logisdom::spanIt(getHtmlCommand(treeItem, WebID, Privilege), logisdom::htmlStyleCommand);
	str += "<br>";
	if (!request.isEmpty())
	{
		if (displayChilds)
		{
			for (int n=0; n<treeItem->childCount(); n++)
			{
				htmlBinder *binder = parent->getTreeItemBinder(treeItem->child(n));
				if (binder)
				{
					str.append(binder->getChildHtml(request, WebID, Privilege));
				}
				else
				{
					QString s = getItemHtml(treeItem->child(n), z, WebID, Privilege);
					if (!s.isEmpty())
					{
						str.append(s);
						str.append("<br>");
					}
				}
			}
			for (int n=0; n<parent->htmlBinderList.count(); n++)
				if (parent->htmlBinderList[n]->menuCheck(treeItem->text(0)))
					str.append(parent->htmlBinderList[n]->getChildHtml(request, WebID, Privilege));
		}
	}
	return str;
}






QString htmlBinder::getItemHtml(QTreeWidgetItem *item, QString&, QString &WebID, int Privilege)
{
	QString text;
	int i = item->text(2).indexOf("{");
	if (item->text(2).isEmpty())
	{
		text.append(logisdom::spanIt(item->text(0) + " = ", logisdom::htmlStyleDetector));
		text.append(logisdom::spanIt(item->text(1), logisdom::htmlStyleValue));
	}
	if (i == -1)
	{
		text.append(logisdom::spanIt(getHtmlCommand(item, WebID, Privilege), logisdom::htmlStyleCommand));
	}
	else
	{
		text.append(logisdom::spanIt(item->text(0) + " = ", logisdom::htmlStyleDetector));
		text.append(logisdom::spanIt("&nbsp;" + item->text(1) + "&nbsp;", logisdom::htmlStyleValue));
		text.append(logisdom::spanIt(getHtmlCommand(item, WebID, Privilege), logisdom::htmlStyleCommand));
	}
	return text;
}





void htmlBinder::setName(const QString &Name)
{
	if (treeItem) treeItem->setText(0, Name);
}




void htmlBinder::setValue(const QString &value)
{
    if (treeItem)
        if (lastValue != value)
        {
            treeItem->setText(1, value);
            lastValue = value;
            emit(valueChanged());
        }
}




void htmlBinder::clearCommand()
{
	if (treeItem) treeItem->setText(2, "");
}





void htmlBinder::addCommand(QString display, QString html, QTreeWidgetItem *item)
{
	if (!item)
	{
		if (!treeItem) return;
		QString command = treeItem->text(2);
		command += "{" + display + "/" + html + "}";
		treeItem->setText(2, command);
		return;
	}
	QString command = item->text(2);
	command += "{" + display + "/" + html + "}";
	item->setText(2, command);
	QTreeWidgetItem *commandItem = new QTreeWidgetItem(item, 1);
	commandItem->setText(0, display);
	commandItem->setText(1, html);
}




QString htmlBinder::getHtmlCommand(QTreeWidgetItem *item, QString &WebID, int Privilege)
{
	QString str;
	int i = 0, ii = 0;
	QString commands = item->text(2);
	QString display = item->text(0);
	if (commands.isEmpty()) return "";
	i = commands.indexOf("{");
	if (i != -1)
	{
		ii = commands.indexOf("}", i);
		while ((i != -1) && (ii != -1))
		{
			QString S = commands.mid(i + 1, ii - i - 1);
			int n = S.indexOf("/");
			if (n != - 1)
			{
				QString link = CRequest "=(" + S.right(S.length() - n - 1) + ")" CUId "=(" + WebID + ")" CMenuId "=(" + ID + ")";
				if (Privilege == Server::FullControl) str += "   " + logisdom::toHtml(S.left(n), link, logisdom::htmlStyleCommand);
					else str +=  "   " + S.left(n);
			}
			i = commands.indexOf("{", ii);
			if (i != -1) ii = commands.indexOf("}", i);
		}
	}
	else
	{
		QString link = CRequest "=(" + commands + ")" CUId "=(" + WebID + ")" CMenuId "=(" + ID + ")";
		if (Privilege == Server::FullControl) str += logisdom::toHtml(display, link, logisdom::htmlStyleCommand);
		else if (commands.right(4) == ".png") str += logisdom::toHtml(display, link, logisdom::htmlStyleCommand);
		else str += commands;			
	}
	str += "&nbsp;";
	return str;
}





void htmlBinder::addParameterCommand(QString ParameterName, QString display, QString html)
{
	QTreeWidgetItem * item = nullptr;
	for (int n=0; n<treeItem->childCount(); n++)
	{
		if (treeItem->child(n)->text(0) == ParameterName) item = treeItem->child(n);
	}
    if (!item)
    {
        item = new QTreeWidgetItem(treeItem, 1);
        item->setText(0, ParameterName);
        //item->setIcon(0, folderIcon);
    }
    if (item)
    {
        QTreeWidgetItem * childItem = nullptr;
        for (int n=0; n<item->childCount(); n++)
        {
            if (item->child(n)->text(0) == display) childItem = item->child(n);
        }
        if (!childItem)	childItem = new QTreeWidgetItem(item, 1);
        if (childItem)
        {
            childItem->setText(0, display);
            childItem->setText(1, html);
        }
        QString command;
        for (int n=0; n<item->childCount(); n++)
        {
            command += "{" + item->child(n)->text(0) + "/" + item->child(n)->text(1) + "}";
        }
        item->setText(2, command);
    }
}





void htmlBinder::delParameterCommand(QString ParameterName, QString display)
{
    QTreeWidgetItem * item = nullptr;
    for (int n=0; n<treeItem->childCount(); n++)
    {
        if (treeItem->child(n)->text(0) == ParameterName) item = treeItem->child(n);
    }
    if (item)
    {
        QTreeWidgetItem * childItem = nullptr;
        for (int n=0; n<item->childCount(); n++)
        {
            if (item->child(n)->text(0) == display) childItem = item->child(n);
        }
        if (childItem)
        {
            item->removeChild(childItem);
            QString command;
            for (int n=0; n<item->childCount(); n++)
            {
                command += "{" + item->child(n)->text(0) + "/" + item->child(n)->text(1) + "}";
            }
            item->setText(2, command);
        }
    }
}




void htmlBinder::setParameterLink(QString ParameterName, QString html)
{
	QTreeWidgetItem * item = nullptr;
	for (int n=0; n<treeItem->childCount(); n++)
	{
		if (treeItem->child(n)->text(0) == ParameterName) item = treeItem->child(n);
	}
	if (!item)
	{
		item = new QTreeWidgetItem(treeItem, 1);
	}
	//item->setIcon(0, folderIcon);
	item->setText(0, ParameterName);
	//item->setText(2, "{" + ParameterName + "/" + html + "}");
	item->setText(2, html);
}





void htmlBinder::removeParameter(QString ParameterName)
{
	QTreeWidgetItem * item = nullptr;
	for (int n=0; n<treeItem->childCount(); n++)
	{
		if (treeItem->child(n)->text(0) == ParameterName) item = treeItem->child(n);
	}
	if (item) delete item;
}




