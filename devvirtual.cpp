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




#include <QtWidgets/QFileDialog>
#include "globalvar.h"
#include "errlog.h"
#include "alarmwarn.h"
#include "interval.h"
#include "formula.h"
#include "onewire.h"
#include "configwindow.h"
#include "devvirtual.h"
#include "inputdialog.h"


devvirtual::devvirtual(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    initialCalcul = true;
    deviceLoading = false;
    ui.gridLayout->addWidget(&trafficUi, 2, 1, 1, 2);
    FormulCalc = new formula(Parent);
    FormulCalc->deviceParent = this;
    saveLecture = false;
    lastsavevalue = logisdom::NA;
    romid = RomID;
    family = romid.right(2);
    ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("Virtual device"));
    calculInterval.setName(tr("Calculate Interval"));
    setupLayout.addWidget(&calculInterval,  4, 1, 1, 1);
    //ValueOnErrorEnable.setText(tr("Value on error"));
    //setupLayout.addWidget(&ValueOnErrorEnable, layoutIndex, 0, 1, 1);
    //setupLayout.addWidget(&ValueOnError, layoutIndex++, 1, 1, 1);
    disable_Device.setText(tr("Disable Device"));
    setupLayout.addWidget(&disable_Device,  layoutIndex++, 0, 1, 1);
    setupLayout.addWidget(FormulCalc, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(FormulCalc, SIGNAL(calcdone()), this, SLOT(setResult()));
}




devvirtual::~devvirtual()
{
}




void devvirtual::stopAll()
{
	//logMsg("stopAll");
	FormulCalc->stopAll();
}




void devvirtual::closeEvent(QCloseEvent*)
{
	//FormulCalc->stopAll();
	//this->close();
}





void devvirtual::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu contextualmenu;
	QAction Calculate(tr("&Calculate"), this);
	QAction Lecture(tr("&Lecture"), this);
    QAction SaveConfig(tr("&Save device config"), this);
    //if (FormulCalc->reprossthread->isRunning()) Calculate.setEnabled(false);
    if (FormulCalc->threadr->isRunning()) Calculate.setEnabled(false);
    QAction Nom(tr("&Name"), this);
    if (FormulCalc->thread->isRunning()) Lecture.setEnabled(false);
    if (FormulCalc->thread->isRunning()) Calculate.setEnabled(false);
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
		contextualmenu.addAction(&Calculate);
        contextualmenu.addAction(&SaveConfig);
    }
	else
	{
		contextualmenu.addAction(&Lecture);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Calculate) LectureManual();
	if (selection == &Lecture) LectureManual();
	if (selection == &Nom) changename();
    if (selection == &SaveConfig) saveDeviceConfig();
}




void devvirtual::saveDeviceConfig()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text (*.txt)"));
    if (!fileName.endsWith(".txt")) fileName.append(".txt");
    bool ok;
    QString str;
    getCfgStr(str, false);
    QString f = FormulCalc->getFormula();
    for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
    {
        onewiredevice *dev = parent->configwin->devicePtArray[n];
        QString devName = dev->getname();
        int index = f.indexOf(devName);
        if (index != -1)
        {
            QString nom = "[[" + devName + "]]";
            f = f.replace(devName, nom);
        }
    }
    for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
    {
        onewiredevice *dev = parent->configwin->devicePtArray[n];
        QString devRomID = dev->getromid();
        QString devName = dev->getname();
        int index = f.indexOf(devRomID);
        if (index != -1)
        {
            QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), tr("Confirm device name : "), QLineEdit::Normal, devName, &ok, parent);
            if (!ok) return;
            if (nom.isEmpty()) return;
            nom = "[[" + nom + "]]";
            f = f.replace(devRomID, nom);
        }
    }
    str += logisdom::saveformat("Formula", f, true);
    str += logisdom::saveformat("NextCalculateInterval", calculInterval.getNext());
    str += logisdom::saveformat("CalculateIntervalMode", calculInterval.getMode());
    str += logisdom::saveformat("CalculateIntervalEnabled", calculInterval.getSaveMode());
    str += logisdom::saveformat("Disable_Device", QString("%1").arg(disable_Device.isChecked()));
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setGenerateByteOrderMark(true);
        out << str;
        file.close();
    }
}



