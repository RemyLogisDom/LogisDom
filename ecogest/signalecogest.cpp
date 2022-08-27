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
#include "signalecogest.h"




signalecogest::signalecogest(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	QDateTime Now = QDateTime::currentDateTime();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Signal MultiGest"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname(romid);
	MainValue = logisdom::NA;
}





void signalecogest::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	contextualmenu.addAction(&Lecture);	
	if (!maison1wirewindow->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
}






void signalecogest::SetOrder(const QString &)
{
}






QString signalecogest::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}




void signalecogest::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	QString V;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			V = logisdom::getvalue("Interlock", scratchpad);
			setscratchpad(V, false);
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void signalecogest::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	QString V;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			V = logisdom::getvalue("Interlock", scratchpad);
			setscratchpad(V, true);
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





bool signalecogest::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
	ScratchPad_Show.setText(devicescratchpad);
	double V = logisdom::NA;
	if (devicescratchpad == "Opened") V = 1;
	if (devicescratchpad == "Closed") V = 0;
	if (V == logisdom::NA) return false;
	MainValue = V;
	htmlBind->setValue(MainValueToStr());
	htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
	if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	MainValueToStr();
	setValid(dataValid);
	emitDeviceValueChanged();
	return true;
}




void signalecogest::setconfig(const QString &strsearch)
{
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Interlock ")));
	x = logisdom::getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = logisdom::getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if ((ok_h != 0) and (h == 0)) show(); else hide();
	setWindowTitle(logisdom::getvalue("Name", strsearch));
}

