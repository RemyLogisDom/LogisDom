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
#include <QtWidgets/QFormLayout>
//QT 6 #include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QColorDialog>
#include <QtGui>
#include <QtNetwork>
#include <QtCore>
#include <cmath>
#include <limits>


#include "globalvar.h"
#include "addicons.h"
#include "addDaily.h"
#include "graphconfig.h"
#include "iconearea.h"
#include "iconf.h"
#include "meteo.h"
#include "commonstring.h"
#include "programevent.h"
#include "addProgram.h"
#include "alarmwarn.h"
#include "vmc.h"
#include "chauffageunit.h"
#include "remote.h"
#include "energiesolaire.h"
#include "programevent.h"
#include "configwindow.h"
#include "addProgram.h"
#include "connection.h"
#include "tableau.h"
#include "errlog.h"
#include "tableauconfig.h"
#include "treehtmlwidget.h"
#include "htmlbinder.h"
#include "server.h"
#include "quazip.h"
#include "quazipfile.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "logisdom.h"
#include "deadevice.h"


bool logisdom::NoHex = false;

#if QT_VERSION < 0x060000
    #define openModeWrite QIODevice::WriteOnly
#else
    #define openModeWrite QIODeviceBase::WriteOnly
#endif

#if QT_VERSION < 0x060000
    #define openModeRead QIODevice::ReadOnly
#else
    #define openModeRead QIODeviceBase::ReadOnly
#endif

logisdom::logisdom(QWidget *parent) : QMainWindow(parent)
{
#include "version.txt"
    loadPlugins();
    repertoiredat = defaultrepertoiredat;
    repertoirezip = defaultrepertoiredat;
    repertoirebackup = defaultrepertoirebackup;
    diag = false;
    remydev = false;
    logTag = false;
    paletteOnTop = false;
    previousSetup = nullptr;
    logfilesizemax = 100000;
    indexupdate = 0;
    lastminute = -1;
    iconFileCheck = false;
    saveInterval = -1;
    configfilename = "maison.cfg";
    mainTreeHtml = new treeHtmlWidget(this);
    mainTreeHtml->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mainTreeHtml, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(MainTreeClick(QPoint)));
    DateFormatDetails = tr("d		the day as number without a leading zero (1 to 31)\n\
dd		the day as number with a leading zero (01 to 31)\n\
ddd		the abbreviated localized day name (e.g. 'Mon' to 'Sun')\n\
dddd	the long localized day name (e.g. 'Monday' to 'Qt::Sunday')\n\
M		the month as number without a leading zero (1-12)\n\
MM		the month as number with a leading zero (01-12)\n\
MMM		the abbreviated localized month name (e.g. 'Jan' to 'Dec')\n\
MMMM	the long localized month name (e.g. 'January' to 'December')\n\
yy		the year as two digit number (00-99)\n\
yyyy	the year as four digit number");

	TimeFormatDetails = ("h	the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)\n\
hh	the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)\n\
H	the hour without a leading zero (0 to 23, even with AM/PM display)\n\
HH	the hour with a leading zero (00 to 23, even with AM/PM display)\n\
m	the minute without a leading zero (0 to 59)\n\
mm	the minute with a leading zero (00 to 59)\n\
AP or A	use AM/PM display. AP will be replaced by either AM or PM\n\
ap or a	use am/pm display. ap will be replaced by either am or pm\n");



// tool bar icon
	greenDotIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/ballgreen.png")));
	yellowDotIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/ballred.png")));
	redDotIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/ballyellow.png")));
	lockedIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/lock.png")));
	unlockedIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/unlock.png")));
	paletteIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/palette.png")));
	configIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/config.png")));
	saveIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/save.png")));
	tableIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/table.png")));
	designIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/design.png")));
	sunIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/soleil.png")));
	switchIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/switch1.png")));
	dailyIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/daily.png")));
	weeklyIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/weekly.png")));
	devicesIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/device.png")));
	configIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/tools.png")));
	graphicIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/graph.png")));
	aboutIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/about.png")));

// status tool bar
	statusAction = new QAction(greenDotIcon, tr("Status"), this);

	lockAction = new QAction(unlockedIcon, tr("Lock"), this);
    connect(lockAction, SIGNAL(triggered(bool)), this, SLOT(swapLock(bool)));

    paletteAction = new QAction(paletteIcon, tr("Palette"), this);
    connect(paletteAction, SIGNAL(triggered(bool)), this, SLOT(paletteShow(bool)));

    saveAction = new QAction(saveIcon, tr("Save"), this);
	saveAction->setEnabled(false);
	connect(saveAction, SIGNAL(triggered(bool)), this, SLOT(saveShow(bool)));

	aboutAction = new QAction(aboutIcon, tr("About"), this);
	connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(aboutShow(bool)));

	tableAction = new QAction(tableIcon, tr("Table"), this);
	connect(tableAction, SIGNAL(triggered(bool)), this, SLOT(tableShow(bool)));

	designAction = new QAction(designIcon, tr("Icones"), this);
	connect(designAction, SIGNAL(triggered(bool)), this, SLOT(designShow(bool)));

	sunAction = new QAction(sunIcon, tr("Sun"), this);
	connect(sunAction, SIGNAL(triggered(bool)), this, SLOT(sunShow(bool)));

	switchAction = new QAction(switchIcon, tr("Switch"), this);
    connect(switchAction, SIGNAL(triggered(bool)), this, SLOT(switchShow(bool)));

	dailyAction = new QAction(dailyIcon, tr("Daily"), this);
	connect(dailyAction, SIGNAL(triggered(bool)), this, SLOT(dailyShow(bool)));

	weeklyAction = new QAction(weeklyIcon, tr("Weekly"), this);
    connect(weeklyAction, SIGNAL(triggered(bool)), this, SLOT(weeklyShow(bool)));

	devicesAction = new QAction(devicesIcon, tr("Devices"), this);
	connect(devicesAction, SIGNAL(triggered(bool)), this, SLOT(devicesShow(bool)));

	configAction = new QAction(configIcon, tr("Setup"), this);
	connect(configAction, SIGNAL(triggered(bool)), this, SLOT(configShow(bool)));

	graphicAction = new QAction(graphicIcon, tr("Graphics"), this);
	connect(graphicAction, SIGNAL(triggered(bool)), this, SLOT(graphicShow(bool)));

// add Actions
	statusBar.addAction(statusAction);
	statusBar.addAction(lockAction);
    statusBar.addAction(paletteAction);
	statusBar.addAction(designAction);
	statusBar.addAction(graphicAction);
	statusBar.addAction(devicesAction);
	statusBar.addAction(dailyAction);
	statusBar.addAction(weeklyAction);
	statusBar.addAction(sunAction);
	statusBar.addAction(switchAction);
	statusBar.addAction(tableAction);
	statusBar.addAction(configAction);
	statusBar.addAction(saveAction);
	statusBar.addAction(aboutAction);
	statusBar.setIconSize(QSize(32, 32));
	addToolBar(Qt::RightToolBarArea, &statusBar);

// Palette setup
	QScrollArea *scroll = new QScrollArea();
	QGridLayout *paletteWidgetLayout = new QGridLayout(&palette);
	paletteWidgetLayout->addWidget(scroll);
	paletteLayout = new QGridLayout(scroll);
	scroll->setWidget(&paletteWidget);
    connect(&alwaysOnTop, SIGNAL(toggled(bool)), this, SLOT(onAllwaysTop(bool)));

    palette.setWindowTitle(tr("Palette"));
	paletteLayout->setAlignment(Qt::AlignTop);
	PreviousButton.setToolTip(tr("Previous"));
	PreviousButton.setEnabled(false);
	PreviousButton.setIcon(QIcon(style()->standardPixmap(QStyle::SP_ArrowLeft)));
    paletteLayout->addWidget(&PreviousButton, 0, 0, 1, 1);
    MainToolButton.setText("...");
    paletteLayout->addWidget(&MainToolButton, 0, 1, 1, 2);
    alwaysOnTop.setText("On Top");
    paletteLayout->addWidget(&alwaysOnTop, 0, 3, 1, 1);

	NextButton.setToolTip(tr("Next"));
	NextButton.setEnabled(false);
	NextButton.setIcon(QIcon(style()->standardPixmap(QStyle::SP_ArrowRight)));
    paletteLayout->addWidget(&NextButton, 0, 4, 1, 1);
	paletteHistoryIndex = -1;

// resize setup
	resizeLayout = new QGridLayout(&resize);
	resize.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
	resize.setWindowTitle(tr("Resize"));
	resizeLayout->setAlignment(Qt::AlignTop);
	resizeIconValue.setRange(1, 999);
	resizeIconValue.setValue(100);
	resizeIconValue.setSuffix(" %");
	labelSize.setText(tr("Size"));
//	resizeLayout->addWidget(&labelSize, 0, 0, 1, 1);
//	resizeLayout->addWidget(&resizeIconValue, 0, 1, 1, 1);

//	resizeAllTab.setChecked(true);
//	resizeAllTab.setText(tr("All Tab"));
//	resizeLayout->addWidget(&resizeAllTab, 1, 0, 1, 1);
//	moveLabels.setChecked(true);
//	moveLabels.setText(tr("Move"));
//	resizeLayout->addWidget(&moveLabels, 2, 0, 1, 1);

	labelWorkSpaceX.setText(tr("Area X Size"));
	workspaceX.setRange(1, 9999);
	workspaceX.setValue(1024);
	workspaceX.setSuffix(" pix");
	resizeLayout->addWidget(&labelWorkSpaceX, 3, 0, 1, 1);
	resizeLayout->addWidget(&workspaceX, 3, 1, 1, 1);

	labelWorkSpaceY.setText(tr("Area Y Size"));
	workspaceY.setRange(1, 9999);
	workspaceY.setValue(768);
	workspaceY.setSuffix(" pix");
	resizeLayout->addWidget(&labelWorkSpaceY, 4, 0, 1, 1);
	resizeLayout->addWidget(&workspaceY, 4, 1, 1, 1);

	htmlBind = new htmlBinder(this, tr("View"));
    LogisDomLocal.setNumberOptions(QLocale::OmitGroupSeparator);

// Icon Tool Bar
	QFormLayout *toolWidgetLayout = new QFormLayout(&iconTools);
	iconToolBar.setOrientation(Qt::Vertical);
	toolWidgetLayout->addWidget(&iconToolBar);

    QIcon icon1(QString::fromUtf8(":/images/images/align_left.png"));
    QAction *action = new QAction(icon1, tr("Align Left"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconAlignLeft(bool)));
	iconToolBar.addAction(action);

    QIcon icon2(QString::fromUtf8(":/images/images/align_right.png"));
    action = new QAction(icon2, tr("Align Right"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconAlignRight(bool)));
	iconToolBar.addAction(action);

    QIcon icon3(QString::fromUtf8(":/images/images/align_top.png"));
    action = new QAction(icon3, tr("Align Top"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconAlignTop(bool)));
	iconToolBar.addAction(action);

    QIcon icon4(QString::fromUtf8(":/images/images/align_bottom.png"));
    action = new QAction(icon4, tr("Align Bottom"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconAlignBottom(bool)));
	iconToolBar.addAction(action);

    QIcon icon5(QString::fromUtf8(":/images/images/rich_marked.png"));
    action = new QAction(icon5, tr("Copy Size"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconCopysize(bool)));
	iconToolBar.addAction(action);

    QIcon icon6(QString::fromUtf8(":/images/images/copy.png"));
    action = new QAction(icon6, tr("Copy Selection"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconCopy(bool)));
	iconToolBar.addAction(action);

    QIcon icon7(QString::fromUtf8(":/images/images/paste.png"));
    action = new QAction(icon7, tr("Paste Selection"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconPaste(bool)));
	iconToolBar.addAction(action);

    QIcon icon8(QString::fromUtf8(":/images/images/undo.png"));
    action = new QAction(icon8, tr("Undo"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconUndo(bool)));
	iconToolBar.addAction(action);

    QIcon icon9(QString::fromUtf8(":/images/images/redo.png"));
    action = new QAction(icon9, tr("Redo"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconRedo(bool)));
	iconToolBar.addAction(action);

    QIcon icon10(QString::fromUtf8(":/images/images/newicon.png"));
    action = new QAction(icon10, tr("Add"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(iconAddOne(bool)));

	iconTools.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
	iconTools.setWindowTitle(tr("Icon Tools"));
}



void logisdom::loadPlugins()
{
    QDir pluginsDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("plugins");
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        //qInfo() << "File " + pluginsDir.absoluteFilePath(fileName) + " found " + pluginLoader.errorString();
        if (plugin) {
            LogisDomInterface *logisdomInterface = qobject_cast<LogisDomInterface *>(plugin);
            if (logisdomInterface) {
                logisdomInterfaces.append(logisdomInterface);
                qInfo() << "Plugin " + fileName + " loaded";
                logisdomInterface->setConfigFileName(fileName);
            }
        }
    }
}


void logisdom::onAllwaysTop(bool checked)
{
    Qt::WindowFlags flags = palette.windowFlags();
    paletteOnTop = checked;
    if (checked)
    {
        flags |= Qt::WindowStaysOnTopHint;
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    palette.setWindowFlags(flags);
    palette.show();
}



logisdom::~logisdom()
{
	delete configwin;
	delete alarmwindow;
	delete ProgEventArea;
	delete MeteoArea;
	delete SwitchArea;
}


void logisdom::showPalette()
{
    paletteShow(true);
}


void logisdom::paletteShow(bool)
{
    palette.show();
    palette.raise();
    if (ui.tabWidget->currentIndex() == 3) changePalette(&MeteoArea->setup);
}

		
void logisdom::saveShow(bool)
{
	saveconfig(configfilename);
}


void logisdom::aboutShow(bool)
{
	aproposde();
}


void logisdom::tableShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showtableauconfig();
}


void logisdom::designShow(bool)
{
	AddIcones();
}



void logisdom::sunShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showenergiesolaire();
}


void logisdom::switchShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showswitch();
}

void logisdom::dailyShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    AddDailyPrg();
}


void logisdom::weeklyShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showaddProgram();
}


void logisdom::devicesShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showdevices();
}


void logisdom::configShow(bool)
{
    showconfig();
}


void logisdom::graphicShow(bool)
{
    if (palette.isHidden()) { palette.show(); palette.raise(); }
    showgraphconfig();
}



bool logisdom::isPaletteHidden()
{
	return palette.isHidden();
}


void logisdom::showIconTools()
{
    //if (palette.isHidden()) { palette.show(); palette.raise(); }
    iconTools.show();
}




QToolBar *logisdom::getToolstatus()
{
	if (iconTools.isHidden()) return nullptr;
	return &iconToolBar;
}



void logisdom::PaletteHide(bool hide)
{
	if (hide) palette.hide(); else
	{
	    palette.show();
	    palette.raise();
	}
}


void logisdom::PaletteClear()
{
    //previousPalette();
}


void logisdom::changePalette(QWidget *setup)
{
    setPalette(setup);
}



void logisdom::setPalette(QWidget *setup)
{
    if (previousSetup)
    {
        previousSetup->hide();
        if (setup) paletteLayout->removeWidget(setup);
    }
    if (previousSetup != setup)
    {
        previousSetup = setup;
        if (paletteHistory.isEmpty()) paletteHistory.append(setup);
        else if (paletteHistory.last() != setup) paletteHistory.append(setup);
        if (paletteHistory.count() > 100) paletteHistory.removeFirst();
		if (paletteHistory.count() > 1)
		{
			PreviousButton.setEnabled(true);
			NextButton.setEnabled(false);
		}
        paletteHistoryIndex = -1;
        checkSetupParent(setup);
    }
    if (setup) paletteLayout->addWidget(setup, 1, 0, 1, PaletteWidth);
	if (setup) setup->show();
	palette.raise();
}





