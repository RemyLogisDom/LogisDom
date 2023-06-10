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





#include <QtGui>
#include "globalvar.h"
#include "energiesolaire.h"
#include "chauffageunit.h"
#include "daily.h"
#include "addDaily.h"
#include "vmc.h"
#include "configwindow.h"
#include "server.h"
#include "logisdom.h"
#include "htmlbinder.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "programevent.h"



ProgramData::ProgramData()
{
//	connect(&Enable, SIGNAL(stateChanged(int)), this, SLOT(change(int)));
	connect(&Time, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(change(QDateTime)));
}




void ProgramData::change()
{
	emit(DataChange(this));
}



void ProgramData::change(int)
{
	emit(DataChange(this));
}




void ProgramData::change(QDateTime)
{
	htmlBind->setParameter("Time", Time.time().toString("HH:mm"));
	htmlBind->setValue(Time.time().toString("HH:mm"));
	emit(DataChange(this));
}





void ProgramData::remoteCommand(const QString &command)
{
	QDateTime T = Time.dateTime();
	if (command == minutePlus5) Time.setDateTime(T.addSecs(300));
	if (command == minuteMinus5) Time.setDateTime(T.addSecs(-300));
	if (command == hourPlus1) Time.setDateTime(T.addSecs(3600));
	if (command == hourMinus1) Time.setDateTime(T.addSecs(-3600));
}





ProgramEvent::ProgramEvent(logisdom *Parent, configwindow *cfgParent)
{
	parent = Parent;
	configParent = cfgParent;
	PrgLayout = new QGridLayout(this);
	setWidgetResizable(true);

	htmlBind = new htmlBinder(parent, tr("Event"));
    //parent->addHtmlBinder(htmlBind);
	connect(htmlBind, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));

    htmlBindChooser = new htmlBinder(parent, tr("Select"), htmlBind->treeItem);
    //parent->addHtmlBinder(htmlBindChooser);
    connect(htmlBindChooser, SIGNAL(remoteCommand(QString)), this, SLOT(remoteCommand(QString)));

	ButtonAuto.setText(tr("Automatic"));
	ButtonAuto.setCheckable(true);
	ButtonGroup.addButton(&ButtonAuto);
    //PrgLayout->addWidget(&ButtonAuto, 0, 0, 1, 1);
    //htmlBindChooser->setParameterLink(ButtonAuto.text(), progAuto);
    //htmlBindChooser->setParameter(ButtonAuto.text(), progAuto);
    htmlBindChooser->addParameterCommand("Mode", cstr::toStr(cstr::Auto), progAuto);

	ButtonOutside.setText(tr("Outside"));
	ButtonOutside.setCheckable(true);

	SwitchToAuto.setText(tr("Switch to Automatic at next time match"));
    //PrgLayout->addWidget(&SwitchToAuto, 0, 1, 1, 1);

	ButtonConfort.setText(tr("Confort"));
	ButtonConfort.setCheckable(true);
	ButtonGroup.addButton(&ButtonConfort);
    //PrgLayout->addWidget(&ButtonConfort, 1, 0, 1, 1);
    //htmlBindChooser->setParameterLink(ButtonConfort.text(), progConfort);
    //htmlBindChooser->setParameter(ButtonConfort.text(), progConfort);
    htmlBindChooser->addParameterCommand("Mode", cstr::toStr(cstr::Confort), progConfort);

    ButtonNuit.setText(tr("Nuit"));
    ButtonNuit.setCheckable(true);
    ButtonGroup.addButton(&ButtonNuit);
    //PrgLayout->addWidget(&ButtonNuit, 1, 1, 1, 1);
    //htmlBindChooser->setParameterLink(ButtonNuit.text(), progNuit);
    //htmlBindChooser->setParameter(ButtonNuit.text(), progNuit);
    htmlBindChooser->addParameterCommand("Mode", cstr::toStr(cstr::Nuit), progNuit);

    ButtonEco.setText(tr("Eco"));
	ButtonEco.setCheckable(true);
	ButtonGroup.addButton(&ButtonEco);
    //PrgLayout->addWidget(&ButtonEco, 1, 2, 1, 1);
    //htmlBindChooser->setParameterLink(ButtonEco.text(), progEco);
    //htmlBindChooser->setParameter(ButtonEco.text(), progEco);
    htmlBindChooser->addParameterCommand("Mode", cstr::toStr(cstr::Eco), progEco);

	ButtonHorsGel.setText(tr("Unfrost"));
	ButtonHorsGel.setCheckable(true);
	ButtonGroup.addButton(&ButtonHorsGel);
    //PrgLayout->addWidget(&ButtonHorsGel, 1, 3, 1, 1);
    //htmlBindChooser->setParameterLink(ButtonHorsGel.text(), progUnfrost);
    //htmlBindChooser->setParameter(ButtonHorsGel.text(), progUnfrost);
    htmlBindChooser->addParameterCommand("Mode", cstr::toStr(cstr::HorsGel), progUnfrost);

	connect(&ButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(updateProcess(int)));
	ButtonAuto.click();
}



