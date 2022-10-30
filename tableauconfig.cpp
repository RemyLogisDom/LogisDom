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
#include "onewire.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "tableauconfig.h"




tableauconfig::tableauconfig(logisdom *Parent)
{
    parent = Parent;
    previousSetup = nullptr;
    int index = 0;
    htmlBind = new htmlBinder(parent, tr("Charts"));
    QFrame *buttonFrame = new QFrame;
    QGridLayout *buttonLayout = new QGridLayout;
    buttonFrame->setLayout(buttonLayout);
    setupLayout.addWidget(buttonFrame, 0, 0, 1, logisdom::PaletteWidth);
    // palette setup
    setup.setLayout(&setupLayout);
    //QIcon tableIcon(QString::fromUtf8(":/images/images/table.png"));
    //TableauTool.setIcon(tableIcon);
    //TableauTool.setIconSize(QSize(logisdom::iconSize, logisdom::iconSize));
    //buttonLayout->addWidget(&TableauTool, 0, 0, 1, 1);

    QIcon addIcon(QString::fromUtf8(":/images/images/edit_add.png"));
    AddButton.setIcon(addIcon);
    AddButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
    AddButton.setToolTip(tr("Add"));
    buttonLayout->addWidget(&AddButton, 1, index++, 1, 1);

    QIcon removeIcon(QString::fromUtf8(":/images/images/edit_remove.png"));
    RemoveButton.setIcon(removeIcon);
    RemoveButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
    RemoveButton.setToolTip(tr("Remove"));
    buttonLayout->addWidget(&RemoveButton, 1, index++, 1, 1);

    //QIcon showIcon(QString::fromUtf8(":/images/images/loupe.png"));
    //ShowButton.setIcon(showIcon);
    //ShowButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
    //ShowButton.setToolTip(tr("Show"));
    //buttonLayout->addWidget(&ShowButton, 1, index++, 1, 1);

    QIcon renameIcon(QString::fromUtf8(":/images/images/rename.png"));
    RenameButton.setIcon(renameIcon);
    RenameButton.setIconSize(QSize(logisdom::statusIconSize, logisdom::statusIconSize));
    RenameButton.setToolTip(tr("Rename"));
    buttonLayout->addWidget(&RenameButton, 1, index++, 1, 1);

    setupLayout.addWidget(&tableauList, 1, 0, 1, logisdom::PaletteWidth);
    connectAll();
}





tableauconfig::~tableauconfig()
{
}




void tableauconfig::DeviceConfigChanged(onewiredevice *device)
{
	for (int n=0; n<tableauPtArray.count(); n++)
		tableauPtArray[n]->updateName(device);
}




void tableauconfig::saveTableau()
{
	for (int n=0; n<tableauPtArray.count(); n++)
		tableauPtArray[n]->save();	
}





void tableauconfig::savePreload()
{
    /*for (int n=0; n<tableauPtArray.count(); n++)
        tableauPtArray[n]->savePreload();*/
}






tableau *tableauconfig::getTableau(const QString &name)
{
    for (int n=0; n<tableauPtArray.count(); n++)
    {
        if (tableauPtArray.at(n)->name == name) return tableauPtArray.at(n);
    }
    return nullptr;
}





tableau *tableauconfig::chooseTableau()
{
    QStringList chartList;
    bool ok;
    for (int n=0; n<tableauPtArray.count(); n++) chartList << tableauPtArray.at(n)->name;
    if (chartList.count() == 0)
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), tr("No charts available"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return nullptr;
    }
    QString chartchoise = inputDialog::getItemPalette(this, tr("Select chart "), tr("Select chart : "), chartList, 0, false, &ok, Qt::Dialog, parent);
    if (!ok) return nullptr;
    for (int n=0; n<tableauPtArray.count(); n++)
    {
        if (chartchoise == tableauPtArray.at(n)->name) return tableauPtArray.at(n);
    }
    return nullptr;
}





void tableauconfig::CurveRowChanged(QWidget *tableau)
{
	parent->setPalette(&setup);
	for (int n=0; n<tableauPtArray.count(); n++)
	{
		if (tableau == &tableauPtArray[n]->setup)
			tableauList.setCurrentRow(n);
	}
}




void tableauconfig::CurveRowChanged (int tableau)
{
	if (tableau == -1) return;
	setPalette(&tableauPtArray[tableau]->setup);
}




void tableauconfig::setPalette(QWidget *tableausetup)
{
	if (previousSetup) previousSetup->hide();
	previousSetup = tableausetup;
	if (tableausetup) setupLayout.addWidget(tableausetup, 6, 0, 1, logisdom::PaletteWidth);
	if (tableausetup) tableausetup->show();
}




void tableauconfig::setPalette()
{
	parent->setPalette(&setup);
}







void tableauconfig::connectAll()
{
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(nouveautableau()));
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(rename()));
	connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(removetableau()));
	connect(&ShowButton, SIGNAL(clicked()), this, SLOT(voir()));
	connect(&TableauTool, SIGNAL(clicked()), this, SLOT(setPalette()));
	connect(&tableauList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
}