void logisdom::nextPalette()
{
	if (paletteHistoryIndex == -1) return;
	if (paletteHistoryIndex < paletteHistory.count())
	{
		paletteHistoryIndex ++;
		if (previousSetup) previousSetup->hide();
        paletteLayout->addWidget(paletteHistory[paletteHistoryIndex], 1, 0, 1, PaletteWidth);
		paletteHistory[paletteHistoryIndex]->show();
		previousSetup = paletteHistory[paletteHistoryIndex];
		if (paletteHistoryIndex == paletteHistory.count() - 1) NextButton.setEnabled(false);
        checkSetupParent(paletteHistory[paletteHistoryIndex]);
        PreviousButton.setEnabled(true);
	}
}




void logisdom::previousPalette()
{
	if (paletteHistoryIndex == -1)
	{
		paletteHistoryIndex = paletteHistory.count() - 2;
		if (previousSetup) previousSetup->hide();
        paletteLayout->addWidget(paletteHistory[paletteHistoryIndex], 1, 0, 1, PaletteWidth);
		paletteHistory[paletteHistoryIndex]->show();
		previousSetup = paletteHistory[paletteHistoryIndex];
        checkSetupParent(paletteHistory[paletteHistoryIndex]);
    }
	else if (paletteHistoryIndex > 0)
	{
		paletteHistoryIndex --;
		if (previousSetup) previousSetup->hide();
        paletteLayout->addWidget(paletteHistory[paletteHistoryIndex], 1, 0, 1, PaletteWidth);
		paletteHistory[paletteHistoryIndex]->show();
		previousSetup = paletteHistory[paletteHistoryIndex];
		if (paletteHistoryIndex == 0) PreviousButton.setEnabled(false);
        checkSetupParent(paletteHistory[paletteHistoryIndex]);
    }
	NextButton.setEnabled(true);
}




void logisdom::checkSetupParent(QWidget *w)
{
    foreach (onewiredevice *dev, configwin->devicePtArray)
    {
        if (&dev->setup == w)
        {
            MainToolButton.setText(tr("Devices"));
            return;
        }
    }
    foreach (graph *g, graphconfigwin->graphPtArray)
    {
        if (&g->setup == previousSetup)
        {
            MainToolButton.setText(tr("Graphic"));
            return;
        }
    }
    if (w == &graphconfigwin->setup) MainToolButton.setText(tr("Graphic"));
    if (w == &tableauConfig->setup) MainToolButton.setText(tr("Array"));
    if (w == &AddDaily->setup) MainToolButton.setText(tr("Daily"));
    if (w == &configwin->setup) MainToolButton.setText(tr("Devices"));
    if (w == &ChauffageArea->setup) MainToolButton.setText(tr("Heating"));
    if (w == &SwitchArea->setup) MainToolButton.setText(tr("Switch"));
    if (w == &MeteoArea->setup) MainToolButton.setText(tr("Weather"));
    if (w == &AddProgwin->setup) MainToolButton.setText(tr("Program"));
    if (w == EnergieSolaire) MainToolButton.setText(tr("Solar"));
}



void logisdom::MainToolClick()
{
    if (paletteHistory.count() == 0) { showdevices(); return; }
    if (previousSetup == nullptr) return;
    foreach (onewiredevice *dev, configwin->devicePtArray)
    {
        if (&dev->setup == previousSetup)
        {
            showdevices();
            return;
        }
    }
    foreach (graph *g, graphconfigwin->graphPtArray)
    {
        if (&g->setup == previousSetup)
        {
            changePalette(&graphconfigwin->setup);
            return;
        }
    }
}


void logisdom::get_ConfigData(QString &data)
{
    QMutexLocker locker(&mutexGetConfig);
    if (configData.isEmpty())
    {
        QFile file(configfilename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
        }
        else
        {
            QTextStream in(&file);
            //if (configwin->textCodec) in.setCodec(configwin->textCodec);
            configData.append(in.readAll());
            file.close();
        }
    }
    data.append(configData);
}


void logisdom::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
    {
        if (palette.isHidden())
        {
            palette.show();
            //palette.raise();
        }
        else palette.hide();
    }
/*    if (ui.tabWidgetIcon->currentWidget()->hasFocus())
    {
        if (event->key() == Qt::Key_4) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveLeft();
        if (event->key() == Qt::Key_6) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveRight();
        if (event->key() == Qt::Key_8) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveUp();
        if (event->key() == Qt::Key_2) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveDown();
        if (event->key() == Qt::Key_Left) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveLeft();
        if (event->key() == Qt::Key_Right) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveRight();
        if (event->key() == Qt::Key_Up) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveUp();
        if (event->key() == Qt::Key_Down) IconeAreaList.at(ui.tabWidgetIcon->currentIndex())->moveDown();
    }*/
}


void logisdom::removeWidget(QWidget *widget)
{
	if (previousSetup == widget) widget->hide();
	previousSetup = nullptr;
	paletteHistory.removeAll(widget);
	if (paletteHistoryIndex > paletteHistory.count() - 1)
	{
		NextButton.setEnabled(false);
		paletteHistoryIndex = paletteHistory.count() - 1;
	}
}




bool logisdom::getTabPix(const QString &name, QBuffer &buffer)
{
    QMutexLocker locker(&MutexgetTabPix);
// check icone area names
    for (int i=0; i<ui.tabWidgetIcon->count(); i++)
            if (ui.tabWidgetIcon->tabText(i) == name)
            {
                    QSize tabSize = maison1wirewindow->ui.tabWidgetIcon->currentWidget()->size();
                    maison1wirewindow->ui.tabWidgetIcon->widget(i)->resize(tabSize);
                    maison1wirewindow->ui.tabWidgetIcon->widget(i)->repaint();
                    if (maison1wirewindow->configwin->ui.checkBoxHtmlSize->isChecked())
                    {
                            //QPixmap pixmap(QPixmap::grabWidget(maison1wirewindow->ui.tabWidgetIcon->widget(i)));
                            QPixmap pixmap(maison1wirewindow->ui.tabWidgetIcon->widget(i)->grab());
                            QSize S = pixmap.size();
                            int size = maison1wirewindow->configwin->ui.spinBoxHtmlSize->value();
                            if (maison1wirewindow->configwin->ui.comboBoxHtmlSize->currentIndex() == 0)
                                    S.scale(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio);
                            else
                                    S.scale(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio);
                            //QPixmap::grabWidget(maison1wirewindow->ui.tabWidgetIcon->widget(i)).scaled(S, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(&buffer, "PNG");
                            maison1wirewindow->ui.tabWidgetIcon->widget(i)->grab().scaled(S, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(&buffer, "PNG");
                    }
                    else    //QPixmap::grabWidget(maison1wirewindow->ui.tabWidgetIcon->widget(i)).save(&buffer, "PNG"); // writes pixmap into bytes in PNG format
                        maison1wirewindow->ui.tabWidgetIcon->widget(i)->grab().save(&buffer, "PNG");
                    return true;
            }
// check graphique names
    for (int i=0; i<ui.tabWidgetGraphic->count(); i++)
            if (ui.tabWidgetGraphic->tabText(i) == name)
            {
                    QSize tabSize = ui.tabWidgetGraphic->currentWidget()->size();
                    graphconfigwin->graphPtArray.at(i)->resize(tabSize);
                    if (!graphconfigwin->graphPtArray.at(i)->ContinousUpdate.isChecked()) graphconfigwin->graphPtArray.at(i)->updategraph(true);
                    graphconfigwin->graphPtArray.at(i)->resize(tabSize);
                    if (configwin->ui.checkBoxHtmlSize->isChecked())
                    {
                            QSize S(graphconfigwin->graphPtArray.at(i)->size());
                            QPixmap pixmap(S);
                            graphconfigwin->graphPtArray.at(i)->render(&pixmap);
                            int size = configwin->ui.spinBoxHtmlSize->value();
                            if (configwin->ui.comboBoxHtmlSize->currentIndex() == 0)
                                    pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&buffer, "PNG");
                            else
                                    pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&buffer, "PNG");
                    }
                    else
                    {
                            QPixmap pixmap(graphconfigwin->graphPtArray.at(i)->size());
                            graphconfigwin->graphPtArray.at(i)->render(&pixmap);
                            pixmap.save(&buffer, "PNG");
                    }
                    return true;
            }
// check chart names
    for (int i=0; i<ui.tabWidgetChart->count(); i++)
            if (ui.tabWidgetChart->tabText(i) == name)
            {
                    if (configwin->ui.checkBoxHtmlSize->isChecked())
                    {
                            QSize S(tableauConfig->tableauPtArray.at(i)->size());
                            QPixmap pixmap(S);
                            tableauConfig->tableauPtArray.at(i)->render(&pixmap);
                            int size = configwin->ui.spinBoxHtmlSize->value();
                            if (configwin->ui.comboBoxHtmlSize->currentIndex() == 0)
                                    pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&buffer, "PNG");
                            else
                                    pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&buffer, "PNG");
                   }
                    else
                    {
                            QPixmap pixmap(tableauConfig->tableauPtArray.at(i)->size());
                            tableauConfig->tableauPtArray.at(i)->render(&pixmap);
                            pixmap.save(&buffer, "PNG");
                    }
                    return true;
            }
// check meteo
    if (name == "meteo")
    {
        QSize tabSize = ui.tabWidget->widget(3)->size();
        ui.tabWidget->widget(3)->resize(tabSize);
        if (configwin->ui.checkBoxHtmlSize->isChecked())
        {
                QSize S(ui.tabWidget->widget(3)->size());
                QPixmap pixmap(S);
                ui.tabWidget->widget(3)->render(&pixmap);
                int size = configwin->ui.spinBoxHtmlSize->value();
                if (configwin->ui.comboBoxHtmlSize->currentIndex() == 0)
                        pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&buffer, "PNG");
                else
                        pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&buffer, "PNG");
       }
        else
        {
                QPixmap pixmap(ui.tabWidget->widget(3)->size());
                ui.tabWidget->widget(3)->render(&pixmap);
                pixmap.save(&buffer, "PNG");
        }
        return true;
    }
    return false;
}





void logisdom::logthis(const QString &filename, const QString &log, QString S)
{
	QMutexLocker locker(&mutexLog);
	QDir localdirectory = QDir("");
	if (!localdirectory.exists(repertoirelog))
		if (!localdirectory.mkdir(repertoirelog))
		{
                        messageBox::criticalHide(this, "log directory","Impossible to create log directory", this);
			return;
		}
	QDateTime now = QDateTime::currentDateTime();
	// Create new empty file
	QString dotnewfilename = QString(repertoirelog) + QDir::separator() + filename + ".new";
	QString dotlogfilename = QString(repertoirelog) + QDir::separator() + filename + ".log";
	QFile dotnewfile(dotnewfilename);
	QFile dotlogfile(dotlogfilename);
	// creat log file if not exists
	if (!dotlogfile.exists())
	{
		if (dotlogfile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream go(&dotlogfile);
			go << now.toString() << " " << filename << ".log file created " << "\n";
			dotlogfile.close();
		}
	}
	// if can open write log text
	if (dotnewfile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream out(&dotnewfile);
		if (S.isEmpty()) out << now.toString("dddd dd/MM/yyyy HH:mm:ss:zzz : ->  ") << log << "\n";
		else out << now.toString() << " ->  " << log << " '" << S << "'" << "\n";
		// append previous log content
		if (dotlogfile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			out << (dotlogfile.readAll());
			dotlogfile.close();
		}
		else
		{
            //messageBox::criticalHide(this, "Open file", "Impossible to open file " + dotlogfilename, this);
			return;
		}
		// if file oversize, truncate it
		dotnewfile.close();
		if (configwin)
		{
			logfilesizemax = configwin->ui.spinBoxLogSize->value() * 1000;
			if (dotnewfile.size() > logfilesizemax) dotnewfile.resize(logfilesizemax);
		}
		// echange files names delete the .log and rename the.new
		dotlogfile.remove();
		dotnewfile.rename(dotlogfilename);
	}
	else
	{
        //messageBox::criticalHide(this,  "Open file", "Impossible to open file " + dotnewfilename, this);
        return;
	}
}





void logisdom::logfile(const QString &log, const QString &S)
{
	logthis(logfilename, log, S);
}



void logisdom::logfile(const QString &log)
{
	logthis(logfilename, log, "");
}






void logisdom::GenError(int ErrID, QString Msg)
{
	QMutexLocker locker(&MutexGenMsg);
	alarmwindow->GeneralErrorLog->SetError(ErrID, Msg);
}





void logisdom::GenMsg(QString Msg)
{
	QMutexLocker locker(&MutexGenMsg);
	alarmwindow->GeneralErrorLog->AddMsg(Msg);
}




void logisdom::DeviceConfigChanged(onewiredevice *device)
{
	tableauConfig->DeviceConfigChanged(device);
	graphconfigwin->DeviceConfigChanged(device);
}





void logisdom::showconfig()
{
	if (configwin->isHidden()) configwin->show(); else configwin->hide();
}




bool logisdom::isRemoteMode()
{
//	QMutexLocker locker(&mutexIsRemote);
	if (RemoteConnection == nullptr) return false;
		else return true;
}



bool logisdom::remoteIsAdmin()
{
	if (RemoteConnection == nullptr) return true;
		else return RemoteConnection->isAdmin();
}


ProgramEvent *logisdom::getProgEventArea()
{
	return ProgEventArea;
}




QTreeWidgetItem *logisdom::addHtmlBinder(htmlBinder *binder)
{
	if (htmlBinderList.contains(binder)) return binder->treeItem;
	QTreeWidgetItem *item = nullptr;
	if (binder->parentItem)
	{
		item = new QTreeWidgetItem(binder->parentItem, 0);
	}
	else
	{
		item = new QTreeWidgetItem(mainTreeHtml, 0);
	}
	htmlBinderList.append(binder);
	treeItemList.append(item);
	binder->ID = binderID.createUuid().toString().remove("\{").remove("}");
    //qDebug() << QString("%1").arg(htmlBinderList.count());
	return item;
}






htmlBinder *logisdom::getTreeItemBinder(QTreeWidgetItem *item)
{
	int n = treeItemList.indexOf(item);
	if (n != -1) return htmlBinderList[n];
	return nullptr;
}




void logisdom::setBinderCommand(QString command)
{
//	GenMsg("Send to Binder  " + ID + "  " + command)
	QString ID = getvalue(CMenuId, command);
	for (int n=0; n<htmlBinderList.count(); n++)
		if (htmlBinderList[n]->ID == ID)
			htmlBinderList[n]->emitSendConfigStr(command);
}



QString logisdom::getHtmlRequest(QString &Request, QString &WebID, int Privilege)
{
    QMutexLocker locker(&ConnectionMutex);
	QString html;
	html += configwin->Header;
	QString ID = getvalue(CMenuId, Request);
	html += "<span class=\"title\">";
	html += configwin->ui.lineEditHtmlTitleText->text();	// Welcome message
	html += "</span>";
	html += "<span class=\"time\">";
	QDateTime now = QDateTime::currentDateTime();
	QString dateFormat = configwin->ui.lineEditHtmlDateFormat->text();
	QString timeFormat = configwin->ui.lineEditHtmlTimeFormat->text();
	if (configwin->ui.checkBoxHtmlDate->isChecked())
	{
		if (dateFormat.isEmpty())  html += "   " + now.toString();
		else html += now.toString(dateFormat);
	}
	if (configwin->ui.checkBoxHtmlTime->isChecked())
	{
		if (timeFormat.isEmpty())  html += "    " + now.toString();
		else html += now.toString(timeFormat);
	}
	html += "<br>";
	html += "</span>";
	html += "<span class=\"menu\">";
	html += "<br>";
	for (int n=0; n<htmlBinderList.count(); n++)
		if (htmlBinderList[n]->parentItem == nullptr)
			html += htmlBinderList[n]->getMainHtml(Request, WebID, Privilege);
	html += "<br>";
	html += "</span>";
	html += "<span class=\"list\">";
	for (int n=0; n<htmlBinderList.count(); n++)
		if (htmlBinderList[n]->ID == ID)
		{
			html += htmlBinderList[n]->getChildHtml(Request, WebID, Privilege, true);
		}
	html += "</span>";
    return html;
}




