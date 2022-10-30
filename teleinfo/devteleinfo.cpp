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





#include "globalvar.h"
#include "server.h"
#include "teleinfo.h"
#include "devteleinfo.h"




devteleinfo::devteleinfo(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ReadNow = false;
	ReadRecNow = false;
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	for (int n=0; n<teleinfo::TeleInfoValMax; n++)
		ParameterList.addItem(teleinfo::TeleInfoValeurtoStr(n));
	setupLayout.addWidget(&ParameterList, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Tele Information EDF"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	LastCounter = logisdom::NA;
	Delta = logisdom::NA;
	MainValue = logisdom::NA;
	validCount = 0;
	counterMode.addItem(tr("Normal"));
	counterMode.addItem(tr("Text"));
	counterMode.addItem(tr("Absolute"));
	counterMode.addItem(tr("Relative"));
	counterMode.addItem(tr("Offset"));
    counterMode.addItem(tr("Real Time"));
    counterMode.addItem(tr("Record Real Time"));
    countInit.setName(tr("Offset update"));
	ui.MainLabel->setText(tr("Differential Counter "));
	setupLayout.addWidget(&counterMode, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	Coef.setValue(1);
	Coef.setDecimals(10);
	Coef.setPrefix(tr("Coef : "));
	setupLayout.addWidget(&Coef, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	Offset.setRange(-2147483640, +2147483640);
	Offset.setPrefix(tr("Offset : "));
	setupLayout.addWidget(&Offset, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&countInit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	SaveOnUpdate.setText(tr("Save only on update"));	
	setupLayout.addWidget(&SaveOnUpdate, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	connect(&SaveOnUpdate, SIGNAL(stateChanged(int)), this, SLOT(saveOnUpdateChanged(int)));
	connect(&counterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
}





bool devteleinfo::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
	scratchpad = devicescratchpad;
	ScratchPad_Show.setText(scratchpad);
	double val = logisdom::NA;
	int parameter = ParameterList.currentIndex();
    if (parameter < 0)
	{
        VStr = "";
		setLocalMainValue(logisdom::NA, false);
		ReadNow = false;
		ReadRecNow = false;
		return false;
	}
    QString Pstr = teleinfo::TeleInfoParamtoStr(parameter);
    QString data = devicescratchpad;
	// avoid OPTARIF BASE et BASE
    if (parameter == teleinfo::BASE) data = data.remove("OPTARIF BASE");
    int index = data.indexOf(Pstr);
    QString txt;
    if (index != -1)
	{
        bool ok;
        int l = Pstr.length() + 1;
        int chkLength = l + teleinfo::TeleInfoValeurLength(parameter) + (teleinfo::horodatage(parameter) * 14) + 2; // +2 to join SP and checksum
        QString chk = data.mid(index, chkLength);
        QByteArray byteArray = chk.toUtf8();
        const char* cString = byteArray.constData();
        unsigned char checksum_Mode1 = 0;
        unsigned char checksum_Mode2 = 0;

        for (int c=0; c<chkLength-1; c++)		// -2 don't count last SP and checksum
        {
            if (c<chkLength-2) checksum_Mode1 += *cString;
            checksum_Mode2 += *cString;
            txt += QString("[%1]").arg(*cString);
            cString++;
        }
        checksum_Mode1 &= 0x3F;
        checksum_Mode1 += 0x20;

        checksum_Mode2 &= 0x3F;
        checksum_Mode2 += 0x20;

        VStr = data.mid(index + l + (teleinfo::horodatage(parameter) * 14), teleinfo::TeleInfoValeurLength(parameter));
        val = VStr.toDouble(&ok);

        if ((checksum_Mode1 == *cString) || (checksum_Mode2 == *cString) )
        {
            ScratchPad_Show.setText(VStr);
            switch (parameter)
			{
			case teleinfo::ADCO:
				if (ok)
				{
					MainValue = val;
					setLocalMainValue(MainValue, false);
				}
				else MainValue = logisdom::NA;
				ADCOstr = VStr;
				ui.MainText->setText(ADCOstr);
				return true;
			default :
				if (ok)
				{
                    long int counter = long(val);
					setResult(counter);
					setLocalMainValue(MainValue, false);
					ui.MainText->setText(QString("%1 pulses, Delta = %2").arg(counter).arg(Delta));
					bool offsetUpdate = countInit.isitnow();
                    bool record = false;
                    if (enregistremode) record = true;
                    if (counterMode.currentIndex() == RecRealTime) record = true;
                    if (record)
					{
						if ((SaveOnUpdate.isChecked()) && (counterMode.currentIndex() == offsetMode))
						{
							if (offsetUpdate) savevalue(QDateTime::currentDateTime(), MainValue, ReadRecNow);
						}
						else savevalue(QDateTime::currentDateTime(), MainValue);
					}
                    if ((counterMode.currentIndex() == offsetMode) && (offsetUpdate)) Offset.setValue(int(counter));
					ReadNow = false;
					ReadRecNow = false;
					return true;
				}
				else setLocalMainValue(translateMainValue(VStr), true);
				validCount = 0;
				ReadNow = false;
				ReadRecNow = false;
				return true;
			}
		}
        //else ScratchPad_Show.setText(QString("Checksum failed checksum = %1 *cString = %2").arg(checksum).arg(*cString) + " " + txt);
	}
	validCount ++;
	if (validCount > 10)
	{
		setLocalMainValue(logisdom::NA, false);
		VStr = Pstr + " " + tr("Not found");
	}
	return false;
}




double devteleinfo::translateMainValue(QString &str)
{
	double v = logisdom::NA;
	int index = ParameterList.currentIndex();
	switch(index)
	{
		case teleinfo::OPTARIF :
				if (str == "BASE") v = 0;
				else if (str == "HC..") v = 1;
				else if (str == "EJP.") v = 2;
				else v = logisdom::NA;
				break;
		case teleinfo::PTEC :
				if (str == "HP..") v = 0;
				else if (str == "HC..") v = 1;
				else if (str == "TH..") v = 2;
				else if (str == "HN..") v = 3;
				else if (str == "PM..") v = 4;
				else if (str == "HCJB") v = 5;
				else if (str == "HCJW") v = 6;
				else if (str == "HCJR") v = 7;
				else if (str == "HPJB") v = 8;
				else if (str == "HPJW") v = 9;
				else if (str == "HPJR") v = 10;
				else v = logisdom::NA;
				break;
	case teleinfo::DEMAIN :
				if (str == "----") v = 0;
				else if (str == "BLEU") v = 1;
				else if (str == "BLAN") v = 2;
				else if (str == "ROUG") v = 3;
				break;
	}
	return v;
}




QString devteleinfo::translateMainValueStr(double v)
{
	QString S;
    int value = int(v);
	int index = ParameterList.currentIndex();
	switch(index)
	{
		case teleinfo::OPTARIF :
				if (value == 0) S = tr("Option Base");
				else if (value == 1) S = tr("Option Heures Creuses");
				else if (value == 2) S = tr("Option EJP");
				else S = cstr::toStr(cstr::NA);
				break;
		case teleinfo::PTEC :
				if (value == 0) S = tr("Heures Pleines");
				else if (value == 1) S = tr("Heures Creuses");
				else if (value == 2) S = tr("Toutes les Heures");
				else if (value == 3) S = tr("Heures Normales");
				else if (value == 4) S = tr("Heures de Pointe Mobile");
				else if (value == 5) S = tr("Heures Creuses Jours Bleus");
				else if (value == 6) S = tr("Heures Creuses Jours Blancs");
				else if (value == 7) S = tr("Heures Creuses Jours Rouges");
				else if (value == 8) S = tr("Heures Pleines Jours Bleus");
				else if (value == 9) S = tr("Heures Pleines Jours Blancs");
				else if (value == 10) S = tr("Heures Pleines Jours Rouges");
				else S = cstr::toStr(cstr::NA);
				break;
		case teleinfo::DEMAIN :
				if (value == 0) S = tr("Non connue");
				else if (value == 1) S = tr("Bleu");
				else if (value == 2) S = tr("Blanc");
				else if (value == 3) S = tr("Rouge");
				else S = cstr::toStr(cstr::NA);
				break;
		default : S = cstr::toStr(cstr::NA);
	}
	return S;
}




bool devteleinfo::readNow()
{
    if (ReadNow or ReadRecNow) return true;
    if (counterMode.currentIndex() == RealTime) return true;
    if (counterMode.currentIndex() == RecRealTime) return true;
    return false;
}



void devteleinfo::setLocalMainValue(double value, bool enregistremode)
{
	MainValue = value;
	ClearWarning();
    if (logisdom::isNotNA(value))
	{
		if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
		htmlBind->setValue(MainValueToStr());
		htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
	}
	else
	{
		htmlBind->setValue(VStr);
		htmlBind->setParameter("Value", VStr);
	}
	emitDeviceValueChanged();
}






void devteleinfo::lecture()
{
    if (master->receiveData()) setValid(ReadRequest); else setValid(dataNotValid);
    if (!master) return;
	switch (master->gettype())
	{
		case TeleInfoType : ReadNow = true;
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void devteleinfo::lecturerec()
{
    if (master->receiveData()) setValid(ReadRequest); else setValid(dataNotValid);
    if (!master) return;
	switch (master->gettype())
	{
		case TeleInfoType : ReadRecNow = true;
		break;
		case RemoteType : master->saveMainValue();
		break;
	}

}







void devteleinfo::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
    QAction Remove(tr("Remove"), this);
    contextualmenu.addAction(&Lecture);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
    if (selection == &Remove) removeme();
}






QString devteleinfo::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}





QString devteleinfo::ValueToStr(double Value, bool)
{
	QString str;
	if (ParameterList.currentIndex() == teleinfo::ADCO) str = ADCOstr;
	else switch (counterMode.currentIndex())
	{
		case TextMode :
			str = translateMainValueStr(Value);
			break;
		default :
            if (int(Value) == logisdom::NA)
			{
				if (VStr.isEmpty()) str = cstr::toStr(cstr::NA);
					else str = translateMainValueStr(Value);
			}
			else str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
			break;
	}
    //str = MainValueToStr();
	ValueButton.setText(str);
	ui.valueui->setText(str);
	return str;
}




void devteleinfo::SetOrder(const QString &)
{
}






void devteleinfo::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("Coef", QString("%1").arg(Coef.value(), 0, 'f', 10));
	str += logisdom::saveformat("counterMode", QString("%1").arg(counterMode.currentIndex()));
	str += logisdom::saveformat("counterOffset", QString("%1").arg(Offset.value()));
	str += logisdom::saveformat("NextCountInit", countInit.getNext());
	str += logisdom::saveformat("CountInitMode", countInit.getMode());
	str += logisdom::saveformat("CountInitEnabled", countInit.getSaveMode());
	str += logisdom::saveformat("SaveOnUpdate", QString("%1").arg(SaveOnUpdate.isChecked()));
	str += logisdom::saveformat("Parameter", QString("%1").arg(ParameterList.currentIndex()));
    str += logisdom::saveformat("ParamName", QString("%1").arg(ParameterList.currentIndex()));
}





void devteleinfo::setconfig(const QString &strsearch)
{
    bool ok = true;
	QString coefstr = logisdom::getvalue("Coef", strsearch);
	double coef = 1;
	if (!coefstr.isEmpty()) coef = coefstr.toDouble(&ok);
	if (ok) Coef.setValue(coef);
	Offset.setEnabled(false);
	countInit.setEnabled(false);
	SaveOnUpdate.setEnabled(false);
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Counter ")));
	int mode = 0;
	mode = logisdom::getvalue("counterMode", strsearch).toInt(&ok);
	if (ok) counterMode.setCurrentIndex(mode);
		else counterMode.setCurrentIndex(absoluteMode);
    int offset = 0;
    offset = logisdom::getvalue("counterOffset", strsearch).toInt(&ok);
    if (ok) Offset.setValue(offset);
	QString next = logisdom::getvalue("NextCountInit", strsearch);
	if (next.isEmpty()) countInit.setNext(QDateTime::currentDateTime());
		else countInit.setNext(QDateTime::fromString (next, Qt::ISODate));
    int savemode = logisdom::getvalue("CountInitMode", strsearch).toInt(&ok);
	if (ok) countInit.setMode(savemode);
	int saveen = logisdom::getvalue("CountInitEnabled", strsearch).toInt(&ok);
	if (!ok) countInit.setSaveMode(true);
		else if (saveen) countInit.setSaveMode(true);
			else  countInit.setSaveMode(false);
	int saveupdate = logisdom::getvalue("SaveOnUpdate", strsearch).toInt(&ok);
	if (ok)
	{
		if (saveupdate) SaveOnUpdate.setCheckState(Qt::Checked);
		else SaveOnUpdate.setCheckState(Qt::Unchecked);
	}
	QString parameter = logisdom::getvalue("Parameter", strsearch);
	int p = parameter.toInt(&ok);
	if (ok) ParameterList.setCurrentIndex(p);
}





void devteleinfo::saveOnUpdateChanged(int state)
{
	switch (state)
	{
		case Qt::Unchecked :
			saveInterval.setEnabled(true);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			saveInterval.SaveEnable.setCheckState(Qt::Unchecked);
			saveInterval.setEnabled(false);
			break;
	}
}






void devteleinfo::modeChanged(int)
{
	QDateTime now = QDateTime::currentDateTime();
	switch (counterMode.currentIndex())
	{
		case TextMode : 
        case RealTime :
        case RecRealTime :
        case NormalMode :
		case absoluteMode : 
			Offset.setEnabled(false);
			countInit.setEnabled(false);
			SaveOnUpdate.setEnabled(false);
			break;
		case relativeMode : 
			Offset.setEnabled(false);
			countInit.setEnabled(false);
			SaveOnUpdate.setEnabled(false);
			break;
		case offsetMode  : 
			Offset.setEnabled(true);
			countInit.setEnabled(true);
			countInit.setNext(now);
			SaveOnUpdate.setEnabled(true);
			break;
	}
}





void devteleinfo::setResult(long int NewValue)
{
	switch (counterMode.currentIndex())
	{
		case NormalMode :
        case RealTime :
        case RecRealTime :
        case TextMode :
			Delta = 0;
			LastCounter = 0;
			MainValue = NewValue;
		break;
		case absoluteMode :
			Delta = 0;
			LastCounter = 0;
			MainValue = NewValue * Coef.value();
		break;
		case relativeMode :
			if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
			LastCounter = NewValue;
			MainValue = Delta * Coef.value();
		break;
		case offsetMode :
			if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
			LastCounter = NewValue;
			MainValue = (NewValue - Offset.value()) * Coef.value();
		break;
	}
}



