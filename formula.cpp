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
#include "onewire.h"
#include "configwindow.h"
#include "weathercom.h"
#include "energiesolaire.h"
#include "meteo.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "formula.h"





formula::formula(logisdom *Parent)
{
	ui.setupUi(this);
	parent = Parent;
	widgetCount = 0;
	reprocessEnabled = true;
	onetwo = 0;
    QStringList keywordPatterns;
    //for (int n=0; n<calcthread::lastOperator; n++) keywordPatterns.append("\\b" + calcthread::op2Str(n) + "\\b");
    highlighter = new Highlighter(ui.textEditFormule->document(), keywordPatterns);
    thread = new QThread;
    threadr = new QThread;
    calcth = new calcthread(this);
	reprossthread = new reprocessthread(this);
    calcth->moveToThread(thread);
    connect(thread, SIGNAL(started()), calcth, SLOT(runprocess()));
    reprossthread->moveToThread(threadr);
    connect(threadr, SIGNAL(started()), reprossthread, SLOT(runprocess()));
    timerCalc.setSingleShot(true);
	connect(&timerCalc, SIGNAL(timeout()), this, SLOT(CalcRequest()));
	ui.progress->setVisible(false);
    startTxt = tr("reprocess");
	stopTxt = tr("stop");
	startCalc = tr("Calculate");
	cancelCalc = tr("Cancel");
	ui.pushButtonCalculate->setText(startCalc);
	ui.pushButtonReprocess->setText(startTxt);
	ui.comboBoxFamily->clear();
	for (int n=0; n<calcthread::lastFamily; n++)
    ui.comboBoxFamily->addItem(calcth->family2Str(n));
	ui.comboBoxFamily->setCurrentIndex(calcthread::famMath);
	connect(ui.listViewOperators, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklList(QPoint)));
	connect(ui.listViewOperators, SIGNAL(currentRowChanged(int)), this, SLOT(rowchange(int)));
	connect(ui.listViewOperators, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(insert(QModelIndex)));
    connect(calcth, SIGNAL(calcFinished(QString)), this, SLOT(calcFinished(QString)), Qt::QueuedConnection);
    connect(calcth, SIGNAL(redirectHttp(QString)), this, SLOT(redirectHttp(QString)), Qt::QueuedConnection);
    connect(calcth, SIGNAL(calcRequest()), this, SLOT(CalcRequest()), Qt::QueuedConnection);
	connect(ui.comboBoxFamily, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFamily(int)));
	connect(ui.pushButtonCalculate, SIGNAL(clicked()), this, SLOT(ClickClacl()));
	connect(ui.pushButtonReprocess, SIGNAL(clicked()), this, SLOT(reprocess()));
    connect(ui.pushButtonDeviceRomID, SIGNAL(clicked()), this, SLOT(getDeviceRomID()));
    connect(ui.pushButtonDeviceName, SIGNAL(clicked()), this, SLOT(getDeviceName()));
    int month = QDateTime::currentDateTime().date().month();
	int year = QDateTime::currentDateTime().date().year();
	ui.dateEdit->setDate(QDate(year, month, 1));
	ui.comboBoxFamily->setCurrentIndex(calcthread::famMath);
    connect(&progressTimer, SIGNAL(timeout()), this, SLOT(progressdisplay()));
    connect(reprossthread, SIGNAL(finished()), this, SLOT(reprocessEnd()), Qt::QueuedConnection);
}




formula::~formula()
{
	delete reprossthread;
	delete calcth;
}





