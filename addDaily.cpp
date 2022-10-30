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
#include <QDateTime>
#include "globalvar.h"
#include "daily.h"
#include "logisdom.h"
#include "configwindow.h"
#include "programevent.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "addDaily.h"




addDaily::addDaily(logisdom *Parent)
{
	parent = Parent;
	previousSetup = nullptr;
	int index = 0;
// palette setup
	setup.setLayout(&setupLayout);

    QFrame *buttonFrame = new QFrame;
    QGridLayout *buttonLayout = new QGridLayout;
    buttonFrame->setLayout(buttonLayout);
    setupLayout.addWidget(buttonFrame, 0, 0, 1, logisdom::PaletteWidth);
    setupLayout.setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    //QIcon DailyIcon(QString::fromUtf8(":/images/images/daily.png"));
    //DailyTool.setIconSize(QSize(logisdom::iconSize, logisdom::iconSize));
    //DailyTool.setIcon(DailyIcon);
    //DailyTool.setToolTip(tr("Daily"));
    //buttonLayout->addWidget(&DailyTool, 0, index++, 1, 1);

	QIcon addIcon(QString::fromUtf8(":/images/images/edit_add.png"));
	AddButton.setIcon(addIcon);
	AddButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
	AddButton.setToolTip(tr("Add"));
    buttonLayout->addWidget(&AddButton, 0, index++, 1, 1);

	QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
	RemoveButton.setIcon(removeIcon);
	RemoveButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
	RemoveButton.setToolTip(tr("Remove"));
    buttonLayout->addWidget(&RemoveButton, 0, index++, 1, 1);

	QIcon renameIcon(QString::fromUtf8(":/images/images/rename.png"));
	RenameButton.setIcon(renameIcon);
	RenameButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
	RenameButton.setToolTip(tr("Rename"));
    buttonLayout->addWidget(&RenameButton, 0, index++, 1, 1);

    setupLayout.addWidget(&List);
	lastwidget = nullptr;
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(Add()));
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(Rename()));
	connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(Remove()));
	connect(&List, SIGNAL(currentRowChanged(int)), this, SLOT(RowChanged(int)));
}




double addDaily::getActualValue(int indexNow, int indexPrevious)
{
	double actualValue = logisdom::NA;
	if ((indexNow < 0) or (indexNow > dailyarray.count()-1)) return actualValue;
	QDateTime now = QDateTime::currentDateTime();
//	GenMsg(QString("indexNow = %1").arg(indexNow));
//	GenMsg(QString("actualValue = %1").arg(actualValue));
	QTime T = now.time();
	actualValue = dailyarray[indexNow]->getValue(T);
    if (logisdom::isNotNA(actualValue)) return actualValue;
		else return dailyarray[indexPrevious]->getLastValue();
}



double addDaily::getValue(int indexNow, int indexPrevious, const QDateTime &datetime)
{
    double actualValue = logisdom::NA;
    if ((indexNow < 0) or (indexNow > dailyarray.count()-1)) return actualValue;
//	GenMsg(QString("indexNow = %1").arg(indexNow));
//	GenMsg(QString("actualValue = %1").arg(actualValue));
    QTime T = datetime.time();
    actualValue = dailyarray[indexNow]->getValue(T);
    if (logisdom::isNotNA(actualValue)) return actualValue;
        else return dailyarray[indexPrevious]->getLastValue();
}



double addDaily::getValueAt(int indexNow, int indexPrevious, const QTime &time)
{
	double actualValue = logisdom::NA;
	if ((indexNow < 0) or (indexNow > dailyarray.count()-1)) return actualValue;
	actualValue = dailyarray[indexNow]->getValue(time);
    if (logisdom::isNotNA(actualValue)) return actualValue;
		else return dailyarray[indexPrevious]->getLastValue();
}




double addDaily::getDailyProgValue(const QString &progName, const QDateTime &T)
{
	double Value = logisdom::NA;
	for (int n=0; n<dailyarray.count(); n++)
		if (dailyarray[n]->Name == progName)
		{
			Value = dailyarray[n]->getValue(T.time());
            if (logisdom::isNA(Value)) Value = dailyarray[n]->getLastValue();
		}
	return Value;
}





void addDaily::updateJourneyBreaks(ProgramData*)
{
	int count = dailyarray.count();
	for (int n=0; n<count; n++)
        dailyarray[n]->sort();
}




void addDaily::Add()
{
bool ok;
QString name;
Retry:
    name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if ((!ok) || name.isEmpty()) return;
	if (dailyProgNameExist(name) != -1)
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	NewOne(name);
}




