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
#include "graphconfig.h"




graphconfig::graphconfig(logisdom *Parent)
{
	parent = Parent;
	currentcolor = Qt::blue;
	previousSetup = nullptr;
	htmlBind = new htmlBinder(parent, tr("Graphics"));
	int index = 0;
	update_counter = 0;
// palette setup
    setup.setLayout(&setupLayout);

    QFrame *buttonFrame = new QFrame;
    QGridLayout *buttonLayout = new QGridLayout;
    buttonFrame->setLayout(buttonLayout);
    setupLayout.addWidget(buttonFrame, 0, 0, 1, logisdom::PaletteWidth);

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

    setupLayout.addWidget(&graphList, 1, 0, 1, logisdom::PaletteWidth);
    connectAll();
}





graphconfig::~graphconfig()
{
}






void graphconfig::CurveRowChanged (int graph)
{
	if (graph == -1) return;
	if (graph < graphPtArray.count())
	{
		setPalette(&graphPtArray[graph]->setup);
		parent->ui.tabWidgetGraphic->setCurrentIndex(graph);
		parent->ui.tabWidget->setCurrentIndex(2);
	}
}




void graphconfig::setPalette(QWidget *graphsetup)
{
    if (!graphsetup) return;
    if (previousSetup) previousSetup->hide();
	previousSetup = graphsetup;
	if (graphsetup) setupLayout.addWidget(graphsetup, 6, 0, 1, logisdom::PaletteWidth);
	if (graphsetup) graphsetup->show();
}




void graphconfig::setPalette()
{
	parent->setPalette(&setup);
}




void graphconfig::setGraphPalette()
{
	int index = graphList.currentIndex().row();
	if (index < 0) return;			// if non selection exit
	parent->setPalette(&graphPtArray[index]->setup);
	voir();
}