QString devvirtual::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    QString webStrResult = FormulCalc->calcth->webStrResult;
    if (logisdom::isNA(MainValue) && webStrResult.isEmpty())
    {
        if (FormulCalc->ui.ValueOnErrorEnable->isChecked()) S = FormulCalc->getOnErrorTxtValue();
        else S = cstr::toStr(cstr::NA);
        if (deviceLoading) S += " " + tr("Loading");
    }
    else if (!webStrResult.isEmpty() && UsetextValues.isChecked()) S = webStrResult;
	if (disable_Device.isChecked()) S = cstr::toStr(cstr::Disabled);
    ui.valueui->setText(S);
	return S;
}




/*QString devvirtual::ValueToStr(double Value, bool noText)
{
    QString str;
    if (Value == logisdom::NA)
    {
        if (FormulCalc->ui.ValueOnErrorEnable->isChecked()) str = FormulCalc->getOnErrorTxtValue();
        else str = cstr::toStr(cstr::NA);
        if (deviceLoading) str += " " + tr("Loading");
    }
    else str = onewiredevice::ValueToStr(Value, noText);
    return str;
}*/



void devvirtual::LectureManual()
{
	//logMsg("LectureManual");
	QMutexLocker locker(&mutexCalc);
	if (parent->isRemoteMode())
	{
                setValid(ReadRequest);
		if (master) master->getMainValue();
	}
	else
	{
		FormulCalc->CalculateThread();
	}
}



void devvirtual::lecture()
{
        if (disable_Device.isChecked()) return;
        QMutexLocker locker(&mutexCalc);
	//logMsg("lecture");
	if (parent->isRemoteMode())
	{
		setValid(ReadRequest);
		if (master) master->getMainValue();
	}
	else
	{
		saveLecture = false;
		if (calculInterval.SaveEnable.isChecked())
		{
			if (calculInterval.isitnow() or initialCalcul)
			{
				setValid(ReadRequest);
				FormulCalc->CalculateThread();
			}
		}
	}
}






void devvirtual::lecturerec()
{
	//logMsg("lecturerec");
    if (disable_Device.isChecked()) return;
	QMutexLocker locker(&mutexCalc);
	saveLecture = true;
	if (parent->isRemoteMode())
	{
		setValid(ReadRequest);
		if (master) master->saveMainValue();
	}
	else
	{
		if (calculInterval.SaveEnable.isChecked())
		{
			if (calculInterval.isitnow() or initialCalcul)
			{
				setValid(ReadRequest);
                FormulCalc->CalculateThread();
			}
			else savevalue(QDateTime::currentDateTime(), MainValue);
		}
		else savevalue(QDateTime::currentDateTime(), MainValue);
	}
}




bool devvirtual::isReprocessing()
{
    return FormulCalc->threadr->isRunning();
}



void devvirtual::setResult()
{
	//logMsg("setResult");
	deviceLoading = FormulCalc->calcth->deviceLoading;
	if (FormulCalc->calcth->deviceLoading)
	{
        if (FormulCalc->ui.ValueOnErrorEnable->isChecked()) MainValue = FormulCalc->getOnErrorValue();
        else MainValue = logisdom::NA;
		setValid(dataWaiting);
	}
	else if ((FormulCalc->calcth->syntax) && (FormulCalc->calcth->dataValid))
	{
		double v = FormulCalc->calcth->threadResult; 
        if (logisdom::isNotNA(v))
		{
			MainValue = v; 
			initialCalcul = false;
			ClearWarning();
			if (saveLecture) savevalue(QDateTime::currentDateTime(), MainValue);
		}
		else
		{
            if (FormulCalc->ui.ValueOnErrorEnable->isChecked())
            {
                MainValue = FormulCalc->getOnErrorValue();
                ClearWarning();
            }
            else
            {
                MainValue = logisdom::NA;
                IncWarning();
                setValid(dataNotValid);
            }
        }
	}
	else
	{
        if (FormulCalc->ui.ValueOnErrorEnable->isChecked())
        {
            MainValue = FormulCalc->getOnErrorValue();
            ClearWarning();
        }
        else
        {
            MainValue = logisdom::NA;
            IncWarning();
            setValid(dataNotValid);
        }
	}
    QString valueStr = MainValueToStr();
    htmlBind->setValue(valueStr);
    htmlBind->setParameter("Value", valueStr.remove(getunit()));
	emitDeviceValueChanged();
    if (listTreeItem) listTreeItem->setText(1,valueStr);
}