void formula::reprocess()
{
	bool ok;
	if (!deviceParent) return;
    if (threadr->isRunning())
	{
		stopreProcess();
		return;
	}
	QDateTime since;
	since.time().setHMS(0, 0, 0);
	since.setDate(ui.dateEdit->date());
	if (since.secsTo(QDateTime::currentDateTime()) < 0) return;
	ui.dateEdit->date().toString("MMM-yyyy");
	if (messageBox::questionHide(this, tr("Confirmer"),
	    tr("Recompiler les fichiers dat depuis ") + ui.dateEdit->date().toString("MMM-yyyy") + \
        "\nLes fichiers de données seront modifiés", parent, QMessageBox::No | QMessageBox::Yes) \
	    == QMessageBox::No) return;
	ui.pushButtonReprocess->setText(stopTxt);
	deviceParent->saveInterval.setEnabled(false);
	ui.progress->setVisible(true);
	ui.progress->setValue(0);
	ui.status->setText(tr("reprocessing"));
	reprossthread->device = deviceParent;
	QDateTime begin = deviceParent->saveInterval.nextOne.dateTime();
    qint64 secs = deviceParent->saveInterval.getSecs(begin);
    //begin.setDate(since.date().addDays(1));
	reprossthread->deviceIndex = nullptr;
// get the first occurence hour
    if (secs != 0)
    {
        while (begin.secsTo(since) < 0)
        {
            if (secs > Minutes2Weeks) begin = begin.addMonths(-1); // for one month
            else begin = begin.addSecs(-secs);
        }
    }
	onewiredevice *device = nullptr;
	if (ui.checkBoxIndexOn->isChecked() && (ui.comboBoxDeviceList->currentIndex() != -1))
	{
	    QString str = ui.comboBoxDeviceList->currentText();
	    int coma = str.indexOf("(");
	    if (coma != -1)
	    {
		int nextcoma = str.indexOf(")", coma + 1);
		if (nextcoma != -1)
		{
		    QString romid = str.mid(coma + 1, nextcoma - coma - 1);
		    device = parent->configwin->DeviceExist(romid);
		}
	    }
	}
	if (device)
	{
	    reprossthread->deviceIndex = device;
	}
	else if (secs == 0)
	{
	    secs = inputDialog::getIntegerPalette(this, tr("Intervalle"), tr("Intervalle en minute : "), 1, 1, 999, 1, &ok, Qt::Dialog, parent);
        if (!ok)
        {
            ui.pushButtonReprocess->setText(startTxt);
            deviceParent->saveInterval.setEnabled(true);
            ui.progress->setValue(0);
            ui.progress->setVisible(false);
            ui.status->setText(tr("aborted"));
            return;
        }
	}
    else
    {
        if (secs > Minutes2Weeks) begin = begin.addMonths(1); // for one month
        begin = begin.addSecs(secs);
    }
    Calc = ui.textEditFormule->toPlainText();
    reprossthread->setCalc(Calc);
    reprossthread->saveInterval = secs;
	reprossthread->timeIndex = begin;
    threadr->start(QThread::IdlePriority);
	progressTimer.start(500);
}





void formula::reProcessEnd()
{
	if (reprossthread->deviceLoading) ui.status->setText(tr("Reprocess canceled, device loading data"));
	else  ui.status->setText(tr("Reprocess finished"));
	onetwo = 0;
	progressTimer.stop();
	ui.progress->setVisible(false);
	ui.pushButtonReprocess->setText(startTxt);
	deviceParent->saveInterval.setEnabled(true);
	deviceParent->clearData();
}





void formula::stopreProcess()
{
	reprossthread->stopRequest = true;
}




void formula::stopAll()
{
    thread->quit();
    threadr->quit();
}



void formula::CalculateThreadRequest(onewiredevice *device)
{
    if (device->isDataLoading()) return;
	timerCalc.start(2000);
}




void formula::reprocessEnd()
{
    threadr->quit();
    ui.textBrowserResult->clear();
	reProcessEnd();
    emit(reprocessdone());
}




