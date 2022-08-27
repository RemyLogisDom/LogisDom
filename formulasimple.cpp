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
#include "onewire.h"
#include "configwindow.h"
#include "weathercom.h"
#include "energiesolaire.h"
#include "meteo.h"
#include "inputdialog.h"
#include "formulasimple.h"





formulasimple::formulasimple(logisdom *Parent)
{
	ui.setupUi(this);
	parent = Parent;
        widgetCount = 0;
	reprocessEnabled = true;
	onetwo = 0;
	calcRequest = false;
        calcth = new calcthread(this);
        timerCalc.setSingleShot(true);
	connect(&timerCalc, SIGNAL(timeout()), this, SLOT(CalcRequest()));
        ui.progress->setVisible(false);
	startTxt = tr("reprocess");
	stopTxt = tr("stop");
	startCalc = tr("Calculate");
	cancelCalc = tr("Cancel");
	ui.pushButtonCalculate->setText(startCalc);
	ui.pushButtonReprocess->setText(startTxt);
	connect(ui.pushButtonCalculate, SIGNAL(clicked()), this, SLOT(ClickClacl()));
	ui.comboBoxFamily->clear();
	for (int n=0; n<calcthread::lastFamily; n++)
		ui.comboBoxFamily->addItem(calcth->family2Str(n));
	ui.comboBoxFamily->setCurrentIndex(calcthread::famMath);
	connect(ui.listViewOperators, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklList(QPoint)));
	connect(ui.listViewOperators, SIGNAL(currentRowChanged(int)), this, SLOT(rowchange(int)));
	connect(ui.listViewOperators, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(insert(QModelIndex)));
	connect(calcth, SIGNAL(finished()), this, SLOT(CalcEnd()), Qt::QueuedConnection);
	connect(calcth, SIGNAL(calcRequest()), this, SLOT(CalcRequest()), Qt::QueuedConnection);
	connect(ui.comboBoxFamily, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFamily(int)));
}





formulasimple::~formulasimple()
{
	delete calcth;
}




void formulasimple::ClickClacl()
{
	if (calcRequest)
	{
		calcth->stopRequest = true;
		calcRequest = false;
		ui.status_Calc->setText(tr("Wait Canceling"));
	}
	else
	{
		calcRequest = true;
		QString C = ui.textEditFormule->toPlainText();
		if (C.contains("[") and C.contains("]") and C.contains("xv") and C.contains("yv"))
		{
			ui.textEditFormule->clear();
			ui.textEditFormule->setText(C.remove("[").remove("]").replace("xv", "X").replace("yv", "Y"));
		}
		CalculateThread();
	}
}




void formulasimple::setName(QString name)
{
	setWindowTitle(name);
}



void formulasimple::removeButtons()
{
        //QWidget *widget = ui.toolBox->widget(0);
        //ui.toolBox->hide();
        //widget->show();
	reprocessEnabled = false;
        ui.pushButtonReprocess->setVisible(false);
        ui.toolBox->widget(1)->setVisible(false);
        ui.toolBox->removeItem(1);
        ui.toolBox->setItemEnabled(1, false);
        ui.status->hide();
}




void formulasimple::changeFamily(int family)
{
	widgetCount = 0;
	if (family == calcthread::famOperator) showOp();
	else if (family == calcthread::famDevices) showDev();
	else
	{
		ui.listViewOperators->clear();
		for (int n=0; n<calcthread::lastOperator; n++)
			if (calcth->op2Family(n) == family) widgetCount++;
		if (!widgetCount) return;
		int index = 0;
		widgetList = new QListWidgetItem *[widgetCount];
		for (int n=0; n<calcthread::lastOperator; n++)
			if (calcth->op2Family(n) == family)
				widgetList[index++] = new QListWidgetItem(calcth->op2Str(n), ui.listViewOperators, n);
	}
}




void formulasimple::rowchange(int index)
{
	ui.textBrowserResult->clear();
	if (ui.comboBoxFamily->currentIndex() == calcthread::famOperator) return;
	if (ui.comboBoxFamily->currentIndex() == calcthread::famDevices) return;
	if (index == -1) return;
	if (index >= widgetCount) return;
	int I = widgetList[index]->type();
	if (I == -1) return;
	if (index < calcthread::lastOperator)
		ui.textBrowserResult->append(calcth->op2Description(I));	
}






void formulasimple::showOp()
{
	ui.listViewOperators->clear();
	int n = 0;
	widgetList = new QListWidgetItem * [7];
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText("+");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText("-");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText("*");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText("/");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText(">");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n++]->setText("<");
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n]->setText("=");
}





void formulasimple::showDev()
{
	ui.listViewOperators->clear();
	int devCount = maison1wirewindow->configwin->devicePtArray.count();
	widgetList = new QListWidgetItem * [devCount];
	for (int n=0; n<devCount; n++)
	{
		widgetList[n] = new QListWidgetItem(ui.listViewOperators);
		widgetList[n]->setText(maison1wirewindow->configwin->devicePtArray[n]->getname() + "  (" + maison1wirewindow->configwin->devicePtArray[n]->getromid() + ")");
	}
}





void formulasimple::insert(QModelIndex index)
{
	int n = index.row();
	if (n == -1) return;
	ui.textBrowserResult->clear();
	QString S = ui.listViewOperators->currentItem()->text();
//	ui.textBrowserResult->clear();
	if (ui.comboBoxFamily->currentIndex() == calcthread::famOperator)
	{
		ui.textEditFormule->insertPlainText(S);
		return;
	}
	if (ui.comboBoxFamily->currentIndex() == calcthread::famDevices)
	{
		ui.textEditFormule->insertPlainText(maison1wirewindow->configwin->devicePtArray[n]->getromid());
		return;
	}
	if (n >= widgetCount) return;
	int I = widgetList[n]->type();
	if (I == -1) return;
	if (I < calcthread::lastOperator)
	{
		ui.textBrowserResult->append(calcth->op2Description(I));	
		ui.textEditFormule->insertPlainText(S);
	}
}





