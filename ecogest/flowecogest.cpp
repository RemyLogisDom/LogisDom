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
#include "ecogest.h"
#include "flowecogest.h"




flowecogest::flowecogest(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	family4 = romid.right(4);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Flow MultiGest"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
    MainValue = logisdom::NA;
    LastCounter = logisdom::NA;
    Delta = logisdom::NA;
    CoefLabel.setText("Coef : ");
    setupLayout.addWidget(&CoefLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    Coef.setText("1");
    setupLayout.addWidget(&Coef, layoutIndex++, 1, 1, logisdom::PaletteWidth/2);
    counterMode.addItem(tr("Absolute"));
    counterMode.addItem(tr("Relative"));
    counterMode.addItem(tr("Offset"));
    countInit.setName(tr("Offset update"));
    setupLayout.addWidget(&counterMode, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    Offset.setRange(-2147483640, +2147483640);
    Offset.setPrefix(tr("Offset : "));
    setupLayout.addWidget(&Offset, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&countInit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    SaveOnUpdate.setText(tr("Save only on update"));
    setupLayout.addWidget(&SaveOnUpdate,  layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&SaveOnUpdate, SIGNAL(stateChanged(int)), this, SLOT(saveOnUpdateChanged(int)));
    connect(&counterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
}





bool flowecogest::setscratchpad(const QString &scratchpad, bool enregistremode)
{
    if ((counterMode.currentIndex() == relativeMode) && !enregistremode) return true;
	ScratchPad_Show.setText(scratchpad);
	bool ok;
	double NewValue = scratchpad.toDouble(&ok);
	if (ok)
	{
        //long int counter = (0x10000 * CMSB) + CLSB;
        long int counter = NewValue;
        setResult(counter, enregistremode);
        setMainValue(MainValue, false);
        //ui.MainText->setText(QString("%1 pulses, Delta = %2").arg(counter).arg(Delta));
        ui.valueui->setText(MainValueToStr());
        bool offsetUpdate = countInit.isitnow();
        if (enregistremode)
        {
            if ((SaveOnUpdate.isChecked()) && (counterMode.currentIndex() == offsetMode))
            {
                if (offsetUpdate) savevalue(QDateTime::currentDateTime(), MainValue, false);
            }
            else savevalue(QDateTime::currentDateTime(), MainValue);
        }
        if ((counterMode.currentIndex() == offsetMode) && (offsetUpdate)) Offset.setValue((int)counter);

        //MainValue = NewValue * Coef.value();
        //savevalue(QDateTime::currentDateTime(), MainValue);
        //MainValueToStr();
        //setValid(dataValid);
        //htmlBind->setValue(MainValueToStr());
        //htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
        //ClearWarning();
        emitDeviceValueChanged();
		return true;
	}
    else
    {
        if (scratchpad == "Opened") setMainValue(1, enregistremode);
        if (scratchpad == "Closed") setMainValue(0, enregistremode);
    }
	return false;
}





void flowecogest::setResult(long int NewValue, bool enregistremode)
{
    switch (counterMode.currentIndex())
    {
        case absoluteMode :
            if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
            if (enregistremode) LastCounter = NewValue;
            MainValue = result(NewValue);
        break;
        case relativeMode :
            if (enregistremode)
            {
                if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0;
                else Delta = NewValue - LastCounter;
                LastCounter = NewValue;
                MainValue = result(Delta);
            }
        break;
        case offsetMode :
            if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
            if (enregistremode) LastCounter = NewValue;
            MainValue = result((NewValue - Offset.value()));
        break;
    }
}




double flowecogest::result(double value)
{
/*    bool ok;
    if (Coef.text().isEmpty()) return value;
    bool a_Valid = true;
    double a = Coef.text().toDouble(&ok);
    if (!ok)
    {
        QString txt = Coef.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (B != 0)) a = A/B; else a_Valid = false;
            }
        }
        else a_Valid = false;
    }
    if (!a_Valid) return value;*/
    double R = Coef.result(value);
    return R;
}



void flowecogest::contextMenuEvent(QContextMenuEvent * event)
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






QString flowecogest::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}




QString flowecogest::ValueToStr(double Value, bool)
{
	QString str;
	if (Value == logisdom::NA) str = cstr::toStr(cstr::NA);
	else str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
	return str;
}




void flowecogest::SetOrder(const QString &)
{
}





void flowecogest::lecture()
{
	QString scratchpad;
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "FAMF")
			{
				QString V = logisdom::getvalue("MeanFlowA", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "FBMF")
			{
				QString V = logisdom::getvalue("MeanFlowB", scratchpad);
				setscratchpad(V, false);
			}
			if (family4 == "FCMF")
			{
				QString V = logisdom::getvalue("MeanFlowB", scratchpad);
				setscratchpad(V, false);
			}
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void flowecogest::lecturerec()
{
setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case MultiGest :
			scratchpad = master->getScratchPad(NetRequestMsg[GetStatus]);
			if (family4 == "FAMF")
			{
				QString V = logisdom::getvalue("MeanFlowA", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "FBMF")
			{
				QString V = logisdom::getvalue("MeanFlowB", scratchpad);
				setscratchpad(V, true);
			}
			if (family4 == "FCMF")
			{
				QString V = logisdom::getvalue("MeanFlowC", scratchpad);
				setscratchpad(V, true);
			}
		break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





void flowecogest::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("Coef", Coef.text());
    str += logisdom::saveformat("counterMode", QString("%1").arg(counterMode.currentIndex()));
    str += logisdom::saveformat("counterOffset", QString("%1").arg(Offset.value()));
    str += logisdom::saveformat("NextCountInit", countInit.getNext());
    str += logisdom::saveformat("CountInitMode", countInit.getMode());
    str += logisdom::saveformat("CountInitEnabled", countInit.getSaveMode());
    str += logisdom::saveformat("SaveOnUpdate", QString("%1").arg(SaveOnUpdate.isChecked()));
}





void flowecogest::setconfig(const QString &strsearch)
{
	bool ok;
    QString coefstr = logisdom::getvalue("Coef", strsearch);
    if (!coefstr.isEmpty()) Coef.setText(coefstr);
    QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	else setname(assignname(tr("Flow ")));
    Offset.setEnabled(false);
    countInit.setEnabled(false);
    SaveOnUpdate.setEnabled(false);
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
}





void flowecogest::saveOnUpdateChanged(int state)
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





void flowecogest::modeChanged(int)
{
    QDateTime now = QDateTime::currentDateTime();
    switch (counterMode.currentIndex())
    {
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
