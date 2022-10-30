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
#include "logisdom.h"
#include "programevent.h"
#include "messagebox.h"
#include "daily.h"




dailyUnit::dailyUnit(logisdom *Parent)
{
	parent = Parent;
    autoTxt = tr("Auto");
	setLayout(&setupLayout);
	setupLayout.addWidget(&Type);
	connect(&Type, SIGNAL(currentIndexChanged(int)), this, SLOT(typechange(int)));
	Time.setDisplayFormat("HH:mm");
	setupLayout.addWidget(&Time);
	connect(&Time, SIGNAL(timeChanged(QTime)), this, SLOT(timeChange(QTime)));
	Before.setText(tr("Before"));
	After.setText(tr("After"));
	setupLayout.addWidget(&Before);
	connect(&Before, SIGNAL(stateChanged(int)), this, SLOT(ClickBefore(int)));
	setupLayout.addWidget(&After);
	connect(&After, SIGNAL(stateChanged(int)), this, SLOT(ClickAfter(int)));
	Value.setValue(19);
	setupLayout.addWidget(&State);
	connect(&State, SIGNAL(currentIndexChanged(int)), this, SLOT(emitChange(int)));
	setupLayout.addWidget(&Value);
	connect(&Value, SIGNAL(valueChanged(double)), this, SLOT(emitChange(double)));
	setMode(daily::Temp);
}




dailyUnit::~dailyUnit()
{
}



void dailyUnit::getCfgStr(QString &str)
{
	str += New_Point_Begin "\n";
	str += logisdom::saveformat("Name", Type.currentText());
	str += logisdom::saveformat("HH", Time.time().toString("HH"));
	str += logisdom::saveformat("mm", Time.time().toString("mm"));
	str += logisdom::saveformat("Value", QString("%1").arg(Value.value()));
	str += logisdom::saveformat("State", State.currentText());
	if (Before.isChecked()) str += logisdom::saveformat("Shift", "Before");
		else if (After.isChecked()) str += logisdom::saveformat("Shift", "After");
		else str += logisdom::saveformat("Shift", "None");
	str += New_Point_Finished "\n";
}




void dailyUnit::setCfgStr(QString &strsearch)
{
	bool ok_h, ok_m, ok;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) Type.setCurrentIndex(Type.findText(Name));
	int HH = logisdom::getvalue("HH", strsearch).toInt(&ok_h, 10);
	int mm = logisdom::getvalue("mm", strsearch).toInt(&ok_m, 10);
	if (ok_h && ok_m) Time.setTime(QTime(HH, mm, 0));
	double v = logisdom::getvalue(Device_Value_Tag, strsearch).toDouble(&ok);
	if (ok) Value.setValue(v);
	QString state = logisdom::getvalue("State", strsearch);
	if (!state.isEmpty()) State.setCurrentIndex(State.findText(state));
	QString shift = logisdom::getvalue("Shift", strsearch);
	if (shift == "Before") Before.setCheckState(Qt::Checked);
	else if (shift == "After") After.setCheckState(Qt::Checked);
}




void dailyUnit::timeChange(QTime)
{
	emit(changed(this));
}




