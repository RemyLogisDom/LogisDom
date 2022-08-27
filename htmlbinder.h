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



#ifndef HTMLBINDER_H
#define HTMLBINDER_H
#include <QtCore>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtGui>

class logisdom;



class htmlBinder : public QWidget
{
	Q_OBJECT
#define showChilds "details"
#define customMenu "custom"
public:
	htmlBinder(logisdom *Parent, QTreeWidgetItem *ParentItem = nullptr);
	htmlBinder(logisdom *Parent, QString MainMenu, QTreeWidgetItem *ParentItem = nullptr);
	~htmlBinder();
// palette setup
	QGridLayout layout;
	QListWidget htmlMenuList;
	logisdom *parent;
	QTreeWidgetItem *parentItem, *treeItem;
	QString ID;
	void widgetSetup();
	htmlBinder *binderParent;
    QString lastName, lastTxt, lastValue;
	void setMainParameter(const QString &Name, const QString &Txt);
	QTreeWidgetItem *setParameter(const QString &Name, const QString &Txt);
	void setName(const QString &Name);
	void setValue(const QString &value);
	void clearCommand();
	bool menuCheck(QString menu);
	void addCommand(QString display, QString html, QTreeWidgetItem *item = nullptr);
	void addParameterCommand(QString ParameterName, QString display, QString html);
    void delParameterCommand(QString ParameterName, QString display);
    void removeParameter(QString ParameterName);
	void setParameterLink(QString ParameterName, QString html);
	void sendParameter(const QString &ParameterName, const QString &value);
	QString getMainHtml(QString &Request, QString &WebID, int Privilege);
	QString getChildHtml(QString &Request, QString &WebID, int Privilege, bool displayChilds = false);
	QString getItemHtml(QTreeWidgetItem *item, QString &Request, QString &WebID, int Privilege);
	QString getHtmlCommand(QTreeWidgetItem *item, QString &WebID, int Privilege);
	void emitSendConfigStr(const QString &command);
	void setHtmlMenulist(QListWidget *List);
	void removeHtmlMenulist(QString name);
	void getCfgStr(QString &str);
	void setCfgStr(QString &str);
private slots:
	void clickHtmlList(QListWidgetItem *item);
private:
signals:
	void remoteCommand(QString);
	void sendConfigStr(QString);
    void valueChanged();
};

#endif