int addDaily::dailyProgNameExist(const QString &name)
{
	for (int n=0; n<dailyarray.count(); n++)
		if (dailyarray[n]->Name == name) return n;
	return -1;
}




void addDaily::RowChanged (int daily)
{
	if (daily == -1) return;
	if (daily < dailyarray.count())
	{
		if (previousSetup) previousSetup->hide();
		previousSetup = dailyarray[daily];
		setupLayout.addWidget(dailyarray[daily], 5, 0);
		dailyarray[daily]->show();
	}
}




void addDaily::Remove()
{
	int index = List.currentIndex().row();
	if (index == -1) return;
	if (messageBox::questionHide(this, tr("Remove ?"),
		tr("Do you really want to remove ") + dailyarray[index]->Name, parent,
		QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			{
				if (previousSetup) previousSetup->hide();
				parent->removeWidget(dailyarray[index]);
				daily *dalyToRemove = dailyarray.takeAt(index);
				parent->configwin->RemoveDaily(dalyToRemove);
				emit RemoveDaily(dalyToRemove->Name);
				delete dalyToRemove;
				previousSetup = nullptr;
				updateList();
			}
}





void addDaily::Rename()
{
	QString nom;
	bool ok;
	int index = List.currentIndex().row();
	if (index == -1) return;
	Retry:
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::NewName), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, dailyarray[index]->Name, &ok, parent);
	if ((!ok) || nom.isEmpty() || (nom == dailyarray[index]->Name)) return;
	bool found = false;
	int count = dailyarray.count();
	for (int n=0; n<count; n++)
		if ((n != index) && (dailyarray[n]->Name == nom))
			found = true;
	if (found)
	{
		messageBox::warningHide(this, tr("Daily name"), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	else
	{
		 dailyarray[index]->Name = nom;
		updateList();
	}
}





daily *addDaily::NewOne(QString Name)
{
QString name;
	if (Name.isEmpty()) name = cstr::toStr(cstr::NoName); else name = Name;
	if (dailyProgNameExist(name) != -1)
	{
		int index = 1;
		int lastunderscore = name.lastIndexOf("_");
		if (lastunderscore == -1) name += QString("_1");
		int n = dailyProgNameExist(name);
		while (n != -1)
		{
			lastunderscore = name.lastIndexOf("_");
			name = name.mid(0, lastunderscore) + QString("_%1").arg(index);
		 	n = dailyProgNameExist(name);
		 	index ++;
	 	}
 	}
	daily *newdaily = new daily(this, parent, name);
	dailyarray.append(newdaily);
	emit(AddNewDaily(newdaily->Name));
	connect(newdaily, SIGNAL(ValueChange()), parent->AddDaily, SLOT(ValueasChanged()));
	connect(newdaily, SIGNAL(change(daily *)), parent->configwin, SLOT(dailyHasChange(daily *)));
	updateList();
	return newdaily;
}





void addDaily::updateList()
{
	List.clear();
	for (int n=0; n<dailyarray.count(); n++)
	{
		QListWidgetItem *widgetList = new QListWidgetItem(&List);
		widgetList->setText(dailyarray[n]->Name);
	}
}





void addDaily::ValueasChanged()
{
	emit(ValueChange());
}




void addDaily::RemovePrgEvt(ProgramData * prog)
{
	for (int n=0; n<dailyarray.count(); n++)
		dailyarray[n]->RemovePrgEvt(prog);
}




void addDaily::AddPrgEvt(ProgramData * prog)
{
	for (int n=0; n<dailyarray.count(); n++)
		dailyarray[n]->AddPrgEvt(prog);
}




void addDaily::changePrgEvt(ProgramData * prog)
{
	for (int n=0; n<dailyarray.count(); n++)
		dailyarray[n]->changePrgEvt(prog);
}




void  addDaily::SaveConfigStr(QString &str)
{
	for (int n=0; n<dailyarray.count(); n++)
	{
		str += "\n" Daily_Unit_Mark "\n";
		dailyarray[n]->getStrConfig(str);
		str += EndMark;
	}
}






void addDaily::readconfigfile(const QString &configdata)
{
	QString ReadName, Mode;
	int ModeIndex;
	bool ok;
	daily *P;
	QString TAG_Begin = Daily_Unit_Mark;
	QString TAG_End = EndMark;
	SearchLoopBegin
		ReadName = logisdom::getvalue("Name", strsearch);
		Mode = logisdom::getvalue("Mode", strsearch);
		ModeIndex = logisdom::getvalue("ModeIndex", strsearch).toInt(&ok);
		P = NewOne(ReadName);
		if (Mode.isEmpty()) P->setMode(Mode);
		else P->setMode(ModeIndex);
		P->setStrConfig(strsearch);
	SearchLoopEnd
}