void dailyUnit::setMode(int mode)
{
	Mode = mode;
	switch 	(mode)
	{
		case daily::Temp : 
			Value.setRange(0 ,35);
			Value.setSingleStep(0.5);
			Value.setDecimals(1);
			Value.setSuffix("°C");
			Value.setEnabled(true);
			State.setEnabled(false);
			State.clear();
			break;
        case daily::Variable :
			Value.setRange(-1 ,100);
			Value.setSingleStep(1);
			Value.setDecimals(0);
			Value.setSuffix("%");
            Value.setSpecialValueText(autoTxt);
			Value.setEnabled(true);
			State.setEnabled(false);
			State.clear();
			break;
        case daily::VariableOpenClose :
            Value.setRange(-1 ,255);
            Value.setSingleStep(1);
            Value.setDecimals(0);
            Value.setSuffix(" s");
            Value.setSpecialValueText(autoTxt);
            Value.setEnabled(true);
            State.setEnabled(false);
            State.clear();
            break;
        case daily::ONOFF :
			Value.setRange(0 ,1);
			Value.setSingleStep(1);
			Value.setDecimals(0);
			Value.setSuffix("");
			Value.setEnabled(false);
			State.setEnabled(true);
			State.clear();
			State.addItem(cstr::toStr(cstr::OFF));	// StateOFF
			State.addItem(cstr::toStr(cstr::ON));	// StateON
            State.addItem(autoTxt);			// stateAuto
			State.addItem(cstr::toStr(cstr::Disabled)); // StateDisabled
			break;
        case daily::OpenClose :
			Value.setRange(0 ,2);
			Value.setSingleStep(1);
			Value.setDecimals(0);
			Value.setSuffix("");
			Value.setEnabled(false);
			State.setEnabled(true);
			State.clear();
            State.addItem(cstr::toStr(cstr::Close));
            State.addItem(cstr::toStr(cstr::Open));
            State.addItem(autoTxt);
			State.addItem(cstr::toStr(cstr::Disabled));
            break;
		case daily::Analog :
		default :
			Value.setRange(-2147483640, +2147483640);
			Value.setSingleStep(0.1);
			Value.setDecimals(10);
			Value.setSuffix("");
			Value.setEnabled(true);
			State.setEnabled(false);
			State.clear();
			break;
	}
}




QString dailyUnit::getStr()
{
	QString Str;
	QTimeEdit *T = parent->ProgEventArea->GetTime(Type.currentText());
	int DelaySecs = Time.time().hour() * 3600 + Time.time().minute() * 60;
	Str += Type.currentText() + "  ";
	if (Type.currentIndex() == 0) Str += Time.time().toString("HH:mm");
		else if (T)
		{
			if (Before.isChecked()) Str += T->time().addSecs(-DelaySecs).toString("HH:mm");
				else if (After.isChecked()) Str += T->time().addSecs(DelaySecs).toString("HH:mm");
				else Str += T->time().toString("HH:mm");	
		}
		else Str += tr("Event removed");
	switch 	(Mode)
	{
		case daily::Temp : Str += QString("   %1 °C").arg(Value.value());
			break;
        case daily::Variable : if (int(Value.value()) == -1) Str += "   " + autoTxt;
			else Str += QString("   %1 %").arg(Value.value());
			break;
        case daily::VariableOpenClose : if (int(Value.value()) == -1) Str += "   " + autoTxt;
            else Str += QString("   %1 s").arg(Value.value());
            break;
        case daily::ONOFF : Str += "   " + State.currentText();
			break;
        case daily::OpenClose : Str += "   " + State.currentText();
            break;
		case daily::Analog : Str += QString("   %1").arg(Value.value());
            break;
        default : break;
	}
	return Str;
}




void dailyUnit::ClickBefore(int)
{
	After.setCheckState(Qt::Unchecked);
	if (Before.isChecked())
	{
		Time.setEnabled(true);
	}
	else
	{
		Time.setEnabled(false);
	}
	emit(changed(this));
}




void dailyUnit::ClickAfter(int)
{
	Before.setCheckState(Qt::Unchecked);
	if (After.isChecked())
	{
		Time.setEnabled(true);
	}
	else
	{
		Time.setEnabled(false);
	}
	emit(changed(this));
}




QTime dailyUnit::time()
{
	QTimeEdit *T = parent->ProgEventArea->GetTime(Type.currentText());
	int DelaySecs = Time.time().hour() * 3600 + Time.time().minute() * 60;
    if (Type.currentIndex() == 0) return Time.time();
    else
    {
        if (!T) return QTime(0, 0, 0);
        else
        {
            if (Before.isChecked()) return T->time().addSecs(-DelaySecs);
            else
            {
                if (After.isChecked()) return T->time().addSecs(DelaySecs);
                else return T->time();
            }
        }
    }
}




void dailyUnit::emitChange(int)
{
	emit(changed(this));
}




void dailyUnit::emitChange(double)
{
	emit(changed(this));
}