void logisdom::declareHtmlMenu(QString menuString)
{
	if (menuString.isEmpty()) return;
	bool found;
	found = false;
	for (int n=0; n<htmlBinderList.count(); n++)
		if (htmlBinderList[n]->treeItem->text(0) == menuString) found = true;
	if (!found)
	{
		htmlBinder *bind = new htmlBinder(this);
		bind->setMainParameter(menuString, "");
		addHtmlBinder(bind);
	}
}




void logisdom::alarmOn(bool alarmFlag)
{
	if (alarmFlag) ui.tabWidget->setTabIcon(4, QIcon(QPixmap(QString::fromUtf8(":/images/images/warning.png"))));
	else  ui.tabWidget->setTabIcon(4, QIcon());
}




void logisdom::setTitle(const QString &txt)
{
    if (txt.isEmpty()) setWindowTitle("LogisDom  " + Version);
    else setWindowTitle("LogisDom  " + Version + "  " + tr("Downloading") + " : " + txt);
}




void logisdom::init(char *argv[])
{
	if (argv)
	{
		int c = QCoreApplication::arguments().count();
        for (int n=1; n<c; n++)
        {
            if (QCoreApplication::arguments().at(n) ==  "diag")
            if (QMessageBox::question(this, tr("Diag mode"), tr("Enter in startup diag mode ?"), tr("&Non"), tr("&Oui"), QString(), 1, 0)) diag = true;
            if (QCoreApplication::arguments().at(n) ==  "remy") remydev = true;
            if (QCoreApplication::arguments().at(n) ==  "log") logTag = true;
            //QList<QByteArray> codecList = QTextCodec::availableCodecs();
            //for (int readconfigfilei=0; i<codecList.count(); i++)
            //{
            //    if (QCoreApplication::arguments().at(n) == codecList.at(i)) text_Codec = codecList.at(i);
            //}
            QDir fileSearch("");
            QStringList filters;
            filters << "*.cfg";
            QStringList fileList = fileSearch.entryList(filters);	// Get file list with .cfg
            for (int i=0; i<fileList.count(); i++)
            {
                if (QCoreApplication::arguments().at(n) == fileList.at(i)) configfilename = fileList.at(i);
            }
        }
	}
// set mainwindows UI
	ui.setupUi(this);
	widgetTabHeating = ui.tabWidget->widget(1);
	tabHeatingText = ui.tabWidget->tabText(1);
	ui.tabWidgetIcon->removeTab(0);
    ui.tabWidgetChart->removeTab(0);
    ui.tabWidgetGraphic->removeTab(0);
	setWindowTitle("LogisDom  " + Version);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);
	RemoteConnection = nullptr;

// initialise general settings
	TempInCelsius = defaultTempUnit;
	IconTabisLocked = false;