void formula::progressdisplay()
{
	if (onetwo == 0)
		ui.status->setText(tr("Reprocessing") + reprossthread->state + "     ");
	if (onetwo == 1)
		ui.status->setText(tr("Reprocessing") + reprossthread->state + " .   ");
	if (onetwo == 2)
		ui.status->setText(tr("Reprocessing") + reprossthread->state + " ..  ");
	if (onetwo == 3)
		ui.status->setText(tr("Reprocessing") + reprossthread->state + " ... ");
	onetwo++;
	if (onetwo > 3) onetwo = 0;
	ui.progress->setValue(reprossthread->progress);
}






void formula::ClickClacl()
{
	if (calcth->calculating)
	{
		calcth->stopRequest = true;
		ui.status_Calc->setText(tr("Wait Canceling"));
	}
	else
	{
        QString C = ui.textEditFormule->toPlainText();
		if (C.contains("[") and C.contains("]") and C.contains("xv") and C.contains("yv"))
		{
            ui.textEditFormule->clear();
            ui.textEditFormule->setText(C.remove("[").remove("]").replace("xv", "X").replace("yv", "Y"));
		}
		CalculateThread();
	}
}




void formula::setName(QString name)
{
	setWindowTitle(name);
}



void formula::removeButtons()
{
    reprocessEnabled = false;
    calcth->clearAutoEnabled();
    ui.pushButtonReprocess->setVisible(false);
    ui.tabWidget->removeTab(2);
    //         ->setVisible(false);
    // ui.tabWidget->removeItem(1);
    // ui.tabWidget->setItemEnabled(1, false);
    ui.status->hide();
}




void formula::changeFamily(int family)
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




QString formula::getOnErrorTxtValue()
{
    return ui.ValueOnError->text();
}




double formula::getOnErrorValue()
{
    double value = logisdom::NA;
    bool ok;
    if (!ui.ValueOnErrorEnable->isChecked()) return value;
    if (ui.ValueOnError->text().isEmpty()) return value;
    bool a_Valid = true;
    double a = ui.ValueOnError->text().toDouble(&ok);
    if (!ok)
    {
        QString txt = ui.ValueOnError->text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) a = A/B; else a_Valid = false;
            }
        }
        else a_Valid = false;
    }
    if (!a_Valid) return value;
    double R = a;
    return R;
 }




void formula::rowchange(int index)
{
    ui.textBrowserDetails->clear();
	if (ui.comboBoxFamily->currentIndex() == calcthread::famOperator) return;
	if (ui.comboBoxFamily->currentIndex() == calcthread::famDevices) return;
	if (index == -1) return;
	if (index >= widgetCount) return;
	int I = widgetList[index]->type();
	if (I == -1) return;
	if (index < calcthread::lastOperator)
        ui.textBrowserDetails->append(calcth->op2Description(I));
}






void formula::showOp()
{
	ui.listViewOperators->clear();
	int n = 0;
    widgetList = new QListWidgetItem * [8];
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
	widgetList[n] = new QListWidgetItem(ui.listViewOperators);
	widgetList[n]->setText("!");
    widgetList[n] = new QListWidgetItem(ui.listViewOperators);
    widgetList[n]->setText("^");
}





void formula::showDev()
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




void formula::getDeviceName()
{
    onewiredevice *device = parent->configwin->chooseDevice();
    if (device)
    {
        ui.textEditFormule->insertPlainText(device->getname());
    }
}



void formula::getDeviceRomID()
{
    onewiredevice *device = parent->configwin->chooseDevice();
    if (device)
    {
        ui.textEditFormule->insertPlainText(device->getromid());
    }
}



void formula::insert(QModelIndex index)
{
	int n = index.row();
	if (n == -1) return;
    ui.textBrowserDetails->clear();
	QString S = ui.listViewOperators->currentItem()->text();
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
        ui.textBrowserDetails->append(calcth->op2Description(I));
        //ui.textEditFormule->insertPlainText(S);
        ui.textEditFormule->insertPlainText(calcth->op2Function(I));
    }
}





void formula::rightclicklList(const QPoint & pos)
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






QString formula::getFormula()
{
    return ui.textEditFormule->toPlainText();
}




