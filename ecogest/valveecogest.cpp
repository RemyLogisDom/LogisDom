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
#include "valveecogest.h"



valveecogest::valveecogest(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	QDateTime Now = QDateTime::currentDateTime();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family4 = romid.right(4);
	familyID = family4.left(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Valve MultiGest"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname(romid);
	htmlBind->addCommand("+", More10);
	htmlBind->addCommand("-", Lest10);
	MainValue = logisdom::NA;
	Decimal.hide();
	Value.setRange(0, 100);
	Value.setSingleStep(1);
	Value.setPrefix(tr("Value : "));
	setupLayout.addWidget(&Value, layoutIndex++, 0, 1, 1);
	Min.setRange(0, 255);
	Min.setSingleStep(1);
	Min.setPrefix(tr("Min : "));
	setupLayout.addWidget(&Min, layoutIndex, 0, 1, 1);
	Max.setRange(0, 255);
	Max.setValue(255);
	Max.setSingleStep(1);
	Max.setPrefix(tr("Max : "));
	setupLayout.addWidget(&Max, layoutIndex++, 1, 1, 1);
	connect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(uisliderChanged(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(uisliderChanged(int)));
	connect(&Value, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(&Min, SIGNAL(valueChanged(int)), this, SLOT(MinChanged(int)));
	connect(&Max, SIGNAL(valueChanged(int)), this, SLOT(MaxChanged(int)));
}





void valveecogest::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	contextualmenu.addAction(&Lecture);	
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
}





void valveecogest::SetOrder(const QString &)
{
}




void valveecogest::remoteCommandExtra(const QString &command)
{
	if (command == More10) setLocalMainValue(MainValue + 10, true);
	if (command == Lest10) setLocalMainValue(MainValue - 10, true);
}




QString valveecogest::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}



QString valveecogest::ValueToStr(double Value, bool)
{
	QString str;
    if (logisdom::isNA(Value)) str = cstr::toStr(cstr::NA);
    else str = parent->LogisDomLocal.toString(Value, 'f', 0) + Unit.text();
	return str;
}



void valveecogest::uisliderChanged(int SliderValue)
{
	disconnect(&Value, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    double min = double(Min.value());
    double max = double(Max.value());
	double index = 112;
	if (max > min)
	{
		double range = max - min;
		index = (SliderValue * range / 100) + min;
	}
	else if (max < min)
	{
		double range = min - max;
		index = min - (SliderValue * range / 100);
	}
	int valueSet = qRound(index);
	Value.setValue(SliderValue);
	if (!parent->isRemoteMode()) if (master) master->addtofifo(QString(familyID + "Val==%1").arg(valueSet));
	connect(&Value, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
}





void valveecogest::sliderChanged(int SliderValue)
{
	disconnect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(uisliderChanged(int)));
	disconnect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(uisliderChanged(int)));
    double min = double(Min.value());
    double max = double(Max.value());
	double index = 112;
	if (max > min)
	{
		double range = max - min;
		index = (SliderValue * range / 100) + min;
	}
	else if (max < min)
	{
		double range = min - max;
		index = min - (SliderValue * range / 100);
	}
	int valueSet = qRound(index);
	ui.horizontalSlider->setValue(SliderValue);
	if (!parent->isRemoteMode()) if (master) master->addtofifo(QString(familyID + "Val==%1").arg(valueSet));
	connect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(uisliderChanged(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(uisliderChanged(int)));
}




bool valveecogest::setscratchpad(const QString &scratchpad, bool enregistremode)
{
	ScratchPad_Show.setText(scratchpad);
	// "121/10/250"
//	GenMsg(devicescratchpad);
	logMsg(scratchpad);
	bool ok = false;
	double val = 0, min = 0, max = 0;
	int l = scratchpad.length();
	int firstSlash = scratchpad.indexOf("/");
	if (firstSlash != -1)
	{
		int secondSlash = scratchpad.indexOf("/", firstSlash + 1);
		if (secondSlash != -1)
		{
			bool ok1, ok2, ok3;
			val = scratchpad.left(firstSlash).toDouble(&ok1);
			min = scratchpad.mid(firstSlash + 1, secondSlash - firstSlash - 1).toDouble(&ok2);
			max = scratchpad.right(l - secondSlash - 1).toDouble(&ok3);
			if (ok1 && ok2 && ok3) ok = true;
		}
	}
	if (!ok) return false;
	disconnect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(uisliderChanged(int)));
	disconnect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(uisliderChanged(int)));
	disconnect(&Value, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	disconnect(&Min, SIGNAL(valueChanged(int)), this, SLOT(MinChanged(int)));
	disconnect(&Max, SIGNAL(valueChanged(int)), this, SLOT(MaxChanged(int)));
    Min.setValue(int(min));
    Max.setValue(int(max));
	double prc = 0;
	if (min < max) prc = (val - min) * 100 / (max - min);
	else prc = (min - val) * 100 / (min - max);
	if (prc < 0) prc = 0;
	if (prc > 100) prc = 100;
	int intprc = qRound(prc);
	ui.horizontalSlider->setValue(intprc);
	Value.setValue(intprc);

//setLocalMainValue
    MainValue = int(intprc);
	htmlBind->setValue(MainValueToStr());
	htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
	if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	MainValueToStr();
	setValid(dataValid);
	emitDeviceValueChanged();

	connect(ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(uisliderChanged(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(uisliderChanged(int)));
	connect(&Value, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(&Min, SIGNAL(valueChanged(int)), this, SLOT(MinChanged(int)));
	connect(&Max, SIGNAL(valueChanged(int)), this, SLOT(MaxChanged(int)));
	return true;
}





void valveecogest::MinChanged(int MinValue)
{
	if (!parent->isRemoteMode())	if (master) master->addtofifo(QString(familyID + "Min==%1").arg(MinValue));
}




void valveecogest::MaxChanged(int MaxValue)
{
	if (!parent->isRemoteMode())	if (master) master->addtofifo(QString(familyID + "Max==%1").arg(MaxValue));
}






void valveecogest::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "S1MV")
			{
				QString V = logisdom::getvalue("S1Val", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "S2MV")
			{
				QString V = logisdom::getvalue("S2Val", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "S3MV")
			{
				QString V = logisdom::getvalue("S3Val", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "S4MV")
			{
				QString V = logisdom::getvalue("S4Val", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "S5MV")
			{
				QString V = logisdom::getvalue("S5Val", scratchpad);
				setscratchpad(V, false);
			}
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void valveecogest::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "S1MV")
			{
				QString V = logisdom::getvalue("S1Val", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "S2MV")
			{
				QString V = logisdom::getvalue("S2Val", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "S3MV")
			{
				QString V = logisdom::getvalue("S3Val", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "S4MV")
			{
				QString V = logisdom::getvalue("S4Val", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "S5MV")
			{
				QString V = logisdom::getvalue("S5Val", scratchpad);
				setscratchpad(V, true);
			}
			//master->getMainValue();
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





void valveecogest::setLocalMainValue(double NewValue, bool enregistremode)
{
	if (NewValue < 0) MainValue = 0;
	else if (NewValue > 100) MainValue = 100;
	else MainValue = NewValue;
    uisliderChanged(int(MainValue));
	htmlBind->setValue(MainValueToStr());
	htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
	if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
	ClearWarning();
	MainValueToStr();
	setValid(dataValid);
	emitDeviceValueChanged();
}







bool valveecogest::isVanneFamily()
{
	return true;
}



void valveecogest::setconfig(const QString &strsearch)
{
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Valve ")));
	x = logisdom::getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = logisdom::getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if ((ok_h != 0) and (h == 0)) show(); else hide();
	setWindowTitle(logisdom::getvalue("Name", strsearch));
}

