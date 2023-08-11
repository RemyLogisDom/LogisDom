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





#include <QtCore>
#include <QtGui>
#include "logisdom.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "curve.h"
#include "graph.h"
#include "graphconfig.h"
#include "configwindow.h"
#include "onewire.h"
#include "htmlbinder.h"
#include "messagebox.h"
#include "globalvar.h"
#include "tableau.h"
#include "tableauconfig.h"




bool DeleteHighlightedItemWhenShiftDelPressedEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key::Key_Delete)
        {
            auto combobox = dynamic_cast<QComboBox *>(obj);
            if (combobox){
                combobox->removeItem(combobox->currentIndex());
                return true;
            }
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}



graph::graph(logisdom *Parent, QString &name)
{
	parent = Parent;
	plot.parent = this;
	previousSetup = nullptr;
	reloading = false;
	busy = false;
    connected = false;
    layout = new QGridLayout(this);
	setLayout(layout);
//	plot.setMargin(5);
    layout->addWidget(&plot, 0, 0, 1, 10);
	HUnit.addItem(tr("Minutes"));
	HUnit.addItem(tr("Hours"));
	HUnit.addItem(tr("Days"));
	HUnit.addItem(tr("Month"));
	//HScale.setPrefix (tr("Scale : "));
	HScale.setRange (1, 999);
	HOffset.setPrefix (tr("Offset : "));
    HOffset.setRange (0, 9999);
	HUnit.setCurrentIndex(Jours);
    rotation.setPrefix (tr("Rotation : "));
	rotation.setRange (0, 90);
	rotation.setSingleStep(5);
	tickFormat.setEditable(true);
    tickFormat.installEventFilter(new DeleteHighlightedItemWhenShiftDelPressedEventFilter);
	tickFormat.addItem("hh:mm");
	tickFormat.addItem("dd-MMM-yy");
	tickFormat.addItem("dd/MM/yy");
	tickFormat.addItem("hh:mm dd-MMM-yyyy");
	ReloadButton.setText(tr("Reload"));
    RemoveScaleButton.setText(tr("Del"));
	CompressBox.setText(tr("Compress"));
    //UpdateButton.setText(tr("Update"));

// palette setup
	layoutIndex = 0;
	setup.setLayout(&setupLayout);
    //QIcon curveIcon(QString::fromUtf8(":/images/images/curve.png"));
    //CurveTool.setIcon(curveIcon);
    //CurveTool.setIconSize(QSize(logisdom::iconSize, logisdom::iconSize));
    //setupLayout.addWidget(&CurveTool, layoutIndex++, 0, 1, 2);
	manualScale.setText(tr("Manual Scale"));
	setupLayout.addWidget(&manualScale, layoutIndex++, 0, 1, 1);
	MaxText.setText(tr("Max : "));
	setupLayout.addWidget(&MaxText, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&MaxVal, layoutIndex++, 1, 1, 1);
	MinText.setText(tr("Min : "));
	setupLayout.addWidget(&MinText, layoutIndex, 0, 1, 1);
	setupLayout.addWidget(&MinVal, layoutIndex++, 1, 1, 1);
    //setupLayout.addWidget(&ContinousUpdate, layoutIndex++, 1, 1, 1);
    //ContinousUpdate.setText(tr("Continous Update"));
    //ContinousUpdate.setToolTip(tr("When disabled, automatic PNG generation will be also disabled for this graphic"));
    //ContinousUpdate.setChecked(true);
	//setupLayout.addWidget(&showLegend);
	AddCurveButton.setText(tr("Add curve"));
	setupLayout.addWidget(&AddCurveButton, layoutIndex, 0, 1, 1);
	RemoveCurveButton.setText(tr("Remove curve"));
	setupLayout.addWidget(&RemoveCurveButton, layoutIndex++, 1, 1, 1);
    AddHistoButton.setText(tr("Add histo"));
    setupLayout.addWidget(&AddHistoButton, layoutIndex++, 0, 1, 1);
    setupLayout.addWidget(&curveList, layoutIndex++, 0, 2, 5);
	Name = name;
	Origin = QDateTime(QDate(yearscalebegin, 1, 1), QTime(0, 0, 0, 0));
	setWindowTitle(Name);
	toggleManual(Qt::Unchecked);
	connect(&plot, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()));
    connect(&plot, SIGNAL(zoomFinished()), this, SLOT(zoomUpdate()));
    connect(&plot, SIGNAL(ToolsOn()), this, SLOT(ToolsOn()));
    connect(&toolTimer, SIGNAL(timeout()), this, SLOT(ToolsOff()));
    toolTimer.setSingleShot(true);
    connect(&ReloadButton, SIGNAL(clicked()), this, SLOT(reload()));
    //connect(&UpdateButton, SIGNAL(clicked()), this, SLOT(UpdateClick()));
    connect(&RemoveScaleButton, SIGNAL(clicked()), this, SLOT(removeScaleLegend()));
	connect(&CompressBox, SIGNAL(stateChanged(int)), this, SLOT(toggleCompress(int)));
	connect(&manualScale, SIGNAL(stateChanged(int)), this, SLOT(toggleManual(int)));
	connect(&MaxVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
	connect(&MinVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
    //connect(&showLegend, SIGNAL(stateChanged(int)), this, SLOT(showLegendChanged(int)));
    //connect(&ContinousUpdate, SIGNAL(stateChanged(int)), this, SLOT(ContinousUpdateChanged(int)));
    connectAll();
}




graph::~graph()
{
}




void graph::CurveRowChanged (int curve)
{
	if (curve == -1) return;
    int curvecount = plot.curve.count();
    if (curve < curvecount) setPalette(&plot.curve[curve]->setup);
    else
    {
        setPalette(&plot.histo[curve-curvecount]->setup);
    }
}





void graph::setPalette(QWidget *curvesetup)
{
	if (previousSetup) previousSetup->hide();
	previousSetup = curvesetup;
	if (curvesetup) setupLayout.addWidget(curvesetup, layoutIndex + 5, 0, 2, 2);
	if (curvesetup) curvesetup->show();
}



void graph::addcurve()
{
	plot.addcurvemenu();
	updateList();
}




void graph::addhisto()
{
// choose chart
    tableau *ChooseTableau = parent->tableauConfig->chooseTableau();
    if (!ChooseTableau) return;
    plot.addhistomenu(ChooseTableau);
    updateList();
}



void  graph::removecurve()
{
	if (previousSetup) previousSetup->hide();
	previousSetup = nullptr;
	int index = curveList.currentIndex().row();
	if (index < 0) return;			// if non selection exit
	if (messageBox::questionHide(this, tr("Remove curver"), tr("Are you sure ?"), parent,
		 QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes) return;
    int curvecount = plot.curve.count();
    if (index < curvecount)
    {
        plot.curve[index]->curve->detach();
        delete plot.curve[index];
        plot.curve.removeAt(index);
    }
    else
    {
        plot.histo[index-curvecount]->histo->detach();
        delete plot.histo[index-curvecount];
        plot.histo.removeAt(index-curvecount);
    }
	plot.replot();
	updateList();
}





void graph::updateList()
{
	curveList.clear();
	for (int n=0; n<plot.curve.count(); n++)
	{
		QListWidgetItem *widgetList = new QListWidgetItem(&curveList);
		QString RomID = plot.curve.at(n)->getRomID();
		widgetList->setText(parent->configwin->getDeviceName(RomID));
	}
    for (int n=0; n<plot.histo.count(); n++)
    {
        QListWidgetItem *widgetList = new QListWidgetItem(&curveList);
        QString name = plot.histo.at(n)->getRomID();
        widgetList->setText(name);
    }
}





void graph::connectAll()
{
    busy = false;
    if (connected) return;
	connect(&HUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitchange(int)));
	connect(&HScale, SIGNAL(valueChanged(int)), this, SLOT(scalechange(int)));
	connect(&HOffset, SIGNAL(valueChanged(double)), this, SLOT(offsetchange(double)));
	connect(&tickFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(updateAxis(int)));
	connect(&tickFormat, SIGNAL(editTextChanged(QString)), this, SLOT(updateAxis(QString)));
	connect(&rotation, SIGNAL(valueChanged(int)), this, SLOT(updateAxis(int)));
	connect(&AddCurveButton, SIGNAL(clicked()), this, SLOT(addcurve()));
    connect(&AddHistoButton, SIGNAL(clicked()), this, SLOT(addhisto()));
    connect(&RemoveCurveButton, SIGNAL(clicked()), this, SLOT(removecurve()));
	connect(&curveList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
	connect(&plot, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()));
	connect(&ReloadButton, SIGNAL(clicked()), this, SLOT(reload()));
	connect(&RemoveScaleButton, SIGNAL(clicked()), this, SLOT(removeScaleLegend()));
	connect(&CompressBox, SIGNAL(stateChanged(int)), this, SLOT(toggleCompress(int)));
	connect(&manualScale, SIGNAL(stateChanged(int)), this, SLOT(toggleManual(int)));
	connect(&MaxVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
	connect(&MinVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
    connected = true;
}



void graph::disconnectAll()
{
	disconnect(&HUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitchange(int)));
	disconnect(&HScale, SIGNAL(valueChanged(int)), this, SLOT(scalechange(int)));
	disconnect(&HOffset, SIGNAL(valueChanged(double)), this, SLOT(offsetchange(double)));
	disconnect(&tickFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(updateAxis(int)));
	disconnect(&tickFormat, SIGNAL(editTextChanged(QString)), this, SLOT(updateAxis(QString)));
	disconnect(&rotation, SIGNAL(valueChanged(int)), this, SLOT(newscale(int)));
	disconnect(&AddCurveButton, SIGNAL(clicked()), this, SLOT(addcurve()));
    disconnect(&AddHistoButton, SIGNAL(clicked()), this, SLOT(addhisto()));
    disconnect(&RemoveCurveButton, SIGNAL(clicked()), this, SLOT(removecurve()));
	disconnect(&curveList, SIGNAL(currentRowChanged(int)), this, SLOT(CurveRowChanged(int)));
	disconnect(&plot, SIGNAL(loadingFinished()), this, SLOT(loadingFinished()));
	disconnect(&ReloadButton, SIGNAL(clicked()), this, SLOT(reload()));
	disconnect(&RemoveScaleButton, SIGNAL(clicked()), this, SLOT(removeScaleLegend()));
	disconnect(&CompressBox, SIGNAL(stateChanged(int)), this, SLOT(toggleCompress(int)));
	disconnect(&manualScale, SIGNAL(stateChanged(int)), this, SLOT(toggleManual(int)));
	disconnect(&MaxVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
	disconnect(&MinVal, SIGNAL(editingFinished()), this, SLOT(updateYaxisScale()));
	busy = true;
    connected = false;
}






void graph::mousePressEvent(QMouseEvent*)
{
	emit(setupClick(&setup));
}



void graph::unitchange(int)
{
    updategraph(true);
}




void graph::scalechange(int)
{
    ToolsOn();
    updategraph(true);
}



void graph::newscale(int)
{
    updategraph(true);
}




void graph::newscale(QString)
{
    updategraph(true);
}



void graph::offsetchange(double)
{
    ToolsOn();
    updategraph(true);
}






void graph::getCfgStr(QString &str)
{
	str += "\nGraphique\n";
    str += logisdom::saveformat("Name", Name, true);
	str += logisdom::saveformat("Unit", QString("%1").arg(HUnit.currentIndex()));
	str += logisdom::saveformat("Periode", QString("%1").arg(HScale.value()));
	str += logisdom::saveformat("Rotation", QString("%1").arg(rotation.value()));
	for (int n=0; n<tickFormat.count(); n++)
		str += logisdom::saveformat(QString("tickFormat%1").arg(n), tickFormat.itemText(n));
	str += logisdom::saveformat("tickSelected", tickFormat.currentText());
	str += logisdom::saveformat("Compressed", QString("%1").arg(CompressBox.isChecked()));
    //str += logisdom::saveformat("ContinousUpdate", QString("%1").arg(ContinousUpdate.isChecked()));
    str += logisdom::saveformat("ShowLegend", QString("%1").arg(showLegend.isChecked()));
	str += logisdom::saveformat("ManualScale", QString("%1").arg(manualScale.isChecked()));
	str += logisdom::saveformat("MinScale", MinVal.text());
	str += logisdom::saveformat("MaxScale", MaxVal.text());
    plot.getCurveConfig(str);
    plot.getHistoConfig(str);
    str += EndMark;
	str += "\n";
}






void graph::setCfgStr(QString &strsearch)
{
	bool ok;
	disconnectAll();
	setUnit(logisdom::getvalue("Unit", strsearch).toInt());
	int periode = logisdom::getvalue("Periode", strsearch).toInt(&ok);
	if ((periode != 0) && ok) setPeriode(periode);
	int rot = logisdom::getvalue("Rotation", strsearch).toInt(&ok);
	if (ok) rotation.setValue(rot);
	QString str = logisdom::getvalue("tickFormat0", strsearch);
	int n = 0;
	if (!str.isEmpty())
	{
		tickFormat.clear();
		while (!str.isEmpty())
		{
			tickFormat.addItem(str);
			str = logisdom::getvalue(QString("tickFormat%1").arg(++n), strsearch);
		}
	}
	QString tick = logisdom::getvalue("tickSelected", strsearch);
	if (!tick.isEmpty())
	{
		if (tickFormat.findText(tick) == -1) tickFormat.addItem(tick);
		int index = tickFormat.findText(tick);
		if (index != -1) tickFormat.setCurrentIndex(index);
	}
	if (logisdom::getvalue("Compressed", strsearch) == "0") CompressBox.setChecked(false);
		else CompressBox.setChecked(true);
//    if (logisdom::getvalue("ContinousUpdate", strsearch) == "0") ContinousUpdate.setChecked(false);
//        else ContinousUpdate.setChecked(true);
//	if (logisdom::getvalue("ShowLegend", strsearch) == "0") showLegend.setChecked(false);
//		else showLegend.setChecked(true);
	if (logisdom::getvalue("ManualScale", strsearch) == "1")
	{
	    manualScale.setChecked(true);
	    toggleManual(Qt::Checked);
	}
	else manualScale.setChecked(false);
	QString min = logisdom::getvalue("MinScale", strsearch);
	if (!min.isEmpty()) MinVal.setText(min);
	QString max = logisdom::getvalue("MaxScale", strsearch);
	if (!max.isEmpty()) MaxVal.setText(max);
	plot.setCurveConfig(strsearch);
    plot.setHistoConfig(strsearch);
    updateList();
    //setScale();
    updategraph(true);
    connectAll();
}




void graph::setScale()
{
    plot.setAxisScaleDraw(QwtPlot::xBottom, new CustomScaleDraw(tickFormat.currentText(), rotation.value()));
	updateYaxisScale();
    //plot.d_zoomer[0]->setZoomBase();
}





void graph::setPeriode(int Periode)
{
	HScale.setValue(Periode);
}




void graph::setUnit(int Unit)
{
	HUnit.setCurrentIndex(Unit);
}






QString graph::getName()
{
	return Name;
}




void graph::setName(QString name)
{
	Name = name;
	setWindowTitle(name);
}



void graph::reload()
{
    toolTimer.start(10000);
	if (mutex.tryLock())
	{
		if (reloading) return;
		reloading = true;
		waitSign(true);
		plot.reload();
        updategraph(true);
		reloading = false;
		mutex.unlock();
	}
}


void graph::UpdateClick()
{
    updategraph(true);
}


void graph::updateAxis(QString)
{
	disconnectAll();
	updateAxis(0);
	connectAll();
}




void graph::updateAxis(int)
{
    ToolsOn();
    plot.setAxisScaleDraw(QwtPlot::xBottom, new CustomScaleDraw(tickFormat.currentText(), rotation.value()));
	updateYaxisScale();
}


void graph::zoomUpdate()
{
    updategraph(true);
}


void graph::updategraph(bool)
{
    if (plot.zoomDelay) { plot.zoomDelay --; return; }
	if (parent->mutexGraph.tryLock())
	{
		double scalestart, scaleend;
        QDateTime now = QDateTime::currentDateTime().addSecs(60);
		QDateTime begin, end;
		waitSign(true);
		disconnectAll();
		long int periode = 1;
        periode = HScale.value();
        switch (HUnit.currentIndex())
		{
            case Unit(Minutes) : unit = 60; scaleDiv = periode/21600.0; break;
            case Unit(Heures) : unit = 3600; scaleDiv = periode/360.0; break;
            case Unit(Jours) : unit = SecsInDays; scaleDiv = periode/24.0; break;
            case Unit(Mois) : unit = SecsInDays * 31; scaleDiv = periode*2.0; break;
            default : unit = 60;
		}
		offset = HOffset.value();
        HOffset.setSingleStep(double(periode) / 10.0);
		double B = (periode + offset) * unit;
		double E = offset * unit;
        begin = now.addSecs(int(-B));
        end = now.addSecs(int(-E));
		scalestart = Origin.secsTo(begin);
        scalestart /= double(SecsInDays);
		scaleend = Origin.secsTo(end);
        scaleend /= double(SecsInDays);
		setScale();
        plot.setAxisScale(QwtPlot::xBottom, scalestart, scaleend, scaleDiv);
		updateYaxisScale();
		plot.update(Origin, begin, end, CompressBox.isChecked());
		connectAll();
		waitSign();
		parent->mutexGraph.unlock();
	}
}





void graph::waitSign(bool wait)
{
	if (wait) busyLabel.setText(tr("Loading data"));
	else busyLabel.setText("");
}



void graph::updateYaxisScale()
{
    toolTimer.start(10000);
	if (manualScale.isChecked())
	{
		bool okMin, okMax;
		double min = MinVal.text().toDouble(&okMin);
		double max = MaxVal.text().toDouble(&okMax);
		if (okMin && okMax)
		{
			plot.setAxisScale(QwtPlot::yLeft, min, max);
			return;
		}
	}
    plot.setAxisAutoScale(QwtPlot::yLeft);
}




void graph::updategraph(QDateTime begin, QDateTime end)
{
	double scalestart, scaleend;
	waitSign(true);
	setScale();
	scalestart = Origin.secsTo(begin);
	scalestart /= SecsInDays;
	scaleend = Origin.secsTo(end);
	scaleend /= SecsInDays;
    plot.setAxisScale(QwtPlot::xBottom, scalestart, scaleend, scaleDiv);
	updateYaxisScale();
	plot.update(Origin, begin, end);
	waitSign();
}





void graph::loadingFinished()
{
	QMutexLocker locker(&mutex);
	waitSign();
}




void graph::AddData(double v, const QDateTime &time, QString RomID)
{
	plot.AddData(v, time, RomID);
}





 void graph::delcurve(QString RomID)		//  Remove curve
{
	plot.delCurve(RomID);
	updateList();
}






 void graph::addcurve(QString name, QString RomID, QColor color)		// New curve
{
	plot.addcurve(name, RomID, color);
	updateList();
}



 void graph::addhisto(QString, QString, QColor)		// New curve
{
    //plot.addhisto(name);
    //updateList();
}



bool graph::IsRomIDthere(QString RomID)
{
	bool answer = false;
	for (int n=0; n<plot.curve.count(); n++)
		if (plot.curve[n]->getRomID() == RomID) answer = true;
	return answer;
}






void graph::setCurveTitle(QString romID, QString Title)
{
	for (int n=0; n<plot.curve.count(); n++)
		if (plot.curve[n]->getRomID() == romID)
		{
			QwtText title;
			//title.setColor(plot.curve[n]->Color);
			title.setText(Title);
			plot.curve[n]->curve->setTitle(title);
		}
	updateList();
}






//void graph::contextMenuEvent(QContextMenuEvent *event)
//{
//	QMenu contextualmenu;
//	QAction Reload(tr("&Reload"), this);
//	if (reloading or busy) Reload.setEnabled(false);
//	QAction Compr(tr("&Compress"), this);
//	if (reloading or busy) Compr.setEnabled(false);
//	QAction Scroll(tr("&Show Control"), this);
//	Scroll.setCheckable(true);
//	Compr.setCheckable(true);
//	Compr.setChecked(compress);
//	QAction RemoveScale(tr("&Remove Current Scale"), this);
//	if ((tickFormat.currentIndex() == -1) or busy or reloading) RemoveScale.setEnabled(false);
//	QAction *selection;
//	contextualmenu.addAction(&Reload);
//	contextualmenu.addAction(&Compr);
//	contextualmenu.addAction(&Scroll);
//	contextualmenu.addAction(&RemoveScale);
//	selection = contextualmenu.exec(mapToGlobal(event->pos()));
//	if (selection == &Reload) reload();
//	if (selection == &Compr) toggleCompress();
//	if (selection == &RemoveScale) removeScaleLegend();
//}





void graph::toggleManual(int state)
{
	if (state == Qt::Checked)
	{
		MaxVal.setEnabled(true);
		MinVal.setEnabled(true);
	}
	else
	{
		MaxVal.setEnabled(false);
		MinVal.setEnabled(false);
	}
	updateYaxisScale();
}


void graph::ToolsOn()
{
    layout->addWidget(&HScale, 1, 0, 1, 1);
    HScale.show();
    layout->addWidget(&HUnit, 1, 1, 1, 1);
    HUnit.show();
    layout->addWidget(&HOffset, 1, 2, 1, 1);
    HOffset.show();
    layout->addWidget(&tickFormat, 1, 3, 1, 1);
    tickFormat.show();
    layout->addWidget(&RemoveScaleButton, 1, 4, 1, 1);
    RemoveScaleButton.show();
    layout->addWidget(&rotation, 1, 5, 1, 1);
    rotation.show();
    layout->addWidget(&ReloadButton, 1, 6, 1, 1);
    ReloadButton.show();
    layout->addWidget(&CompressBox, 1, 7, 1, 1);
    CompressBox.show();
    layout->addWidget(&busyLabel, 1, 8, 1, 1);
    busyLabel.show();
    toolTimer.start(10000);
}


void graph::ToolsOff()
{
    plot.showTools = false;
    layout->removeWidget(&HScale);
    HScale.hide();
    layout->removeWidget(&HUnit);
    HUnit.hide();
    layout->removeWidget(&HOffset);
    HOffset.hide();
    layout->removeWidget(&tickFormat);
    tickFormat.hide();
    layout->removeWidget(&RemoveScaleButton);
    RemoveScaleButton.hide();
    layout->removeWidget(&rotation);
    rotation.hide();
    layout->removeWidget(&ReloadButton);
    ReloadButton.hide();
    layout->removeWidget(&CompressBox);
    CompressBox.hide();
    layout->removeWidget(&busyLabel);
    busyLabel.hide();
}





void graph::removeScaleLegend()
{
	int index = tickFormat.currentIndex();
	if (index != -1) tickFormat.removeItem(index);
}





void graph::toggleCompress(int)
{
    toolTimer.start(10000);
    updategraph(true);
}






void graph::showLegendChanged(int state)
{
	if (state == Qt::Unchecked)
	{

	}
	else
	{

	}
}



bool graph::isCurveAlone(QString romID)
{
	if (plot.curve.count() == 0) return false;
	if (plot.curve.count() > 1) return false;
	if (plot.curve[0]->getRomID() == romID) return true;
	return false;
}