void tableauconfig::disconnectAll()
{
	disconnect(&AddButton, SIGNAL(clicked()), this, SLOT(nouveautableau()));
	disconnect(&RenameButton, SIGNAL(clicked()), this, SLOT(rename()));
	disconnect(&RemoveButton, SIGNAL(clicked()), this, SLOT(removetableau()));
	disconnect(&ShowButton, SIGNAL(clicked()), this, SLOT(voir()));
	disconnect(&TableauTool, SIGNAL(clicked()), this, SLOT(setPalette()));
	disconnect(&tableauList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
}




void tableauconfig::readconfigfile(const QString &configdata)
{
// Extract tableau
	QString ReadName;
	QString TAG_Begin = "New_Tableau_Begin";
	QString TAG_End = "New_Tableau_Finished";
	SearchLoopBegin
	ReadName = logisdom::getvalue("Name", strsearch);
	if (!ReadName.isEmpty())
	{
                tableau *NewTableau = nouveautableau(ReadName);
		NewTableau->show();
		NewTableau->raise();
                NewTableau->setTableauConfig(strsearch);		
	}
	SearchLoopEnd
}





void  tableauconfig::SaveConfigStr(QString &str)
{
//	int x, y;
// 	str += "\nTableau\n";
//	str += logisdom::saveformat("Window_Hidden", QString("%1").arg(tableauPtArray[n]->isHidden()));
//	str += EndMark;
//	str += "\n";
	for (int n=0; n<tableauPtArray.count(); n++)
		tableauPtArray[n]->getTableauConfig(str);
}







void tableauconfig::updateList()
{
	tableauList.clear();
	for (int n=0; n<tableauPtArray.count(); n++)
	{
		QListWidgetItem *widgetList = new QListWidgetItem(&tableauList);
		widgetList->setText(tableauPtArray[n]->getName());
	}
}







void tableauconfig::rename()
{
	QString nom;
	bool ok;
	int index = tableauList.currentIndex().row();
	if (index == -1) return;
	Retry:
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Rename), cstr::toStr(cstr::Rename), QLineEdit::Normal, tableauPtArray[index]->getName(), &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (nom == tableauPtArray[index]->getName()) return;
	bool found = false;
	int count = tableauPtArray.count();
	for (int n=0; n<count; n++)
		if ((n != index) && (tableauPtArray[n]->getName() == nom))
			found = true;
	if (found)
	{
        messageBox::warningHide(this, tr("Table name already exist"), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	else
	{
		tableauPtArray[index]->setName(nom);
        parent->ui.tabWidgetChart->setTabText(index, nom);
        updateList();
	}
}






void tableauconfig::nouveautableau()
{
	QString nom;
	bool ok;
Retry:
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	bool found = false;
	int count = tableauPtArray.count();
	for (int n=0; n<count; n++)
		if (tableauPtArray[n]->getName() == nom)
			found = true;
	if (!found) nouveautableau(nom);
	else
	{
		messageBox::warningHide(this, tr("New table"), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
}





void tableauconfig::removetableau()
{
	int index = tableauList.currentIndex().row();
	if (index == -1) return;
	if (messageBox::questionHide(this, tr("Remove table"),
			tr("Are you sure you want to remove table \n") + tableauPtArray[index]->getName() , parent,
			QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			{
				if (previousSetup) previousSetup->hide();
                parent->ui.tabWidgetChart->removeTab(index);
                tableauPtArray.removeAt(index);
                previousSetup = nullptr;
				updateList();
			}
}





void tableauconfig::voir()
{
	int index = tableauList.currentIndex().row();
	if (index == -1) return;
    parent->ui.tabWidget->setCurrentIndex(5);
    parent->ui.tabWidgetChart->setCurrentIndex(index);
}







tableau *tableauconfig::nouveautableau(QString Name)
{
	QString name;
	disconnectAll();
	if (Name == "") name = cstr::toStr(cstr::NoName); else name = Name;
	bool found = false;
	int count = tableauPtArray.count();
	for (int n=0; n<count; n++)
		if (tableauPtArray[n]->getName() == name)
			found = true;
    if (found) name = Name + "_1";
/*	{
		int index = 1;
		int lastunderscore = name.lastIndexOf("_");
		if (lastunderscore == -1) name += QString("_1");
		bool newfound = false;
		int count = tableauPtArray.count();
		for (int n=0; n<count; n++)
			if (tableauPtArray[n]->getName() == name)
				newfound = true;
		while (found)
		{
			lastunderscore = name.lastIndexOf("_");
			name = name.mid(0, lastunderscore) + QString("_%1").arg(index);
			newfound = false;
			int count = tableauPtArray.count();
			for (int n=0; n<count; n++)
				if (tableauPtArray[n]->getName() == name)
					newfound = true;
		 	index ++;
	 	}
    }*/
	tableau *Tableau = new tableau(&name, parent);
	tableauPtArray.append(Tableau);
    parent->ui.tabWidgetChart->addTab(Tableau, name);
    parent->ui.tabWidgetChart->setCurrentIndex(parent->ui.tabWidgetChart->count() - 1);
    //Chart *chart = new Chart();
    //chartPtArray.append(chart);
    //parent->ui.tabWidgetChart->addTab(chart, name);
    //parent->ui.tabWidgetChart->setCurrentIndex(parent->ui.tabWidgetChart->count() - 1);
    updateList();
	connectAll();
	connect(Tableau, SIGNAL(setupClick(QWidget*)), this, SLOT(CurveRowChanged(QWidget*)));
    htmlBind->setParameterLink(name, name + ".png");
    return Tableau;
}