// create configwindow
	configwin = new configwindow(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create alarm window
	alarmwindow = new alarmwarn(this); 
	QGridLayout *AlarmLayout = new QGridLayout(ui.Alarm);
	ui.Alarm->setLayout(AlarmLayout);
	AlarmLayout->addWidget(alarmwindow, 0, 0, 1, 1);
	connect(alarmwindow, SIGNAL(alarmOn(bool)), this, SLOT(alarmOn(bool)));
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create icons window
    ui.tabWidgetIcon->setMovable(true);
    myicons = new addicons(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create daily window
	AddDaily = new addDaily(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create AddProgram window
	AddProgwin = new weeklyProgram(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create graphique setup window
	graphconfigwin = new graphconfig(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create tableau setup window
	tableauConfig = new tableauconfig(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create energie solaire
	EnergieSolaire = new energiesolaire(this);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);

// create connections Main
	connect(ui.actionQuitter, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionShow_palette, SIGNAL(triggered()), this, SLOT(showPalette()));
    connect(ui.actionConfig, SIGNAL(triggered()), configwin, SLOT(show()));
    connect(ui.actionSaveConfig, SIGNAL(triggered()), this, SLOT(saveconfigtxt()));
	connect(ui.action_propos_de, SIGNAL(triggered()), this, SLOT(aproposde()));
	connect(ui.actionAlarmes, SIGNAL(triggered()), alarmwindow, SLOT(show()));
    connect(ui.actionDead_device, SIGNAL(triggered()), this, SLOT(deadDevice()));
    connect(ui.actionSet_Window_to_Workspace, SIGNAL(triggered()), this, SLOT(setWindowSize()));
    connect(ui.actionResize_Workspace, SIGNAL(triggered()), this, SLOT(showresize()));

// Palette connection
	connect(&NextButton, SIGNAL(clicked()), this, SLOT(nextPalette()));
	connect(&PreviousButton, SIGNAL(clicked()), this, SLOT(previousPalette()));
    connect(&MainToolButton, SIGNAL(clicked()), this, SLOT(MainToolClick()));
    palette.setGeometry(100, 100, 400, 600);


// Resize connection
	//connect(&resizeIconValue, SIGNAL(valueChanged(int)), this, SLOT(resizeChanged(int)));
	connect(&workspaceX, SIGNAL(valueChanged(int)), this, SLOT(workSpaceResizeChanged(int)));
	connect(&workspaceY, SIGNAL(valueChanged(int)), this, SLOT(workSpaceResizeChanged(int)));

	connect(&timerUpdate, SIGNAL(timeout()), this, SLOT(update()));

    ChauffageArea = new ChauffageScrollArea(this);
    ui.scrollAreaHeating->setWidget(ChauffageArea);
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    ProgEventArea = new ProgramEvent(this, configwin);
    ui.scrollAreaEvent->setWidget(ProgEventArea);

    ChauffageArea->displayLayout.addWidget(&ProgEventArea->ButtonAuto, 0, 0, 1, 1);
    ChauffageArea->displayLayout.addWidget(&ProgEventArea->ButtonConfort, 0, 1, 1, 1);
    ChauffageArea->displayLayout.addWidget(&ProgEventArea->ButtonNuit, 0, 2, 1, 1);
    ChauffageArea->displayLayout.addWidget(&ProgEventArea->ButtonEco, 0, 3, 1, 1);
    ChauffageArea->displayLayout.addWidget(&ProgEventArea->ButtonHorsGel, 0, 4, 1, 1);

	SwitchArea = new SwitchScrollArea(this);
	ui.scrollAreaSwitch->setWidget(SwitchArea);
	connect(SwitchArea, SIGNAL(setupClick(QWidget*)), this, SLOT(setPalette(QWidget*)));

	QGridLayout *MeteoLayout = new QGridLayout(ui.Weather);
	ui.Weather->setLayout(MeteoLayout);
	MeteoArea = new meteo(ui.Weather, this);
	MeteoLayout->addWidget(MeteoArea, 0, 0, 1, 1);

    Lock(false);
// Program Area connections
	connect(configwin, SIGNAL(ProgChanged(ProgramData *)), AddDaily, SLOT(updateJourneyBreaks(ProgramData *)));
	connect(configwin, SIGNAL(ProgChanged(ProgramData *)), SwitchArea, SLOT(updateJourneyBreaks(ProgramData *)));
	connect(ProgEventArea, SIGNAL(removeProgramEvent(ProgramData *)), configwin, SLOT(RemovePrgEvt(ProgramData *)));
	connect(ProgEventArea, SIGNAL(addProgramEvent(ProgramData *)), AddDaily, SLOT(AddPrgEvt(ProgramData *)));
	connect(ProgEventArea, SIGNAL(removeProgramEvent(ProgramData *)), AddDaily, SLOT(RemovePrgEvt(ProgramData *)));
	connect(ProgEventArea, SIGNAL(changeProgramEvent(ProgramData *)), AddDaily, SLOT(changePrgEvt(ProgramData *)));
    connect(ProgEventArea, SIGNAL(changeProgramEvent(ProgramData *)), SwitchArea, SLOT(updateJourneyBreaks(ProgramData *)));

    QString configdata;
    get_ConfigData(configdata);
    bool next = true;
	if (diag) if (QMessageBox::question(this, tr("Create Config"), tr("Create Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
    foreach (LogisDomInterface *interface, logisdomInterfaces) {
           configwin->ui.tabWidget->addTab(interface->widgetUi(), interface->getName());
           configwin->LocalTabsNumber ++; }
    if (next) configwin->readconfigfile(configdata);
    foreach (LogisDomInterface *interface, logisdomInterfaces) {
        connect(interface->getObject(), SIGNAL(newDevice(LogisDomInterface*, QString)), this, SLOT(newPluginDevice(LogisDomInterface*, QString)), Qt::QueuedConnection);
        connect(interface->getObject(), SIGNAL(newDeviceValue(QString, QString)), this, SLOT(newPluginDeviceValue(QString, QString)), Qt::QueuedConnection);
        connect(interface->getObject(), SIGNAL(deviceSelected(QString)), this, SLOT(pluginDeviceSelected(QString)));
        connect(interface->getObject(), SIGNAL(updateInterfaceName(LogisDomInterface*, QString)), this, SLOT(pluginUpdateNames(LogisDomInterface*, QString)));
        interface->readConfig();
        int tab = configwin->ui.tabWidget->indexOf(interface->widgetUi());
        if (tab != -1) configwin->ui.tabWidget->setTabText(tab, interface->getName()); }
	next = true;
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Create Virtual Devices"), tr("Create Virtual Devices ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next) configwin->createVirtualDevices(configdata);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Meteo Config"), tr("Read Meteo Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next) MeteoArea->readconfigfile(configdata);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Solaire Config"), tr("Read Solaire Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next) EnergieSolaire->readconfigfile(configdata);
	next = true;
	if (isRemoteMode())
	{
		if (diag) if (QMessageBox::question(this, tr("Read Remote Config"), tr("Read Remote Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
		if (next) readconfigfile(configdata);
		timerUpdate.start(startdelay);
		saveAction->setEnabled(true);
		return;
	}
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Main Config"), tr("Read Main Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
    if (next) readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Tab Config"), tr("Read Tab Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
    //if (next) readIconTabconfigfile(configdata);
    //QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read ProgEvent Config"), tr("Read ProgEvent Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next && ProgEventArea) ProgEventArea->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Daily Config"), tr("Read Daily Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next && AddDaily) AddDaily->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read AddProg Config"), tr("Read AddProg Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next && AddProgwin) AddProgwin->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Chauffage Config"), tr("Read Chauffage Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next && ChauffageArea) ChauffageArea->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Tableau Config"), tr("Read Tableau Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next && tableauConfig) tableauConfig->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Graphic Config"), tr("Read Graphic Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
    if (next && graphconfigwin) graphconfigwin->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Read Switch Config"), tr("Read Switch Config ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
    if (next && SwitchArea) SwitchArea->readconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    if (next) readIconTabconfigfile(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
//	if (configwin->hasSimulatedMaster())
//	QMessageBox::warning(this, cstr::toStr(cstr::MainName), tr("The application is running in simulated mode, data shown are fake"), QMessageBox::AcceptRole, QMessageBox::NoIcon);
	next = true;
	if (diag) if (QMessageBox::question(this, tr("Start timer update"), tr("Start timer update ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No) next = false;
	if (next) timerUpdate.start(startdelay);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    next = true;
    ui.tabWidget->setCurrentIndex(0);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	ui.tabWidget->setCurrentIndex(1);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	ui.tabWidget->setCurrentIndex(2);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	ui.tabWidget->setCurrentIndex(3);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
	if (next) setTabs(configdata);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
// get html file list
    QDir webFiles(repertoirehtml);
	QStringList filters;
	filters << "*.html" << "*.htm";
	QStringList fileList = webFiles.entryList(filters);
	for (int n=0; n<fileList.count(); n++)
	{
		configwin->htmlBindWebMenu->setParameter(fileList.at(n), fileList.at(n));
		configwin->htmlBindWebMenu->setParameterLink(fileList.at(n), fileList.at(n));
	}
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui.tabWidgetPrg, SIGNAL(currentChanged(int)), this, SLOT(tabPrgChanged(int)));
    connect(ui.tabWidgetGraphic, SIGNAL(currentChanged(int)), this, SLOT(tabGraphChanged(int)));
    configwin->startNetwork();
    saveAction->setEnabled(true);
	QDateTime now = QDateTime::currentDateTime();
    sessionID = now.toString("ddMMyyhhmmss");
    logfile(tr("LogisDom Starts"));
    QFile file("qtabwidget.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString style = in.readAll();
        file.close();
        ui.tabWidget->tabBar()->setStyleSheet(style);
        ui.tabWidgetIcon->tabBar()->setStyleSheet(style);
        ui.tabWidgetPrg->tabBar()->setStyleSheet(style);
        ui.tabWidgetChart->tabBar()->setStyleSheet(style);
        ui.tabWidgetGraphic->tabBar()->setStyleSheet(style);
        alarmwindow->ui.errortab->tabBar()->setStyleSheet(style);
    }
}






void logisdom::closeEvent(QCloseEvent *event)
{
	iconTools.hide();
    configwin->hide();
    if (messageBox::questionHide(this, tr("Leave LogisDom ?"), tr("Do you really want to leave LogisDom ?"), this, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
	{
		bool save = false;
		if (configwin->ui.checkBoxSaveQuit->isChecked())
		{
			if (configwin->ui.checkBoxAskSave->isChecked())
			{
				if (messageBox::questionHide(this, tr("Save setup ?"), tr("Do you want to save setup ?"), this, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
				save = true; else save = false;
			}
			else save = true;
		}
        if (!isRemoteMode()) tableauConfig->savePreload();
        if (!isRemoteMode()) configwin->setNextSave();
		if (save) saveconfig(configfilename);
		logfile(tr("Close Connections"));
		configwin->closeNet1Wire();
		logfile(tr("End ofsession"));
        close();
		QCoreApplication::exit(0);
	}
	else event->ignore();
}






void logisdom::restart()
{
        if (messageBox::questionHide(this, tr("Confirm Restart ?"), tr("Do you really want to restart LogisDom ?"), this, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
	{
            logfile(tr("Application restart"));
            saveconfig(configfilenamerestart);
            if (QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()))) QCoreApplication::exit(0);
	}
}




void logisdom::restartnoconfirm()
{
        logfile(tr("Application restart"));
        saveconfig(configfilenamerestart);
        if (QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()))) QCoreApplication::exit(0);
}



void logisdom::valueTranslate(QString &txt)
{
	configwin->valueTranslate(txt);
}



void logisdom::htmlTranslate(QString &datHtml, const QString &id)
{
	configwin->htmlTranslate(datHtml, id);
}




void logisdom::htmlLinkCommand(QString &command)
{
//qDebug() << QString("htmlLinkCommand  ") + command;
	QStringList list = command.split(htmlsperarator);
	if (list.count() == 0) return;
	for (int n=0; n<mainTreeHtml->topLevelItemCount(); n++)
	{
		if (list.at(0) == mainTreeHtml->topLevelItem(n)->text(0))
		{
			QTreeWidgetItem *child = nullptr;
			QTreeWidgetItem *item = mainTreeHtml->topLevelItem(n);
			if (!item) return;
			int count = list.count();
			if (count > 2) count = 2;
			//for (int i=1; i<list.count()-1; i++)
			for (int i=1; i<count; i++)
			{
//qDebug() << QString("list at %1 ").arg(i);
//qDebug() << list.at(i);

				child = nullptr;
				for (int c=0; c<item->childCount(); c++)
				{
//qDebug() << "item " + item->child(c)->text(0);
					if (item->child(c)->text(0) == list.at(i))
					{
						child = item->child(c);
						break;
					}
				}
				if (child) item = child; else break;
			}
			if (child)
			{
//qDebug() << "child " + child->text(0);
				htmlBinder *binder = getTreeItemBinder(child);
				if (binder)
				{
					QString command = list.last();
//qDebug() << "emit " + command;
					binder->emitSendConfigStr(command);
				}
			}
		}
	}
}



htmlBinder* logisdom::getBinder(QString &command)
{
    //qDebug() << "getBinder : " + command;
    QStringList list = command.split(htmlsperarator);
    if (list.count() < 1) return nullptr;
    for (int a=0; a<mainTreeHtml->topLevelItemCount(); a++)
    {   // check fisrt hierachic level ex : declenchement, chauffage, evenement ...
        if (list.first() == mainTreeHtml->topLevelItem(a)->text(0))
        {
            QTreeWidgetItem *item = mainTreeHtml->topLevelItem(a);
            if (!item) return nullptr;
            for (int b=0; b<item->childCount(); b++)
            {   // check second hierachic level ex : nom du chauffage, il existe un html pour cet item
                if (list.at(1) == item->child(b)->text(0))
                {
                    int n = treeItemList.indexOf(item->child(b));
                    if (list.count() == 2)
                    {
                        if (n != -1) return htmlBinderList[n];
                        return nullptr;
                    }
                    else    // check third hierachic level, il n'existe pas de html binder pour cet item
                    {
                        for (int c=0; c<item->child(b)->childCount(); c++)
                        {
                            if (list.at(2) == item->child(b)->child(c)->text(0))
                            {
                                if (list.count() > 3) return nullptr;
                                if (n != -1) return htmlBinderList[n];
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }
    }
    return nullptr;
    //binderici
}



QString logisdom::getHtmlValue(QString &command)
{
	QStringList list = command.split(htmlsperarator);
	if (list.count() == 0) return "";
    //qDebug() << command;
	for (int n=0; n<mainTreeHtml->topLevelItemCount(); n++)
	{
		if (list.at(0) == mainTreeHtml->topLevelItem(n)->text(0))
		{
			QTreeWidgetItem *child = nullptr;
			QTreeWidgetItem *item = mainTreeHtml->topLevelItem(n);
			if (!item) return "";
			for (int i=1; i<list.count(); i++)
			{
				child = nullptr;
				if (!item) return "not found " + command;
				for (int c=0; c<item->childCount(); c++)
				{
					if (item->child(c)->text(0) == list.at(i))
					{
						child = item->child(c);
						break;
					}
				}
				if (child) item = child;
			}
            if (item) return item->text(1);
		}
	}
	return "error in command : " + command;
}





void logisdom::GetMenuHtml(QString *str, QString &ID, int Privilege)
{
	if (Privilege == Server::FullControl)
		str->append(toHtml(tr("Restart Application"), NetRequestMsg[MenuRestart], ID, logisdom::htmlStyleMenu));
}





void logisdom::RestartMenuHtml(QString *str, QString &ID, int Privilege)
{
	if (Privilege == Server::FullControl)
		str->append(toHtml(tr("Confirm Restart Application"), NetRequestMsg[ConfirmRestart], ID, logisdom::htmlStyleMenu));
}





QString logisdom::toHtml(QString Text, QString Order, QString &ID, int style)
{
    QString str;
	str.append("<a href=\"" CRequest "=(");
	str.append(Order);
	str.append(")" CUId "=(");
	str.append(ID);
	if (style < 0) str.append(")\"><span");
	else
	{
		str.append(")\"><span style='");
		switch (style)
		{
			case htmlStyleMenu : str.append("menu"); break;
			case htmlStyleTime : str.append("time"); break;
			case htmlStyleList : str.append("list"); break;
			case htmlStyleDetector : str.append("detector"); break;
			case htmlStyleValue : str.append("value"); break;
			case htmlStyleCommand : str.append("command"); break;
			default : str.append("menu"); break;
		}
		str.append("'>");
	}
	str.append(Text);
	str.append("</span></a>");
	return str;
}






QString logisdom::toHtml(QString Text, QString Link, int style)
{
	QString str;
	str.append("<a href=\"" + Link);
	str.append("\">");
	str.append(spanIt(Text, style));
	str.append("</a>");
	return str;
}



bool logisdom::AreSame(double a, double b)
{
    return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
}

bool logisdom::AreNotSame(double a, double b)
{
    return std::fabs(a - b) > std::numeric_limits<double>::epsilon();
}

bool logisdom::isZero(double a)
{
    return std::fabs(a) < std::numeric_limits<double>::epsilon();
}

bool logisdom::isNotZero(double a)
{
    return std::fabs(a) > std::numeric_limits<double>::epsilon();
}

bool logisdom::isNA(double a)
{
    return std::fabs(a - logisdom::NA) < std::numeric_limits<double>::epsilon();
}

bool logisdom::isNotNA(double a)
{
    return std::fabs(a - logisdom::NA) > std::numeric_limits<double>::epsilon();
}





QString logisdom::styleToString(int style)
{
	QString s;
	switch (style)
	{
		case htmlStyleMenu : s = "menu"; break;
		case htmlStyleTime : s = "time"; break;
		case htmlStyleList : s = "list"; break;
		case htmlStyleDetector : s = "detector"; break;
		case htmlStyleValue : s = "value"; break;
		case htmlStyleCommand : s = "command"; break;
		default : s = "menu"; break;
	}
	return s;
}




QString logisdom::getvalue(QString search, const QString &str)
{
	int l = str.length();
	if (l == 0) return "";

	int i = str.indexOf(search);
	if (i == -1) return "";

	int coma = str.indexOf("(", i);
	if (coma == -1) return "";

	int nextcoma = str.indexOf(")", coma + 1);
	if (nextcoma == -1) nextcoma = str.indexOf("(", coma + 1);
	if (nextcoma == -1) return "";

	QString result = str.mid(coma + 1, nextcoma - coma - 1);
	if (!result.isEmpty())
	{
        if (result.mid(0, 4) == "HEX:")
		{
            QString Hex;
#if QT_VERSION < 0x060000
            Hex.append(result.remove(0, 4));
#else
            Hex.append(result.remove(0, 4).toLatin1());
#endif \
    // Qt 6 QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
            QByteArray F = QByteArray::fromHex(Hex.toLatin1());
            result = F; // Qt 6 Utf8Codec->toUnicode(F);
        }
	}
	return result;
}





QByteArray logisdom::getvalue(QString search, QByteArray &Str)
{
    QString str;
    str.append(Str);
	int l = str.length();
	if (l == 0) return "";

    int i = str.indexOf(search);
	if (i == -1) return "";

	int coma = str.indexOf("(", i);
	if (coma == -1) return "";

	int nextcoma = str.indexOf(")", coma + 1);
	if (nextcoma == -1) nextcoma = str.indexOf("(", coma + 1);
	if (nextcoma == -1) return "";

    QString result = str.mid(coma + 1, nextcoma - coma - 1);
    if (!result.isEmpty())
    {
        if (result.mid(0, 4) == "HEX:")
        {
            QString Hex;
            Hex.append(result.remove(0, 4));
            //QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
            QByteArray F = QByteArray::fromHex(Hex.toLatin1());
            result = F;
        }
    }
    return result.toLatin1();
}


// change also in function configwindow::toHtml();   remote::addGetFiletoFifo(QString name)    remote::init(QString userName, QString password)




QString logisdom::saveformat(QString name, QString value, bool hex)
{
    if ((hex) && (!NoHex))
	{
        QString F = value.toUtf8().toHex().toUpper();
		return name + " = (HEX:" + F + ")\n";
	}
	return name + " = (" + value + ")\n";
}




QString logisdom::filenameformat(QString RomID, int month, int year)
{
	return RomID + QString("_%1").arg(month, 2, 10, QChar('0'))  + QString("-%1").arg(year, 4, 10, QChar('0'));
}




QString logisdom::spanIt(QString str, int style)
{
	QString r;
	if (!str.isEmpty())
	{
		r.append("<span class='");
		r.append(styleToString(style));
		r.append("'>");
		r.append(str);
		r.append("</span>");
	}
	return r;
}




void logisdom::dragEnterEvent(QDragEnterEvent*)
{
}




void logisdom::dragMoveEvent(QDragMoveEvent*)
{
}





void logisdom::dropEvent(QDropEvent *)
{
}




void logisdom::mouseDoubleClickEvent(QMouseEvent *)
{
}



void logisdom::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
            QMenu contextualmenu;
            QAction AddAction(cstr::toStr(cstr::Add), this);
            QAction RemoveAction(cstr::toStr(cstr::Remove), this);
            QAction RenameAction(cstr::toStr(cstr::Rename), this);
            QAction LockAction(cstr::toStr(cstr::Lock), this);
            QAction UnLockAction(cstr::toStr(cstr::UnLock), this);
            QAction ExportPageAction("Export Page", this);
            QAction ExportSelectionAction("Export Selection", this);
            QAction ImportAction("Import New Page", this);
            QAction ImportIntoAction("Import Into Page", this);
            QAction SaveAsHtmlAction("Save as html", this);
            QAction SetBackGroundColor("Set background color", this);
            QAction GetHtmlLink("Open in html", this);
            QAction EnableHtmlAction("HTML Enable", this);
            QAction AddText("Add Text", this);
            QAction *selection;
            if (ui.tabWidget->currentIndex() == 0)
            {
                int index = ui.tabWidgetIcon->currentIndex();
                contextualmenu.addAction(&AddAction);
                contextualmenu.addAction(&RemoveAction);
                contextualmenu.addAction(&RenameAction);
                if (IconTabisLocked)
                {
                    contextualmenu.addAction(&UnLockAction);
                    ImportIntoAction.setEnabled(false);
                    ImportAction.setEnabled(false);
                    RenameAction.setEnabled(false);
                    AddAction.setEnabled(false);
                    RenameAction.setEnabled(false);
                    RemoveAction.setEnabled(false);
                    SetBackGroundColor.setEnabled(false);
                    AddText.setEnabled(false);
                }
                else
                {
                    contextualmenu.addAction(&LockAction);
                    if (index != -1)
                    {
                        if (IconeAreaList.at(index)->hasSelection())
                            contextualmenu.addAction(&ExportSelectionAction);
                    }
                }
                if (IconeAreaList.count() == 0)
                {
                    RemoveAction.setEnabled(false);
                    RenameAction.setEnabled(false);
                    ImportIntoAction.setEnabled(false);
                    RenameAction.setEnabled(false);
                    ExportPageAction.setEnabled(false);
                    SaveAsHtmlAction.setEnabled(false);
                    GetHtmlLink.setEnabled(false);
                    SetBackGroundColor.setEnabled(false);
                }
                contextualmenu.addAction(&ImportIntoAction);
                contextualmenu.addAction(&ExportPageAction);
                contextualmenu.addAction(&ImportAction);
                contextualmenu.addAction(&SaveAsHtmlAction);
                contextualmenu.addAction(&GetHtmlLink);
                contextualmenu.addAction(&SetBackGroundColor);
                contextualmenu.addAction(&AddText);
                if (index != -1)
                {
                    EnableHtmlAction.setCheckable(true);
                    EnableHtmlAction.setChecked(IconeAreaList.at(index)->isHtmlEnabled());
                    contextualmenu.addAction(&EnableHtmlAction);
                }
#if QT_VERSION < 0x060000
                selection = contextualmenu.exec(event->globalPos());
#else
                selection = contextualmenu.exec(event->globalPosition().toPoint());
#endif
                if (selection == &AddAction) addIconTab();
                if (selection == &RemoveAction) removeIconTab();
                if (selection == &RenameAction) renameIconTab();
                if (selection == &LockAction) Lock(true);
                if (selection == &UnLockAction) Lock(false);
                if (selection == &ExportPageAction) exportPage();
                if (selection == &ExportSelectionAction) exportSelection();
                if (selection == &ImportAction) importPage();
                if (selection == &ImportIntoAction) importIntoPage();
                if (selection == &SaveAsHtmlAction) saveHtmlPage();
                if (selection == &GetHtmlLink) getHtmlLink();
                if (selection == &SetBackGroundColor) setBackGroundColor();
                if (selection == &AddText) AddTextZone();
                if (selection == &EnableHtmlAction)
                {
                    if (index != -1)
                    {
                        bool state = IconeAreaList.at(index)->isHtmlEnabled();
                        IconeAreaList.at(index)->setHtmlEnabled(!state);
                    }
                }
            }
    }
    //if (event->button() == Qt::LeftButton)
    //{
    //}
}



void logisdom::exportSelection()
{
    int index = ui.tabWidgetIcon->currentIndex();
    if (index == -1) return;
    QString zipFileName = ui.tabWidgetIcon->tabText(index) + ".zip";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), zipFileName, tr("zip (*.zip)"), nullptr, QFileDialog::DontConfirmOverwrite);
    if (!fileName.endsWith(".zip")) fileName.append(".zip");
    QFile file(fileName);
    if (file.exists())
    {
        if ((messageBox::questionHide(this, tr("Already exist ?"), tr("Do you really want to overwrite ") + fileName, this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
        file.remove();
    }
    QString str;
    IconeAreaList.at(index)->SaveSelectionConfigStr(str);
    QuaZip zipFile(fileName);
    if (zipFile.open(QuaZip::mdCreate))
    {
        if (zipFile.getZipError() == UNZ_OK)
        {
            QString cfgFileName = ui.tabWidgetIcon->tabText(index) + ".cfg";
            QuaZipFile outZipFile(&zipFile);
            // Qt 6
            // if(outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(cfgFileName, cfgFileName)))
            if(outZipFile.open(openModeWrite, QuaZipNewInfo(cfgFileName, cfgFileName)))
            {
                    //QByteArray data;
                    //data.append(str);
                    outZipFile.write(str.toLatin1());
                    outZipFile.close();
            }
            QStringList icons;
            for (int n=0; n<IconeAreaList.at(index)->IconList.count(); n++)
            {
                if (IconeAreaList.at(index)->hasSelection())
                {
                    QString fileName = IconeAreaList.at(index)->IconList.at(n)->path;
                    if (!icons.contains(fileName)) icons.append(fileName);
                    // check icon name with underscore
                    for (int step=1; step<10; step++)
                    {
                        QString N = QString("_%1").arg(step);
                        QString NFile = fileName;
                        NFile = NFile.insert(NFile.length() - 4, N);
                        if (!icons.contains(NFile)) icons.append(NFile);
                    }
                }
            }
            for (int n=0; n<icons.count(); n++)
            {
                QFile iconFile(icons.at(n));
                if (iconFile.open(QIODevice::ReadOnly))
                {
                    QFileInfo iconInfo(iconFile);
                    if(outZipFile.open(openModeWrite, QuaZipNewInfo(iconInfo.fileName(), iconInfo.fileName())))
                    {
                        QByteArray data;
                        data.append(iconFile.readAll());
                        iconFile.close();
                        outZipFile.write(data);
                        outZipFile.close();
                    }
                }
            }
        }
        zipFile.close();
    }
}



void logisdom::exportPage()
{
    int index = ui.tabWidgetIcon->currentIndex();
    if (index == -1) return;
    QString zipFileName = ui.tabWidgetIcon->tabText(index) + ".zip";
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), zipFileName, tr("zip (*.zip)"), nullptr, QFileDialog::DontConfirmOverwrite);
    if (!fileName.endsWith(".zip")) fileName.append(".zip");
    QFile file(fileName);
    if (file.exists())
    {
        if ((messageBox::questionHide(this, tr("Already exist ?"), tr("Do you really want to overwrite ") + fileName, this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
        file.remove();
    }
    QString str;
    IconeAreaList.at(index)->SaveConfigStrIconPath(str);
    QuaZip zipFile(fileName);
    if (zipFile.open(QuaZip::mdCreate))
    {
        if (zipFile.getZipError() == UNZ_OK)
        {
            QString cfgFileName = ui.tabWidgetIcon->tabText(index) + ".cfg";
            QuaZipFile outZipFile(&zipFile);
            if(outZipFile.open(openModeWrite, QuaZipNewInfo(cfgFileName, cfgFileName)))
            {
                    QString data;
                    data.append(str);
                    outZipFile.write(data.toLatin1());
                    outZipFile.close();
            }
            QStringList icons;
            for (int n=0; n<IconeAreaList.at(index)->IconList.count(); n++)
            {
                QString fileName = IconeAreaList.at(index)->IconList.at(n)->path;
                if (!icons.contains(fileName))
                {
                    icons.append(fileName);
                    // check icon name with underscore
                    for (int step=1; step<10; step++)
                    {
                        QString N = QString("_%1").arg(step);
                        QString NFile = fileName;
                        NFile = NFile.insert(NFile.length() - 4, N);
                        if (!icons.contains(NFile)) icons.append(NFile);
                        //qDebug() << "Check _ " + NFile;
                    }
                }
            }
            for (int n=0; n<icons.count(); n++)
            {
                QFile iconFile(icons.at(n));
                if (iconFile.open(QIODevice::ReadOnly))
                {
                    QFileInfo iconInfo(iconFile);
                    if(outZipFile.open(openModeWrite, QuaZipNewInfo(iconInfo.fileName(), iconInfo.fileName())))
                    {
                        QByteArray data;
                        data.append(iconFile.readAll());
                        iconFile.close();
                        outZipFile.write(data);
                        outZipFile.close();
                    }
                }
            }
        }
        zipFile.close();
    }
}




int logisdom::htmlPageExist(QString name)
{
    for (int n=0; n<ui.tabWidgetIcon->count(); n++)
    {
        if (IconeAreaList[n]->isHtmlEnabled())
        {
            if (ui.tabWidgetIcon->tabText(n) + ".html" == name) return n;
            if (ui.tabWidgetIcon->tabText(n) + ".htm" == name) return n;
        }
    }
    return -1;
}



void logisdom::getTabHtml(int index, QString &html)
{
    QMutexLocker locker(&ConnectionMutex);
    if (index < 0) return;
    if (index >= ui.tabWidgetIcon->count()) return;
    QTextStream out(&html);
    QString name = ui.tabWidgetIcon->tabText(index);
    getHtmlHeader(out, name, IconeAreaList.at(index)->getBackGroundColor());
    IconeAreaList.at(index)->getHtml(out);
}


void logisdom::AddTextZone()
{
    int index = ui.tabWidgetIcon->currentIndex();
    if (index < 0) return;
    IconeAreaList.at(index)->AddTextZone();
}



void logisdom::setBackGroundColor()
{
    int index = ui.tabWidgetIcon->currentIndex();
    if (index < 0) return;
    QColor currentcolor = QColorDialog::getColor(IconeAreaList.at(index)->backgroundColor);
    if (!currentcolor.isValid()) return;
    IconeAreaList.at(index)->setBackGroundColor(currentcolor);
}


void logisdom::getHtmlLink()
{
    int index = ui.tabWidgetIcon->currentIndex();
    if (index < 0) return;
    bool state = IconeAreaList.at(index)->isHtmlEnabled();
    if (state == false)
    {
        if ((messageBox::questionHide(this, tr("html must be enabled for this page"), tr("Do you want to enable it ?"), this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
    }
    IconeAreaList.at(index)->setHtmlEnabled(true);
    if (configwin->ui.checkBoxServer->checkState() == Qt::Unchecked)
    {
        if ((messageBox::questionHide(this, tr("Server is needed"), tr("Do you want to activate it ?"), this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
        configwin->show();
        configwin->ui.tabWidgetGeneral->setCurrentIndex(2);
        configwin->ui.tabWidget->setCurrentIndex(0);
        configwin->ui.checkBoxServer->setCheckState(Qt::Checked);
        if (configwin->server->ConnectionUsers.isEmpty())
        {
            if ((messageBox::questionHide(this, tr("No user defined"), tr("Do you want to create one ?"), this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
            configwin->show();
            configwin->ui.tabWidgetGeneral->setCurrentIndex(2);
            configwin->ui.tabWidget->setCurrentIndex(0);
            configwin->addUser();
        }
    }
    int admin_exist = 0;
    if (configwin->server->ConnectionUsers.count() > 0)
    {
        int nb = configwin->server->ConnectionUsers.count();
        for (int n=0; n<nb; n++)
        {
            if (configwin->server->ConnectionUsers.at(n).Rigths == Server::FullControl) admin_exist= n;
        }
    }
    //parent->htmlWebPreview.setUrl(QUrl("http://127.0.0.1:1220/request=(GetMainMenu)user=(...)password=(...)"));
    QString url = "http://127.0.0.1:";
    url += QString("%1").arg(configwin->ui.spinBoxPortServer->value());
    url += "/request=(";
    url += ui.tabWidgetIcon->tabText(index);
    url += ".htm)user=(";
    url += configwin->server->ConnectionUsers.at(admin_exist).Name;
    url += ")password=(";
    url += configwin->server->ConnectionUsers[admin_exist].PassWord;
    url += ")";
    QDesktopServices::openUrl(QUrl(url));
}



void logisdom::saveHtmlPage()
{
    QDir webDir = QDir("");
    if (!webDir.exists(defaultrepertoirehtml))
        if (!webDir.mkdir(defaultrepertoirehtml))
        {
            GenMsg(tr("Cannot create web directory"));
            return;
        }
    QString pictPath = defaultrepertoirehtml;
    if (pictPath.right(1) != QDir::separator()) pictPath.append(QDir::separator());
    pictPath.append(repertoireicon);
    pictPath.append(QDir::separator());
    QDir pictDir = QDir("");
    if (!pictDir.exists(pictPath))
        if (!pictDir.mkdir(pictPath))
        {
            GenMsg(tr("Cannot create pict directory"));
            return;
        }
    int index = ui.tabWidgetIcon->currentIndex();
    if (index < 0) return;
    QString name = ui.tabWidgetIcon->tabText(index);
    QString html;
    IconeAreaList.at(index)->getHtml(html);
    QString path = defaultrepertoirehtml;
    if (path.right(1) != QDir::separator()) path.append(QDir::separator());
    path.append(ui.tabWidgetIcon->tabText(index) + ".html");
    QFile htmlFile(path);
    if (htmlFile.exists())
        if ((messageBox::questionHide(this, tr("Already exist ?"), tr("Do you really want to overwrite ") + path, this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
    if(htmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&htmlFile);
        getHtmlHeader(out, name, IconeAreaList.at(index)->getBackGroundColor());
        out << html;
        htmlFile.close();
        QStringList iconFilePath, iconFileName;
        for (int n=0; n<IconeAreaList.at(index)->IconList.count(); n++)
        {
            QString fileName = IconeAreaList.at(index)->IconList.at(n)->path;
            if (!iconFilePath.contains(fileName))
            {
                iconFilePath.append(fileName);
                QFileInfo info(fileName);
                iconFileName.append(info.fileName());
            }
        }
        for (int n=0; n<iconFilePath.count(); n++)
        {
            QFile iconFile(iconFilePath.at(n));
            iconFile.copy(pictPath + iconFileName.at(n));
        }
        messageBox::warningHide(this, "Html page", tr("Current tab has been save to") + "\n" + path, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
    }
}




void logisdom::getHtmlHeader(QTextStream &out, QString &title, QString backgroundColorHex)
{
    QString codec;
    //codec = configwin->getCodecName();
    if (codec.isEmpty()) codec = "UTF-8";
    out << "<html>\n\
<head>\n\
<meta http-equiv=\"X-UA-Compatible\" content=\"IE=EmulateIE8\">\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=" + codec +"\">\n\
<title>" + title +"</title>\n\
<meta name=\"generator\" content=\"LogisDom html pages - http://logisdom.fr\">\n\
<style type=\"text/css\">\n\
body\n{\nbackground-color: #" + backgroundColorHex + ";\n\
color: #000000;\n\
font-family: Arial;\n\
font-size: 13px;\n\
margin: 0;\n\
padding: 0;\n\
}\n\
</style>\n";
}




void logisdom::importIntoPage()
{
  QString getFileName;
  getFileName = QFileDialog::getOpenFileName(this, tr("Select cfg file "), "", "zip (*.zip)");
  if (getFileName.isEmpty()) return;
  QFile file (getFileName);
  QFileInfo fileInfo(file);
  QString tabName = fileInfo.baseName();
  QuaZip zip(getFileName);
  if(zip.open(QuaZip::mdUnzip))
  {
      //QTextCodec *codec = zip.getFileNameCodec();
      //zip.setFileNameCodec(codec);
      QuaZipFileInfo info;
      QuaZipFile zipFile(&zip);
      QString name;
      QString configdata;
      QStringList fileRenamed, newNameOfFileRenamed;
      for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
      {
          if (zip.getCurrentFileInfo(&info))
          {
              if(zipFile.getZipError() == UNZ_OK)
              {
                  name = zipFile.getActualFileName();
                  if(zipFile.open(QIODevice::ReadOnly))
                  {
                      if (name.endsWith(".cfg"))
                      {
                          configdata.append(zipFile.readAll());
                          zipFile.close();
                      }
                      if (name.endsWith(".png"))
                      {
                          QDir localdirectory = QDir("");
                          if (!localdirectory.exists(repertoireicon))
                              if (!localdirectory.mkdir(repertoireicon)) return;
                          QFile iconFile(QString(repertoireicon) + QDir::separator() + name);
                          if (!fileRenamed.contains(name, Qt::CaseSensitive))
// check if file name already exists
// if already exists, check size, if size is equal use exiting file
// if size is not equal, rename imported file with _%1
                          {
                              if (iconFile.exists())
                              {

                                  int index = 1;
                                  QString nameAleradyThere = name;
                                  nameAleradyThere.chop(4);
                                  QString newFileName = nameAleradyThere + QString("_%1.png").arg(index);
                                  QString newFilePath = QString(repertoireicon) + QDir::separator() + newFileName;
                                  QFile newFile(newFilePath);
                                  while(newFile.exists())
                                  {
                                      index ++;
                                      newFileName = nameAleradyThere + QString("_%1.png").arg(index);
                                      newFilePath = QString(repertoireicon) + QDir::separator() + newFileName;
                                      newFile.setFileName(newFilePath);
                                  }
                                  if (newFile.open(QIODevice::WriteOnly))
                                  {
                                      newFile.write(zipFile.readAll());
                                      newFile.close();
                                      zipFile.close();
                                  }
                                  //qDebug() << QString("newFileSize = %1").arg(newFile.size());
                                  //qDebug() << QString("iconFile = %1").arg(iconFile.size());
                                  if (newFile.size() == iconFile.size())
                                  {
                                      //qDebug() << newFile.fileName() + " Same size";
                                      newFile.remove();
                                      //qDebug() << "File removed";
                                  }
                                  else
                                  {
                                      fileRenamed.append(name);
                                      newNameOfFileRenamed.append(newFileName);
                                      //qDebug() << newFile.fileName() + " not same size";
                                  }
                              }
                              else
                              {
                                  if (iconFile.open(QIODevice::WriteOnly))
                                  {
                                      iconFile.write(zipFile.readAll());
                                      iconFile.close();
                                      zipFile.close();
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }
      int index = ui.tabWidgetIcon->currentIndex();
      for(int n=0; n<fileRenamed.count(); n++)
      {
          QString strOriginal = fileRenamed.at(n) + ")";
          QString strNew = newNameOfFileRenamed.at(n) + ")";
          configdata.replace(strOriginal, strNew.toLatin1());
      }
      IconeAreaList.at(index)->appendconfigfile(configdata);
      zip.close();
  }
}



void logisdom::importPage()
{
    bool ok;
    QString getFileName;
    getFileName = QFileDialog::getOpenFileName(this, tr("Select cfg file "), "", "zip (*.zip)");
    if (getFileName.isEmpty()) return;
    QFile file (getFileName);
    QFileInfo fileInfo(file);
    bool found = true;
    QString tabName = fileInfo.baseName();
    while (found)
    {
        found = false;
        for (int n=0; n<IconeAreaList.count(); n++) if (ui.tabWidgetIcon->tabText(n) == fileInfo.baseName()) found = true;
        if (found)
        {
            QString nom = inputDialog::getTextPalette(this, tr("Name already exist"), tr("New tab name :"), QLineEdit::Normal, "", &ok, this);
            if (!ok) return;
            if (nom.isEmpty()) return;
            found = false;
            for (int n=0; n<IconeAreaList.count(); n++) if (ui.tabWidgetIcon->tabText(n) == nom) found = true;
            tabName = nom;
        }
    }
    QuaZip zip(getFileName);
    if(zip.open(QuaZip::mdUnzip))
    {
        //QTextCodec *codec = zip.getFileNameCodec();
        //zip.setFileNameCodec(codec);
        QuaZipFileInfo info;
        QuaZipFile zipFile(&zip);
        QString name;
        //QByteArray configdata;
        QString configdata;
        QStringList fileRenamed, newNameOfFileRenamed;
        for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
        {
            if (zip.getCurrentFileInfo(&info))
            {
                if(zipFile.getZipError() == UNZ_OK)
                {
                    name = zipFile.getActualFileName();
                    if(zipFile.open(QIODevice::ReadOnly))
                    {
                        if (name.endsWith(".cfg"))
                        {
                            configdata.append(zipFile.readAll());
                            zipFile.close();
                        }
                        if (name.endsWith(".png"))
                        {
                            QDir localdirectory = QDir("");
                            if (!localdirectory.exists(repertoireicon))
                                if (!localdirectory.mkdir(repertoireicon)) return;
                            QFile iconFile(QString(repertoireicon) + QDir::separator() + name);
                            if (!fileRenamed.contains(name, Qt::CaseSensitive))
 // check if file name already exists
 // if already exists, check size, if size is equal use exiting file
 // if size is not equal, rename imported file with _%1
                            {
                                if (iconFile.exists())
                                {

                                    int index = 1;
                                    QString nameAleradyThere = name;
                                    nameAleradyThere.chop(4);
                                    QString newFileName = nameAleradyThere + QString("_%1.png").arg(index);
                                    QString newFilePath = QString(repertoireicon) + QDir::separator() + newFileName;
                                    QFile newFile(newFilePath);
                                    while(newFile.exists())
                                    {
                                        index ++;
                                        newFileName = nameAleradyThere + QString("_%1.png").arg(index);
                                        newFilePath = QString(repertoireicon) + QDir::separator() + newFileName;
                                        newFile.setFileName(newFilePath);
                                    }
                                    if (newFile.open(QIODevice::WriteOnly))
                                    {
                                        newFile.write(zipFile.readAll());
                                        newFile.close();
                                        zipFile.close();
                                    }
                                    //qDebug() << QString("newFileSize = %1").arg(newFile.size());
                                    //qDebug() << QString("iconFile = %1").arg(iconFile.size());
                                    if (newFile.size() == iconFile.size())
                                    {
                                        //qDebug() << newFile.fileName() + " Same size";
                                        newFile.remove();
                                        //qDebug() << "File removed";
                                    }
                                    else
                                    {
                                        fileRenamed.append(name);
                                        newNameOfFileRenamed.append(newFileName);
                                        //qDebug() << newFile.fileName() + " not same size";
                                    }
                                }
                                else
                                {
                                    if (iconFile.open(QIODevice::WriteOnly))
                                    {
                                        iconFile.write(zipFile.readAll());
                                        iconFile.close();
                                        zipFile.close();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        IconeArea *iconArea = new IconeArea(this);
        srcoolCatchArrow *scroll = new srcoolCatchArrow(iconArea);
        scroll->setParent(ui.tabWidgetIcon);
        scroll->setWidget(iconArea);
        ui.tabWidgetIcon->addTab(scroll, tabName);
        iconArea->setGeometry(0, 0, workspaceX.value(), workspaceY.value());
        for(int n=0; n<fileRenamed.count(); n++)
        {
            QString strOriginal = fileRenamed.at(n) + ")";
            QString strNew = newNameOfFileRenamed.at(n) + ")";
            configdata.replace(strOriginal, strNew.toLatin1());
        }
        iconArea->readconfigfile(configdata.toLatin1());
        IconeAreaList.append(iconArea);
        htmlBind->setParameterLink(name, name + ".png");
        ui.tabWidgetIcon->setCurrentIndex(ui.tabWidgetIcon->count() - 1);
        zip.close();
    }
}



void logisdom::addIconTab()
{
	bool ok;
Retry :
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, this);
	if (!ok) return;
	if (nom.isEmpty()) return;
	for (int i=0; i<ui.tabWidgetIcon->count(); i++)
		if (ui.tabWidgetIcon->tabText(i) == nom)
		{
            messageBox::warningHide(this, "LogisDom", cstr::toStr(cstr::AlreadyExixts), this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto Retry;
		}
    IconeArea *iconArea = new IconeArea(this);
    srcoolCatchArrow *scroll = new srcoolCatchArrow(iconArea);
    scroll->setParent(ui.tabWidgetIcon);
    scroll->setWidget(iconArea);
    //scroll->setWidget(iconArea);
    ui.tabWidgetIcon->addTab(scroll, nom);
    iconArea->setGeometry(0, 0, workspaceX.value(), workspaceY.value());
    IconeAreaList.append(iconArea);
    ui.tabWidgetIcon->setCurrentIndex(ui.tabWidgetIcon->count()-1);
    htmlBind->setParameterLink(nom, nom + ".png");
}




QString logisdom::getTabName(int tab)
{
    if (tab < 0) return "";
    if (tab < ui.tabWidgetIcon->count()) return ui.tabWidgetIcon->tabText(tab);
    return "";
}



void logisdom::loadRequest(onewiredevice* device)
{
    if (deviceLoadRequest.contains(device)) return;
    if (deviceLoadRequest.isEmpty())
    {
        deviceLoadRequest.append(device);
        device->startLoading();
    }
    else
    {
        deviceLoadRequest.append(device);
    }
}




void logisdom::finishedDataLoading(onewiredevice *device)
{
    if (deviceLoadRequest.contains(device)) deviceLoadRequest.removeAll(device);
    if (deviceLoadRequest.isEmpty()) return;
    deviceLoadRequest.first()->startLoading();
}




void logisdom::removeIconTab()
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
        if ((messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to remove ") + "\n" + ui.tabWidgetIcon->tabText(index), this, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
	ui.tabWidgetIcon->removeTab(index);
	delete IconeAreaList.takeAt(index);
}


bool logisdom::isLocked()
{
    return IconTabisLocked;
}


void logisdom::renameIconTab()
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	bool ok;
    QString text = inputDialog::getTextPalette(this, tr("Rename"), "", QLineEdit::Normal, ui.tabWidgetIcon->tabText(index), &ok, this);
	if (ok && !text.isEmpty()) 
	{
		htmlBind->removeParameter(ui.tabWidgetIcon->tabText(index));
		ui.tabWidgetIcon->setTabText(index, text);
		htmlBind->setParameterLink(text, text + ".png");
	}
}




void logisdom::swapLock(bool)
{
    Lock(!IconTabisLocked);
}



void logisdom::Lock(bool state)
{
	IconTabisLocked = state;
	for (int n=0; n<IconeAreaList.count(); n++)
		IconeAreaList[n]->setLockedState(IconTabisLocked);
    if (SwitchArea) SwitchArea->Lock(state);
	if (IconTabisLocked)
	{
		lockAction->setIcon(lockedIcon);
		lockAction->setToolTip(tr("Locked"));
	}
	else
	{
		lockAction->setIcon(unlockedIcon);
		lockAction->setToolTip(tr("Unlocked"));
	}
    configwin->Lock(state);
    foreach (LogisDomInterface *interface, logisdomInterfaces) { interface->setLockedState(state); }
}




void logisdom::lockClicked(QMouseEvent*)
{
    Lock(!IconTabisLocked);
}



void logisdom::setWindowSize()
{
    int width = workspaceX.value();
    int height = workspaceY.value();
    QSize newSize(width, height);
    QRect rect = geometry();
    rect.setSize(QSize(width + 120, height + 110));
    setGeometry(rect);
}



void logisdom::showresize()
{
	resize.show();
}



void logisdom::workSpaceResizeChanged(int)
{
	for (int n=0; n<IconeAreaList.count(); n++)
		IconeAreaList[n]->setGeometry(0, 0, workspaceX.value(), workspaceY.value());
    setWindowSize();
}




void logisdom::iconAlignLeft(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconAlignLeft();
}



void logisdom::iconAlignRight(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconAlignRight();
}


void logisdom::iconAlignTop(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconAlignTop();
}



void logisdom::iconAlignBottom(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconAlignBottom();
}


void logisdom::iconCopysize(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconCopysize();
}



void logisdom::iconCopy(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconCopy(strCopyIcon);
}



void logisdom::iconPaste(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconPaste(strCopyIcon);
}



void logisdom::iconUndo(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconUndo();
}

void logisdom::iconRedo(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconRedo();
}



void logisdom::iconAddOne(bool)
{
	int index = ui.tabWidgetIcon->currentIndex();
	if (index < 0) return;
	IconeAreaList.at(index)->iconAddOne();
}



void logisdom::newPluginDevice(LogisDomInterface *interface, QString RomID)
{
    onewiredevice *device = configwin->DeviceExist(RomID);
    if (!device) device = configwin->NewPluginDevice(RomID, interface);
    showPalette();
    if (device) setPalette(&device->setup);
}


void logisdom::pluginDeviceSelected(QString RomID)
{
    //qDebug() << RomID;
    onewiredevice *device = configwin->DeviceExist(RomID);
    if (device) setPalette(&device->setup);
}



void logisdom::pluginUpdateNames(LogisDomInterface* interface, QString str)
{
    int tab = configwin->ui.tabWidget->indexOf(interface->widgetUi());
    if (tab == -1) return;
    configwin->ui.tabWidget->setTabText(tab, str);
}


void logisdom::newPluginDeviceValue(QString RomID, QString Value)
{
    onewiredevice *device = configwin->DeviceExist(RomID);
    bool ok;
    double V = Value.toDouble(&ok);
    if (device) if (ok) device->setPluginMainValue(V);
}


void logisdom::MainTreeClick(const QPoint &pos)
{
	QStringList path;
	QString str;
    //bool link = false;
	QTreeWidgetItem *item = mainTreeHtml->currentItem();
	if (!item) return;
	if (item->childCount() == 0)
	{
		str = htmlsperarator + item->text(1);
        //link = true;
	}
	else
	{
		str = htmlsperarator + item->text(0);
	}
	QTreeWidgetItem *parent = item->parent();
	if (parent == configwin->htmlBindWebMenu->treeItem)
	{
		QString port = "80";
		if (configwin->server) port = QString("%1").arg(configwin->ui.spinBoxPortServer->value());
		if (str.mid(0, 1) == htmlsperarator) str = str.remove(0, 1);
		path.append("http://localhost:" + port + "/request=(" + str + ")user=()password=()");
		path.append("http://localhost:" + port + "/" + str);
	}
	else
	{
		QTreeWidgetItem *topItem = mainTreeHtml->topLevelItem(0);
        int index = 0;
		while (parent && (item != topItem))
		{
			item = parent;
			parent = item->parent();
            if (!str.isEmpty() && (item->text(0) != "Mode"))
                str = htmlsperarator + item->text(0) + str;
            index ++;
		}
		if (str.mid(0, 1) == htmlsperarator) str = str.remove(0, 1);
		path.append(str);
        //path.append("window.location='command=(" + str + ")webid=()'");
	}
	if (path.count() == 0) return;
	QMenu contextualmenu;
	QAction *action[path.count()];
	for (int n=0; n<path.count(); n++)
	{
		action[n] = new QAction(path.at(n), this);
		contextualmenu.addAction(action[n]);
	}
	QAction *selection;
	selection = contextualmenu.exec(mainTreeHtml->mapToGlobal(pos));
	for (int n=0; n<path.count(); n++)
	{
		if (selection == action[n])
		{
			QClipboard *clipboard = QApplication::clipboard();
			clipboard->setText(path.at(n));
		}
		action[n] = new QAction(path.at(n), this);
		contextualmenu.addAction(action[n]);
	}
	for (int n=0; n<path.count(); n++) delete action[n];
}




void logisdom::resizeChanged(int size)
{
	if (resizeAllTab.isChecked())
	{
		for (int n=0; n<IconeAreaList.count(); n++)
			IconeAreaList[n]->resize(size, moveLabels.isChecked());
	}
	else
	{
		int index = ui.tabWidgetIcon->currentIndex();
		if (index < 0) return;
        IconeAreaList.at(index)->resize(size, moveLabels.isChecked());
	}
}



void logisdom::showenergiesolaire()
{
    changePalette(EnergieSolaire);
}



void logisdom::showaddProgram()
{
    changePalette(&AddProgwin->setup);
}



void logisdom::AddIcones()
{
	ui.tabWidget->setCurrentIndex(0);
	if (myicons->isHidden())
	{
		myicons->show();
        myicons->piecesList->clear();
        myicons->piecesList->reload();
	}
	else myicons->hide();
}



void logisdom::AddDailyPrg()
{
    changePalette(&AddDaily->setup);
    setTabWidget(1);
    if (ui.tabWidgetPrg->currentIndex() != 2)
    {
        disconnect(ui.tabWidgetPrg, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
        ui.tabWidgetPrg->setCurrentIndex(2);
        connect(ui.tabWidgetPrg, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    }
}


void logisdom::tabChanged(int index)
{
    if (index == 2) changePalette(&graphconfigwin->setup);
    if (index == 3) changePalette(&MeteoArea->setup);
    if (index == 5) changePalette(&tableauConfig->setup);
    if (index == 1) tabPrgChanged(ui.tabWidgetPrg->currentIndex());
}



void logisdom::tabPrgChanged(int index)
{
    if (index == 0) changePalette(&SwitchArea->setup);
    if (index == 1) changePalette(&ChauffageArea->setup);
    if (index == 2) changePalette(&AddDaily->setup);
}

void logisdom::tabGraphChanged(int index)
{
    setPalette(&graphconfigwin->graphPtArray.at(index)->setup);
}


void logisdom::showgraphconfig()
{
    changePalette(&graphconfigwin->setup);
    setTabWidget(2);
}




void logisdom::showswitch()
{
    changePalette(&SwitchArea->setup);
    setTabWidget(1);
    disconnect(ui.tabWidgetPrg, SIGNAL(currentChanged(int)), this, SLOT(tabPrgChanged(int)));
    ui.tabWidgetPrg->setCurrentIndex(0);
    connect(ui.tabWidgetPrg, SIGNAL(currentChanged(int)), this, SLOT(tabPrgChanged(int)));
}



void logisdom::showdevices()
{
    changePalette(&configwin->setup);
}



void logisdom::setTabWidget(int tab)
{
    if (configwin->ui.checkBoxHideHeating->isChecked())
    {
        tab--;
        if (tab == 0) return;
    }
    if (ui.tabWidget->currentIndex() != tab)
    {
        disconnect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
        ui.tabWidget->setCurrentIndex(tab);
        connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    }
}


void logisdom::showtableauconfig()
{
    changePalette(&tableauConfig->setup);
    setTabWidget(5);
}



void logisdom::saveconfig()
{
	saveconfig(configfilename);
}



void logisdom::saveconfigtxt()
{
    NoHex = true;
    QString selFilter="All files (*.*)";
    QString fileName = QFileDialog::getSaveFileName(this,"Save file",QDir::currentPath(), "Text files (*.txt);;All files (*.*)",&selFilter);
    if (!fileName.isEmpty()) saveconfig(fileName);
    NoHex = false;
}



void logisdom::saveDat(QString fileName, QString data)
{
    fileSaver.appendData(fileName, data);
    fileSaver.repertoiredat = repertoiredat;
    if (!fileSaver.isRunning()) fileSaver.start();
}


void logisdom::saveconfig(QString fileName)
{
	QFile file(fileName);
    //QByteArray configdata;
    QString configdata;
// Read original config file to get device config
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		GenMsg("cannot read configuration file  " + fileName);
	}
	else
	{
		QTextStream in(&file);
		while (!in.atEnd())
		{
            //QString line = in.readLine();
            configdata.append(in.readLine() + "\n");
		}
		file.close();
	}
	QStringList devRomID, devConfig;
	QString ReadRomID;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
		ReadRomID = getvalue("RomID", strsearch);
		if ((!ReadRomID.isEmpty()) && (devRomID.indexOf(ReadRomID) == -1))
		{
			devRomID.append(ReadRomID);
			devConfig.append(strsearch);
		}
	SearchLoopEnd
	for (int n=0; n<configwin->devicePtArray.count(); n++)
	{
		int found = devRomID.indexOf(configwin->devicePtArray[n]->getromid());
		if (found != -1)
		{
			devRomID.removeAt(found);
			devConfig.removeAt(found);
		}
	}
// Rename file with .bak_xxx extension
	QFile renFile(fileName);
	if (renFile.exists())
	{
	    bool found = true;
	    int n = 1;
        while ((found) && (n<100))
	    {
            QString backupFileName = fileName + QString(".backup_%1").arg(n);
            QFile backupFile(backupFileName);
            if (!backupFile.exists())
            {
                renFile.rename(backupFileName);
                found = false;
            }
            n ++;
	    }
	    if (n > 99)
	    {
            QFile firstFile(fileName + ".backup_1");
            firstFile.remove();
            for (int n=2; n<100; n++)
            {
                QFile nextFile(fileName + QString(".backup_%1").arg(n));
                nextFile.rename(fileName + QString(".backup_%1").arg(n-1));
            }
            QString backupFileName = fileName + ".backup_99";
            QFile backupFile(backupFileName);
            if (!backupFile.exists()) renFile.rename(backupFileName);
	    }
	}
// Open and write file
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		GenError(47, fileName);
		return;
	}
	QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
	QString str;
    QDateTime now = QDateTime::currentDateTime();
    str += "Configuration file " + now.toString() + "\n";
// Original Device Config
    for (int n=0; n<devRomID.count(); n++)
        str += devConfig[n] + "\n" + EndMark "\n";
    getSaveStr(str);
	out << str;
    file.close();
}




void logisdom::getSaveStr(QString &str)
{

// Main window
    SaveConfigStr(str);

// Icon Area
    SaveIconConfigStr(str);

// Meteo
    MeteoArea->SaveConfigStr(str);

// Chauffage
    ChauffageArea->SaveConfigStr(str);

// Solaire
    EnergieSolaire->SaveConfigStr(str);

// Daliy Programs
    AddDaily->SaveConfigStr(str);

// Remote
    SaveRemoteStatusStr(str);

// Graphiques
    if (graphconfigwin) graphconfigwin->SaveConfigStr(str);

// Tableau
    if (tableauConfig) tableauConfig->SaveConfigStr(str);

// Program Events
    if (ProgEventArea) ProgEventArea->SaveConfigStr(str);

// Net 1 Wire Devices / Config
    if (configwin) configwin->SaveConfigStr(str);

// Weekly Programs
    if (AddProgwin) AddProgwin->SaveConfigStr(str);

// Switches
    if (SwitchArea) SwitchArea->SaveConfigStr(str);

// One Wire Devices
    if (configwin) configwin->GetDevicesStr(str);

// Plugins
    foreach (LogisDomInterface *interface, logisdomInterfaces)
        interface->saveConfig();
}




void  logisdom::SaveConfigStr(QString &str)
{
int x = 0, y = 0;
		str += "\n" Main_Window "\n";
// Main Window
		str += saveformat("Name", windowTitle());
        x = geometry().x();
        y = geometry().y();
		str += saveformat("Window_X", QString("%1").arg(x));
		str += saveformat("Window_Y", QString("%1").arg(y));
		str += saveformat(Window_Width_Mark, QString("%1").arg(width()));
		str += saveformat(Window_Height_Mark, QString("%1").arg(height()));
		str += saveformat("Window_Hidden", QString("%1").arg(isHidden()));
// WorkSpace (icon)
		x = workspaceX.value();
		y = workspaceY.value();
		str += saveformat("WorkSpace_X", QString("%1").arg(x));
		str += saveformat("WorkSpace_Y", QString("%1").arg(y));
// Palette
        str += saveformat("Palette_X", QString("%1").arg(palette.geometry().x()));
        str += saveformat("Palette_Y", QString("%1").arg(palette.geometry().y()));
        str += saveformat("Palette_Width", QString("%1").arg(palette.geometry().width()));
        str += saveformat("Palette_Height", QString("%1").arg(palette.geometry().height()));
		str += saveformat("Palette_Hidden", QString("%1").arg(palette.isHidden()));
        str += saveformat("paletteOnTop", QString("%1").arg(paletteOnTop));
// Config
		x = configwin->geometry().x();
		y = configwin->geometry().y();
		str += saveformat("Config_X", QString("%1").arg(x));
		str += saveformat("Config_Y", QString("%1").arg(y));
		str += saveformat("Config_Width", QString("%1").arg(configwin->width()));
		str += saveformat("Config_Height", QString("%1").arg(configwin->height()));
// Icon
		x = myicons->geometry().x();
		y = myicons->geometry().y();
		str += saveformat("Icon_X", QString("%1").arg(x));
		str += saveformat("Icon_Y", QString("%1").arg(y));
		str += saveformat("Icon_Width", QString("%1").arg(myicons->width()));
		str += saveformat("Icon_Height", QString("%1").arg(myicons->height()));
// currentTabs
		if (ui.tabWidget->count() > 0) str += saveformat("currentTab", QString("%1").arg(ui.tabWidget->currentIndex()));
		if (ui.tabWidgetIcon->count() > 0) str += saveformat("currentIconTab", QString("%1").arg(ui.tabWidgetIcon->currentIndex()));
		if (ui.tabWidgetPrg->count() > 0) str += saveformat("currentPrgTab", QString("%1").arg(ui.tabWidgetPrg->currentIndex()));
		if (ui.tabWidgetGraphic->count() > 0) str += saveformat("currentGraphicTab", QString("%1").arg(ui.tabWidgetGraphic->currentIndex()));
// LogTab
		alarmwindow->GeneralErrorLog->getCfgStr(str);
// Tree Column width
		str += saveformat("TreeColumnWidth1", QString("%1").arg(mainTreeHtml->columnWidth(0)));
		str += saveformat("TreeColumnWidth2", QString("%1").arg(mainTreeHtml->columnWidth(1)));
		str += saveformat("TreeColumnWidth3", QString("%1").arg(mainTreeHtml->columnWidth(2)));
// Palette tree devices checkbox
        str += saveformat("DeviceTreeCheckBox", QString("%1").arg(configwin->checkBoxTree.isChecked()));
        str += EndMark;
        str += "\n";
}




void logisdom::SaveRemoteStatusStr(QString &str)
{
if (isRemoteMode()) 
		{
 		str += "\n"  Remote_Mode  "\n";
		str += saveformat("IPaddress", RemoteConnection->getipaddress());
		str += saveformat("Port", QString("%1").arg(RemoteConnection->getport()));
		str += saveformat("userName", RemoteConnection->getUserName());
		str += saveformat("passWord", RemoteConnection->getPassWord());
		if (RemoteConnection->logenabled()) str += saveformat("Log", "1"); else str += saveformat("Log", "0");
		str += EndMark;
		str += "\n";
	}
	else
	{
 		str += "\n"  Server_Mode  "\n";
                str += saveformat(Server_Port, QString("%1").arg(configwin->ui.spinBoxPortServer->value()));
		if (configwin->serverIsListening())
				str += saveformat("Server_Listening", "1");
			else 
				str += saveformat("Server_Listening", "0");
		str += EndMark;
		str += "\n";
	}
}




void  logisdom::SaveIconConfigStr(QString &str)
{
	int count = ui.tabWidgetIcon->count();
	for (int n=0; n<count; n++)
	{
		str += "\n" Icon_Tab_Begin "\n";
		str += saveformat("Tab_Name", ui.tabWidgetIcon->tabText(n));
		if (IconTabisLocked) str += saveformat("Locked", "1");
			else str += saveformat("Locked", "0");
		IconeAreaList[n]->SaveConfigStr(str);
        str += End_Icon_Tab;
        str += "\n";
	}
}






void logisdom::readconfigfile(const QString &configdata)
{
    QScreen *screen = QApplication::screens().at(0);
    int screenWidth = screen->availableSize().width();
    int screenHeight = screen->availableSize().height();
    QString ReadRomID;
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0;
	QString TAG_Begin = Main_Window;
	QString TAG_End = EndMark;
	SearchLoopBegin	
	x = getvalue("Window_X", strsearch).toInt(&ok_x, 10);
	y = getvalue("Window_Y", strsearch).toInt(&ok_y, 10);
	w = getvalue(Window_Width_Mark, strsearch).toInt(&ok_w, 10);
	h = getvalue(Window_Height_Mark, strsearch).toInt(&ok_h, 10);
//	if (x > Xmax) x = 100;
//	if (y > Ymax) y = 100;
	if ((ok_x + ok_y + ok_w + ok_h) != 0) setGeometry(x, y, w, h);
	h = getvalue("Window_Hidden", strsearch).toInt(&ok_h, 10);
	if (ok_h && h) hide(); else show();
// WorkSpace
	x = getvalue("WorkSpace_X", strsearch).toInt(&ok_x, 10);
	y = getvalue("WorkSpace_Y", strsearch).toInt(&ok_y, 10);
	if (ok_x) workspaceX.setValue(x);
	if (ok_y) workspaceY.setValue(y);
	//if (iconArea) if (ok_x && ok_y) iconArea->setGeometry(0, 0, x, y);
// Palette
	x = getvalue("Palette_X", strsearch).toInt(&ok_x, 10);
	y = getvalue("Palette_Y", strsearch).toInt(&ok_y, 10);
	w = getvalue("Palette_Width", strsearch).toInt(&ok_w, 10);
	h = getvalue("Palette_Height", strsearch).toInt(&ok_h, 10);
    if (ok_x && ok_y && ok_w && ok_h) palette.setGeometry(x, y, w, h);

	h = getvalue("Palette_Hidden", strsearch).toInt(&ok_h, 10);
	if (ok_h && h) palette.hide(); else palette.show();
    h = getvalue("paletteOnTop", strsearch).toInt(&ok_h, 10);
    Qt::WindowFlags flags = palette.windowFlags();
    disconnect(&alwaysOnTop, SIGNAL(toggled(bool)), this, SLOT(onAllwaysTop(bool)));
    if (ok_h && h)
    {
        paletteOnTop = true;
        flags |= Qt::WindowStaysOnTopHint;
        alwaysOnTop.setChecked(true);
    }
    else
    {
        paletteOnTop = false;
        flags &= ~Qt::WindowStaysOnTopHint;
        alwaysOnTop.setChecked(false);
    }
    palette.setWindowFlags(flags);
    connect(&alwaysOnTop, SIGNAL(toggled(bool)), this, SLOT(onAllwaysTop(bool)));
// Config
	x = getvalue("Config_X", strsearch).toInt(&ok_x, 10);
	y = getvalue("Config_Y", strsearch).toInt(&ok_y, 10);
	w = getvalue("Config_Width", strsearch).toInt(&ok_w, 10);
	h = getvalue("Config_Height", strsearch).toInt(&ok_h, 10);
    if (x > screenWidth) x = 20;
    if (y > screenHeight) y = 20;
	if ((ok_x + ok_y + ok_w + ok_h) != 0) configwin->setGeometry(x, y, w, h);
// Icon
	x = getvalue("Icon_X", strsearch).toInt(&ok_x, 10);
	y = getvalue("Icon_Y", strsearch).toInt(&ok_y, 10);
	w = getvalue("Icon_Width", strsearch).toInt(&ok_w, 10);
	h = getvalue("Icon_Height", strsearch).toInt(&ok_h, 10);
    if (x > screenWidth) x = 100;
    if (y > screenHeight) y = 100;
	if ((ok_x + ok_y + ok_w + ok_h) != 0) myicons->setGeometry(x, y, w, h);
// LogTab
	alarmwindow->GeneralErrorLog->setCfgStr(strsearch);
// Tree clumn width
	x = getvalue("TreeColumnWidth1", strsearch).toInt(&ok_x);
	if ((ok_x) && (x > 20)) mainTreeHtml->setColumnWidth(0, x);
	x = getvalue("TreeColumnWidth2", strsearch).toInt(&ok_x);
	if ((ok_x) && (x > 20)) mainTreeHtml->setColumnWidth(1, x);
	x = getvalue("TreeColumnWidth2", strsearch).toInt(&ok_x);
	if ((ok_x) && (x > 20)) mainTreeHtml->setColumnWidth(2, x);
    // Palette tree devices checkbox
    bool ok;
    bool DC = logisdom::getvalue("DeviceTreeCheckBox", strsearch).toInt(&ok);
    if (ok) configwin->checkBoxTree.setChecked(DC);
    SearchLoopEnd
}





void logisdom::setTabs(const QString &configdata)
{
	int t;
	bool ok;
	QString TAG_Begin = Main_Window;
	QString TAG_End = EndMark;
	SearchLoopBegin	
	t = getvalue("currentTab", strsearch).toInt(&ok, 10);
	if (ok) ui.tabWidget->setCurrentIndex(t);
	t = getvalue("currentIconTab", strsearch).toInt(&ok, 10);
	if (ok) ui.tabWidgetIcon->setCurrentIndex(t);
	t = getvalue("currentPrgTab", strsearch).toInt(&ok, 10);
	if (ok) ui.tabWidgetPrg->setCurrentIndex(t);
	t = getvalue("currentGraphicTab", strsearch).toInt(&ok, 10);
	if (ok) ui.tabWidgetGraphic->setCurrentIndex(t);
	SearchLoopEnd
}





void logisdom::readIconTabconfigfile(const QString &configdata)
{
	bool ok;
	QString ReadRomID, ReadName;
	QString TAG_Begin = Icon_Tab_Begin;
	QString TAG_End = End_Icon_Tab;
	SearchLoopBegin
		if (!strsearch.isEmpty())
		{
            QString data;
			data.append(strsearch);
			QString name = getvalue("Tab_Name", strsearch);
			int Locked = getvalue("Locked", strsearch).toInt(&ok);
			IconeArea *iconArea = new IconeArea(this);
            srcoolCatchArrow *scroll = new srcoolCatchArrow(iconArea);
            scroll->setParent(ui.tabWidgetIcon);
            scroll->setWidget(iconArea);
			scroll->setWidget(iconArea);
			ui.tabWidgetIcon->addTab(scroll, name);
			iconArea->setGeometry(0, 0, workspaceX.value(), workspaceY.value());
            iconArea->readconfigfile(data.toLatin1());
			IconeAreaList.append(iconArea);
            if (ok and Locked) Lock(true);
			htmlBind->setParameterLink(name, name + ".png");
		}
	SearchLoopEnd

    if (IconeAreaList.count() == 0)
    {
        QString name = tr("New Tab");
        IconeArea *iconArea = new IconeArea(this);
        srcoolCatchArrow *scroll = new srcoolCatchArrow(iconArea);
        scroll->setParent(ui.tabWidgetIcon);
        scroll->setWidget(iconArea);
        scroll->setWidget(iconArea);
        ui.tabWidgetIcon->addTab(scroll, name);
        iconArea->setGeometry(0, 0, workspaceX.value(), workspaceY.value());
        IconeAreaList.append(iconArea);
        htmlBind->setParameterLink(name, name + ".png");
    }
}





void logisdom::showTabWidget(int)
{
}






void logisdom::update(bool force)
{
	if (configwin->isUploading()) return;
	timerUpdate.stop();
	QDateTime now = QDateTime::currentDateTime();
    if (lastminute != now.time().minute() || force)
	{
		switch (indexupdate)
		{
			case 0 :
                GenMsg(QString("checkTimeMatch"));
                ProgEventArea->checkTimeMatch();
				indexupdate ++;
				break;
			case 1 :
                GenMsg(QString("trier"));
                ProgEventArea->trier();
                GenMsg(QString("Convert"));
                configwin->convert();
				indexupdate ++;
				break;
			case 2 :
                if (configwin->ui.LectureRecBox->isChecked()) {
				GenMsg(QString("LectureRecAll"));
                configwin->LectureRecAll(); }
                indexupdate ++;
				break;
			case 3 :
            if (configwin->ui.IconBox->isChecked()) {
                GenMsg(QString("IconeAreaList"));
                for (int n=0; n<IconeAreaList.count(); n++)
				{
					IconeAreaList.at(n)->setValue();
					iconFileCheck = false;
					if (iconFileCheck) IconeAreaList.at(n)->checkFile(iconFileCheck);
                } }
				indexupdate ++;
				break;
			case 4 :
            if (configwin->ui.MeteoBox->isChecked()) {
                GenMsg(QString("Meteo"));
                MeteoArea->timerupdate(); }
				indexupdate ++;
				break;
			case 5 :
            if (configwin->ui.Chauffagebox->isChecked()) {
                GenMsg(QString("ChauffageArea->process()"));
                ChauffageArea->process(); }
				indexupdate ++;
				break;
			case 6 :
            if (configwin->ui.VirtualDeviceBox->isChecked()) {
                GenMsg(QString("configwin->LectureRecVD()"));
                configwin->LectureRecVD(); }
				indexupdate ++;
				break;
			case 7 :
            if (configwin->ui.SendAllBox->isChecked()) {
                GenMsg(QString("configwin->server->sendAll()"));
                configwin->server->sendAll(); }
				indexupdate ++;
				break;
			case 8 :
				//GenMsg(QString("graphconfigwin->updateGraphs()"));
				//if (graphconfigwin) graphconfigwin->updateGraphs();
                if (graphconfigwin) graphconfigwin->raz_counter();
				indexupdate ++;
				break;
			case 9 :
            if (configwin->ui.SaveTableauBox->isChecked()) {
                GenMsg(QString("tableauConfig->saveTableau()"));
                if (tableauConfig) tableauConfig->saveTableau(); }
				indexupdate ++;
				break;
			case 10 :
            if (configwin->ui.ZipFileBox->isChecked()) {
                GenMsg(QString("checkDatZipFile"));
                configwin->checkDatZipFile(); }
				indexupdate ++;
				break;
			case 11 :
            if (configwin->ui.NextSaveBox->isChecked()) {
                GenMsg(QString("saveNextSave"));
                configwin->setNextSave(); }
				indexupdate ++;
				break;
			case 12 :
				indexupdate ++;
				break;
			case 13 :
			default :
				lastminute = now.time().minute();
                GenMsg(QString("Update finished"));
				indexupdate = 0;
                mainStatus();
				break;
		}
		timerUpdate.start(200);
		QCoreApplication::processEvents(QEventLoop::AllEvents);
	}
    else
	{
		if (configwin) configwin->generatePng();
        if (graphconfigwin)
        {
            if (configwin->ui.GraphixBox->isChecked())
                graphconfigwin->updateGraphs();
        }
		timerUpdate.start(1000);
	}
	if (configwin->ui.checkBoxkSaveInterval->isChecked())
	{
		saveInterval ++;
		if ((saveInterval/60) >= configwin->ui.spinBoxSaveInterval->value())
		{
			saveInterval = 0;
			saveconfig(configfilename);
		}
	}
}



void logisdom::mainStatus()
{
	bool allValid = true;
	QString txt;
	for (int n=0; n<configwin->devicePtArray.count(); n++)
		if (!configwin->devicePtArray[n]->isValid())
		{
			if (!txt.isEmpty()) txt += "\n";
			txt += configwin->devicePtArray[n]->getname() + " " + tr("not valid");
			allValid = false;
		}
	if (allValid)
	{
		statusAction->setIcon(greenDotIcon);
		statusAction->setToolTip(tr("All ok"));
	}
	else
	{
		statusAction->setIcon(redDotIcon);
		statusAction->setToolTip(txt);
	}
}



QString logisdom::getrepertoiredat()
{
	return repertoiredat;
}




QString logisdom::getrepertoirezip()
{
	return repertoirezip;
}



QString logisdom::getrepertoirehtml()
{
	return repertoirehtml;
}



/*void logisdom::UnCompressDataFile()
{
	bool deletefiles = false;
	QStringList files = QFileDialog::getOpenFileNames( this, tr("Select one or more files to uncompress"), repertoiredat, "zat (*.zat)");
	if (files.isEmpty()) return;
	if ((messageBox::questionHide(this, tr("Delete files ?"), tr("Do you want to delete original files ?"), this, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)) deletefiles = true;;
	QStringList::Iterator it = files.begin();
	while(it != files.end())
	{
		UnCompressFile(*it, deletefiles);
		++it;
	}
}*/






void logisdom::CompressFile(const QString &filename, bool deletefile)
{
	QFile file(filename);
	if ((file.open(QIODevice::ReadOnly)) && (filename.right(4) == dat_ext))
	{
		QByteArray data = file.readAll();
		QByteArray cmpdata = qCompress(data);
		int L = filename.length();
		QString zipname = filename.mid(0, L-4) + compdat_ext;
		QFile zfile(zipname);
		if(zfile.open(QIODevice::WriteOnly))
		{
			zfile.write(cmpdata);
			zfile.close();
			if (deletefile) file.remove();
		}
	}
}





void logisdom::ZipFile(const QString &filename)
{
	QFile file(filename);
	QFileInfo fileInfo(filename);
	if ((file.open(QIODevice::ReadOnly)) && ((filename.right(4) == compdat_ext) or ((filename.right(4) == dat_ext))))
	{
		QByteArray cmpdata = file.readAll();
		file.close();
		QByteArray data;
		QString name = fileInfo.fileName();
		if (name.right(4) == compdat_ext) data.append(qUncompress(cmpdata));
		if (name.right(4) == dat_ext) data.append(cmpdata);
		int posUnderScore = name.indexOf("_");
		if (posUnderScore > 0)
		{
			QString romid = name.left(posUnderScore);
			name = fileInfo.fileName();
			name.chop(4);
			name += dat_ext;
			QString year = name.right(8).left(4);
			QString zipFileName = repertoirezip + QDir::separator() + romid + "_" + year + ".zip";
			QuaZip zipFile(zipFileName);
			if (!zipFile.open(QuaZip::mdAdd)) zipFile.open(QuaZip::mdCreate);
			if (zipFile.getZipError() == UNZ_OK)
			{
				QuaZipFile outZipFile(&zipFile);
                if(outZipFile.open(openModeWrite, QuaZipNewInfo(name, name)))
                {
					outZipFile.write(data);
					if(outZipFile.getZipError() != UNZ_OK) GenMsg("Cannot create Zip file " + zipFileName);
					outZipFile.close();
					file.remove();
				}
			}
		}
	}
}





void logisdom::UnCompressFile(const QString &filename, bool deletefile)
{
	QFile file(filename);
	if ((file.open(QIODevice::ReadOnly)) && (filename.right(4) == compdat_ext))
	{
		QByteArray cmpdata = file.readAll();
		QByteArray data = qUncompress(cmpdata);
		int L = filename.length();
		QString datname = filename.mid(0, L-4) + dat_ext;
		QFile datfile(datname);
		if(datfile.open(QIODevice::WriteOnly))
		{
			datfile.write(data);
			datfile.close();
			if (deletefile) file.remove();
		}
	}
}



void logisdom::deadDevice()
{
    bool ok;
    if (messageBox::questionHide(this, tr("Dead device replacement"), tr("This action cannot be undone, in case of problem you will have to restore manually a backup configuration file\nThis action must be used if you want to replace a defective device by a new one and affect the defective device data and all configuration to the new device\nYou must start LogisDom with the defective device out of the network. After this window you will be ask to select the dead device and the new device witch must be present on the network. LogisDom will create a new configuration file replacing all occurence of the defective device RomID by the new one. Current defective device .dat file will be copied with the new device RomID as all the zipped dat files\nThe application will exit by itself, you must relaunch it manually.\nAre you sure you want to begin this action ?"), this, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
// create anterior device list not found
    QString msg;
    QString cfgfilename = configfilename;
    QFile file(cfgfilename);
    QTextStream in(&file);
    QString configdata;
// Read original config file to get all device config and extract dead devices
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        GenMsg("cannot read configuration file  " + cfgfilename);
        return;
    }
    else
    {
        while (!in.atEnd())
        {
            //QString line = in.readLine();
            configdata.append(in.readLine() + "\n");
        }
    }
    file.close();
    deadevice *deadDev;
    QString oldDevice, newDevice;
    deadDev = new deadevice();
    deadDev->oldDevice = &oldDevice;
    deadDev->newDevice = &newDevice;
    //onewiredevice *device = nullptr;
    QStringList devRomID, devConfig;
    QString ReadRomID;
    QString TAG_Begin = One_Wire_Device;
    QString TAG_End = EndMark;
// generate list of missing for choose purpuses
    SearchLoopBegin
        ReadRomID = getvalue("RomID", strsearch);
        if (!ReadRomID.isEmpty())
        {
            if (!configwin->deviceexist(ReadRomID))
            {
                devRomID.append(ReadRomID);
                devConfig.append(strsearch);
                QString devRomIDend = ReadRomID.right(2);
                if (devRomIDend.startsWith("_")) ReadRomID.chop(2);
                ReadRomID.right(2).toInt(&ok, 16);
                if (ok) deadDev->addOldDevice(ReadRomID);
            }
        }
    SearchLoopEnd
// generate actual device list
    QString str;
    //QByteArray deviceData;
    QString deviceData;
    if (configwin) configwin->GetDevicesStr(str); else return;
    deviceData.append(str);
    index_a = deviceData.indexOf("\n" + TAG_Begin, 0);
    do  {
        if (index_a != -1)
        {
            index_b = deviceData.indexOf("\n" + TAG_End, index_a);
            if (index_b != -1)
            {
                strsearch = deviceData.mid(index_a, index_b - index_a);
                ReadRomID = getvalue("RomID", strsearch);
                if (!ReadRomID.isEmpty())
                {
                    QString devRomIDend = ReadRomID.right(2);
                    if (devRomIDend.startsWith("_")) ReadRomID.chop(2);
                    ReadRomID.right(2).toInt(&ok, 16);
                    if (ok) deadDev->addDevice(ReadRomID);
                }
            }
        }
        index_a = deviceData.indexOf("\n" + TAG_Begin, index_b);
        index ++;
    } while(index_a != -1);
// select dead device
    //device = nullptr;
    deadDev->exec();
    delete deadDev;
    if ((oldDevice.isEmpty()) || (newDevice.isEmpty())) return;
// save original device config
    QStringList oldDevList, newDevList;
    net1wire::getDeviceList(oldDevice, oldDevList);
    net1wire::getDeviceList(newDevice, newDevList);
    //QStringList originalDevConfig;
    //for (int n=0; n<oldDevList.count(); n++)
    //{
    //    int index = devRomID.indexOf(oldDevList.at(n));
    //    if (index != -1) originalDevConfig.append(devConfig.at(index));
     //   else originalDevConfig.append("");
    //}
// Rename file with .backup_xx extension
    if (file.exists())
    {
        bool found = true;
        int n = 1;
        while ((found) && (n<100))
        {
            QString backupFileName = cfgfilename + QString(".backup_%1").arg(n);
            QFile backupFile(backupFileName);
            if (!backupFile.exists())
            {
                file.rename(backupFileName);
                found = false;
            }
            n ++;
        }
        if (n > 99)
        {
            QFile firstFile(cfgfilename + ".backup_1");
            firstFile.remove();
            for (int n=2; n<100; n++)
            {
                QFile nextFile(cfgfilename + QString(".backup_%1").arg(n));
                nextFile.rename(cfgfilename + QString(".backup_%1").arg(n-1));
            }
            QString backupFileName = cfgfilename + ".backup_99";
            QFile backupFile(backupFileName);
            if (!backupFile.exists())
            {
                if (file.rename(backupFileName)) GenMsg("Configuration file was renamed as " + backupFileName + " for backup");
                else GenMsg("Configuration could not be renamed as " + backupFileName + " for backup, access was denied");
            }
            else GenMsg("Configuration could not be renamed as " + backupFileName + " for backup, file already exists");
        }
    }

// save original data with replaced RomIDs
    QFile newcfgfile(cfgfilename);
    QTextStream out(&newcfgfile);
    if (newcfgfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        out.setGenerateByteOrderMark(true);
        QString oldDeviceBA, newDeviceBA, oldDeviceHex, newDeviceHex, remconfigdata, newconfigdata;
        oldDeviceBA.append(oldDevice);
        newDeviceBA.append(newDevice);
        oldDeviceHex.append(oldDevice.toUtf8().toHex().toUpper());
        newDeviceHex.append(newDevice.toUtf8().toHex().toUpper());
    // if new device had already config data we must remove it before exchanging RomIDs from old to new, otherwise next launch could get 2 config data for the same new RomID
        remconfigdata = configdata.replace("One_Wire_Device\nRomID = (" + newDeviceBA, "");
    // replace all RomID
        newconfigdata = remconfigdata.replace(oldDeviceBA, newDeviceBA);
        out << newconfigdata.replace(oldDeviceHex, newDeviceHex);
        newcfgfile.close();
        msg = "Configuration file was generated replacing all occurence of " + oldDevice + " by " + newDevice;
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        GenMsg(msg);
    }
    else
    {
        msg = "File " + file.fileName() + " could not be opened, operation aborted";
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        GenMsg(msg);
        return;
    }
// rename actual .dat file with .dead extension
    QDateTime now = QDateTime::currentDateTime();
    for (int n=0; n<oldDevList.count(); n++)
    {
        QString newfilename = QString(getrepertoiredat()) + oldDevList.at(n) + "_" + now.toString("MM-yyyy") + dat_ext;
        QString deadfilename = QString(getrepertoiredat()) + oldDevList.at(n) + "_" + now.toString("MM-yyyy") + ".dead";
        QFile file(newfilename);
        if (file.exists())
        {
            if (file.copy(deadfilename))
            {
                msg = "File " + file.fileName() + " was copied as " + deadfilename;
                messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
            else
            {
                msg = "File " + file.fileName() + " could not be copied as " + deadfilename;
                messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
        }
        else
        {
            msg = "File " + file.fileName() + " was not found to copy as " + deadfilename;
            messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            GenMsg(msg);
        }
    }
    messageBox::warningHide(this, cstr::toStr(cstr::MainName), "Dead device " + oldDevice + tr(".dat files were copied with .dead extension"), this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
// rename old dat file with new RomID
    for (int n=0; n<oldDevList.count(); n++)
    {
        QString oldfilename = QString(getrepertoiredat()) + oldDevList.at(n) + "_" + now.toString("MM-yyyy") + dat_ext;
        QString newfilename = QString(getrepertoiredat()) + newDevList.at(n) + "_" + now.toString("MM-yyyy") + dat_ext;
        QFile oldfile(oldfilename);
        QFile newfile(newfilename);
        if (newfile.exists())
        {
            if (newfile.remove())
            {
                msg = "File " + newfile.fileName() + " was removed";
                //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
            else
            {
                msg = "File " + newfile.fileName() + " could not be removed";
                //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
        }
        if (oldfile.exists())
        {
            if (oldfile.rename(newfilename))
            {
                msg = "File " + oldfile.fileName() + " was renamed as " + newfilename;
                //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
            else
            {
                msg = "File " + oldfile.fileName() + " could not be renamed as " + newfilename;
                //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                GenMsg(msg);
            }
        }
    }
    messageBox::warningHide(this, cstr::toStr(cstr::MainName), "Dead device data file(s) renamed with new RomID", this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
    bool error = false;
// rename zip file and content
    for (int n=0; n<oldDevList.count(); n++)
    {
        int year = QDateTime::currentDateTime().date().year();
        QString oldZipFileName = QString(getrepertoiredat()) + oldDevList.at(n) + "_" + QString("%1").arg(year, 4, 10, QChar('0')) + ".zip";
        QString newZipFileName = QString(getrepertoiredat()) + newDevList.at(n) + "_" + QString("%1").arg(year, 4, 10, QChar('0')) + ".zip";
        QFile file(oldZipFileName);
        while (file.exists()) // loop for each year decreasing, stop when no file for the current year
        {
            QFile newZipfile(newZipFileName);
            if (newZipfile.exists())
            {
                if (newZipfile.remove())
                {
                    msg = "File " + newZipfile.fileName() + " was deleted";
                    //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                    GenMsg(msg);
                }
                else
                {
                    msg = "File " + newZipfile.fileName() + " could not be deleted";
                    //messageBox::warningHide(this, cstr::toStr(cstr::MainName), msg, this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                    GenMsg(msg);
                }
            }
            QuaZip oldZipFile(oldZipFileName);
            QuaZip newZipFile(newZipFileName);
            if (!newZipFile.open(QuaZip::mdAdd)) newZipFile.open(QuaZip::mdCreate);
            if (oldZipFile.open(QuaZip::mdUnzip) && newZipFile.getZipError() == UNZ_OK)
            {
                QuaZipFileInfo info;
                QuaZipFile oldDatFile(&oldZipFile);
                QuaZipFile newDatFile(&newZipFile);
                QString currentfilename;
                for(bool more=oldZipFile.goToFirstFile(); more; more=oldZipFile.goToNextFile())
                {
                    if (oldZipFile.getCurrentFileInfo(&info))
                    {
                        if (oldDatFile.getZipError() == UNZ_OK)
                        {
                            currentfilename = oldDatFile.getActualFileName();
                            if (currentfilename.contains(oldDevList.at(n)))
                            {
                                QString newcurrentfilename = currentfilename.replace(oldDevList.at(n), newDevList.at(n));
                                if (oldDatFile.open(openModeRead) && newDatFile.open(openModeWrite, QuaZipNewInfo(newcurrentfilename, newcurrentfilename)))
                                {
                                    QByteArray data = oldDatFile.readAll();
                                    oldDatFile.close();
                                    newDatFile.write(data);
                                    if (newDatFile.getZipError() != UNZ_OK)
                                    {
                                        QString msg = newcurrentfilename + " cannot be written";
                                        GenMsg(msg);
                                        error = true;
                                    }
                                    newDatFile.close();
                                    if (newDatFile.getZipError() != UNZ_OK)
                                    {
                                        msg = "File " + currentfilename + " could not be closed";
                                        GenMsg(msg);
                                        error = true;
                                    }
                                }
                            }
                        }
                    }
                }
                oldZipFile.close();
                newZipFile.close();
                if (error) messageBox::warningHide(this, cstr::toStr(cstr::MainName), "Zip file " + oldZipFileName + " could not be done correctly", this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
                else messageBox::warningHide(this, cstr::toStr(cstr::MainName), "Zip file " + oldZipFileName + " was renamed with new RomID", this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            }
            else messageBox::warningHide(this, cstr::toStr(cstr::MainName), "Zip file " + oldZipFileName + " could not be renamed", this, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            year --;
            oldZipFileName = QString(getrepertoiredat()) + oldDevList.at(n) + "_" + QString("%1").arg(year, 4, 10, QChar('0')) + ".zip";
            newZipFileName = QString(getrepertoiredat()) + newDevList.at(n) + "_" + QString("%1").arg(year, 4, 10, QChar('0')) + ".zip";
            file.setFileName(oldZipFileName);
        }
    }
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(QCoreApplication::applicationFilePath()))) QCoreApplication::exit(0);
}



void logisdom::aproposde()
{
      messageBox::aboutHide(this, QString("à  propos de"),
                           QString("Cette application gère :\n"
                           "USB300 EnOcean\n"
                           "Capteurs 1 wire via interface ethernet\n"
                           "Modules X10, PLC Bus via tranceiver RS232/Ethernet\n"
                           "Vannes de radiateur radio pilotées STA800, des compteurs d'énergie électrique\n"
                           "Compteurs d'énergie électrique via One Wire\n"
                           "Téléinfo compteur EDF\n"
                           "Calcul statistique en temps réel\n"
                           "Gestion et Régulation pour installation Solaire/Bois/Electrique\n"
                           "Connection Régulation Solaire ROTEX et RESOL\n"
                           "\n"
                           "\n"
                           "http://logisdom.fr\n"
                           "mail : remy.carisio@orange.fr\n"
                           "\n"
				   "Version ") + Version + "\n" + VersionDate
				   + "\n\n\n", this);
}