bool devvirtual::isVirtualFamily()
{
	return true;
}




bool devvirtual::IncWarning()
{
	if (!WarnEnabled.isChecked()) return true;
	RetryWarning ++;
	if (RetryWarning <= 3) return false;
	parent->alarmwindow->GeneralErrorLog->SetError(40, romid);
	parent->alarmwindow->ui.errortab->setCurrentIndex(0);
	setValid(dataNotValid);
	parent->mainStatus();
	return true;
}





void devvirtual::GetConfigStr(QString &str)
{
	//QString Fo = F.replace('(', "[");
	//QString Fc = Fo.replace(')', "]");
	str += logisdom::saveformat("Formula", FormulCalc->getFormula(), true);
	str += logisdom::saveformat("NextCalculateInterval", calculInterval.getNext());
	str += logisdom::saveformat("CalculateIntervalMode", calculInterval.getMode());
	str += logisdom::saveformat("CalculateIntervalEnabled", calculInterval.getSaveMode());
	str += logisdom::saveformat("Disable_Device", QString("%1").arg(disable_Device.isChecked()));
    str += logisdom::saveformat("ValueOnErrorEnabled", QString("%1").arg(FormulCalc->ui.ValueOnErrorEnable->isChecked()));
    str += logisdom::saveformat("ValueOnErrorTxt", FormulCalc->ui.ValueOnError->text());
}




void devvirtual::setconfig(const QString &strsearch)
{
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0, ok;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (Name.isEmpty()) setname(assignname("Virtual"));
	else setname(Name);
	x = logisdom::getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = logisdom::getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if ((ok_h != 0) and (h == 0)) show(); else hide();
	setWindowTitle(name);
    //QString F = logisdom::getvalue("Formula", strsearch);
    QString str = strsearch;
    QString result;
    QString search = "Formula";
    int l = str.length();
    if (l != 0)
    {
        int i = str.indexOf(search);
        if (i != -1)
        {
            int coma = str.indexOf("(", i);
            if (coma != -1)
            {
                int nextcoma = str.indexOf(")", coma + 1);
                if (nextcoma == -1) nextcoma = str.indexOf("(", coma + 1);
                if (nextcoma != -1)
                {
                    result = str.mid(coma + 1, nextcoma - coma - 1);
                    if (!result.isEmpty())
                    {
                        if (result.mid(0, 4) == "HEX:")
                        {
                           QByteArray Hex;
#if QT_VERSION < 0x060000
                           Hex.append(result.remove(0, 4).toUtf8());
#else
                           Hex.append(result.remove(0, 4).toLatin1());
#endif
                            // Qt 6 QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
                            QByteArray F = QByteArray::fromHex(Hex);
                            result = F; // Qt 6 Utf8Codec->toUnicode(F);
                        }
                        else
                        {
                            QString Fo = result.replace('[', "(");
                            result = Fo.replace(']', ")");
                        }
                    }
                }
            }
        }
    }
    FormulCalc->setFormula(result);
    QString next = logisdom::getvalue("NextCalculateInterval", strsearch);
	if (next.isEmpty()) calculInterval.setNext(QDateTime::currentDateTime());
		else calculInterval.setNext(QDateTime::fromString (next, Qt::ISODate));
	int savemode = logisdom::getvalue("CalculateIntervalMode", strsearch).toInt(&ok);
	if (ok) calculInterval.setMode(savemode);
	int saveen = logisdom::getvalue("CalculateIntervalEnabled", strsearch).toInt(&ok);
	if (ok)
	{
		if (saveen) calculInterval.setSaveMode(true);
		else calculInterval.setSaveMode(false);
	}
	else calculInterval.setSaveMode(true);
	int disable_Dev = logisdom::getvalue("Disable_Device", strsearch).toInt(&ok);
        if (ok && disable_Dev) disable_Device.setChecked(true);
    int errsaveen = logisdom::getvalue("ValueOnErrorEnabled", strsearch).toInt(&ok);
    if (ok && errsaveen) FormulCalc->ui.ValueOnErrorEnable->setChecked(true);
    FormulCalc->ui.ValueOnError->setText(logisdom::getvalue("ValueOnErrorTxt", strsearch));
}

