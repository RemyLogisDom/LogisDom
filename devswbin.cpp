/****************************************************************************
**
** Copyright (C) 2006-2011 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "globalvar.h"
#include "server.h"
#include "teleinfo.h"
#include "devswbin.h"




devswbin::devswbin(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	//ReadNow = false;
	//ReadRecNow = false;
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	//for (int n=0; n<teleinfo::TeleInfoValMax; n++)
	//	ParameterList.addItem(teleinfo::TeleInfoValeurtoStr(n));
	//setupLayout.addWidget(&ParameterList, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Binary Switch"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	MainValue = logisdom::NA;
	//validCount = 0;
	//RT.show();
	//counterMode.addItem(tr("Normal"));
	//counterMode.addItem(tr("Text"));
	//counterMode.addItem(tr("Absolute"));
	//counterMode.addItem(tr("Relative"));
	//counterMode.addItem(tr("Offset"));
	//countInit.setName(tr("Offset update"));
	//ui.MainLabel->setText(tr("Differential Counter "));
	//setupLayout.addWidget(&counterMode, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	//Coef.setValue(1);
	//Coef.setDecimals(10);
	//Coef.setPrefix(tr("Coef : "));
	//setupLayout.addWidget(&Coef, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	//Offset.setRange(-2147483640, +2147483640);
	//Offset.setPrefix(tr("Offset : "));
	//setupLayout.addWidget(&Offset, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	//setupLayout.addWidget(&countInit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	//SaveOnUpdate.setText(tr("Save only on update"));
	//setupLayout.addWidget(&SaveOnUpdate, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	//connect(&SaveOnUpdate, SIGNAL(stateChanged(int)), this, SLOT(saveOnUpdateChanged(int)));
	//connect(&counterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
}





bool devswbin::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	scratchpad = devicescratchpad;
	ScratchPad_Show.setText(scratchpad);
	//if (!RT.isChecked())
//		if (!(ReadNow or ReadRecNow)) return true;
//	double val = logisdom::NA;
/*	int parameter = ParameterList.currentIndex();
	if (parameter < 0)
	{
		VStr = "";
		setLocalMainValue(logisdom::NA, false);
		ReadNow = false;
		ReadRecNow = false;
		return false;
	}
	QString Pstr = teleinfo::TeleInfoParamtoStr(parameter);
	QString line = devicescratchpad;
	// avoid OPTARIF BASE et BASE
	if (parameter == teleinfo::BASE) line = line.remove("OPTARIF BASE");
	int index = line.indexOf(Pstr);
	if (index != -1)
	{
		bool ok;
		int l = Pstr.length() + 1;
		int chkLength = l + teleinfo::TeleInfoValeurLength(parameter) + 2; // +2 to join CP and checksum
		QString chk = line.mid(index, chkLength);
		const char *ch = qPrintable(chk);
		unsigned char checksum = 0;
		for (int c=0; c<chkLength-2; c++)		// -2 don't count last SP and checksum
		{
			checksum += *ch;
			ch++;
		}
		ch++;		// move pointer to checksum
		unsigned char ck = *ch;
		checksum &= 0x3F;
		checksum += 0x20;
		VStr = line.mid(index + l, teleinfo::TeleInfoValeurLength(parameter));
		val = VStr.toDouble(&ok);
		//for (int n=0; n<data.length(); n++) logtxt += QString("[%1]").arg((unsigned char)data[n]);
		//logMsg(devicescratchpad);
		if (checksum == ck)
		{
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
			break;
			default :
				if (ok)
				{
					long int counter = (long int)val;
					setResult(counter);
					setLocalMainValue(MainValue, false);
					ui.MainText->setText(QString("%1 pulses, Delta = %2").arg(counter).arg(Delta));
					bool offsetUpdate = countInit.isitnow();
					if (enregistremode)
					{
						if ((SaveOnUpdate.isChecked()) && (counterMode.currentIndex() == offsetMode))
						{
							if (offsetUpdate) savevalue(QDateTime::currentDateTime(), MainValue, ReadRecNow);
						}
						else savevalue(QDateTime::currentDateTime(), MainValue);
					}
					if ((counterMode.currentIndex() == offsetMode) && (offsetUpdate)) Offset.setValue((int)counter);
					ReadNow = false;
					ReadRecNow = false;
					return true;
				}
				else setLocalMainValue(translateMainValue(VStr), true);
				validCount = 0;
				ReadNow = false;
				ReadRecNow = false;
				return true;
			break;
			}
		}
	}
	validCount ++;
	if (validCount > 10)
	{
		setLocalMainValue(logisdom::NA, false);
		VStr = Pstr + " " + tr("Not found");
	}*/
	return false;
}





void devswbin::setLocalMainValue(double value, bool enregistremode)
{
	MainValue = value;
	ClearWarning();
	if (value != logisdom::NA)
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






void devswbin::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
/*	switch (master->gettype())
	{
		case TeleInfoType : ReadNow = true;
		break;
		case RemoteType : master->getMainValue();
		break;
	}*/
}





void devswbin::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
/*	switch (master->gettype())
	{
		case TeleInfoType : ReadRecNow = true;
		break;
		case RemoteType : master->saveMainValue();
		break;
	}*/

}







void devswbin::contextMenuEvent(QContextMenuEvent * event)
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






QString devswbin::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}





QString devswbin::ValueToStr(double Value)
{
	QString str;
/*	if (ParameterList.currentIndex() == teleinfo::ADCO) str = ADCOstr;
	else switch (counterMode.currentIndex())
	{
		case TextMode :
			str = translateMainValueStr(Value);
			break;
		default :
			if (Value == logisdom::NA)
			{
				if (VStr.isEmpty()) str = cstr::toStr(cstr::NA);
					else str = translateMainValueStr(Value);
			}
			else str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
			break;
	}
	str = checkValid(str);
	ValueButton.setText(str);
	ui.valueui->setText(str);*/
	return str;
}




void devswbin::SetOrder(QString)
{
}






void devswbin::GetConfigStr(QString &str)
{
/*	str += logisdom::saveformat("Coef", QString("%1").arg(Coef.value(), 0, 'f', 10));
	str += logisdom::saveformat("counterMode", QString("%1").arg(counterMode.currentIndex()));
	str += logisdom::saveformat("counterOffset", QString("%1").arg(Offset.value()));
	str += logisdom::saveformat("NextCountInit", countInit.getNext());
	str += logisdom::saveformat("CountInitMode", countInit.getMode());
	str += logisdom::saveformat("CountInitEnabled", countInit.getSaveMode());
	str += logisdom::saveformat("SaveOnUpdate", QString("%1").arg(SaveOnUpdate.isChecked()));
	str += logisdom::saveformat("Parameter", QString("%1").arg(ParameterList.currentIndex()));*/
}





void devswbin::setconfig(const QString &strsearch)
{
/*	bool ok;
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
//	int offset = 0;
//	offset = logisdom::getvalue("counterOffset", strsearch).toInt(&ok);
//	if (ok) Offset.setValue(offset);
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
	if (ok) ParameterList.setCurrentIndex(p);*/
}





void devswbin::saveOnUpdateChanged(int state)
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








void devswbin::setResult(long int NewValue)
{
/*	switch (counterMode.currentIndex())
	{
		case NormalMode :
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
	}*/
}