void formulasimple::rightclicklList(const QPoint & pos)
{
	QModelIndex index = ui.listViewOperators->currentIndex();
	QMenu contextualmenu;
	QAction ins("insert", this);
	contextualmenu.addAction(&ins);
	int I = index.row();
	if (I == -1) return;
	contextualmenu.addAction(&ins);
	QAction *selection;
	selection = contextualmenu.exec(ui.listViewOperators->mapToGlobal(pos));
	if (selection == &ins) insert(index);
}






QString formulasimple::getFormula()
{
	return  ui.textEditFormule->toPlainText();
}




void formulasimple::setFormula(QString F)
{
	ui.textEditFormule->clear();
	ui.textEditFormule->append(F);
}





double formulasimple::Calculate(double target)
{
	calcth->target = target;
	ui.textBrowserResult->clear();
	Calc = ui.textEditFormule->toPlainText();
	Calc = Calc.remove(" ");
	Calc = Calc.remove("\n");
	ui.textBrowserResult->append(Calc);
	ui.status_Calc->setText(tr("Processing"));
	calcth->Calc = Calc;
	ui.textBrowserResult->append(calcth->textBrowserResult);
	ui.status_Calc->setText(tr("Finished at") + QDateTime::currentDateTime().toString(" dd.MM.yyyy  HH:mm:ss"));
	return calcth->calculate();
}




double formulasimple::getValue()
{
  //  QMutexLocker locker(&MutexThreadRequest);
    if (lastCalc.isValid())
    {
    	QDateTime now = QDateTime::currentDateTime();
	if (lastCalc.secsTo(now) >= 60) CalculateThread();
    }
    else
    {
	    lastCalc = QDateTime::currentDateTime();
	    CalculateThread();
    }
    return calcth->threadResult;
}





void formulasimple::CalculateThreadRequest()
{
	QMutexLocker locker(&MutexThreadRequest);
	lastCalc = QDateTime::currentDateTime();
	//QString log = "\n" + QDateTime::currentDateTime().toString() + " : Signal Value Changed";
	//QString filename = QString(deviceParent->getromid() + ".txt");
	//QFile file(filename);
	//QTextStream out(&file);
	//file.open(QIODevice::Append | QIODevice::Text);
	//out << log;
	//if (device->isDataLoading())
	//{
	//	log = "\n" + QDateTime::currentDateTime().toString() + " : Device is loading";
	//	out << log;
	//	timerCalc.stop();
	//	file.close();
	//}
	timerCalc.start(2000);
	//file.close();
}




void formulasimple::CalcRequest()
{
	//QString log = "\n" + QDateTime::currentDateTime().toString() + " : CalcRequest";
	//QString filename = QString(deviceParent->getromid() + ".txt");
	//QFile file(filename);
	//QTextStream out(&file);
	//file.open(QIODevice::Append | QIODevice::Text);
	//out << log;
	//file.close();
	if (deviceParent)
	{
	    if (deviceParent->calculInterval.getSecs() == 0) CalculateThread();
	}
	else CalculateThread();
}



void formulasimple::CalculateThread()
{
	if (!SenderMutex.tryLock()) return;
	if (calcth->calculating) return;
	ui.textBrowserResult->clear();
	ui.pushButtonCalculate->setText(cancelCalc);
	ui.pushButtonReprocess->setEnabled(false);
	Calc = ui.textEditFormule->toPlainText();
	Calc = Calc.remove(" ");
	Calc = Calc.remove("\n");
	ui.textBrowserResult->append(Calc);
	ui.status_Calc->setText(tr("Processing"));
	calcth->Calc = Calc;
	CalcTimer.start();
	calcth->start();
	SenderMutex.unlock();
}





void formulasimple::CalcEnd()
{
	QString str;
	QDateTime now = QDateTime::currentDateTime();
	ui.textBrowserResult->clear();
	ui.textBrowserResult->append(Calc);
	ui.pushButtonCalculate->setText(startCalc);
	ui.pushButtonReprocess->setEnabled(true);
	ui.textBrowserResult->append(calcth->textBrowserResult);
	if (calcth->stopRequest) str += tr("Canceled at");
		else str += tr("Finished at");
	lastCalc = now;
	str += now.toString(" dd.MM.yyyy  HH:mm:ss");
	CalcTime = CalcTimer.elapsed();
	double Ts = CalcTime/1000;
	double Tm = CalcTime/60000;
	if (Tm > 1) str += QString(" in %1 min").arg(Tm, 0, 'f', 1);
	else if (Ts > 1) str += QString(" in %1 sec").arg(Ts, 0, 'f', 1);
	else str += QString(" in %1 ms").arg(CalcTime);
	ui.textBrowserResult->append(str);
	ui.status_Calc->setText(str);
	calcRequest = false;
	if (reprocessEnabled)
	    if (ui.comboBoxDeviceList->count() != calcth->deviceList.count())
	    {
		ui.comboBoxDeviceList->clear();
		for (int n=0; n<calcth->deviceList.count(); n++)
		{
		    onewiredevice *device = calcth->deviceList.at(n);
		    if (device) ui.comboBoxDeviceList->addItem(device->getname() + "(" + device->getromid() + ")");
		}
	    }
	if (!calcth->stopRequest) emit(calcdone());
}





void formulasimple::stopAll()
{
	calcth->quit();
	disconnect(calcth, SIGNAL(finished()), this, SLOT(CalcEnd()));
}


