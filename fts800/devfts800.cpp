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
#include "server.h"
#include "devfts800.h"




devfts800::devfts800(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	Decimal.hide();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Valve STA 800"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	MainValue = logisdom::NA;
}





void devfts800::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Nom(tr("&Name"), this);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Nom) changename();
}





void devfts800::SetOrder(const QString &)
{
}


void devfts800::lecture()
{
	if (!master) return;
	switch (master->gettype())
	{
		case Ezl50_FTS800 : MainValueToStr();
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void devfts800::lecturerec()
{
	if (!master) return;
	switch (master->gettype())
	{
		case Ezl50_FTS800 : MainValueToStr();
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





bool devfts800::isVanneFamily()
{
	return true;
}



void devfts800::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Valve ")));
}



QString devfts800::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}



void devfts800::assignMainValueLocal(double value)
{
    int prc = (int) value;
    if (prc < 0) prc = 0;
    else if (prc > 99) prc = 99;
    int vanne = 0;
    QString P = romid.right(4);
    if (P.left(1) == "V") vanne = P.mid(1, 1).toInt();
    else vanne = P.left(2).toInt();
    QString data = QString("Zone%1==%2").arg(vanne).arg((int)(prc*255/99));
    QString empty = "";
    if (master) master->addtofifo(SendOrder, empty, data);
}