void graphconfig::connectAll()
{
	connect(&AddButton, SIGNAL(clicked()), this, SLOT(nouveaugraph()));
	connect(&RenameButton, SIGNAL(clicked()), this, SLOT(rename()));
	connect(&RemoveButton, SIGNAL(clicked()), this, SLOT(removegraph()));
	connect(&ShowButton, SIGNAL(clicked()), this, SLOT(voir()));
	connect(&GraphicTool, SIGNAL(clicked()), this, SLOT(setPalette()));
	connect(&graphList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
}



void graphconfig::disconnectAll()
{
	disconnect(&AddButton, SIGNAL(clicked()), this, SLOT(nouveaugraph()));
	disconnect(&RenameButton, SIGNAL(clicked()), this, SLOT(rename()));
	disconnect(&RemoveButton, SIGNAL(clicked()), this, SLOT(removegraph()));
	disconnect(&ShowButton, SIGNAL(clicked()), this, SLOT(voir()));
	disconnect(&GraphicTool, SIGNAL(clicked()), this, SLOT(setPalette()));
	disconnect(&graphList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
}




void graphconfig::readconfigfile(const QString &configdata)
{
// Extract graphiques
	QString ReadName;
	QString TAG_Begin = "Graphique";
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadName = logisdom::getvalue("Name", strsearch);
	if (!ReadName.isEmpty())
	{
		graph *NewGraph;
		NewGraph = nouveaugraph(ReadName);
                if (NewGraph) NewGraph->setCfgStr(strsearch);
	}
	SearchLoopEnd
}





void  graphconfig::SaveConfigStr(QString &str)
{
	for (int n=0; n<graphPtArray.count(); n++)
		graphPtArray[n]->getCfgStr(str);
}




void graphconfig::AddData(double v, const QDateTime &time, QString RomID)
{
	QMutexLocker locker(&mutexAddData);
	for (int n=0; n<graphList.count(); n++) 
		graphPtArray[n]->AddData(v, time, RomID);
}






void graphconfig::updateList()
{
	graphList.clear();
	for (int n=0; n<graphPtArray.count(); n++)
	{
		QListWidgetItem *widgetList = new QListWidgetItem(&graphList);
		widgetList->setText(graphPtArray[n]->getName());
	}
}






void graphconfig::rename()
{
	QString nom;
	bool ok;
	int index = graphList.currentIndex().row();
	if (index == -1) return;
	Retry:
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::NewName), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, graphPtArray[index]->getName(), &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (nom == graphPtArray[index]->getName()) return;
	bool found = false;
	int count = graphPtArray.count();
	for (int n=0; n<count; n++)
		if ((n != index) && (graphPtArray[n]->getName() == nom))
			found = true;
	if (found)
	{
		messageBox::warningHide(this, tr("Graphic name"), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	else
	{
		graphPtArray[index]->setName(nom);
		parent->ui.tabWidgetGraphic->setTabText(index, nom);
		updateList();
	}
}





void graphconfig::DeviceConfigChanged(onewiredevice *device)
{
	if (device)
		for (int n=0; n<graphPtArray.count(); n++)
			graphPtArray[n]->setCurveTitle(device->getromid(), device->getname());
}





void graphconfig::raz_counter()
{
	update_counter = 0;
}




void graphconfig::updateGraphs()
{
/*	for (int n=0; n<graphPtArray.count(); n++)
	{
		graphPtArray[n]->updategraph();
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}*/
	if (update_counter < graphPtArray.count()) graphPtArray[update_counter]->updategraph();
	update_counter++;
	QCoreApplication::processEvents(QEventLoop::AllEvents);
}




void graphconfig::ReloadGraphs()
{
	for (int n=0; n<graphPtArray.count(); n++)
		graphPtArray[n]->reload();
}




void graphconfig::addcurve(graph * Selection, QString romid, QColor color)
{
	QString name= romid;
	onewiredevice * device = maison1wirewindow->configwin->DeviceExist(romid);
	if (device != nullptr) name = device->getname();
	Selection->addcurve(name, romid, color);
}





void graphconfig::nouveaugraph()
{
	QString nom;
	bool ok;
Retry:
    nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	bool found = false;
	int count = graphPtArray.count();
	for (int n=0; n<count; n++)
		if (graphPtArray[n]->getName() == nom)
			found = true;
	if (!found) nouveaugraph(nom);
	else
	{
		messageBox::warningHide(this, tr("New graphic"), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	parent->ui.tabWidget->setCurrentIndex(2);
}




void graphconfig::createGraph(onewiredevice *device)
{
	bool graphExist = false;
	if (!device) return;
	QString RomID, Name;
	RomID = device->getromid();
	Name = device->getname();
	for (int n=0; n<graphPtArray.count(); n++)
		if (graphPtArray[n]->isCurveAlone(RomID))
		{
			graphExist = true;
			parent->ui.tabWidget->setCurrentIndex(2);
			parent->ui.tabWidgetGraphic->setCurrentIndex(n);
			break;
		}
	if (!graphExist)
	{
		graph *NewGraph = nouveaugraph(Name);
                if (NewGraph) NewGraph->addcurve(Name, RomID, currentcolor);
	}
}






void graphconfig::removegraph()
{
	int index = graphList.currentIndex().row();
	if (index == -1) return;
	if (messageBox::questionHide(this, tr("Remove graphic"),
			tr("Are you sure you want to remove graphic \n") + graphPtArray[index]->getName() , parent,
			QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			{
				if (previousSetup) previousSetup->hide();
				graphPtArray[index]->setup.hide();
				parent->removeWidget(&graphPtArray[index]->setup);
				graphPtArray[index]->setAttribute(Qt::WA_DeleteOnClose, true);
				graphPtArray[index]->close();
				graphPtArray.removeAt(index);
				parent->ui.tabWidgetGraphic->removeTab(index);
				previousSetup = nullptr;
				updateList();
			}
}





void graphconfig::voir()
{
	int index = graphList.currentIndex().row();
	if (index == -1) return;
	parent->ui.tabWidgetGraphic->setCurrentIndex(index);
	parent->ui.tabWidget->setCurrentIndex(2);
}







graph *graphconfig::nouveaugraph(QString Name)
{
	QString name;
	disconnectAll();
	if (Name == "") name = cstr::toStr(cstr::NoName); else name = Name;
	bool found = false;
	int count = graphPtArray.count();
	for (int n=0; n<count; n++)
		if (graphPtArray[n]->getName() == name)
			found = true;
	if (found)
	{
		int index = 1;
		int lastunderscore = name.lastIndexOf("_");
		if (lastunderscore == -1) name += QString("_1");
		bool newfound = false;
		int count = graphPtArray.count();
		for (int n=0; n<count; n++)
			if (graphPtArray[n]->getName() == name)
				newfound = true;
        while (newfound)
		{
			lastunderscore = name.lastIndexOf("_");
			name = name.mid(0, lastunderscore) + QString("_%1").arg(index);
			newfound = false;
			int count = graphPtArray.count();
			for (int n=0; n<count; n++)
				if (graphPtArray[n]->getName() == name)
					newfound = true;
			index ++;
		}
	}
	graph *Graph = new graph(parent, name);
	if (!Graph) return nullptr;
	graphPtArray.append(Graph);
	int n = parent->ui.tabWidgetGraphic->count();
	parent->ui.tabWidgetGraphic->addTab(Graph, name);
	parent->ui.tabWidgetGraphic->setCurrentIndex(n);
	updateList();
	connectAll();
	connect(Graph, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
	//connect(this, SIGNAL(updateSignal()), Graph, SLOT(updategraph()));
	htmlBind->setParameterLink(name, name + ".png");
	return Graph;
}