void dailyUnit::typechange(int)
{
	if (Type.currentIndex() == 0)
	{
		Time.setEnabled(true);
		Before.setEnabled(false);
		After.setEnabled(false);
		Before.setCheckState(Qt::Unchecked);
		After.setCheckState(Qt::Unchecked);
	}
	else if ((!Before.isChecked()) and (!After.isChecked()))
	{
		Time.setEnabled(false);
		Before.setEnabled(true);
		After.setEnabled(true);
	}
	emit(changed(this));
}



void dailyUnit::RemovePrgEvt(ProgramData *prog)
{
	int index = Type.findText(prog->Button.text());
	if (index != -1) Type.removeItem(index);
}




void dailyUnit::AddPrgEvt(ProgramData *prog)
{
	Type.addItem(prog->Button.text());
}




double dailyUnit::getValue()
{
	switch (Mode)
	{
		case daily::Temp : return Value.value();
        case daily::Variable : return Value.value();
        case daily::VariableOpenClose : return Value.value();
        case daily::ONOFF : return double(State.currentIndex());
        case daily::OpenClose : return double(State.currentIndex());
		case daily::Analog : return Value.value();
        default : break;
	}
	return logisdom::NA;
}




daily::daily(QWidget*, logisdom *Parent, const QString &name, bool userChange)
{
    ui.setupUi(this);
    timeLayout = new QGridLayout(ui.framedui);
    parent = Parent;
	previousSetup = nullptr;
	Name = name;
    ui.Mode->addItem(tr("Temperature"));
    ui.Mode->addItem(tr("Ligth"));
    ui.Mode->addItem(tr("ON/OFF"));
    ui.Mode->addItem(tr("Open/Close"));
    ui.Mode->addItem(tr("Numeric"));
    ui.Mode->addItem(tr("Variable Open/Close"));
    connect(ui.List, SIGNAL(currentRowChanged(int)), this, SLOT(RowChanged(int)));
    if (!userChange) ui.Mode->hide();
    connect(ui.AddButton, SIGNAL(clicked()), this, SLOT(newOne()));
    connect(ui.RemoveButton, SIGNAL(clicked()), this, SLOT(removepoint()));
    connect(ui.Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMode(int)));
}




daily::~daily()
{
    for (int n=0; n<dailyList.count(); n++) delete dailyList[n];
}




void daily::RowChanged(int dailyUnit)
{
	if (dailyUnit == -1) return;
    if (dailyUnit < dailyList.count())
	{
		if (previousSetup) previousSetup->hide();
		previousSetup = dailyList[dailyUnit];
        timeLayout->addWidget(dailyList.at(dailyUnit), dailyList.count(), 0, 1, 1);
        dailyList[dailyUnit]->show();
	}
}




void daily::changeMode(int mode)
{
    for (int n=0; n<dailyList.count(); n++)	dailyList[n]->setMode(mode);
	updateList();
}







void daily::sort()
{
	bool invert = true;
    int count = dailyList.count();
	if (count > 1)
	{
		while (invert)
 		{
			invert = false;
			for (int n=0; n<count-1; n++)
			{
				if (dailyList[n]->time().secsTo(dailyList[n+1]->time()) < 0)
				{
                    dailyList.swap(n, n+1);
					invert = true;
				}
			}
		}
	}
}




void daily::updateList()
{
	sort();
    ui.List->clear();
    for (int n=0; n<dailyList.count(); n++)
	{
        QListWidgetItem *widgetList = new QListWidgetItem(ui.List);
		widgetList->setText(dailyList[n]->getStr());
	}
}





void daily::updateList(dailyUnit *)
{
	updateList();
}






void daily::newOne()
{
	addpoint();
    RowChanged(dailyList.count() - 1);
}




dailyUnit *daily::addpoint()
{
	dailyUnit *P = new dailyUnit(parent);
    dailyList.append(P);
	parent->ProgEventArea->getComboList(&P->Type);
	connect(P, SIGNAL(changed(dailyUnit *)), this, SLOT(updateList(dailyUnit *)));
	P->Type.setCurrentIndex(0);
    P->setMode(ui.Mode->currentIndex());
	updateList();
	return P;
}