void formula::setFormula(QString F)
{
    ui.textEditFormule->clear();
    ui.textEditFormule->append(F);
}





double formula::Calculate(double target)
{
	calcth->target = target;
	ui.textBrowserResult->clear();
    Calc = ui.textEditFormule->toPlainText();
	ui.textBrowserResult->append(Calc);
	ui.status_Calc->setText(tr("Processing"));
	ui.textBrowserResult->append(calcth->textBrowserResult);
	ui.status_Calc->setText(tr("Finished at") + QDateTime::currentDateTime().toString(" dd.MM.yyyy  HH:mm:ss"));
    if (ui.ValueOnErrorEnable->isChecked()) calcth->valueOnError = ui.ValueOnError->text(); else calcth->valueOnError.clear();
    return calcth->calculate(Calc);
}




double formula::getValue()
{
    if (lastCalc.isValid())
    {
        QDateTime now = QDateTime::currentDateTime();
        if (lastCalc.secsTo(now) >= 60) CalculateThread();
        else if (lastCalc.time().minute() != now.time().minute()) CalculateThread();
    }
    else
    {
	    lastCalc = QDateTime::currentDateTime();
	    CalculateThread();
    }
    double result = calcth->threadResult;
    if (logisdom::isNA(result)) result = getOnErrorValue();
    return result;
}






void formula::CalcRequest()
{
	if (deviceParent)
	{
	    if (deviceParent->calculInterval.getSecs() == 0) CalculateThread();
	}
}





void formula::CalculateThread()
{
    if (thread->isRunning()) return;
	if (calcth->calculating) return;
	if (!SenderMutex.tryLock()) return;
	ui.textBrowserResult->clear();
	ui.pushButtonCalculate->setText(cancelCalc);
	ui.pushButtonReprocess->setEnabled(false);
    Calc = ui.textEditFormule->toPlainText();
    ui.textBrowserResult->append(Calc);
    if (ui.ValueOnErrorEnable->isChecked()) calcth->valueOnError = ui.ValueOnError->text(); else calcth->valueOnError.clear();
	ui.status_Calc->setText(tr("Processing"));
    calcth->setCalc(Calc);
	CalcTimer.start();
    thread->start();
	SenderMutex.unlock();
}




void formula::redirectHttp(const QString &str)
{
    QString Calc = ui.textEditFormule->toPlainText();
    QString RedirectedCalc;
    QStringList list = Calc.split("\n");
    for (int n=0; n<list.count(); n++)
    {
        if (list.at(n).startsWith("webpage="))
        {
            RedirectedCalc.append("webpage=" + str + "\n");
        }
        else RedirectedCalc.append(list.at(n) + "\n");
    }
    //ui.textEditFormule->setText(RedirectedCalc);
}



void formula::calcFinished(const QString &str)
{
    thread->quit();
    QString txt;
    txt.append(str);
    QDateTime now = QDateTime::currentDateTime();
    ui.pushButtonCalculate->setText(startCalc);
    ui.pushButtonReprocess->setEnabled(true);
    QString time_txt;
    if (calcth->stopRequest) time_txt += tr("Canceled at");
        else time_txt += tr("Finished at");
    lastCalc = now;
    time_txt += now.toString(" dd.MM.yyyy  HH:mm:ss");
    CalcTime = CalcTimer.elapsed();
    double Ts = CalcTime/1000;
    double Tm = CalcTime/60000;
    if (Tm > 1) time_txt += QString(" in %1 min").arg(Tm, 0, 'f', 1);
    else if (Ts > 1) time_txt += QString(" in %1 sec").arg(Ts, 0, 'f', 1);
    else time_txt += QString(" in %1 ms").arg(CalcTime);
    ui.status_Calc->setText(time_txt);
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
    ui.textBrowserResult->clear();
    ui.textBrowserResult->append(txt);
    if (!calcth->stopRequest) emit(calcdone());
}





