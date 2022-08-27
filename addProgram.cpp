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




#include <QtCore>
#include <QtGui>
#include "globalvar.h"
#include "addDaily.h"
#include "daily.h"
#include "logisdom.h"
#include "chauffageunit.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "addProgram.h"




weeklyProgramUnit::weeklyProgramUnit(logisdom *Parent, QString name)
{
	parent = Parent;
	Name  = name;
// palette setup
	setup.setLayout(&setupLayout);
        NameButton.setText(name);
	setupLayout.addWidget(&NameButton, 0, 0, 1, 2);
	for (int n=0; n<weeklyProgram::NbDays; n++)
	{
		DaysCombo[n] = new QComboBox(this);
		DaysName[n] = new QLabel(this);
		setupLayout.addWidget(DaysCombo[n], n + 1, 1, 1, 1);
		setupLayout.addWidget(DaysName[n], n + 1, 0, 1, 1);
	}
	DaysName[0]->setText(tr("Monday"));
	DaysName[1]->setText(tr("Tuesday"));
	DaysName[2]->setText(tr("Wednesday"));
	DaysName[3]->setText(tr("Thursday"));
	DaysName[4]->setText(tr("Friday"));
	DaysName[5]->setText(tr("Saturday"));
	DaysName[6]->setText(tr("Sunday"));
}






weeklyProgramUnit::~weeklyProgramUnit()
{
	for (int n=0; n<weeklyProgram::NbDays; n++)
	{
		delete DaysCombo[n];
		delete DaysName[n];
	}
}






void weeklyProgramUnit::AddDaily(QString Name)
{
	for (int d=0; d<weeklyProgram::NbDays; d++)
		DaysCombo[d]->addItem(Name);
}




void weeklyProgramUnit::RemoveDaily(QString Name)
{
	for (int d=0; d<weeklyProgram::NbDays; d++)
		DaysCombo[d]->removeItem(DaysCombo[d]->findText(Name));
}




weeklyProgram::weeklyProgram(logisdom *Parent)
{
	parent = Parent;
	int index = 0;
// palette setup
        setup.setLayout(&setupLayout);

    QFrame *buttonFrame = new QFrame;
    QGridLayout *buttonLayout = new QGridLayout;
    buttonFrame->setLayout(buttonLayout);
    setupLayout.addWidget(buttonFrame, 0, 0, 1, logisdom::PaletteWidth);

    //QIcon WeeklyIcon(QString::fromUtf8(":/images/images/weekly.png"));
    //ToolButton.setIconSize(QSize(logisdom::iconSize, logisdom::iconSize));
    //ToolButton.setIcon(WeeklyIcon);
    //ToolButton.setToolTip(tr("Weekly"));
    //buttonLayout->addWidget(&ToolButton, 0, index++, 1, 1);

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
	previousSetup = nullptr;
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(Add()));
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(Rename()));
	connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(Remove()));
	connect(&List, SIGNAL(currentRowChanged(int)), this, SLOT(RowChanged(int)));
}





QString weeklyProgram::dayToString(int D)
{
	QString Str;
	switch (D)
	{
		case Monday : Str = tr("Monday"); break;
		case Tuesday : Str = tr("Tuesday"); break;
		case Wednesday : Str = tr("Wednesday"); break;
		case Thursday : Str = tr("Thursday"); break;
		case Friday : Str = tr("Friday"); break;
		case Saturday : Str = tr("Saturday"); break;
		case Sunday : Str = tr("Sunday"); break;
		default : Str = "Unkown"; break;
	}
	return Str;
}



void weeklyProgram::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
	{
		QMenu contextualmenu;
		QAction contextualaction0(cstr::toStr(cstr::Add), this);
		QAction contextualaction1(cstr::toStr(cstr::Remove), this);
		//QAction contextualaction2(cstr::toStr(cstr::Lock), this);
		//QAction contextualaction3(cstr::toStr(cstr::Unlock), this);
		contextualmenu.addAction(&contextualaction0);
		contextualmenu.addAction(&contextualaction1);
		//if (Locked == false) contextualmenu.addAction(&contextualaction2);
		//else contextualmenu.addAction(&contextualaction3);
		QAction *selection;
		selection = contextualmenu.exec(event->globalPos());
        if (selection == &contextualaction0) Add();
        if (selection == &contextualaction1) Remove();
		//if (selection == &contextualaction2) verrouiller();
		//if (selection == &contextualaction3) deverrouiller();
	}
    if (event->button() != Qt::LeftButton) return;
}






void weeklyProgram::RowChanged (int row)
{
	if (row == -1) return;
	if (row < program.count())
	{
		if (previousSetup) previousSetup->hide();
		previousSetup = &program[row]->setup;
		setupLayout.addWidget(&program[row]->setup, 5, 0, 1, logisdom::PaletteWidth);
		program[row]->setup.show();
	}
}