void daily::removepoint()
{
    int index = ui.List->currentIndex().row();
	if (index == -1) return;
	if ((messageBox::questionHide(this, tr("Remove event"),
			tr("Are you sure you want to remove this event\n") , parent,
			QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
			{
				if (previousSetup) previousSetup->hide();
				dailyList[index]->setAttribute(Qt::WA_DeleteOnClose, true);
				dailyList[index]->close();
                dailyList.removeAt(index);
				previousSetup = nullptr;
				updateList();
			}
}




void daily::removepoint(int n)
{
	if (n < 0) return;
    if (n >= dailyList.count()) return;
	if (previousSetup) previousSetup->hide();
	dailyList[n]->setAttribute(Qt::WA_DeleteOnClose, true);
	dailyList[n]->close();
    dailyList.removeAt(n);
	previousSetup = nullptr;
	updateList();
}




void daily::RemovePrgEvt(ProgramData * prog)
{
	bool found;
	int count;
	do
	{
		found = false;
        count = dailyList.count();
		for (int n=0; n<count; n++)
		{
			if (dailyList[n]->Type.currentText() == prog->Button.text())
			{
				removepoint(n);
				found = true;
				break;
			}
		}
	} while (found);
    count = dailyList.count();
	for (int n=0; n<count; n++) dailyList[n]->RemovePrgEvt(prog);
	updateList();
}





void daily::AddPrgEvt(ProgramData *prog)
{
    for (int n=0; n<dailyList.count(); n++) dailyList[n]->AddPrgEvt(prog);
}




void daily::changePrgEvt(ProgramData*)
{
	updateList();
}




double daily::getValue(const QTime &T)
{
	double actualValue = logisdom::NA;
    int count = dailyList.count();
	if (count == 0) return actualValue;
	if (count == 1)		// let value from previous day taking over
	{
		if (T.secsTo(dailyList[0]->time()) > 0) return actualValue;
		else return dailyList[0]->getValue();
	}
	for (int n=0; n<count-1; n++)
	{
		if ((T.secsTo(dailyList[n]->time()) <= 0) and (T.secsTo(dailyList[n+1]->time()) > 0)) return dailyList[n]->getValue();
	}
	if (T.secsTo(dailyList[count-1]->time()) <= 0)	return dailyList[count-1]->getValue();
	return actualValue;
}





double daily::getLastValue()
{
	double actualValue = logisdom::NA;
    int count = dailyList.count();
	if (count == 0) return actualValue;
	actualValue = dailyList[count-1]->getValue();
	return actualValue;
}





void daily::setMode(QString mode)
{
    if (!mode.isEmpty()) ui.Mode->setCurrentIndex(ui.Mode->findText(mode));
}




void daily::setMode(int mode)
{
    ui.Mode->setCurrentIndex(mode);
}



int daily::getMode()
{
    return ui.Mode->currentIndex();
}



void  daily::getStrConfig(QString &str)
{
    str += logisdom::saveformat("Name", Name, true);
    str += logisdom::saveformat("Mode", ui.Mode->currentText(), true);
    str += logisdom::saveformat("ModeIndex", QString("%1").arg(ui.Mode->currentIndex()));
    for (int n=0; n<dailyList.count(); n++) dailyList[n]->getCfgStr(str);
}




void  daily::setStrConfig(const QString &strsearch)
{
	int currentindex, nextindex;
	currentindex = strsearch.indexOf(New_Point_Begin, 0);
	nextindex = currentindex;
	do
	{
		if (currentindex != -1)
		{
			nextindex = strsearch.indexOf(New_Point_Finished, currentindex);
			if (nextindex != -1)
			{
				QString NewPoint = strsearch.mid(currentindex, nextindex - currentindex);
				addpoint()->setCfgStr(NewPoint);
			}
		}
		currentindex = strsearch.indexOf(New_Point_Begin, nextindex);
	}
	while (currentindex != -1);
}


