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


#include <QtGui>
#include <QTreeWidget>
#include "globalvar.h"
#include "alarmwarn.h"
#include "configwindow.h"
#include "logisdom.h"
#include "onewire.h"
#include "weathercom.h"




weathercom::weathercom(QWidget *parent) : QTreeWidget(parent)
{
	QStringList labels;
    labels << tr("Description") << tr("Value");
    header()->setSectionResizeMode(QHeaderView::Interactive);
    setHeaderLabels(labels);
}




QTreeWidgetItem *weathercom::setFolder(QString Type, QTreeWidgetItem * parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, Type);
    return item;
}




QTreeWidgetItem *weathercom::setData(QString Type, QString Value, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, Type);
    item->setText(1, Value);
    return item;
}




bool weathercom::read(const QByteArray &data)
{
	clear();
    QJsonDocument jsonData = QJsonDocument::fromJson(data);
    QJsonObject mainObject = jsonData.object();
    QJsonValue result = mainObject.value(QString("list"));
    QJsonArray array = result.toArray();
    //qDebug() << QString("%1").arg(array.count());
    for (int n=0; n<array.count(); n++)
    {
        //qWarning() << array.at(n);
        QJsonObject data = array.at(n).toObject();
        QTreeWidgetItem *item = new QTreeWidgetItem(this);
        item->setText(0, QString("%1").arg(n));
        parseJsonObject(data, item);
    }
    return true;
}




void weathercom::parseJsonObject(const QJsonObject &objectList, QTreeWidgetItem *parentItem)
{
    if (!parentItem) return;
    QStringList list = objectList.keys();
    foreach (const QString &str, list)
    {
        if (objectList.value(str).isString())
        {
            //qDebug() << str + "->" + objectList.value(str).toString();
            setData(str, objectList.value(str).toString(), parentItem);
        }
        else if (objectList.value(str).isDouble())
        {
            if (str == "dt")
            {
                quint64 value = quint64(objectList.value(str).toInt());
                setData(str, QString("%1").arg(value), parentItem);
            }
            else
            {
                double value = objectList.value(str).toDouble();
                setData(str, QString("%1").arg(value), parentItem);
            }
        }
        if (objectList.value(str).isObject())
        {
                QTreeWidgetItem *item = setFolder(str, parentItem);
                parseJsonObject(objectList.value(str).toObject(), item);
        }
        if (objectList.value(str).isArray())
        {
            QJsonArray array = objectList.value(str).toArray();
            if (array.count() > 0)
            {
                //qDebug() << parentItem->text(0) + "->" + str;
                for (int i=0; i<array.count(); i++)
                {
                    QJsonObject data = array.at(i).toObject();
                    QTreeWidgetItem *item = setFolder(str, parentItem);
                    parseJsonObject(data, item);
                }
            }
        }
    }
}



/*
bool weathercom::dateValid(QString S)
{
    QDateTime Now = QDateTime::currentDateTime();
	int firstspace = S.indexOf(" ");
	int secondspace = S.indexOf(" ", firstspace + 1);
	int thirdspace = S.indexOf(" ", secondspace + 1);
	if ((firstspace != -1) && (thirdspace != -1))
	{
		bool valid = true;
		bool ok;
		QString T = S.mid(0, firstspace);
		int fisrtslash = T.indexOf("/");
		int secondslash = T.indexOf("/", fisrtslash + 1);
		QDateTime webTime;// = QDateTime::currentDateTime().addSecs(-7200);
		if ((fisrtslash != -1) && (secondslash != -1))
		{
			int Month = T.mid(0, fisrtslash).toInt(&ok);
			if (!ok) valid = false;
			int Day = T.mid(fisrtslash + 1, secondslash - fisrtslash - 1).toInt(&ok);
			if (!ok) valid = false;
			int Year = T.mid(secondslash + 1, T.length() - secondslash - 1).toInt(&ok);
			if (!ok) valid = false;
            webTime.setDate(QDate(Year + 2000, Month, Day));
            //qDebug() <<  webTime.toString();
            //GenMsg(QString("M = %1  D = %2  Y = %3").arg(Month).arg(Day).arg(Year));
		}
		else valid = false;
		QString D = S.mid(firstspace + 1, thirdspace - firstspace);
		int dot = D.indexOf(":");
		int space = D.indexOf(" ");
		if ((dot != -1) && (space != -1))
		{
			int Hour = D.mid(0, dot).toInt(&ok);
			if (!ok) valid = false;
			int Minute = D.mid(dot + 1, space - dot - 1).toInt(&ok);
			if (!ok) valid = false;
			int PM = D.indexOf("PM");
			if (PM != 1) Hour += 12;
			//GenMsg(QString("H = %1 M = %2").arg(Hour).arg(Minute));
			webTime.setTime(QTime(Hour, Minute, 0));
		}
		else valid = false;
		if ((valid) && (webTime.isValid()) && (webTime.secsTo(Now) < 3600))
		{
			//GenMsg(webTime.toString("HH:mm dd/MM/yy") + " is valid");
			return true;
		}
		//else GenMsg(webTime.toString("HH:mm dd/MM/yy") + " is NOT valid");
		//GenMsg(webTime.toString("HH:mm dd/MM/yy"));
	}					
	return false;
}

*/