void weeklyProgram::Remove()
{
	int index = List.currentIndex().row();
	if (index == -1) return;
	if (messageBox::questionHide(this, tr("Remove ?"),
		tr("Do you really want to remove ") + program[index]->Name, parent,
		QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			{
				if (previousSetup) previousSetup->hide();
				previousSetup = nullptr;
				weeklyProgramUnit *programToRemove = program.takeAt(index);
				updateList();
				emit(RemoveProgram(programToRemove->Name));
			}
}





void weeklyProgram::Rename()
{
/*




	QString nom;
	bool ok;
	int index = List.currentIndex().row();
	if (index == -1) return;
	bool paletteHidden = maison1wirewindow->isPaletteHidden();
	maison1wirewindow->PaletteHide(true);
	maison1wirewindow->PaletteHide(paletteHidden);
	Retry:
	nom = inputDialog::getText(this, cstr::toStr(cstr::NewName), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, dailyarray[index]->Name, &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (nom == dailyarray[index]->Name) return;
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
	}*/
}





int weeklyProgram::weeklyProgNameExist(const QString &name)
{
	for (int n=0; n<program.count(); n++)
		if (program[n]->Name == name) return n;
	return -1;
}







void weeklyProgram::Add()
{
	bool ok;
 	QString name;
Retry:
    name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, maison1wirewindow);
	if ((!ok) || name.isEmpty()) return;
	if (weeklyProgNameExist(name) != -1)
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	NewOne(name);
}






weeklyProgramUnit *weeklyProgram::NewOne(QString Name)
{
QString name;
	if (Name.isEmpty()) name = cstr::toStr(cstr::NoName); else name = Name;
	if (weeklyProgNameExist(name) != -1)
	{
		int index = 1;
		int lastunderscore = name.lastIndexOf("_");
		if (lastunderscore == -1) name += QString("_1");
		int n = weeklyProgNameExist(name);
		while (n != -1)
		{
			lastunderscore = name.lastIndexOf("_");
			name = name.mid(0, lastunderscore) + QString("_%1").arg(index);
		 	n = weeklyProgNameExist(name);
		 	index ++;
	 	}
 	}
	weeklyProgramUnit *newprogram = new weeklyProgramUnit(parent, name);
	program.append(newprogram);
//	connect(newprogram->Name, SIGNAL(textChanged(QString)), this, SLOT(updateName(QString)));
	emit(AddProgram(Name));
	connect(parent->AddDaily, SIGNAL(AddNewDaily(QString)), newprogram, SLOT(AddDaily(QString)));
	connect(parent->AddDaily, SIGNAL(RemoveDaily(QString)), newprogram, SLOT(RemoveDaily(QString)));
	for (int n=0; n<parent->AddDaily->dailyarray.count(); n++) newprogram->AddDaily(maison1wirewindow->AddDaily->dailyarray[n]->Name);
	updateList();
	return newprogram;
}




int weeklyProgram::getPreviousProgram(int indexPrg)
{
	if (indexPrg == -1) return -1;
	int dailyIndex = -1;
	QDateTime now = QDateTime::currentDateTime().addDays(-1);
	int day = now.date().dayOfWeek(); // 1 = Monday ... 7 = Sunday
	dailyIndex = program[indexPrg]->DaysCombo[day - 1]->currentIndex();
	return dailyIndex;
}




int weeklyProgram::getPreviousProgram(const QString &name, const QDateTime &T)
{
    int indexPrg = -1;
    for (int n=0; n<program.count(); n++)
    {
        if (program.at(n)->Name == name)
        {
            indexPrg = n;
            break;
        }
    }
    if (indexPrg == -1) return -1;
    int dailyIndex = -1;
    int day = T.date().dayOfWeek(); // 1 = Monday ... 7 = Sunday
    dailyIndex = program[indexPrg]->DaysCombo[day - 1]->currentIndex();
    return dailyIndex;
}




int weeklyProgram::getActualProgram(int indexPrg)
{
    if (indexPrg == -1) return -1;
    int dailyIndex = -1;
    QDateTime now = QDateTime::currentDateTime();
    int day = now.date().dayOfWeek(); // 1 = Monday ... 7 = Sunday
    dailyIndex = program[indexPrg]->DaysCombo[day - 1]->currentIndex();
    return dailyIndex;
}




int weeklyProgram::getActualProgram(const QString &name, const QDateTime &T)
{
    int indexPrg = -1;
    for (int n=0; n<program.count(); n++)
    {
        if (program.at(n)->Name == name)
        {
            indexPrg = n;
            break;
        }
    }
    if (indexPrg == -1) return -1;
    int dailyIndex = -1;
    int day = T.date().dayOfWeek(); // 1 = Monday ... 7 = Sunday
    dailyIndex = program[indexPrg]->DaysCombo[day - 1]->currentIndex();
    return dailyIndex;
}



void weeklyProgram::updateList()
{
	List.clear();
	for (int n=0; n<program.count(); n++)
	{
		QListWidgetItem *widgetList = new QListWidgetItem(&List);
		widgetList->setText(program[n]->Name);
	}
}




void weeklyProgram::updateName(const QString&)
{
	int count = program.count();
	for (int n=0; n<count; n++)
	{
		//programList.setItemText(n, program[n]->Name->text());
		parent->ChauffageArea->setProgramName(n, program[n]->Name);
	}
}







void  weeklyProgram::SaveConfigStr(QString &str)
{
	int count = program.count();
	for (int n=0; n<count; n++)
	{
	 	str += "Program Unit\n";
        str += logisdom::saveformat("Name", program[n]->Name, true);
		for (int i=0; i<weeklyProgram::NbDays; i++)
            str += logisdom::saveformat(dayToString(i), program[n]->DaysCombo[i]->currentText(), true);
		str += EndMark;
		str +="\n";
	}
}





void weeklyProgram::readconfigfile(const QString &configdata)
{
	weeklyProgramUnit * newprogram;
	QString ReadName;
	QString TAG_Begin = "Program Unit";
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadName = logisdom::getvalue("Name", strsearch);
	newprogram = NewOne(ReadName);
	for (int i=0; i<weeklyProgram::NbDays; i++)
		newprogram->DaysCombo[i]->setCurrentIndex(newprogram->DaysCombo[i]->findText(logisdom::getvalue(dayToString(i), strsearch)));
	SearchLoopEnd
}