ProgramEvent::~ProgramEvent()
{
	delete PrgLayout;
}




void ProgramEvent::remoteCommand(const QString &command)
{
	if (command == progAuto) ButtonAuto.click();
	if (command == progOutside) ButtonOutside.click();
	if (command == progConfort) ButtonConfort.click();
    if (command == progNuit) ButtonNuit.click();
    if (command == progEco) ButtonEco.click();
	if (command == progUnfrost) ButtonHorsGel.click();
	for (int n=0; n<PrgEvntList.count(); n++)
		if (command == PrgEvntList[n]->Button.text().remove(' ')) PrgEvntList[n]->Button.click();
}




void ProgramEvent::getHtmlMenu(QString *str, QString &ID, int Privilege)
{
	str->append(tr("Current Program is "));
	QString current = getButtonName();
	if (current.isEmpty()) str->append(ButtonAuto.text());
	else str->append(current);
	str->append("<br><br>");
	if (Privilege == Server::FullControl)
	{
		str->append(logisdom::toHtml(ButtonAuto.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
		str->append("<br>");
		str->append(logisdom::toHtml(ButtonConfort.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
		str->append("<br>");
        str->append(logisdom::toHtml(ButtonNuit.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
        str->append("<br>");
        str->append(logisdom::toHtml(ButtonEco.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
		str->append("<br>");
		//str->append(toHtml(ButtonOutside.text(), ID, "command"));
		//str->append("<br>");
		str->append(logisdom::toHtml(ButtonHorsGel.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
		str->append("<br>");
		for (int n=0; n<PrgEvntList.count(); n++)
		{
			str->append(logisdom::toHtml(PrgEvntList[n]->Button.text(), NetRequestMsg[SwitchProgram], ID, logisdom::htmlStyleCommand));
			str->append("<br>");
		}
	}
	else
	{
		for (int n=0; n<PrgEvntList.count(); n++)
		{
			str->append(PrgEvntList[n]->Button.text()+ "<br>");
		}
		str->append(ButtonAuto.text());
		//str->append(ButtonOutside.text());
		str->append(ButtonHorsGel.text());
		str->append(ButtonConfort.text());
        str->append(ButtonNuit.text());
        str->append(ButtonEco.text());
	}
}







void ProgramEvent::SwitchPrg(QString prg)
{
	if (prg == ButtonAuto.text()) ButtonAuto.click();
	if (prg == ButtonOutside.text()) ButtonOutside.click();
	if (prg == ButtonConfort.text()) ButtonConfort.click();
    if (prg == ButtonNuit.text()) ButtonNuit.click();
    if (prg == ButtonEco.text()) ButtonEco.click();
	if (prg == ButtonHorsGel.text()) ButtonHorsGel.click();
	for (int n=0; n<PrgEvntList.count(); n++)
		if (prg == PrgEvntList[n]->Button.text()) PrgEvntList[n]->Button.click();
}





void ProgramEvent::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		QMenu contextualmenu;
		QAction ActionAdd(cstr::toStr(cstr::Add), this);
		QAction ActionRemove(cstr::toStr(cstr::Remove), this);
		QAction ActionSort(tr("Sort"), this);
		QAction ActionAddSunRise(tr("Add Sunrise"), this);
		QAction ActionAddSunSet(tr("Add Sunset"), this);
		contextualmenu.addAction(&ActionAdd);
		contextualmenu.addAction(&ActionRemove);
		contextualmenu.addAction(&ActionSort);
		bool found = false;
		for (int n=0; n<PrgEvntList.count(); n++)
			if (PrgEvntList[n]->TimeMode ==  ProgramEvent::Sunrise) found = true;
		if (!found) contextualmenu.addAction(&ActionAddSunRise);
		found = false;
		for (int n=0; n<PrgEvntList.count(); n++)
			if (PrgEvntList[n]->TimeMode ==  ProgramEvent::Sunset) found = true;
		if (!found) contextualmenu.addAction(&ActionAddSunSet);
		QAction *selection;
#if QT_VERSION < 0x060000
        selection = contextualmenu.exec(event->globalPos());
#else
        selection = contextualmenu.exec(event->globalPosition().toPoint());
#endif
		QDateTime now = QDateTime::currentDateTime();
		if (!parent->isRemoteMode())
		{
            if (selection == &ActionAdd) AddEvent("", now.time(), 1, ProgramEvent::None);
			if (selection == &ActionRemove) supprimer();
			if (selection == &ActionSort) trier();
            if (selection == &ActionAddSunRise) AddEvent(tr("Sunrise"), QTime(0, 0), 1, ProgramEvent::Sunrise);
            if (selection == &ActionAddSunSet) AddEvent(tr("Sunset"), QTime(0, 0), 1, ProgramEvent::Sunset);
		}
	}
	if (event->button() != Qt::LeftButton) return;
}






void ProgramEvent::updateProcess(int)
{
    //if (isAutomatic()) htmlBind->setValue(ButtonAuto.text());
    //else htmlBind->setValue(getButtonName());
    //parent->ChauffageArea->process();
    //        htmlBind->setParameter(ModeTxt, AutoModeTxt);
    if (isAutomatic()) htmlBindChooser->setParameter("Mode", ButtonAuto.text());
    else htmlBindChooser->setParameter("Mode", getButtonName());
    parent->ChauffageArea->process();

}





void ProgramEvent::checkTimeMatch()
{
	if (isAutomatic()) return;
	if (isConfort()) return;
	if (isEco()) return;
	if (isUnfreeze()) return;
	if (!SwitchToAuto.isChecked()) return;
	QDateTime now = QDateTime::currentDateTime();
	for (int n=0; n<PrgEvntList.count(); n++)
		if ((now.time().hour() == PrgEvntList[n]->Time.time().hour()) and (now.time().minute() == PrgEvntList[n]->Time.time().minute())) ButtonAuto.click();
}






void ProgramEvent::SaveConfigStr(QString &str)
{
	str += logisdom::saveformat("SelectionText", ButtonGroup.checkedButton()->text());
	for (int n=0; n<PrgEvntList.count(); n++)
	{
                str += Program_Event_Mark "\n";
        str += logisdom::saveformat("Name", PrgEvntList[n]->Button.text(), true);
		str += logisdom::saveformat("HH", QString("%1").arg(PrgEvntList[n]->Time.time().hour()));
		str += logisdom::saveformat("mm", QString("%1").arg(PrgEvntList[n]->Time.time().minute()));
//		if (PrgEvntList[n]->Enable.isChecked()) str += logisdom::saveformat("Enabled", "1"); else  str += logisdom::saveformat("Enabled", "0");
		str += logisdom::saveformat("TimeMode", QString("%1").arg(PrgEvntList[n]->TimeMode));
		str += EndMark;
		str +="\n";
	}
}




void ProgramEvent::readconfigfile(const QString &configdata)
{
	QString data, Selection;
	data.append(configdata);
	Selection = logisdom::getvalue("SelectionText", data);
	QString  ReadName;
	bool okh, okm, oken,  oktm, foundMark = false;
	int HH, mm, Enabled, TimeMode;
	QString TAG_Begin = Program_Event_Mark;
	QString TAG_End = EndMark;
	SearchLoopBegin
	foundMark = true;
	ReadName = logisdom::getvalue("Name", strsearch);
	HH = logisdom::getvalue("HH", strsearch).toInt(&okh, 10);
	mm = logisdom::getvalue("mm", strsearch).toInt(&okm, 10);
	Enabled = logisdom::getvalue("Enabled", strsearch).toInt(&oken, 10);
	TimeMode = logisdom::getvalue("TimeMode", strsearch).toInt(&oktm, 10);
    if (okh && okm && !ReadName.isEmpty()) AddEvent(ReadName, QTime(HH, mm), Enabled, TimeMode);
	SearchLoopEnd
    if (!foundMark)
	{
         AddEvent(tr("Wake Up"), QTime(7, 30), 1, ProgramEvent::None);
         AddEvent(tr("Lunch"), QTime(12, 25), 1, ProgramEvent::None);
         AddEvent(tr("Evening"), QTime(19, 15), 1, ProgramEvent::None);
         AddEvent(tr("Sleep"), QTime(23, 10), 1, ProgramEvent::None);
    }
	trier();
	if (Selection == ButtonEco.text()) ButtonEco.setChecked(true);
    if (Selection == ButtonNuit.text()) ButtonNuit.setChecked(true);
    if (Selection == ButtonConfort.text()) ButtonConfort.setChecked(true);
	if (Selection == ButtonHorsGel.text()) ButtonHorsGel.setChecked(true);
	if (Selection == ButtonOutside.text()) ButtonOutside.setChecked(true);
    for (int n=0; n<PrgEvntList.count(); n++)
		if (Selection == PrgEvntList[n]->Button.text()) PrgEvntList[n]->Button.setChecked(true);
    updateProcess(0);
}





void ProgramEvent::trier()
{
	int count = PrgEvntList.count();
	for (int n=0; n<count; n++)
	{
		PrgLayout->removeWidget(&PrgEvntList[n]->Button);
		PrgLayout->removeWidget(&PrgEvntList[n]->Time);
//		PrgLayout->removeWidget(&PrgEvntList[n]->Enable);
		if (PrgEvntList[n]->TimeMode == ProgramEvent::Sunrise) PrgEvntList[n]->Time.setTime(parent->EnergieSolaire->getSunRise());
		if (PrgEvntList[n]->TimeMode == ProgramEvent::Sunset) PrgEvntList[n]->Time.setTime(parent->EnergieSolaire->getSunSet());
	}
	bool invert = true;
	if (count > 1)
	{
		while (invert)
 		{
			invert = false;
			for (int n=0; n<count-1; n++)
			{
				if (PrgEvntList[n]->Time.time().secsTo(PrgEvntList[n+1]->Time.time()) < 0)
				{
#if QT_VERSION < 0x060000
                    PrgEvntList.swap(n, n+1);
#else
                    PrgEvntList.swapItemsAt(n, n+1);
#endif
					invert = true;
				}
			}
		}
	}
	for (int n=0; n<count; n++)
	{
		PrgLayout->addWidget(&PrgEvntList[n]->Button, n + 2, 0, 1, 1);
		PrgLayout->addWidget(&PrgEvntList[n]->Time, n + 2, 1, 1, 1);
        //PrgLayout->addWidget(&PrgEvntList[n]->Enable, n + 2, 2, 1, 1);
	}
}




void ProgramEvent::supprimer()
{
	bool ok;
	QStringList items;
	int count = PrgEvntList.count();
	for (int n=0; n<count; n++) items << PrgEvntList[n]->Button.text();
    QString item = inputDialog::getItemPalette(this, cstr::toStr(cstr::Remove), cstr::toStr(cstr::Remove), items, 0, false, &ok, parent);
	if (!ok) return;
	if (item.isEmpty()) return;
	for (int n=0; n<PrgEvntList.count(); n++)
		if (PrgEvntList[n]->Button.text() == item)
		{
			ProgramData *toRemove =  PrgEvntList[n];
			PrgLayout->removeWidget(&PrgEvntList[n]->Button);
			PrgLayout->removeWidget(&PrgEvntList[n]->Time);
//			PrgLayout->removeWidget(&PrgEvntList[n]->Enable);
			PrgEvntList.removeAt(n);
			emit(removeProgramEvent(toRemove));
			delete toRemove;
			trier();
			return;
		}
}




void ProgramEvent::AddEvent(QString Name, QTime Time, int, int TimeMode)
{
	bool ok;
	QString nom = Name;
	if (nom.isEmpty())
	{
Retry :
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, nom, &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	for (int i=0; i<PrgEvntList.count(); i++)
		if (PrgEvntList[i]->Button.text() == nom)
		{
			messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto Retry;
		}
	}
	int i = PrgEvntList.count();
	ProgramData *newPrg = new ProgramData;
	newPrg->htmlBind = new htmlBinder(parent, nom, htmlBind->treeItem);
	PrgEvntList.append(newPrg);
	newPrg->Button.setText(nom);
	newPrg->Button.setCheckable(true);
	ButtonGroup.addButton(&newPrg->Button);
	PrgLayout->addWidget(&newPrg->Button, i + 2, 0, 1, 1);
	newPrg->Time.setTime(Time);
	newPrg->Time.setDisplayFormat( "'" + nom + "   '" + "HH:mm");
	newPrg->TimeMode = TimeMode;
	if (TimeMode != daily::None) newPrg->Time.setReadOnly(true);
	if (TimeMode == ProgramEvent::Sunrise) newPrg->Time.setTime(parent->EnergieSolaire->getSunRise());
	if (TimeMode == ProgramEvent::Sunset) newPrg->Time.setTime(parent->EnergieSolaire->getSunSet());
	newPrg->htmlBind->setValue(newPrg->Time.time().toString("HH:mm"));
	QTreeWidgetItem *item = newPrg->htmlBind->setParameter("Time", newPrg->Time.time().toString("HH:mm"));
	if (item)
	{
		newPrg->htmlBind->addCommand("+", hourPlus1, item);
		newPrg->htmlBind->addCommand("-:", hourMinus1, item);
		newPrg->htmlBind->addCommand("+", minutePlus5, item);
		newPrg->htmlBind->addCommand("-", minuteMinus5, item);
	}
	connect(newPrg->htmlBind, SIGNAL(remoteCommand(QString)), newPrg, SLOT(remoteCommand(QString)));
	PrgLayout->addWidget(&newPrg->Time, i + 2, 1, 1, 1);
//	PrgLayout->addWidget(&newPrg->Enable, i + 2, 2, 1, 1);
//	if (Enabled) newPrg->Enable.setChecked(true);
	connect(newPrg, SIGNAL(DataChange(ProgramData *)), this, SLOT(ProgHasChanged(ProgramData *)));
	trier();
	configParent->ProgHasChanged(newPrg);
	QString N = nom;
	htmlBindChooser->setParameterLink(N, nom.remove(' '));
	emit(addProgramEvent(newPrg));
}




void ProgramEvent::ProgHasChanged(ProgramData *prog)
{
	emit(changeProgramEvent(prog));
}



void ProgramEvent::getComboList(QComboBox * Combo)
{
	QString curText = Combo->currentText();
	Combo->clear();
	Combo->addItem(tr("Time"));
	for (int n=0; n<PrgEvntList.count(); n++) Combo->addItem(PrgEvntList[n]->Button.text());
	Combo->setCurrentIndex(Combo->findText(curText));
}




QTimeEdit *ProgramEvent::GetTime(QString Name)
{
	for (int n=0; n<PrgEvntList.count(); n++)
	{
		if (PrgEvntList[n]->Button.text() == Name)
		{
			return &PrgEvntList[n]->Time;
		}
	}
	return nullptr;
}


bool ProgramEvent::isAutomatic()
{
	return ButtonAuto.isChecked();
}



bool ProgramEvent::isOutside()
{
	return ButtonOutside.isChecked();
}



bool ProgramEvent::isConfort()
{
	return ButtonConfort.isChecked();
}


bool ProgramEvent::isNuit()
{
    return ButtonNuit.isChecked();
}



bool ProgramEvent::isEco()
{
	return ButtonEco.isChecked();
}



bool ProgramEvent::isUnfreeze()
{
	return ButtonHorsGel.isChecked();
}




QString ProgramEvent::getButtonName()
{
	if (ButtonAuto.isChecked()) return "";
	if (ButtonOutside.isChecked()) return ButtonOutside.text();
	if (ButtonHorsGel.isChecked()) return ButtonHorsGel.text();
	if (ButtonConfort.isChecked()) return ButtonConfort.text();
    if (ButtonNuit.isChecked()) return ButtonNuit.text();
    if (ButtonEco.isChecked()) return ButtonEco.text();
	for (int n=0; n<PrgEvntList.count(); n++)
		if (PrgEvntList[n]->Button.isChecked()) return PrgEvntList[n]->Button.text();
	return "";
}



QTime ProgramEvent::getButtonTime()
{
	QTime T(0, 0);
	for (int n=0; n<PrgEvntList.count(); n++)
        if (PrgEvntList[n]->Button.isChecked()) { T.setHMS(PrgEvntList[n]->Time.time().hour(), PrgEvntList[n]->Time.time().minute(), 0); return T; }
	return T;
}


