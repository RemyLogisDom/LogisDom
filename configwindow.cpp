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



#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDialogButtonBox>
#include <QCryptographicHash>
#include "simplecrypt.h"
#include "iconearea.h"
#include "graphconfig.h"
#include "alarmwarn.h"
#include "errlog.h"
#include "x10.h"
#include "deveo.h"
#include "teleinfo.h"
#include "mbus.h"
#include "ecogest.h"
#include "devonewire.h"
#include "lcdonewire.h"
#include "ledonewire.h"
#include "modbus.h"
#include "devvirtual.h"
#include "devfinder.h"
#include "fts800.h"
#include "ha7net.h"
#include "plcbus.h"
#include "net1wire.h"
#include "enocean.h"
#include "rps2.h"
#include "resol.h"
#include "devrps2.h"
#include "devresol.h"
#include "server.h"
#include "formula.h"
#include "connection.h"
#include "configmanager.h"
#include "logisdom.h"
#include "remote.h"
#include "onewire.h"
#include "chauffageunit.h"
#include "programevent.h"
#include "tableauconfig.h"
#include "addDaily.h"
#include "daily.h"
#include "vmc.h"
#include "treehtmlwidget.h"
#include "htmlbinder.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "configwindow.h"
#include "mailsender.h"
#include "globalvar.h"
#include "mail/mailserver.h"
#include "mail/mimemessage.h"
#include "mail/emailaddress.h"
#include "mail/mimetext.h"
#include "mail/serverreply.h"


#ifdef WIN32
#include <windows.h>
#define SER_PORT "\\\\.\\COM91"  //COM4
#else
#define SER_PORT "/dev/ttyUSB0"//! the Serial Port Device
#endif


configwindow::configwindow(logisdom *Parent)
{
	ui.setupUi(this);
    findCodecs();
    setCodec("");
    for (int n=1; n<LastType; n++) ui.comboBoxInterface->addItem(NetTypeStr[n]);
    parent = Parent;
    Extra = ui.tabWidgetGeneral->widget(6);
    ui.tabWidgetGeneral->removeTab(6);
	server = new Server(parent);
    configmanager = new configManager(parent, this, false);
	flagCheckZipFiles = true;
    QGridLayout *ValueLayout = new QGridLayout(ui.tabWidget->widget(TabValues));
	ValueLayout->addWidget(parent->mainTreeHtml, 1, 1, 1, 2);
	lastPng = QDateTime::currentDateTime();
	htmlBindNetworkMenu = new htmlBinder(parent);
	htmlBindNetworkMenu->setMainParameter(tr("Configuration"), "");
	htmlBindDeviceMenu = new htmlBinder(parent);
	htmlBindDeviceMenu->setMainParameter(tr("Devices"), "");
	htmlBindWebMenu = new htmlBinder(parent);
	htmlBindWebMenu->setMainParameter(tr("Web Pages"), "");
	ui.lineEditHtmlHeaderCSS->setText("a:link { text-decoration:none; color:black } a:visited { text-decoration:none; color:black } a:hover { text-decoration:underline;color:#0000ff}");
	ui.lineEditHtmlTimeCSS->setText("font-size:100%;border:1px solid black; background-color:grey;");
	ui.lineEditHtmlMenuCSS->setText("");
	ui.lineEditHtmlListCSS->setText("");
	ui.lineEditHtmlDetectorCSS->setText("");
	ui.lineEditHtmlValueCSS->setText("");
	ui.lineEditHtmlCommandCSS->setText("");
	setCSS();
	iconPngIndex = 0;
	graphPngIndex = 0;
    chartPngIndex = 0;
    configmanagerTr = new configManager(parent, this, true);
    QGridLayout ValueTrLayout(ui.tabWidget->widget(TabValuesTr));
    ValueTrLayout.addWidget(configmanagerTr, 1, 1, 1, 2);
	setup.setLayout(&setupLayout);
	checkBoxTree.setText(tr("Tree View"));
	OneWireTree.setColumnCount(2);
    OneWireTree.setColumnWidth(0, 250);
    setupLayout.addWidget(&checkBoxTree);
    setupLayout.addWidget(&Listfilter);
    setupLayout.addWidget(&OneWireList);
    QString txt = tr("Width");
	ui.comboBoxSize->addItem(txt);
	ui.comboBoxHtmlSize->addItem(txt);
	txt = tr("Height");
	ui.comboBoxHtmlSize->addItem(txt);
	ui.comboBoxSize->addItem(txt);
    txt = tr("Width & Height");
    ui.comboBoxHtmlSize->addItem(txt);
    ui.comboBoxSize->addItem(txt);
    ui.lineEditHtmlTimeFormat->setToolTip(parent->TimeFormatDetails);
	ui.lineEditHtmlDateFormat->setToolTip(parent->DateFormatDetails);
	ui.listWidget->setSortingEnabled(true);
	connect(ui.checkBoxServer, SIGNAL(stateChanged(int)), this, SLOT(startstopserver(int)));
    connect(server, SIGNAL(clientbegin(Connection*)), this, SLOT(NewClient(Connection*)));
    connect(server, SIGNAL(newRequest(QString)), this, SLOT(newRequest(QString)));
    connect(server, SIGNAL(clientend(Connection*)), this, SLOT(ClientisGone(Connection*)));
	connect(ui.listViewUsers, SIGNAL(customContextMenuRequested(QPoint)), this , SLOT(rightclicklList(QPoint)));
	connect(&OneWireList, SIGNAL(itemClicked (QListWidgetItem*)), this, SLOT(setPalette(QListWidgetItem*)));
	connect(&OneWireTree, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT(setTreePalette(QTreeWidgetItem*, int)));
	connect(&checkBoxTree, SIGNAL(stateChanged(int)), this, SLOT(treeViewSwitch(int)));
    connect(&Listfilter, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString)));
    connect(ui.pushButtonPNGFolder, SIGNAL(clicked()), this, SLOT(choosePngFolder()));
	connect(ui.pushButtonAdd, SIGNAL(clicked()), this, SLOT(addHtmlMenu()));
	connect(ui.pushButtonRemove, SIGNAL(clicked()), this, SLOT(removeHtmlMenu()));
    connect(ui.pushButtonAddInterface, SIGNAL(clicked()), this, SLOT(ajouter()));
    connect(ui.pushButtonAddVD, SIGNAL(clicked()), this, SLOT(addVDev()));
    connect(ui.pushButtonRemoveVD, SIGNAL(clicked()), this, SLOT(removeVD()));
    connect(ui.pushButtonLoadVDConfig, SIGNAL(clicked()), this, SLOT(loadVD()));
    connect(ui.checkBoxGenPng, SIGNAL(stateChanged(int)), this, SLOT(pngEnabled(int)));
	connect(ui.checkBoxSize, SIGNAL(stateChanged(int)), this, SLOT(pngResize(int)));
    connect(ui.comboBoxSize, SIGNAL(currentIndexChanged(int)), this, SLOT(pngResize(int)));
    connect(ui.checkBoxHtmlSize, SIGNAL(stateChanged(int)), this, SLOT(htmlEnabled(int)));
	connect(ui.checkBoxHideHeating, SIGNAL(stateChanged(int)), this, SLOT(HideHeatingTab(int)));
	connect(ui.lineEditHtmlTitleText, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlTitleCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlHeaderCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlTimeCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlMenuCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlListCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlDetectorCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlValueCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlCommandCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
    connect(ui.comboBoxCodecs, SIGNAL(currentIndexChanged(int)), this, SLOT(codecIndexChanged(int)));
    connect(ui.lineEditCRCCalc, SIGNAL(textChanged(QString)), this, SLOT(CRCCalChanged(QString)));
    connect(ui.pushButtonAddHtml, SIGNAL(clicked()), this, SLOT(addHtml()));
    connect(ui.radioButtonCRC8, SIGNAL(clicked()), this, SLOT(CRCCLicked()));
    connect(ui.radioButtonCRC16, SIGNAL(clicked()), this, SLOT(CRCCLicked()));
    connect(ui.pushButtonRemoveHtml, SIGNAL(clicked()), this, SLOT(removeHtml()));
	connect(ui.comboBoxHtml, SIGNAL(currentIndexChanged(int)), this, SLOT(changeHtmlSetup(int)));
	connect(ui.lineEditHtmlTitleCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextTitleCSS(QPoint)));
	connect(ui.lineEditHtmlHeaderCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextHeaderCSS(QPoint)));
	connect(ui.lineEditHtmlTimeCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextTimeCSS(QPoint)));
	connect(ui.lineEditHtmlMenuCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuCSS(QPoint)));
	connect(ui.lineEditHtmlListCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextListCSS(QPoint)));
	connect(ui.lineEditHtmlDetectorCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextDetectorCSS(QPoint)));
	connect(ui.lineEditHtmlValueCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextValueCSS(QPoint)));
	connect(ui.lineEditHtmlCommandCSS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextCommandCSS(QPoint)));
	connect(ui.pushButtonDataPath, SIGNAL(clicked()), this, SLOT(DataPathClicked()));
	connect(ui.pushButtonZipPath, SIGNAL(clicked()), this, SLOT(ZipPathClicked()));
	connect(ui.pushButtonBackupPath, SIGNAL(clicked()), this, SLOT(BackupPathClicked()));
	connect(ui.pushButtonHtmlPath, SIGNAL(clicked()), this, SLOT(HtmlPathClicked()));
	connect(ui.lineEditDataFolder, SIGNAL(editingFinished()), this, SLOT(DataPathChanged()));
	connect(ui.lineEditZipFolder, SIGNAL(editingFinished()), this, SLOT(ZipPathChanged()));
	connect(ui.lineEditBackupFolder, SIGNAL(editingFinished()), this, SLOT(BackupPathChanged()));
	connect(ui.lineEditHtmlFolder, SIGNAL(editingFinished()), this, SLOT(HtmlPathChanged()));
    connect(ui.pushButtonSendMail, SIGNAL(clicked()), this, SLOT(sendHtmlMail()));
    connect(ui.lineEditHtmlTitleText, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
    connect(ui.pushButtonUpdateNow, SIGNAL(clicked()), this, SLOT(updateNow()));
    connect(ui.PushButtonBackupNow, SIGNAL(clicked()), this, SLOT(backupNow()));
    connect(ui.lineEditSMTPServer, SIGNAL(textChanged(QString)), this, SLOT(changeSMTPServer(QString)));
    connect(ui.lineEditSMTPPassWord, SIGNAL(textChanged(QString)), this, SLOT(changeSMTPPassword(QString)));
    connect(ui.lineEditSMTPLogin, SIGNAL(textChanged(QString)), this, SLOT(changeSMTPLogin(QString)));
    connect(ui.lineEditSMTPPort, SIGNAL(textChanged(QString)), this, SLOT(changeSMTPPort(QString)));
    connect(ui.checkBoxSMTPSecure, SIGNAL(stateChanged(int)), this, SLOT(changeSMTPSecure(int)));
    connect(ui.tabWidget, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(renameTab(int)));
    boutonBackupStr = ui.PushButtonBackupNow->text();
    connect(&backupTimer, SIGNAL(timeout()), this, SLOT(backupTimerTimeOut()));
    backupTimer.start(60000);
    connect(&fileBackup, SIGNAL(finished()), this, SLOT(backupFinished()));
    htmlEnabled(Qt::Unchecked);
	pngResize(Qt::Unchecked);
	pngEnabled(Qt::Unchecked);
    connect(&eMailSender, SIGNAL(logMessage(QString)), this, SLOT(logMailSender(QString)));
    connect(&smsSender, SIGNAL(logMessage(QString)), this, SLOT(logSMSSender(QString)));
}





configwindow::~configwindow()
{
	delete server;
	delete configmanager;
}




onewiredevice *configwindow::chooseDevice()
{
    onewiredevice *device = nullptr;
    devfinder *devFinder;
    devFinder = new devfinder(parent);
    devFinder->sort();
    devFinder->exec();
    device = devFinder->choosedDevice;
    delete devFinder;
    return device;
}


void configwindow::Lock(bool state)
{
    foreach (net1wire *net, net1wirearray) { net->Lock(state); }
}


void configwindow::contextHeaderCSS(const QPoint &pos)
{
	menuEventfor(ui.lineEditHtmlHeaderCSS, pos);
}
void configwindow::contextTitleCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlTitleCSS, pos);
}
void configwindow::contextMenuCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlMenuCSS, pos);
}
void configwindow::contextTimeCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlTimeCSS, pos);
}
void configwindow::contextListCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlListCSS, pos);
}
void configwindow::contextDetectorCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlDetectorCSS, pos);
}
void configwindow::contextValueCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlValueCSS, pos);
}
void configwindow::contextCommandCSS(const QPoint & pos)
{
	menuEventfor(ui.lineEditHtmlCommandCSS, pos);
}






void configwindow::menuEventfor(QLineEdit *line, const QPoint &pos)
{
// Color
//	QMenu colorMenu(tr("Color"));
//	QAction centered(tr("Centered"), this);
//	textAlignMenu.addAction(&centered);
// text font
// text border
	QString str;
	QMenu mainMenu;
	QAction color(tr("color:"), this);
	QAction backgroundcolor(tr("background-color:"), this);
	QAction lineheight(tr("line-height:100%"), this);
	QAction fontsize(tr("font-size:20px"), this);
	QAction fontweight(tr("font-weight:900"), this);
	mainMenu.addAction(&color);
	mainMenu.addAction(&backgroundcolor);
	mainMenu.addAction(&lineheight);
	mainMenu.addAction(&fontsize);
	mainMenu.addAction(&fontweight);
//	mainMenu.addMenu(&textAlignMenu);
	QAction *selection;
	selection = mainMenu.exec(line->mapToGlobal(pos));
	if (selection == &color) str = getColorCode("color:#");
	if (selection == &backgroundcolor) str = getColorCode("background-color:#");
	if (selection == &lineheight) str = "line-height:200%;";
	if (selection == &fontsize) str = "font-size:20px;";
	if (selection == &fontweight) str = "font-weight:900;";
	line->insert(str);
}


// http://www.w3schools.com/css/css_link.asp


QString configwindow::getColorCode(QString head)
{
	QString str;
	QColor color = QColorDialog::getColor();
	if (!color.isValid()) return "";
	QString r = QString("%1").arg(color.red(), 2, 16, QChar('0'));
	QString g = QString("%1").arg(color.green(), 2, 16, QChar('0'));
	QString b = QString("%1").arg(color.blue(), 2, 16, QChar('0'));
	str = head + r + g + b + ";";
	return str;
}




void configwindow::addHtml()
{
	bool ok;
Retry :
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	int found = ui.comboBoxHtml->findText(nom);
	if (found != -1)
	{
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto Retry;
	}
	disconnect(ui.comboBoxHtml, SIGNAL(currentIndexChanged(int)), this, SLOT(changeHtmlSetup(int)));
	ui.comboBoxHtml->addItem(nom);
	connect(ui.comboBoxHtml, SIGNAL(currentIndexChanged(int)), this, SLOT(changeHtmlSetup(int)));
	htmlSetup *setup = new htmlSetup;
	setup->welcomeMessage = ui.lineEditHtmlTitleText->text();
	setup->htmlTitle = ui.lineEditHtmlTitleCSS->text();
	setup->htmlHeader = ui.lineEditHtmlHeaderCSS->text();
	setup->htmlTime = ui.lineEditHtmlTimeCSS->text();
	setup->htmlMenu = ui.lineEditHtmlMenuCSS->text();
	setup->htmlList = ui.lineEditHtmlListCSS->text();
	setup->htmlDetector = ui.lineEditHtmlDetectorCSS->text();
	setup->htmlValue = ui.lineEditHtmlValueCSS->text();
	setup->htmlCommand = ui.lineEditHtmlCommandCSS->text();
	htmlLayout.append(setup);
}




void configwindow::removeHtml()
{
	int index = ui.comboBoxHtml->currentIndex();
	if (index == -1) return;
	if (messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to ") + ui.comboBoxHtml->currentText(), parent, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes) return;
	ui.comboBoxHtml->removeItem(index);
	htmlSetup *setup = htmlLayout.takeAt(index);
	delete setup;
	updateHtmlPreview("");
}



void configwindow::changeHtmlSetup(int)
{
	int index = ui.comboBoxHtml->currentIndex();
	if (index == -1) return;

	disconnect(ui.lineEditHtmlTitleText, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlTitleCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlHeaderCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlTimeCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlMenuCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlListCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlDetectorCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlValueCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	disconnect(ui.lineEditHtmlCommandCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));

	ui.lineEditHtmlTitleText->setText(htmlLayout[index]->welcomeMessage);
	ui.lineEditHtmlHeaderCSS->setText(htmlLayout[index]->htmlHeader);
	ui.lineEditHtmlTitleCSS->setText(htmlLayout[index]->htmlTitle);
	ui.lineEditHtmlTimeCSS->setText(htmlLayout[index]->htmlTime);
	ui.lineEditHtmlMenuCSS->setText(htmlLayout[index]->htmlMenu);
	ui.lineEditHtmlListCSS->setText(htmlLayout[index]->htmlList);
	ui.lineEditHtmlDetectorCSS->setText(htmlLayout[index]->htmlDetector);
	ui.lineEditHtmlValueCSS->setText(htmlLayout[index]->htmlValue);
	ui.lineEditHtmlCommandCSS->setText(htmlLayout[index]->htmlCommand);
	updateHtmlPreview("");

	connect(ui.lineEditHtmlTitleText, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlTitleCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlHeaderCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlTimeCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlMenuCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlListCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlDetectorCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlValueCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));
	connect(ui.lineEditHtmlCommandCSS, SIGNAL(textChanged(QString)), this, SLOT(updateHtmlPreview(QString)));

}



void configwindow::setCSS()
{
	QString TitleCSS = ".title{ " + ui.lineEditHtmlTitleCSS->text() + " }";
	QString TimeCSS = ".time{ " + ui.lineEditHtmlTimeCSS->text() + " }";
	QString MenuCSS = ".menu{ " + ui.lineEditHtmlMenuCSS->text() + " }";
	QString ListCSS = ".list{ " + ui.lineEditHtmlListCSS->text() + " }";
	QString DetectorCSS = ".detector{ " + ui.lineEditHtmlDetectorCSS->text() + " }";
	QString ValueCSS = ".value{ " + ui.lineEditHtmlValueCSS->text() + " }";
	QString CommandCSS = ".command{ " + ui.lineEditHtmlCommandCSS->text() + " }";
	Header = "<head><title>" + ui.lineEditHtmlTitleText->text()\
	+ "</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" /><style type=\"text/css\">"\
	+ TitleCSS + TimeCSS + MenuCSS + ListCSS + DetectorCSS + ValueCSS + CommandCSS\
    + ui.lineEditHtmlHeaderCSS->text() + "</style></head>"+\
"<form action=\"myform.cgi\">\
<input type=\"password\" name=\"password\" id=\"password\" maxlength=\"6\">\
<label for=\"password\"> Type a fake password here</label>\
<br><input type=\"submit\" value=\"Submit\">\
</form>";
}



void configwindow::HideHeatingTab(int state)
{
	if (state)
	{
		//parent->ui.tabWidget->setTabEnabled(1, false);
		parent->ui.tabWidget->removeTab(1);
	}
	else
	{
		//parent->ui.tabWidget->setTabEnabled(1, true);
		parent->ui.tabWidget->insertTab(1, parent->widgetTabHeating, parent->tabHeatingText);
	}
}




void configwindow::CRCCLicked()
{
    CRCCalChanged(ui.lineEditCRCCalc->text());
}



void configwindow::CRCCalChanged(QString scratchpad)
{
    bool ok;
    //QString scratchpad = ui.lineEditCRCCalc->text();
    //int L = (scratchpad.length() / 2);
    //byteVec_t scratchpadcrc = byteVec_t(L+1);
    //for (int n=0; n<L; n++)
    //{
    //    scratchpadcrc[n] = scratchpad.mid(n * 2, 2).toInt(&ok, 16);
    //    if (!ok)
    //    {
    //        ui.lineEditCRCResult->setText("");
    //        return;
    //    }
    //}
    QVector <uint8_t> v;
    for (int n=0; n<scratchpad.length(); n+=2) v.append(uint8_t(scratchpad.mid(n, 2).toUInt(&ok, 16)));
    uint8_t crccalc8 = onewiredevice::calcCRC8(v);
    uint16_t crccalc16 = onewiredevice::calcCRC16(v);

    //uint8_t crccalc8 = onewiredevice::calcCRC8(&scratchpadcrc[0], L);
    //uint16_t crccalc16 = onewiredevice::calcCRC16(&scratchpadcrc[0], L);

    if (ui.radioButtonCRC8->isChecked())
    {
        ui.lineEditCRCResult->setText(QString("%1").arg(crccalc8, 2, 16, QLatin1Char('0')).toUpper());
    }
    else if (ui.radioButtonCRC16->isChecked())
    {
        ui.lineEditCRCResult->setText(QString("%1").arg(crccalc16, 4, 16, QLatin1Char('0')).toUpper());
    }
    else ui.lineEditCRCResult->setText("");
}


void configwindow::changeSMTPServer(QString str)
{
    eMailSender.smtpServer = str;
}


void configwindow::changeSMTPPassword(QString str)
{
    eMailSender.smptPassword = str;
}


void configwindow::changeSMTPLogin(QString str)
{
    eMailSender.eMailSender = str;
}

void configwindow::changeSMTPPort(QString str)
{
    eMailSender.smtpPort = str;
}



void configwindow::changeSMTPSecure(int state)
{
    bool enabled = true;
    if (state != Qt::Checked) enabled = false;
    //ui.lineEditSMTPPassWord->setEnabled(enabled);
    eMailSender.useSSL = enabled;
}


void configwindow::logMailSender(QString msg)
{
    parent->GenError(90, msg);
}


void configwindow::logSMSSender(QString msg)
{
    parent->GenError(91, msg);
}

//http://127.0.0.1:1220/request=(GetMainMenu)user=()password=()command=()

void configwindow::updateHtmlPreview(QString)
{
	int index = ui.comboBoxHtml->currentIndex();
	if (index != -1)
	{
		htmlLayout[index]->welcomeMessage = ui.lineEditHtmlTitleText->text();
		htmlLayout[index]->htmlTitle = ui.lineEditHtmlTitleCSS->text();
		htmlLayout[index]->htmlTime = ui.lineEditHtmlTimeCSS->text();
		htmlLayout[index]->htmlHeader = ui.lineEditHtmlHeaderCSS->text();
		htmlLayout[index]->htmlMenu = ui.lineEditHtmlMenuCSS->text();
		htmlLayout[index]->htmlList = ui.lineEditHtmlListCSS->text();
		htmlLayout[index]->htmlDetector = ui.lineEditHtmlDetectorCSS->text();
		htmlLayout[index]->htmlValue = ui.lineEditHtmlValueCSS->text();
		htmlLayout[index]->htmlCommand = ui.lineEditHtmlCommandCSS->text();
	}
	setCSS();
	parent->setPalette(&parent->htmlPreview);
    /*QUrl url = parent->htmlWebPreview.url();
	if (url.isEmpty())
	{
		if (server->ConnectionUsers.count() > 0)
		{
            //parent->htmlWebPreview.setUrl(QUrl("http://127.0.0.1:1220/request=(GetMainMenu)user=(...)password=(...)"));
			QString url = "http://127.0.0.1:";
			url += QString("%1").arg(ui.spinBoxPortServer->value());
			url += "/request=(GetMainMenu)user=(";
			url += server->ConnectionUsers[0].Name;
			url += ")password=(";
			url += server->ConnectionUsers[0].PassWord;
			url += ")";
            //parent->htmlWebPreview.setUrl(QUrl(url));
		}
    }
    //else parent->htmlWebPreview.reload();*/
}




void configwindow::rightclicklList(const QPoint & pos)
{
	QModelIndex index = ui.listViewUsers->currentIndex();
	QMenu contextualmenu;
	QAction add(tr("Add User"), this);
	QAction remove(tr("Remove User"), this);
    QAction showExtra(tr("Show extra"), this);
    QAction *selection;
    int state = QApplication::keyboardModifiers();
    if (state & Qt::ControlModifier)
    {
        if (ui.tabWidgetGeneral->count() == 6) contextualmenu.addAction(&showExtra);
    }
    contextualmenu.addAction(&add);
    if (index.row() != -1)
    {
        contextualmenu.addAction(&remove);
    }
    selection = contextualmenu.exec(ui.listViewUsers->mapToGlobal(pos));
    if (selection == &add) addUser();
    if (selection == &remove) removeUser(index.row());
    if (selection == &showExtra) ui.tabWidgetGeneral->addTab(Extra, "Extra");
}




void configwindow::filterChanged(QString)
{
    updateDeviceList();
}


void configwindow::treeViewSwitch(int state)
{
	updateDeviceList();
	if (state == Qt::Unchecked)
	{
		setupLayout.removeWidget(&OneWireTree);
		OneWireTree.hide();
		setupLayout.addWidget(&OneWireList);
		OneWireList.show();
	}
	if (state == Qt::Checked)
	{
		setupLayout.removeWidget(&OneWireList);
		OneWireList.hide();
		setupLayout.addWidget(&OneWireTree);
		OneWireTree.show();
	}
}






void configwindow::closeNet1Wire()
{
	for (int n=0; n<net1wirearray.count(); n++) net1wirearray[n]->closeTCP();
}




void configwindow::startNetwork()
{
	for (int n=0; n<net1wirearray.count(); n++) net1wirearray[n]->startTCP();
}




void configwindow::generatePng()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 d = lastPng.secsTo(now);
    if ((d < (60 * ui.spinBoxPNGDelay->value())) && ((iconPngIndex == 0) && (graphPngIndex == 0) && (chartPngIndex == 0))) return;
    if ((iconPngIndex == 0) && (graphPngIndex == 0) && (chartPngIndex == 0)) lastPng = lastPng.addSecs(60 * ui.spinBoxPNGDelay->value());
	if (!ui.checkBoxGenPng->isChecked()) return;
    //qDebug() << QString ("iconPngIndex %1 graphPngIndex %2 chartPngIndex %3").arg(iconPngIndex).arg(graphPngIndex).arg(chartPngIndex);
    //qDebug() << "last : " + lastPng.toString();
    //qDebug() << "now : " + now.toString();
    int sizeH = parent->configwin->ui.spinBoxSizeH->value();
    int sizeW = parent->configwin->ui.spinBoxSizeW->value();
    QString path;
	if (ui.lineEditPngPath->text().isEmpty()) path = QString(repertoirepng) + QDir::separator();
		else path = ui.lineEditPngPath->text() + QDir::separator();
	QDir localdirectory = QDir("");
	if (!localdirectory.exists(path))
		if (!localdirectory.mkdir(path))
		{
                        parent->GenMsg(tr("Cannot create png directory"));
                        //messageBox::criticalHide(parent, tr("png directory"),tr("Impossible to create png directory"), parent);
			return;
		}
        QMutexLocker locker(&parent->MutexgetTabPix);
        if (iconPngIndex < parent->IconeAreaList.count())
        {
                QPixmap pixmap(parent->IconeAreaList.first()->grab());
                QSize S = pixmap.size();
                QString fileName = parent->ui.tabWidgetIcon->tabText(iconPngIndex) + ".png";
                QString filePathName = path + fileName;
                parent->IconeAreaList.at(iconPngIndex)->repaint();
                parent->IconeAreaList.at(iconPngIndex)->update();
                if (parent->configwin->ui.checkBoxSize->isChecked())
                {
                    if (parent->configwin->ui.comboBoxSize->currentIndex() == 0)   // Witdh
                            S.scale(sizeW, S.height() * sizeW / S.width(), Qt::IgnoreAspectRatio);
                    else if (parent->configwin->ui.comboBoxSize->currentIndex() == 1)  // Height
                            S.scale(S.width() * sizeH / S.height(), sizeH, Qt::IgnoreAspectRatio);
                    else if (parent->configwin->ui.comboBoxSize->currentIndex() == 2)  // Both
                        S.scale(sizeW, sizeH, Qt::IgnoreAspectRatio);
                    QPixmap pixmap = parent->IconeAreaList.at(iconPngIndex)->grab().scaled(S, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    savePng.appendData(filePathName, pixmap);
                }
                else
                {
                    QPixmap pixmap = parent->IconeAreaList.at(iconPngIndex)->grab();
                    savePng.appendData(filePathName, pixmap);
                }
                if (!savePng.isRunning()) savePng.start();
                iconPngIndex ++;
		return;
	}
	if (graphPngIndex < parent->ui.tabWidgetGraphic->count())
	{
        //if (parent->graphconfigwin->graphPtArray.at(graphPngIndex)->ContinousUpdate.isChecked())
        //{
            QSize tabSize = parent->ui.tabWidgetGraphic->currentWidget()->size();
            QString fileName = parent->ui.tabWidgetGraphic->tabText(graphPngIndex) + ".png";
            QString filePathName = path + fileName;
            //QFile imagePng(path + fileName);
            //if (imagePng.open(QIODevice::ReadWrite))
            //{
            parent->graphconfigwin->graphPtArray.at(graphPngIndex)->resize(tabSize);
            QSize S(parent->graphconfigwin->graphPtArray.at(graphPngIndex)->size());
            QPixmap pixmap(S);
            parent->graphconfigwin->graphPtArray.at(graphPngIndex)->render(&pixmap);
            if (parent->configwin->ui.checkBoxSize->isChecked())
            {
                if (parent->configwin->ui.comboBoxSize->currentIndex() == 0)        // Witdh
                {
                    //pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                    QPixmap pix(pixmap.scaled(sizeW, S.height() * sizeW / S.width(), Qt::IgnoreAspectRatio));
                    savePng.appendData(filePathName, pix);
                }
                else if (parent->configwin->ui.comboBoxSize->currentIndex() == 1)   // Height
                {
                    //pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                    QPixmap pix(pixmap.scaled(sizeH, S.height() * sizeH / S.width(), Qt::IgnoreAspectRatio));
                    savePng.appendData(filePathName, pix);
                }
                else if (parent->configwin->ui.comboBoxSize->currentIndex() == 2)   // Both
                {
                    //pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                    QPixmap pix(pixmap.scaled(sizeW, sizeH, Qt::IgnoreAspectRatio));
                    savePng.appendData(filePathName, pix);
                }
            }
            else
            {
                QPixmap pixmap(parent->graphconfigwin->graphPtArray.at(graphPngIndex)->size());
                parent->graphconfigwin->graphPtArray.at(graphPngIndex)->render(&pixmap);
                //pixmap.save(&imagePng, "PNG");
                savePng.appendData(filePathName, pixmap);
            }
                //imagePng.close();
            //}
        //}
        if (!savePng.isRunning()) savePng.start();
        graphPngIndex ++;
		return;
	}
    if (chartPngIndex < parent->ui.tabWidgetChart->count())
    {
        QSize tabSize = parent->ui.tabWidgetChart->currentWidget()->size();
        QString fileName = parent->ui.tabWidgetChart->tabText(chartPngIndex) + ".png";
        QString filePathName = path + fileName;
        //QFile imagePng(path + fileName);
        //if (imagePng.open(QIODevice::ReadWrite))
        //{
            parent->tableauConfig->tableauPtArray.at(chartPngIndex)->resize(tabSize);
            QSize S(parent->tableauConfig->tableauPtArray.at(chartPngIndex)->size());
            QPixmap pixmap(S);
            parent->tableauConfig->tableauPtArray.at(chartPngIndex)->render(&pixmap);
            if (parent->configwin->ui.checkBoxSize->isChecked())
            {
                if (parent->configwin->ui.comboBoxSize->currentIndex() == 0)  // Witdh
                {
                    //pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                    //QPixmap pix(pixmap.scaled(size, S.height() * size / S.width(), Qt::IgnoreAspectRatio));
                    QPixmap pix(pixmap.scaledToWidth(sizeW));
                    savePng.appendData(filePathName, pix);
                }
                else if (parent->configwin->ui.comboBoxSize->currentIndex() == 1)  // Height
                {
                    //pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                    //QPixmap pix(pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio));
                    QPixmap pix(pixmap.scaledToHeight(sizeH));
                    savePng.appendData(filePathName, pix);
                }
                else if (parent->configwin->ui.comboBoxSize->currentIndex() == 2)       // Both
                {
                    QPixmap pix(pixmap.scaled(sizeW, sizeH, Qt::IgnoreAspectRatio));
                    savePng.appendData(filePathName, pix);
                }
             }
            else
            {
                QPixmap pixmap(parent->tableauConfig->tableauPtArray.at(chartPngIndex)->size());
                parent->tableauConfig->tableauPtArray.at(chartPngIndex)->render(&pixmap);
                //pixmap.save(&imagePng, "PNG");
                savePng.appendData(filePathName, pixmap);
            }
            //imagePng.close();
        //}
        if (!savePng.isRunning()) savePng.start();
        chartPngIndex ++;
        return;
    }
    QString fileName = "meteo.png";
    QString filePathName = path + fileName;
    //QFile imagePng(path + fileName);
    //if (imagePng.open(QIODevice::ReadWrite))
    //{
        if (parent->configwin->ui.checkBoxSize->isChecked())
		{
			QSize S(parent->ui.tabWidget->widget(3)->size());
			QPixmap pixmap(S);
			parent->ui.tabWidget->widget(3)->render(&pixmap);
            int sizeW = parent->configwin->ui.spinBoxSizeW->value();
            int sizeH = parent->configwin->ui.spinBoxSizeH->value();
            if (parent->configwin->ui.comboBoxSize->currentIndex() == 0)   // Witdh
            {
                //pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                //QPixmap pix(pixmap.scaled(S.width() * sizeW / S.height(), sizeW, Qt::IgnoreAspectRatio));
                QPixmap pix(pixmap.scaledToWidth(sizeW));
                savePng.appendData(filePathName, pix);
                if (!savePng.isRunning()) savePng.start();
            }
            if (parent->configwin->ui.comboBoxSize->currentIndex() == 1)   // Height
            {
                //pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio).save(&imagePng, "PNG");
                //QPixmap pix(pixmap.scaled(S.width() * size / S.height(), size, Qt::IgnoreAspectRatio));
                QPixmap pix(pixmap.scaledToHeight(sizeH));
                savePng.appendData(filePathName, pix);
                if (!savePng.isRunning()) savePng.start();
            }
            else        // Both
            {
                QPixmap pix(pixmap.scaled(sizeW, sizeH, Qt::IgnoreAspectRatio));
                savePng.appendData(filePathName, pix);
                if (!savePng.isRunning()) savePng.start();
            }
		}
		else
		{
			QPixmap pixmap(parent->ui.tabWidget->widget(3)->size());
			parent->ui.tabWidget->widget(3)->render(&pixmap);
            savePng.appendData(filePathName, pixmap);
            if (!savePng.isRunning()) savePng.start();
            //pixmap.save(&imagePng, "PNG");
		}
        //imagePng.close();
    //}
	iconPngIndex = 0;
	graphPngIndex = 0;
    chartPngIndex = 0;
	return;
}




void configwindow::pngEnabled(int state)
{
	switch (state)
	{
		case Qt::Unchecked : 
			ui.spinBoxPNGDelay->setEnabled(false);
			ui.lineEditPngPath->setEnabled(false);
			ui.pushButtonPNGFolder->setEnabled(false);
			ui.checkBoxSize->setEnabled(false);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			ui.spinBoxPNGDelay->setEnabled(true);
			ui.lineEditPngPath->setEnabled(true);
			ui.pushButtonPNGFolder->setEnabled(true);
			ui.checkBoxSize->setEnabled(true);
			break;
	}
	pngResize(ui.checkBoxSize->checkState());
}




void configwindow::pngResize(int)
{
    int  checked = ui.checkBoxSize->checkState();
    int config = ui.comboBoxSize->currentIndex();
    switch (checked)
	{
		case Qt::Unchecked : 
			ui.comboBoxSize->setEnabled(false);
            ui.spinBoxSizeH->setEnabled(false);
            ui.spinBoxSizeW->setEnabled(false);
            break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
            if (config == 0)    // Width
            {
                ui.comboBoxSize->setEnabled(true);
                ui.spinBoxSizeH->setEnabled(false);
                ui.spinBoxSizeW->setEnabled(true);
            }
            if (config == 1)    // Heigth
            {
                ui.comboBoxSize->setEnabled(true);
                ui.spinBoxSizeH->setEnabled(true);
                ui.spinBoxSizeW->setEnabled(false);
            }
            if (config == 2)    // Both
            {
                ui.comboBoxSize->setEnabled(true);
                ui.spinBoxSizeH->setEnabled(true);
                ui.spinBoxSizeW->setEnabled(true);
            }
            break;
	}
}



void configwindow::htmlEnabled(int state)
{
	switch (state)
	{
		case Qt::Unchecked : 
			ui.comboBoxHtmlSize->setEnabled(false);
			ui.spinBoxHtmlSize->setEnabled(false);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			ui.comboBoxHtmlSize->setEnabled(true);
			ui.spinBoxHtmlSize->setEnabled(true);
			break;
	}
}





void configwindow::addHtmlMenu()
{
	bool ok;
retry:
    QString Name = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if ((ok) and !Name.isEmpty())
	{
		bool found = false;
		for (int n=0; n<ui.listWidget->count(); n++)
			if (ui.listWidget->item(n)->text() == Name) found = true;
		if (found)
		{
            messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto retry;
		}
		addHtmlMenu(Name);
	}
}




void configwindow::addHtmlMenu(QString name)
{
	bool found = false;
	for (int n=0; n<ui.listWidget->count(); n++)
		if (ui.listWidget->item(n)->text() == name) found = true;
	if (found) return;
	QListWidgetItem *widgetList = new QListWidgetItem(ui.listWidget);
	widgetList->setText(name);
	parent->declareHtmlMenu(name);
	for (int n=0; n<devicePtArray.count(); n++)
	{
		devicePtArray[n]->setHtmlMenulist(ui.listWidget);
	}
	ui.listWidget->sortItems();
}





void configwindow::removeHtmlMenu()
{
	int index = ui.listWidget->currentIndex().row();
	if (index == -1) return;
	if ((messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to remove ") + ui.listWidget->currentItem()->text(), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
	{
		QListWidgetItem *widgetList = ui.listWidget->takeItem(index);
		for (int n=0; n<devicePtArray.count(); n++)
		{
			devicePtArray[n]->removeHtmlMenulist(widgetList->text());
		}
		delete widgetList;
	}
}




void configwindow::addUser()
{
	bool ok;
 retryName:
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	int count = server->ConnectionUsers.count();
	int index = -1;
	for (int n=0; n<count; n++)
		if (server->ConnectionUsers[n].Name == nom) index = n;
	if (index != -1)
	{
        messageBox::warningHide(this, nom, cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto retryName;
	}
retry :
    QString PassWord = inputDialog::getTextPalette(this, cstr::toStr(cstr::PassWord), cstr::toStr(cstr::PassWord), QLineEdit::Password, "", &ok, parent);
	if (!ok) return;
	if (PassWord.isEmpty()) return;
    QString CfrmPassWord = inputDialog::getTextPalette(this, cstr::toStr(cstr::CfrmPassWord), cstr::toStr(cstr::CfrmPassWord), QLineEdit::Password, "", &ok, parent);
	if (!ok) return;
	if (CfrmPassWord.isEmpty()) return;
	if (PassWord	!= CfrmPassWord)
	{
		messageBox::warningHide(this, tr("Password are not identical"), tr("Password are not identical\nPlease try again"), parent, QMessageBox::Retry);
		goto retry;
	}
	QStringList items;
	QString LimitedStr(tr("Limited"));
	QString AdminStr(tr("Administrator"));
	items << LimitedStr << AdminStr;
	QString item = inputDialog::getItemPalette(this, tr("User type"), tr("User type : "), items, 0, false, &ok, Qt::Dialog, parent);
	if (!ok) return;
	if (item.isEmpty()) return;
	int account = 0;
	if (item == LimitedStr) account = Limited;
	if (item == AdminStr) account = Admin;
	Server::UsersLogin NewUser;
	NewUser.Name = nom;
	NewUser.PassWord = PassWord;
	NewUser.Rigths = account;
	server->ConnectionUsers.append(NewUser);
	updateUsersList();
}






void configwindow::updateUsersList()
{
	ui.listViewUsers->clear();
	if (server->ConnectionUsers.count() > 0)
	{
		for (int n=0; n<server->ConnectionUsers.count(); n++)
		{
			QListWidgetItem *widgetList = new QListWidgetItem(ui.listViewUsers);
			if (server->ConnectionUsers[n].Rigths == Server::FullControl) widgetList->setText(server->ConnectionUsers[n].Name + tr(" (Admin)"));
			else widgetList->setText(server->ConnectionUsers[n].Name + tr(" (Limited)"));
		}
	}
}





void configwindow::removeUser(int index)
{
	if (index == -1)  return;
	if (messageBox::questionHide(this, tr("Remove user"), tr("Are you sure you want to remove ")
		 + server->ConnectionUsers[index].Name + " ?", parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
			 server->ConnectionUsers.removeAt(index);
	updateUsersList();
}




void configwindow::editUser(int)
{
	updateUsersList();
}




void configwindow::startstopserver(int state)
{
	if (state == Qt::Unchecked)
	{
		server->close();
		ui.lineEditServer->setText(tr("tcp remote server is closed"));
	}
	if (state == Qt::Checked)
	{
            if(!server->listen(QHostAddress::Any, ui.spinBoxPortServer->value()))
                ui.lineEditServer->setText(tr("tcp remote server could not be started"));
                else ui.lineEditServer->setText(tr("Server ready port %1, %2 client\n").arg(server->serverPort()).arg(server->clients()));
        }
}




void configwindow::newRequest(QString data)
{
    ui.textEdit->append(data);
}



void configwindow::NewClient(Connection *client)
{
	ui.lineEditServer->setText(tr("Server ready port %1, %2 client\n").arg(server->serverPort()).arg(server->clients()));
	QString Msg = client->getName() + tr(" is connected");
	QDateTime now = QDateTime::currentDateTime();
	parent->logthis(clientfilename, Msg, "");
    QHostAddress adr(client->tcp->peerAddress().toIPv4Address());
    //ui.textEdit->append(now.toString("HH:mm:ss  :  ") + "open socket : " + adr.toString());
    QString currenttext = ui.textEdit->toPlainText();
    if (currenttext.length() > sizeMax) ui.textEdit->setText(currenttext.mid(sizeMax - 1000));
    ui.textEdit->moveCursor(QTextCursor::End);
}




void configwindow::ClientisGone(Connection *client)
{
	ui.lineEditServer->setText(tr("Server ready port %1, %2 client\n").arg(server->serverPort()).arg(server->clients()));
    QDateTime now = QDateTime::currentDateTime();
    QHostAddress adr(client->tcp->peerAddress().toIPv4Address());
    ui.textEdit->append(now.toString("HH:mm:ss  :  ") + "close socket : " + adr.toString());
    if (!client->getName().isEmpty())
	{
		QString Msg = client->getName() + tr(" disconnected");
		parent->logthis(clientfilename, Msg, "");
        ui.textEdit->append(now.toString("dddd dd/MM/yyyy HH:mm  :  ") + Msg);
		QString currenttext = ui.textEdit->toPlainText();
        if (currenttext.length() > sizeMax) ui.textEdit->setText(currenttext.mid(sizeMax - 1000));
		ui.textEdit->moveCursor(QTextCursor::End);
	}
	client->deleteLater();
}




bool configwindow::serverIsListening()
{
	return server->isListening();
}




bool configwindow::saveOnChange()
{
    if (ui.checkBoxSaveOnChange->isChecked()) return true;
    return false;
}





void configwindow::LectureAll()
{
	for (int n=0; n<devicePtArray.count(); n++)
		if (!devicePtArray.at(n)->isVirtualFamily())
		{
			devicePtArray.at(n)->lecture();
			QCoreApplication::processEvents(QEventLoop::AllEvents);
		}
}





void configwindow::LectureRecAll()
{
    foreach (onewiredevice *dev, devicePtArray) { // read counter first
        QString devRomID = dev->getromid();
        if (!devRomID.isEmpty()) {
            QString RomID = devRomID.left(16);
            QString family = RomID.right(2);
            if (family == family2423) dev->lecturerec(); } }
    foreach (onewiredevice *dev, devicePtArray) {
        QString devRomID = dev->getromid();
        if (!devRomID.isEmpty()) {
            QString RomID = devRomID.left(16);
            QString family = RomID.right(2);
            if (family != family2423) dev->lecturerec(); } }
/*    for (int n=0; n<devicePtArray.count(); n++)
		if (!devicePtArray.at(n)->isVirtualFamily())
		{
			devicePtArray.at(n)->lecturerec();
			QCoreApplication::processEvents(QEventLoop::AllEvents);
        }*/
}



void configwindow::convert()
{
	for (int n=0; n<net1wirearray.count(); n++)
		net1wirearray.at(n)->convert();
}





void configwindow::setNextSave()
{
	QFile nextSaveFile("next.sav");
	QTextStream out(&nextSaveFile);
	if(nextSaveFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		for (int n=0; n<devicePtArray.count(); n++)
		{
			out << devicePtArray[n]->getromid() + "#" + devicePtArray[n]->saveInterval.getNext() + "\n";
		}
		nextSaveFile.close();
	}
}




void configwindow::valueTranslate(QString &txt)
{
    QMutexLocker locker(&mutexTranslate);
    QString newTxt = txt;
    for (int n=0; n<devicePtArray.count(); n++)
    {
        QString name = "[[" + devicePtArray.at(n)->getname() + "]]";
        QString romid = "[[" + devicePtArray.at(n)->getromid() + "]]";
        QString value = devicePtArray.at(n)->MainValueToStr();
        int Lname = name.length();
        int Lromid = romid.length();
        bool foundname = true;
        bool foundromid = true;
        while (foundname | foundromid)
        {
            int indexname = newTxt.indexOf(name);
            if (indexname == -1) foundname = false;
            else
            {
                newTxt.remove(indexname, Lname);
                newTxt.insert(indexname, value);
            }
            int indexromid = newTxt.indexOf(romid);
            if (indexromid == -1) foundromid = false;
            else
            {
                newTxt.remove(indexromid, Lromid);
                newTxt.insert(indexromid, value);
            }
        }
        txt.clear();
        txt.append(newTxt);
    }
/*    int index = txt.indexOf("[[");
    if (index != -1)
    {
        int index_end = txt.indexOf("[[", index);
        if (index_end != -1)
        {
            QString str = txt.mid(index, index_end - index);
            txt = txt.remove(index, index_end - index);
            QString tag = QDateTime::toString(str);
            txt = txt.insert(tag);
        }
    }*/
}


void configwindow::replacewebid(QString &datHtml, const QString &id)
{
	QString translatedHtml = datHtml;
	bool found = true;
	int lastPos = 0;
	while (found)
	{
		int index = translatedHtml.indexOf("webid=(", lastPos + 7);
		if (index < 0)
		{
			found = false;
		}
		else
		{
			translatedHtml = translatedHtml.insert(index + 7, id);
			lastPos = index;
		}
//parent->GenMsg("no href found");
//parent->GenMsg("found href link : " + req);
//parent->GenMsg("found href link replaced by : " + link);
	}
	datHtml.clear();
	datHtml.append(translatedHtml);
}





void configwindow::htmlTranslate(QString &datHtml, const QString &id)
{
//parent->GenMsg("htmlTranslate id=" + id);
	QString translatedHtml = datHtml;
	int index = 0;
	int ouv = translatedHtml.indexOf("[[", index);
	while (ouv > 0)
	{
		index = ouv + 2;
		int ferm = translatedHtml.indexOf("]]", index);
		if (ferm > 0)
		{
			QString str = translatedHtml.mid(ouv + 2, ferm - ouv - 2);
			QStringList split = str.split(htmlsperarator);
			if (split.count() == 1)
			{
				onewiredevice *device = DeviceExist(str);
				if (!device) device = Devicenameexist(str);
				if (device)
				{
					QString value = device->MainValueToStr();
					translatedHtml.remove(ouv, ferm - ouv + 2);
					translatedHtml.insert(ouv, value);
				}
			}
			else if (split.count() > 1)
			{
				translatedHtml.remove(ouv, ferm - ouv + 2);
				translatedHtml.insert(ouv, parent->getHtmlValue(str));
			}
		}
		ouv = translatedHtml.indexOf("[[", index);
	}
	replacewebid(translatedHtml, id);
	datHtml.clear();
	datHtml.append(translatedHtml);
}



void configwindow::checkDatZipFile()
{
	QDateTime now = QDateTime::currentDateTime();
	// don't check before 1:00am on the first day of the month to let graph being updated
	if ((now.date().day() == 1) && (now.time().hour() == 0))
	{
		flagCheckZipFiles = true;
		return;
	}
	// Check zip files in enabled
	if (!flagCheckZipFiles) return;
	for (int n=0; n<devicePtArray.count(); n++)
	{
		if (devicePtArray[n]->zipPreviousDatFile()) return;
	}
	// Wait until next month
	flagCheckZipFiles = false;
}




void configwindow::LectureVD()
{
	for (int n=0; n<devicePtArray.count(); n++)
		if (devicePtArray[n]->isVirtualFamily())
			devicePtArray[n]->lecture();
}





void configwindow::LectureRecVD()
{
	for (int n=0; n<devicePtArray.count(); n++)
		if (devicePtArray[n]->isVirtualFamily())
			devicePtArray[n]->lecturerec();
}



bool configwindow::deviceexist(const QString &RomID)
{
	QMutexLocker locker(&mutex);
    if (RomID == "") return false;
	for (int n=0; n<devicePtArray.count(); n++)
        if (devicePtArray[n]->getromid() == RomID) return true;
	return false;
}





bool configwindow::deviceexist(onewiredevice *device)
{
	QMutexLocker locker(&mutex);
	if (!device) return false;
	for (int n=0; n<devicePtArray.count(); n++)
        if (devicePtArray[n] == device) return true;
	return false;
}




onewiredevice *configwindow::DeviceExist(const QString &RomID)
{
    if (deviceList.contains(RomID)) return deviceList[RomID];
    //QMutexLocker locker(&mutex);
    //if (RomID == "") return nullptr;
    //for (int n=0; n<devicePtArray.count(); n++)
    //	if (devicePtArray[n]->getromid() == RomID) return devicePtArray[n];
	return nullptr;
}



onewiredevice *configwindow::EoDeviceExist(const QString &RomID)
{
    QMutexLocker locker(&mutex);
    if (RomID == "") return nullptr;
    if (RomID.contains("_"))
    {
        for (int n=0; n<devicePtArray.count(); n++)
            if ((devicePtArray[n]->getromid().left(10) == RomID.left(10)) && (devicePtArray[n]->getromid().right(4) == RomID.right(4))) return devicePtArray[n];
    }
    else
    {
        for (int n=0; n<devicePtArray.count(); n++)
            if ((devicePtArray[n]->getromid().left(10) == RomID.left(10)) && (devicePtArray[n]->getromid().right(2) == RomID.right(2))) return devicePtArray[n];
    }
    return nullptr;
}




bool configwindow::devicenameexist(const QString &Name)
{
	QMutexLocker locker(&mutex);
	if (Name == "") return true;
	for (int n=0; n<devicePtArray.count(); n++)
			if (devicePtArray[n]->getname() == Name)
				return true;
	return false;
}






onewiredevice *configwindow::Devicenameexist(const QString &Name)
{
	QMutexLocker locker(&mutex);
	if (Name == "") return nullptr;
	for (int n=0; n<devicePtArray.count(); n++)
			if (devicePtArray[n]->getname() == Name)
				return devicePtArray[n];
	return nullptr;
}



QString configwindow::getDeviceName(QString &RomID)
{
	if (RomID == "") return "";
	for (int n=0; n<devicePtArray.count(); n++)
			if (devicePtArray[n]->getromid() == RomID)
				return devicePtArray[n]->getname();
	return "";
}



int configwindow::DeviceCount()
{
	return devicePtArray.count();
}



onewiredevice *configwindow::NewPluginDevice(const QString &RomID, LogisDomInterface *interface)
{
    onewiredevice *device = new onewiredevice(nullptr, parent, RomID);
    if (!device) return nullptr;
    device->logEnabled.hide();
    device->WarnEnabled.hide();
    device->setPluginInterface(interface);
    devicePtArray.append(device);
    deviceList.insert(RomID, device);
    connect(device, SIGNAL(DeviceConfigChanged(onewiredevice*)), parent, SLOT(DeviceConfigChanged(onewiredevice*)));
    device->setHtmlMenulist(ui.listWidget);
    QString configdata;
    parent->get_ConfigData(configdata);
    readconfigfilefordevice(configdata, device);
    emit(newDeviceAdded(device));
    connect(device, SIGNAL(DeviceValueChanged(onewiredevice*)), this, SLOT(DeviceValueChanged(onewiredevice*)));
    connect(device, SIGNAL(DeviceConfigChanged(onewiredevice*)), this, SLOT(DeviceConfigChanged(onewiredevice*)));
    connect(device, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
    connect(device, SIGNAL(saveDat(QString, QString)), parent, SLOT(saveDat(QString, QString)), Qt::QueuedConnection);
    emit(DeviceChanged(device));
    updateDeviceList();
    return device;
}


onewiredevice *configwindow::NewDevice(const QString &newRomID, net1wire *master)
{
    if (deviceexist(newRomID)) return DeviceExist(newRomID);
    //parent->GenMsg(tr("New RomID   ") + newRomID);
    QMutexLocker locker(&mutexNewDevice);
    QString RomID = newRomID;
    //if (parent->diag)
    //if (!QMessageBox::question(this, "Diag mode", "Create new Device " + RomID + " ?", tr("&Non"), tr("&Oui"), QString(), 1, 0)) return nullptr;
    QString family4 = RomID.right(4);
    QString family8 = RomID.right(8);
    QString family = RomID.right(2);
    if (family.left(1) == "_")
    {
        RomID.chop(2);
        family = RomID.right(2);
        family8 = RomID.right(8);
    }
    //parent->GenMsg(tr("Check New RomID   ") + RomID);
// Check CRC for dallas devices
	if ((family == family18B20) or (family == family1822) or (family == family2408) or (family == family2406)
        or (isFamily2413) or (family == family2423) or (family == family2438)
        or (family == family2450) or (family == familyLCDOW))
	{
// 8D00000085C8C126
		bool ok;
		QString ID = RomID.right(14);
		QString CRC = RomID.left(2);
        //int L = (ID.length()/2);
        //byteVec_t romidcrc = byteVec_t(L+1);
        //for (int n=0; n<L; n++) romidcrc[n] = ID.mid((L-n-1)*2, 2).toInt(&ok, 16);
        uint8_t crc = uint8_t(CRC.toUInt(&ok, 16));
        //uint8_t crccalc = onewiredevice::calcCRC8(&romidcrc[0], L);

        QVector <uint8_t> v;
        for (int n=ID.length()-2; n>=0; n-=2) v.append(uint8_t(ID.mid(n, 2).toUInt(&ok, 16)));
        uint8_t crccalc = onewiredevice::calcCRC8(v);

        //qDebug() << "ID=" + ID + "    CRC=" + CRC;
        if (crc != crccalc)
		{
			parent->GenMsg(tr("New 1 Wire device wrong CRC") + " " + RomID + " " + tr("device not created"));
			parent->GenMsg(QString("crc = %1").arg(crc));
			parent->GenMsg(QString("crccalc = %1").arg(crccalc));
            //qDebug() << QString("Wrong CRC " + ID + "    crccalc=%1").arg(crccalc);
            return nullptr;
		}
		else parent->GenMsg(tr("New Dallas device CRC Ok") + " " + RomID);
	}
    RomID = newRomID;
    //qDebug() << family8;
    onewiredevice *device = nullptr;
	if (family == family1820) device = new ds1820(master, parent, RomID);
	else if (family == family18B20) device = new ds18b20(master, parent, RomID);
	else if (family == family1822) device = new ds1822(master, parent, RomID);
    else if (family == family1825) device = new ds1825(master, parent, RomID);
    else if (family == familyLedOW) device = new ledonewire(master, parent, RomID);
    else if (family == familyLCDOW) device = new lcdonewire(master, parent, RomID);
    else if (family == familySTA800) device = new devfts800(master, parent, RomID);
	else if (family == familyMultiGestValve) device = new valveecogest(master, parent, RomID);
	else if (family == familyMultiGestSwitch) device = new switchecogest(master, parent, RomID);
	else if (family == familyMultiGestWarn) device = new signalecogest(master, parent, RomID);
	else if (family == familyMultiGestFlow) device = new flowecogest(master, parent, RomID);
	else if (family == familyMultiGestMode) device = new modecogest(master, parent, RomID);
	else if (family == familyVirtual) device = new devvirtual(parent->RemoteConnection, parent, RomID);
	else if (family == familyTeleInfo) device = new devteleinfo(master, parent, RomID);
	else if (family == familyRps2) device = new devrps2(master, parent, RomID);
	else if (family == familyResol) device = new devresol(master, parent, RomID);
	else if (family == familyMBus) device = new devmbus(master, parent, RomID);
	else if (family == familyModBus) device = new devmodbus(master, parent, RomID);
    else if (family == family2406) device = new ds2406(master, parent, RomID);
    else if (family == family2408) device = new ds2408(master, parent, RomID);
    else if (isFamily2413) device = new ds2413(master, parent, RomID);
    else if (family == family2423) device = new ds2423(master, parent, RomID);
    else if (family == family2438) device = new ds2438(master, parent, RomID);
    else if (family == family2450) device = new ds2450(master, parent, RomID);
    else if (family4 == familyAM12) device = new am12(master, parent, RomID);
	else if (family4 == familyLM12) device = new lm12(master, parent, RomID);
	else if (family4 == familyPLCBUS) device = new devpclbus(master, parent, RomID);
    else if (family8 == familyeoA52001) device = new eoA52001(master, parent, RomID);   // Vanne Chauffage EnOcean
    else if (family8 == familyeoD2010F) device = new eoD2010F(master, parent, RomID);   // Switch 1 voie EnOcean
    else if (family8 == familyeoD20112) device = new eoD20112(master, parent, RomID);   // Switch 2 voies EnOcean
    else if ((family8.left(4) == QString(familyeoA504XX).left(4)) && (family8.right(2) == familyEOcean)) device = new eoA504xx(master, parent, RomID);  // Thermometre Humidité EnOcean
    else if ((family8.left(4) == QString(familyeoA502XX).left(4)) && (family8.right(2) == familyEOcean)) device = new eoA502xx(master, parent, RomID);  // Thermometre EnOcean
    else if ((family8.left(2) == QString(familyeoF6XXXX).left(2)) && (family8.right(2) == familyEOcean)) device = new eoF6XXXX(master, parent, RomID);  // Switch EnOcean
    else return nullptr; //device = new onewiredevice(master, RomID);
    if (!device) return nullptr;
	devicePtArray.append(device);
    deviceList.insert(RomID, device);
	connect(device, SIGNAL(DeviceConfigChanged(onewiredevice*)), parent, SLOT(DeviceConfigChanged(onewiredevice*)));
	if (!parent->isRemoteMode())
	{
            device->setHtmlMenulist(ui.listWidget);
            QString configdata;
            parent->get_ConfigData(configdata);
            readconfigfilefordevice(configdata, device);
	}
	emit(newDeviceAdded(device));
	connect(device, SIGNAL(DeviceValueChanged(onewiredevice*)), this, SLOT(DeviceValueChanged(onewiredevice*)));
	connect(device, SIGNAL(DeviceConfigChanged(onewiredevice*)), this, SLOT(DeviceConfigChanged(onewiredevice*)));
	connect(device, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
    connect(device, SIGNAL(saveDat(QString, QString)), parent, SLOT(saveDat(QString, QString)), Qt::QueuedConnection);
    emit(DeviceChanged(device));
	return device;
}


QString unaccent(const QString s)
{
  QString s2 = s.normalized(QString::NormalizationForm_D);
  QString out;
  for (int i=0,j=s2.length(); i<j; i++)
  {
    // strip diacritic marks
    if (s2.at(i).category()!=QChar::Mark_NonSpacing &&
        s2.at(i).category()!=QChar::Mark_SpacingCombining)
    {
         out.append(s2.at(i));
    }
  }
  return out;
}

void configwindow::updateDeviceList()
{
	QMutexLocker locker(&mutex);
	OneWireList.clear();
    for (int i=0; i<OneWireTree.topLevelItemCount(); i++)
    {
        while (OneWireTree.topLevelItem(i)->childCount())
        {
            OneWireTree.topLevelItem(i)->removeChild(OneWireTree.topLevelItem(i)->child(0));
        }
    }
    //OneWireTree.clear();
    for (int n=0; n<devicePtArray.count(); n++)
	{
        QString name = devicePtArray.at(n)->getname();
        QString uname = unaccent(name);
        //QString normalisedName(name.normalized(QString::NormalizationForm_D));
        devicePtArray.at(n)->clearListTreeItem();
        //if (Listfilter.text().isEmpty() || normalisedName.replace(QRegExp("[^a-zA-Z\\s]"), "").contains(Listfilter.text(), Qt::CaseInsensitive))
        if (Listfilter.text().isEmpty() || uname.contains(unaccent(Listfilter.text()), Qt::CaseInsensitive))
        {
            QString txt = devicePtArray.at(n)->getromid() + "  "  + name;
            QString val = devicePtArray.at(n)->MainValueToStr();
            QString masterStr = devicePtArray.at(n)->getMasterName();
            // add to list view
            QListWidgetItem *widgetList = new QListWidgetItem(&OneWireList);
            widgetList->setText(txt);
            // add to tree view
            if (!masterStr.isEmpty())
            {
                QTreeWidgetItem *masterItem = nullptr;
                for (int i=0; i<OneWireTree.topLevelItemCount(); i++)		// check if master exist
                    if (OneWireTree.topLevelItem(i)->text(0) == masterStr) masterItem = OneWireTree.topLevelItem(i);
                if (!masterItem)
                {
                    masterItem = new QTreeWidgetItem(&OneWireTree, 0);
                    masterItem->setExpanded(true);
                    masterItem->setText(0, masterStr);
                }
                QTreeWidgetItem *item = new QTreeWidgetItem(masterItem, 0);
                item->setText(0, txt);
                item->setText(1, val);
                item->setText(2, QString("%1").arg(n));
                devicePtArray.at(n)->setListTreeItem(item);
            }
            else
            {
                masterStr = tr("Virtual Devices");
                QTreeWidgetItem *masterItem = nullptr;
                for (int i=0; i<OneWireTree.topLevelItemCount(); i++)		// check if master exist
                    if (OneWireTree.topLevelItem(i)->text(0) == masterStr) masterItem = OneWireTree.topLevelItem(i);
                if (!masterItem)
                {
                    masterItem = new QTreeWidgetItem(&OneWireTree, 0);
                    masterItem->setExpanded(true);
                    masterItem->setText(0, masterStr);
                }
                QTreeWidgetItem *item = new QTreeWidgetItem(masterItem, 0);
                item->setExpanded(true);
                item->setText(0, txt);
                item->setText(1, val);
                item->setText(2, QString("%1").arg(n));
                devicePtArray.at(n)->setListTreeItem(item);
            }
        }
	}
	for (int n=0; n<net1wirearray.count(); n++) net1wirearray[n]->UpdateLocalDeviceList();
}






void configwindow::setPalette(int index)
{
	if (index == -1) return;
	if (index < devicePtArray.count())
		parent->setPalette(&devicePtArray[index]->setup);
}





void configwindow::setTreePalette(QTreeWidgetItem *item, int)
{
	if (!item) return;
	bool ok;
	int index = item->text(2).toInt(&ok);
	if (!ok) return;
	if (index < devicePtArray.count())
		parent->setPalette(&devicePtArray[index]->setup);
}



void configwindow::setPalette(QListWidgetItem* item)
{
	for (int n=0; n<OneWireList.count(); n++)
		if (item == OneWireList.item(n))
			parent->setPalette(&devicePtArray[n]->setup);
}




void configwindow::DeviceConfigChanged(onewiredevice *device)
{
	emit(DeviceChanged(device));
	updateDeviceList();
}



void configwindow::DeviceValueChanged(onewiredevice *device)
{
	emit(DeviceChanged(device));
}


void configwindow::ProgHasChanged(ProgramData *prog)
{
	emit(ProgChanged(prog));
}



void configwindow::dailyHasChange(daily *Daily)
{
	configmanager->ChangeDaily(Daily);
}




void configwindow::RemoveDaily(daily *Daily)
{
	configmanager->RemoveDaily(Daily);
}



void configwindow::RemovePrgEvt(ProgramData *prog)
{
	configmanager->RemovePrgEvt(prog);
	configmanagerTr->RemovePrgEvt(prog);
}






void configwindow::DisplayDevice(int index)
{
	if (index < 0) return;
	devicePtArray[index]->show();
}





void configwindow::ShowDevice()
{
	QModelIndex I = OneWireList.currentIndex();
	int index = I.row();
	if (index == -1) return;
	devicePtArray[index]->hide();
	devicePtArray[index]->show();
}




void configwindow::mousePressEvent(QMouseEvent *)
{
/*	if (event->button() == Qt::RightButton)
		{
		 QMenu contextualmenu;
		QAction Add(tr("&Add Master"), this);
		 QAction Remove(tr("&Remove Master"), this);
		 QAction Rename(tr("&Rename Master"), this);
		 QAction AddVD(tr("&Add Virtual Device"), this);
         QAction LoadVD(tr("&Load Virtual Device description"), this);
         QAction RemoveVD(tr("&Remove Virtual Device"), this);
		 contextualmenu.addAction(&Add);
         if (ui.tabWidget->currentIndex() >= LocalTabsNumber) contextualmenu.addAction(&Remove);
         if (ui.tabWidget->currentIndex() >= LocalTabsNumber) contextualmenu.addAction(&Rename);
		 contextualmenu.addAction(&AddVD);
		 contextualmenu.addAction(&RemoveVD);
         contextualmenu.addAction(&LoadVD);
         QAction *selection;
		 selection = contextualmenu.exec(event->globalPos());
         if (selection == &Add) EnOceanType();
		 if (selection == &Remove) supprimer();
		 if (selection == &Rename) renommer();
		 if (selection == &AddVD) addVD();
         if (selection == &LoadVD) loadVD();
         if (selection == &RemoveVD) removeVD();
		}
    if (event->button() != Qt::LeftButton) return;*/
}





void configwindow::loadVD()
{
    bool ok;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load device description"), "", "");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    if (textCodec) in.setCodec(textCodec);
    QString text = in.readAll();
    file.close();
    QString check = One_Wire_Device;
    if (!text.mid(0, 20).contains(check))
    {
        messageBox::warningHide(this, tr("Wrong file"), tr("This file is corrupted") + text.mid(0, 10), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }
    QString finalText;
    QTextStream txt(&text);
    while (!txt.atEnd())
    {
        QString line = txt.readLine();
        QString f = logisdom::getvalue("Formula", line);
        if (f.isEmpty())
        {
            finalText.append(line);
        }
        else
        {
            int index = 0;
            int open = f.indexOf("[[");
            index = open + 2;
            QList <onewiredevice*> devicelist;
            QStringList deviceList;
            for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
            {
                deviceList << parent->configwin->devicePtArray[n]->getname();
                devicelist.append(parent->configwin->devicePtArray.at(n));
            }
            while(open != -1)
            {
                int close = f.indexOf("]]", index);
                if (close != -1)
                {
                    QString name = f.mid(open + 2, close - open - 2);
                    bool ok;
                    int i = -1;
                    QString devicechoise = inputDialog::getItemPalette(this, tr("Select device "), tr("Select device for ") + name, deviceList, i, false, &ok, Qt::Dialog, parent);
                    if (!ok) return;
                    onewiredevice *device = parent->configwin->Devicenameexist(devicechoise);
                    if (device) f = f.replace("[[" + name + "]]", device->getromid());
                    else return;
                }
                open = f.indexOf("[[", index);
                index = open + 2;
            }
            finalText.append(logisdom::saveformat("Formula", f, true));
        }
    }
Retry:
    QString nom = inputDialog::getTextPalette(this, tr("New virtual device name"), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
    if (!ok) return;
    if (nom.isEmpty()) return;
    if (parent->configwin->devicenameexist(nom))
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        goto Retry;
    }
    onewiredevice *device;
    for (int id=1; id<999; id++)
    {
        QString RomID = ("00000000" + QString("%1").arg(id, 3, 10, QChar('0')) + familyVirtual);
        if (!deviceexist(RomID))
        {
            device = NewDevice(RomID, nullptr);
            if (device)
            {
                device->setCfgStr(finalText);
                device->setname(nom);
                device->show();
            }
            break;
        }
    }
}


void configwindow::findCodecs()
{
    QMap<QString, QTextCodec *> codecMap;
    QRegularExpression iso8859RegExp("^ISO[- ]8859-([0-9]+).*$");
    QRegularExpressionMatch match;

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith(QLatin1String("UTF-8"))) {
            rank = 1;
        } else if (sortKey.startsWith(QLatin1String("UTF-16"))) {
            rank = 2;
        } else if ((match = iso8859RegExp.match(sortKey)).hasMatch()) {
            if (match.captured(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QLatin1Char('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    codecs = codecMap.values();
    ui.comboBoxCodecs->clear();
    foreach (QTextCodec *codec, codecs) ui.comboBoxCodecs->addItem(codec->name(), codec->mibEnum());
}



void configwindow::setCodec(QString codec)
{
    if (codec.isEmpty()) ui.comboBoxCodecs->setCurrentIndex(ui.comboBoxCodecs->findText("UTF-8")); //By default, codec used is the codec of the system.
    else
    {
        int index = ui.comboBoxCodecs->findText(codec);
        if (index != -1)
        {
            ui.comboBoxCodecs->setCurrentIndex(index);
        }
        else textCodec = nullptr;
    }
}




void configwindow::addVDev()
{
    bool ok;
Retry :
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
    if (!ok) return;
    if (nom.isEmpty()) return;
    if (parent->configwin->devicenameexist(nom))
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        goto Retry;
    }
    onewiredevice *device = nullptr;
    for (int id=1; id<999; id++)
    {
        QString RomID = ("00000000" + QString("%1").arg(id, 3, 10, QChar('0')) + familyVirtual);
        if (!deviceexist(RomID))
        {
            device = NewDevice(RomID, nullptr);
            if (device) device->setname(nom);
            if (device) device->show();
            parent->setPalette(&device->setup);
            parent->PaletteHide(false);
            hide();
            return;
        }
    }
}



onewiredevice *configwindow::addVD(QString name)
{
    bool ok;
    QString nom = name;
Retry :
    if (name.isEmpty())
    {
        nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
        if (!ok) return nullptr;
        if (nom.isEmpty()) return nullptr;
        if (parent->configwin->devicenameexist(nom))
        {
            messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            goto Retry;
        }
    }
    else
    {
        int index = 1;
        QString newName = name + QString("_%1").arg(index);
        while (parent->configwin->devicenameexist(newName))
        {
            index ++;
            newName = name + QString("_%1").arg(index);
        }
        nom = newName;
    }
	onewiredevice *device;
	for (int id=1; id<999; id++)
	{
		QString RomID = ("00000000" + QString("%1").arg(id, 3, 10, QChar('0')) + familyVirtual);
		if (!deviceexist(RomID))
		{
			device = NewDevice(RomID, nullptr);
			if (device) device->setname(nom);
			if (device) device->show();
            return device;
		}
	}
    return nullptr;
}



void configwindow::removeVD()
{
	bool ok;
	QStringList items;
	for (int n=0; n<devicePtArray.count(); n++)
		if (devicePtArray[n]->getromid().right(2) == familyVirtual)
			items << devicePtArray[n]->getname();
	if (items.count() == 0) return;
	QString item = inputDialog::getItemPalette(this, tr("Remove Virtual Device"), tr("Device : "), items, 0, false, &ok, Qt::Dialog, parent);
	if (!ok) return;
	if (item.isEmpty()) return;
	if ((messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to ") + item, parent, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes)) return;
	onewiredevice *device = parent->configwin->Devicenameexist(item);
	if (device)
	{
		device->stopAll();
		parent->removeWidget(&device->setup);
		device->setAttribute(Qt::WA_DeleteOnClose, true);
		int index = devicePtArray.indexOf(device);
		if (index != -1)
		{
			net1wire *master = devicePtArray.at(index)->getMaster();
			if (master)
			{
				/// remove from localdevice
			}
			devicePtArray.removeAt(index);
            deviceList.remove(device->getromid());
			updateDeviceList();
			device->close();
		}
	}
}



void configwindow::renameTab(int id)
{
   if (id < LocalTabsNumber) return;
    if (parent->isLocked()) return;
    QDialog dlg;
    QVBoxLayout la(&dlg);
    QLineEdit ed;
    la.addWidget(&ed);
    QDialogButtonBox bb(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    la.addWidget(&bb);
    dlg.setLayout(&la);
    ed.setText(ui.tabWidget->tabText(id));
    connect(&bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if(dlg.exec() == QDialog::Accepted)
    {
        for (int n=LocalTabsNumber; n<ui.tabWidget->count(); n++)
            if (ui.tabWidget->tabText(id) == ed.text()) return;
        ui.tabWidget->setTabText(id, ed.text());
        net1wirearray.at(id-LocalTabsNumber)->setName(ed.text());
    }
    //qDebug() << "Rename : " + ed.text();
}



void configwindow::supprimer()
{
    int index = ui.tabWidget->currentIndex();
    if (index < LocalTabsNumber) return;
    if (messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to ") + ui.tabWidget->tabText(index), parent, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes) return;
    ui.tabWidget->removeTab(index);
    net1wirearray.at(index - LocalTabsNumber)->setTobeDeleted();
    messageBox::warningHide(this, "Application restart", "You must save and restart the application\nto remove definitely the controler", parent, QMessageBox::Ok);
}




void configwindow::ajouter()
{
	bool ok;
Retry :
    int item = ui.comboBoxInterface->currentIndex();
    if (item < 0) return;
    item += 1;
    QString nom = inputDialog::getTextPalette(this, NetTypeStr[item], cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	for (int i=0; i<ui.tabWidget->count(); i++)
		if (ui.tabWidget->tabText(i) == nom)
		{
            messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
			goto Retry;
		}
    int port = 0;
    QString ip = "000.000.000.000";
    if (item != EOceanType)
    {
        ip = inputDialog::getTextPalette(this, tr("IP Address"), tr("IP Address"), QLineEdit::Normal, ip, &ok, parent);
        if (!ok) return;
        if (ip.isEmpty()) return;
        if (item == Ha7Net) port = QInputDialog::getInt(this, tr("Port"), tr("Port "), ha7net::default_port, 0, 99999, 1, &ok);
        else if (item == TCP_ResolType) port = QInputDialog::getInt(this, tr("Port"), tr("Port "), resol::default_port, 0, 99999, 1, &ok);
        else port = inputDialog::getIntegerPalette(this, tr("Port"), tr("Port "), default_port_EZL50, 0, 99999, 1, &ok, Qt::Dialog, parent);
        if (!ok) return;
    }
    else
    {
        ip = SER_PORT;
    }
    net1wire * master;
    if (item == Ha7Net) master = newmaster(nom, Ha7Net);
    else if (item == Ezl50_FTS800) master = newmaster(nom, Ezl50_FTS800);
    else if (item == Ezl50_X10) master = newmaster(nom, Ezl50_X10);
    else if (item == Ezl50_PlcBus) master = newmaster(nom, Ezl50_PlcBus);
    else if (item == MultiGest) master = newmaster(nom, MultiGest);
    else if (item == RemoteType) master = newmaster(nom, RemoteType);
    else if (item == MBusType) master = newmaster(nom, MBusType);
    else if (item == TeleInfoType) master = newmaster(nom, TeleInfoType);
    else if (item == TCP_RPS2Type) master = newmaster(nom, TCP_RPS2Type);
    else if (item == TCP_HA7SType) master = newmaster(nom, TCP_HA7SType);
    else if (item == TCP_ResolType) master = newmaster(nom, TCP_ResolType);
    else if (item == ModBus_Type) master = newmaster(nom, ModBus_Type);
    else if (item == EOceanType) master = newmaster(nom, EOceanType);
    else return;
	if (master == nullptr) return;
	master->setipaddress(ip);
	master->setport(port);
	master->init();
    ui.tabWidget->setCurrentIndex(ui.tabWidget->count() - 1);
}





net1wire *configwindow::newmaster(QString Name, int Type)
{
	net1wire *master = nullptr;
	QUuid uid;
	QString Uid;
	bool found = false;
	do
	{
		found = false;
		Uid = uid.createUuid().toString();
		Uid = Uid.remove("}");
		Uid = Uid.right(8).toUpper();
		for (int n=0; n<net1wirearray.count(); n++)
		{
			if (net1wirearray[n]->getUid() == Uid) found = true;
		}
	} while (found);
	if (Name == "") Name = "sans nom";
	int n = net1wireList.findText(Name);
	if (n != -1)
	{
		int index = 1;
		int lastunderscore = Name.lastIndexOf("_");
		if (lastunderscore == -1) Name += QString("_1");
		n = net1wireList.findText(Name);
		while (n != -1)
		{
			lastunderscore = Name.lastIndexOf("_");
			Name = Name.mid(0, lastunderscore) + QString("_%1").arg(index);
			n = net1wireList.findText(Name);
			 index ++;
		}
	}
	switch (Type)
	{
		case Ha7Net :			master = new ha7net(parent); break;
		case MBusType :			master = new mbus(parent); break;
		case Ezl50_FTS800 :		master = new fts800(parent); break;
        case Ezl50_X10 :		master = new x10(parent); break;
		case Ezl50_PlcBus :		master = new plcbus(parent); break;
		case MultiGest :		master = new ecogest(parent); break;
		case TeleInfoType :		master = new teleinfo(parent); break;
		case TCP_RPS2Type :		master = new rps2(parent); break;
		case TCP_HA7SType :		master = new ha7s(parent); break;
        case TCP_ResolType :	master = new resol(parent); break;
		case ModBus_Type :		master = new modbus(parent); break;
        case EOceanType :       master = new eocean(parent); break;
        default:                master = new net1wire(parent); break;
	}
	if (!master) return nullptr;
	net1wirearray.append(master);
	master->setAccessibleName(Name);
	master->setname(Name);
	master->setunID(Uid);
	net1wireList.addItem(Name);
	connect(master, SIGNAL(TcpStateConnected(net1wire*)), this, SLOT(TcpStateConnected(net1wire*)));
	connect(master, SIGNAL(TcpStateDisonnected(net1wire*)), this, SLOT(TcpStateDisonnected(net1wire*)));
	ui.tabWidget->addTab(master, master->accessibleName());
	QCoreApplication::processEvents(QEventLoop::AllEvents);
	return master;
}






void configwindow::TcpStateConnected(net1wire *master)
{
	int index = ui.tabWidget->indexOf(master);
	if (index != -1) ui.tabWidget->setTabIcon(index, QIcon(QPixmap(QString::fromUtf8(":/images/images/button_ok.png"))));
}




void configwindow::TcpStateDisonnected(net1wire *master)
{
	int index = ui.tabWidget->indexOf(master);
	if (index != -1) ui.tabWidget->setTabIcon(index, QIcon(QPixmap(QString::fromUtf8(":/images/images/critical.png"))));
}




bool configwindow::isUploading()
{
	for (int n=0; n<net1wirearray.count(); n++)
	if (net1wirearray[n]->isUploading()) return true;
	return false;
}





net1wire *configwindow::MasterExist(const QString &IPHex)
{
	if (IPHex == "") return nullptr;
	for (int n=0; n<net1wirearray.count(); n++)
	{
		QString IP = net1wirearray[n]->getipaddress();
		QString toHex = net1wire::IPtoHex(IP);
		if (toHex == IPHex) return net1wirearray[n];
	}
	return nullptr;
}





void configwindow::readconfigfile(QString &configdata)
{
	bool ok;
    //QFile Cfgfile(parent->configfilename);
    QFile Cfgfile("remote.cfg");
    //if (Cfgfile.open(QIODevice::ReadOnly | QIODevice::Text))
    if (Cfgfile.exists())
        if (messageBox::questionHide(this, tr("No configuration file ?"), tr("No configuration file was found\nDo you want to use remote connection ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
		{
			QString ip = "remyfr.dnsalias.com";
            ip = inputDialog::getTextPalette(this, tr("IP Address"), tr("IP Address"), QLineEdit::Normal, ip, &ok, parent);
			if (!ok) return;
			if (ip.isEmpty()) return;
			int port = inputDialog::getIntegerPalette(this, tr("Port"), tr("Port "), default_port_remote, 0, 99999, 1, &ok, Qt::Dialog, parent);
			if (!ok) return;
			if (ip.isEmpty()) return;
			QString rmstr =  tr("Remote Connection");
			QString name, password;
nameagain:
            name = inputDialog::getTextPalette(this, tr("Name"), tr("Please enter your name : "), QLineEdit::Normal, name, &ok, parent);
			if (!ok) return;
			if (name.isEmpty()) 
			{
				if (messageBox::questionHide(this, tr("Name is empty !"), tr("Name cannot be empty\nDo you want to abort ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
				return;
				else goto nameagain;
			}
			if (name.isEmpty()) return;
passwordagain:
            password = inputDialog::getTextPalette(this, tr("Password"), tr("Please enter the password : "), QLineEdit::Password, password, &ok, parent);
			if (!ok) return;
			if (password.isEmpty()) 
			{
				if ((messageBox::questionHide(this, tr("Password is empty !"), tr("Password cannot be empty\nDo you want to abort ?"), parent,QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes))
				return;
				else goto passwordagain;
			}
			if (password.isEmpty()) return;
			parent->RemoteConnection = new remote(parent);
			if (!parent->isRemoteMode()) return;
			parent->RemoteConnection->setAccessibleName(rmstr);
			parent->RemoteConnection->setname(rmstr);
			ui.tabWidget->addTab(parent->RemoteConnection, rmstr);
            parent->RemoteConnection->setipaddress(ip);
			parent->RemoteConnection->setport(port);
            parent->RemoteConnection->init(name, password);
            ui.tabWidget->setCurrentIndex(ui.tabWidget->count() - LocalTabsNumber);
			ui.checkBoxServer->setEnabled(false);
			return;
		}
    //Cfgfile.close();
	setRemoteMode(configdata);
	if (parent->isRemoteMode()) return;
	setServerMode(configdata);
    readPathConfig(configdata);
// Check Net 1 Wire modules
	QString TAG_Begin = Net1Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	int Typeint = logisdom::getvalue("Type", strsearch).toInt();
	QString Type = NetTypeStr[Typeint];
	QString  Name = logisdom::getvalue("TabName", strsearch);
    if ((Type != "") and (Typeint !=  -1))
	{
		net1wire *master = newmaster(Name, Typeint);
		if (master != nullptr)
			master->setCfgStr(strsearch);
	}
	SearchLoopEnd
    readPngConfig(configdata);
    readHtmlConfig(configdata);
    readHtmlSetupConfig(configdata);
    readGeneralConfig(configdata);
    server->readconfigfile(configdata);
    updateUsersList();
}





void configwindow::readHtmlSetupConfig(QString &configdata)
{
	QString comboSelection;
	disconnect(ui.comboBoxHtml, SIGNAL(currentIndexChanged(int)), this, SLOT(changeHtmlSetup(int)));
	QString TAG_Begin = "HTMLSetup_Config";
	QString TAG_End = "HTMLSetup_End";
	SearchLoopBegin
	comboSelection = logisdom::getvalue("HtmlComboSelection", strsearch);
	QByteArray data;
	data.append(strsearch);
    readHtmlSetupBlock(data);
	SearchLoopEnd
	int i = ui.comboBoxHtml->findText(comboSelection);
	connect(ui.comboBoxHtml, SIGNAL(currentIndexChanged(int)), this, SLOT(changeHtmlSetup(int)));
	if (i != -1) ui.comboBoxHtml->setCurrentIndex(i);
    changeHtmlSetup(0);
}






void configwindow::readHtmlSetupBlock(QByteArray &configdata)
{
    QString TAG_Begin = "HTMLBlock_Begin";
    QString TAG_End = "HTMLBlock_End";
    SearchLoopBegin
    QString ComboName = logisdom::getvalue("ComboName", strsearch);
    ui.comboBoxHtml->addItem(ComboName);
    htmlSetup *setup = new htmlSetup;
    if (setup)
    {
        htmlLayout.append(setup);
        setup->welcomeMessage = logisdom::getvalue("HtmlWelcomeMessage", strsearch);
        setup->htmlTitle = logisdom::getvalue("HtmlTitle", strsearch);
        setup->htmlHeader = logisdom::getvalue("HtmlHeader", strsearch);
        setup->htmlTime = logisdom::getvalue("HtmlTime", strsearch);
        setup->htmlMenu = logisdom::getvalue("HtmlMenu", strsearch);
        setup->htmlList = logisdom::getvalue("HtmlList", strsearch);
        setup->htmlDetector = logisdom::getvalue("HtmlDetector", strsearch);
        setup->htmlValue = logisdom::getvalue("HtmlValue", strsearch);
        setup->htmlCommand = logisdom::getvalue("HtmlCommand", strsearch);
    }
    SearchLoopEnd
}





void configwindow::readGeneralConfig(QString &configdata)
{
    bool ok;
    QString TAG_Begin = "GENERAL_Config";
    QString TAG_End = "GENERAL_End";
    SearchLoopBegin
    int SaveOnQuit = logisdom::getvalue("SaveOnQuit", strsearch).toInt(&ok);
    if ((ok) && (SaveOnQuit)) ui.checkBoxSaveQuit->setCheckState(Qt::Checked);
    int AskBeforeSave = logisdom::getvalue("AskBeforeSave", strsearch).toInt(&ok);
    if ((ok) && (AskBeforeSave)) ui.checkBoxAskSave->setCheckState(Qt::Checked);
    int AutoSave = logisdom::getvalue("AutoSave", strsearch).toInt(&ok);
    if ((ok) && (AutoSave)) ui.checkBoxkSaveInterval->setCheckState(Qt::Checked);
    int SaveChange = logisdom::getvalue("SaveChange", strsearch).toInt(&ok);
    if ((ok) && (SaveChange)) ui.checkBoxSaveOnChange->setCheckState(Qt::Checked);
    int SaveInterval = logisdom::getvalue("SaveInterval", strsearch).toInt(&ok);
    if ((ok) && (SaveInterval)) ui.spinBoxSaveInterval->setValue(SaveInterval);
    int HideHeatingTab = logisdom::getvalue("HideHeatingTab", strsearch).toInt(&ok);
    if ((ok) && (HideHeatingTab)) ui.checkBoxHideHeating->setCheckState(Qt::Checked);
    int LogFileSize = logisdom::getvalue("LogFileSize", strsearch).toInt(&ok);
    if ((ok) && (LogFileSize)) ui.spinBoxLogSize->setValue(LogFileSize);
    int ColWidth = logisdom::getvalue("TreeDevivceColumWidth", strsearch).toInt(&ok);
    if ((ok) && (ColWidth > 20)) OneWireTree.setColumnWidth(0, ColWidth);
    else OneWireTree.setColumnWidth(0, 200);
    QString codec = logisdom::getvalue("TextCodec", strsearch);
    if (!codec.isEmpty()) setCodec(codec);
    SearchLoopEnd
}




void configwindow::readPathConfig(const QString &configdata)
{
    bool ok;
    enablePathConfig();
    QString TAG_Begin = "PATH_Config";
    QString TAG_End = "PATH_End";
    SearchLoopBegin
    ui.lineEditDataFolder->setText(logisdom::getvalue("DataFolderPath", strsearch));
    ui.lineEditZipFolder->setText(logisdom::getvalue("ZipFolderPath", strsearch));
    ui.lineEditBackupFolder->setText(logisdom::getvalue("BackupFolderPath", strsearch));
    ui.lineEditHtmlFolder->setText(logisdom::getvalue("BackupHtmlPath", strsearch));
    int index = logisdom::getvalue("BackupInterval", strsearch).toInt(&ok);
    if ((ok) && (index >= 0) && (index < 3)) ui.comboBoxBackupInterval->setCurrentIndex(index);
    ui.timeEditBackupInterval->setTime(QTime::fromString(logisdom::getvalue("BackupTime", strsearch)));
    index = logisdom::getvalue("SaveConfigInterval", strsearch).toInt(&ok);
    SearchLoopEnd
    DataPathChanged();
    ZipPathChanged();
    BackupPathChanged();
    HtmlPathChanged();
    QDateTime now = QDateTime::currentDateTime();
    nextBackup = now;
    nextBackup.setTime(ui.timeEditBackupInterval->time());
    if (nextBackup.secsTo(now) > 0) nextBackup = nextBackup.addDays(1);
}



void configwindow::enablePathConfig()
{
    ui.lineEditDataFolder->setEnabled(true);
    ui.lineEditZipFolder->setEnabled(true);
    ui.lineEditBackupFolder->setEnabled(true);
    ui.pushButtonDataPath->setEnabled(true);
    ui.pushButtonZipPath->setEnabled(true);
    ui.pushButtonBackupPath->setEnabled(true);
    ui.comboBoxBackupInterval->setEnabled(true);
    ui.comboBoxBackupInterval->addItem(tr("Never"));
    ui.comboBoxBackupInterval->addItem(tr("Every day at"));
    //ui.comboBoxBackupInterval->addItem(tr("Every week at "));
    //ui.comboBoxSaveConfig->setEnabled(true);
    //ui.comboBoxSaveConfig->addItem(tr("Never"));
    //ui.comboBoxSaveConfig->addItem(tr("Every day at"));
    //ui.comboBoxSaveConfig->addItem(tr("Every"));
}




void configwindow::getHtmlLinks(QString user, QString password, QString &str)
{
    // http://remyfr.dnsalias.com:1220/request=(GetMainMenu)user=(demo)password=(demo)
    str += "\n\n" + tr("Main page html automated : ");
    //str += "http://" + ui.lineEditAuthLogin->text() + ":" + QString("%1").arg(ui.spinBoxPortServer->value()) + "/request=(GetMainMenu)user=(" + user + ")" + "password=(" + password + ")\n";

    int htmlcount = htmlBindWebMenu->treeItem->childCount();
    str += QString ("\n%1 ").arg(htmlcount) + tr("custom html pages") + "\n";
    //for (int n=0; n<htmlcount; n++)
    //    str += "http://" + ui.lineEditAuthLogin->text() + ":" + QString("%1").arg(ui.spinBoxPortServer->value()) + "/request=(" + htmlBindWebMenu->treeItem->child(n)->text(0) +")user=(" + user + ")" + "password=(" + password + ")\n";
}




void configwindow::updateNow()
{
    parent->update(true);
}



void configwindow::backupTimerTimeOut()
{
    if (fileBackup.isRunning()) return;
    nextBackup.setTime(ui.timeEditBackupInterval->time());
    if (nextBackup.secsTo(QDateTime::currentDateTime()) < 0) return;
    nextBackup = nextBackup.addDays(1);
    backupNow();
}


void configwindow::backupNow()
{
    if (fileBackup.isRunning())
    {
        fileBackup.abort = true;
    }
    else
    {
        ui.PushButtonBackupNow->setText(tr("Runing"));
        fileBackup.datFolder = parent->repertoiredat;
        fileBackup.zipFolder = parent->repertoirezip;
        fileBackup.iconFolder = repertoireicon;
        fileBackup.iconFolder.append(QDir::separator());
        fileBackup.configFileName = parent->configfilename;
        fileBackup.backupFolder = parent->repertoirebackup;
        fileBackup.start();
    }
}



void configwindow::backupFinished()
{
    ui.PushButtonBackupNow->setEnabled(true);
    ui.PushButtonBackupNow->setText(boutonBackupStr);
}


void configwindow::sendHtmlMail()
{
    eMailSender.smtpServer = ui.lineEditSMTPServer->text();
    if (eMailSender.smtpServer.isEmpty())
    {
        messageBox::warningHide(this, tr("SMTP Empty"), tr("SMTP Server is Empty\n"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }
    eMailSender.eMailSender = ui.lineEditSMTPLogin->text();
    if (eMailSender.eMailSender.isEmpty())
    {
        messageBox::warningHide(this, tr("SMTP Login Empty"), tr("SMTP Login is Empty\nIt will be use to send the email"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }
    if (ui.checkBoxSMTPSecure->isChecked())
    {
        if (ui.lineEditSMTPPort->text().isEmpty()) eMailSender.smtpPort = "465"; else eMailSender.smtpPort = ui.lineEditSMTPPort->text();
        //qDebug() <<  eMailSender.smtpPort;
        eMailSender.smptPassword = ui.lineEditSMTPPassWord->text();
        if (eMailSender.smptPassword.isEmpty())
        {
            messageBox::warningHide(this, tr("Password Empty"), tr("Password is Empty\n"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            return;
        }
    }
    else
    {
        if (ui.lineEditSMTPPort->text().isEmpty()) eMailSender.smtpPort = "25"; else eMailSender.smtpPort = ui.lineEditSMTPPort->text();
    }
    //QStringList mailTo;
   // mailTo.append(address);
    //QString smtp = ui.lineEditSmpt->text();
    //QString mailpass = ui.lineEditSMTPPassWord->text();
    /*if (mailPsw.isEmpty())
    {
        messageBox::warningHide(this, tr("Password id not set"), tr("Please fill a password\n"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }*/
    ui.pushButtonSendMail->setEnabled(false);
    QString textSubject = "LogisDom Html links "; // + ui.lineEditHtmlTitleText->text();
    QString textMail;
    if (server->ConnectionUsers.count() == 0)
    {
        textMail += tr("No user is defined, no link can be generated");
    }
    int pngcount = parent->IconeAreaList.count();
    textMail += QString ("\n%1 ").arg(pngcount) + tr("PNG pictures") + "\n";
    //for (int n=0; n<pngcount; n++)
    //    textMail += "http://" + ui.lineEditAuthLogin->text() + ":" + QString("%1").arg(ui.spinBoxPortServer->value()) + "/" + parent->getTabName(n) + ".png\n";
    textMail += "\n\n";
    int count = server->ConnectionUsers.count();
    for (int n=0; n<count; n++)
    {
        if (server->ConnectionUsers.at(n).Rigths == Server::FullControl)    // admin link
        {
            textMail += tr("Admin html links");
            getHtmlLinks(server->ConnectionUsers.at(n).Name, server->ConnectionUsers.at(n).PassWord, textMail);
            break;
        }
    }
    for (int n=0; n<count; n++)
    {
        if (server->ConnectionUsers.at(n).Rigths == Server::ReadOnly)    // user link
        {
            textMail += "\n\n\n" + tr("Limited account html links");
            getHtmlLinks(server->ConnectionUsers.at(n).Name, server->ConnectionUsers.at(n).PassWord, textMail);
            break;
        }
    }
    sendMailThread::eMailContent *message = new sendMailThread::eMailContent;
    message->destinataire = eMailSender.eMailSender;
    message->nom = "LogisDom";
    message->text = textMail;
    message->objet = textSubject;
    eMailSender.appendeMail(message);
    if (!eMailSender.isRunning()) eMailSender.start();
    ui.pushButtonSendMail->setEnabled(true);
}




void configwindow::codecIndexChanged(int index)
{
    if (index < 0) textCodec = nullptr;
    if (index < codecs.count()) textCodec = codecs.at(index);
}



QString configwindow::getCodecName()
{
    QString codec;
    codec.append(ui.comboBoxCodecs->currentText());
    return codec;
}




void configwindow::DataPathClicked()
{
	QString actualdir = ui.lineEditDataFolder->text();
	if (QDir::isRelativePath(actualdir)) actualdir = QDir(actualdir).absolutePath();
    QString directory = QFileDialog::getExistingDirectory(this, tr("Data Directory"), actualdir, QFileDialog::ShowDirsOnly);
    if (directory.isEmpty()) return;
    if (directory.contains(QDir::currentPath()))
	{
        directory.remove(QDir::currentPath());
        if (!directory.isEmpty()) directory = directory.remove(0, 1);
        else directory = defaultrepertoiredat;
	}
    QString newPath = directory;
    if (newPath.right(1) != QDir::separator()) newPath.append(QDir::separator());
    if (newPath == parent->repertoirebackup)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for backup"), parent);
        return;
    }
    if (newPath == parent->repertoirehtml)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for html"), parent);
        return;
    }
/*    if (newPath == parent->repertoirezip)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for zip"), parent);
        return;
    }*/
    if (directory == repertoireicon)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for icons"), parent);
        return;
    }
    if (directory == ui.lineEditPngPath->text())
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for PNG"), parent);
        return;
    }
    if (!directory.isEmpty())
	{
        ui.lineEditDataFolder->setText(directory);
        QString path = directory;
		if (path.right(1) != QDir::separator()) path.append(QDir::separator());
		ui.lineEditDataFolder->setToolTip(path);
		parent->repertoiredat = path;
	}
}




void configwindow::choosePngFolder()
{
    QString actualdir = ui.lineEditPngPath->text();
    if (QDir::isRelativePath(actualdir)) actualdir = QDir(actualdir).absolutePath();
    QString directory;
    if (ui.lineEditPngPath->text().isEmpty())
        directory = QFileDialog::getExistingDirectory(this, tr("png folder"), QDir::currentPath());
    else
        directory = QFileDialog::getExistingDirectory(this, tr("png folder"), ui.lineEditPngPath->text());
    if (directory.isEmpty()) return;
    if (directory.contains(QDir::currentPath()))
    {
        directory.remove(QDir::currentPath());
        if (!directory.isEmpty()) directory = directory.remove(0, 1);
        else directory = defaultrepertoirepng;
    }
    QString newPath = directory;
    if (newPath.right(1) != QDir::separator()) newPath.append(QDir::separator());
    if (newPath == parent->repertoirebackup)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for backup"), parent);
        return;
    }
    if (newPath == parent->repertoiredat)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for data"), parent);
        return;
    }
    if (newPath == parent->repertoirehtml)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for html"), parent);
        return;
    }
    if (directory == repertoireicon)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for icons"), parent);
        return;
    }
    if (directory == ui.lineEditPngPath->text())
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for PNG"), parent);
        return;
    }
    if (!directory.isEmpty()) ui.lineEditPngPath->setText(directory);
}





void configwindow::ZipPathClicked()
{
	QString actualdir = ui.lineEditZipFolder->text();
	if (QDir::isRelativePath(actualdir)) actualdir = QDir(actualdir).absolutePath();
    QString directory = QFileDialog::getExistingDirectory(this, tr("Zip Directory"), actualdir, QFileDialog::ShowDirsOnly);
    if (directory.contains(QDir::currentPath()))
	{
        directory.remove(QDir::currentPath());
        if (!directory.isEmpty()) directory = directory.remove(0, 1);
        else directory = defaultrepertoiredat;
	}
    QString newPath = directory;
    if (newPath.right(1) != QDir::separator()) newPath.append(QDir::separator());
    if (newPath == parent->repertoirebackup)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for backup"), parent);
        return;
    }
    if (newPath == parent->repertoirehtml)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for html"), parent);
        return;
    }
    if (newPath == parent->repertoirezip)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for zip"), parent);
        return;
    }
    if (directory == repertoireicon)
    {
        messageBox::criticalHide(parent, tr("Directory already in used"),tr("Directory already used for icons"), parent);
        return;
    }
    if (!directory.isEmpty())
	{
        QString path = directory;
		if (path.right(1) != QDir::separator()) path.append(QDir::separator());
        ui.lineEditZipFolder->setToolTip(path);
		parent->repertoirezip = path;
        ui.lineEditZipFolder->setText(directory);
    }
}




void configwindow::BackupPathClicked()
{
	QString actualdir = ui.lineEditBackupFolder->text();
	if (QDir::isRelativePath(actualdir)) actualdir = QDir(actualdir).absolutePath();
	QString dir = QFileDialog::getExistingDirectory(this, tr("Backup Directory"), actualdir, QFileDialog::ShowDirsOnly);
	if (dir.contains(QDir::currentPath()))
	{
		dir.remove(QDir::currentPath());
		if (!dir.isEmpty()) dir = dir.remove(0, 1);
		else dir = defaultrepertoirebackup;
	}
	if (!dir.isEmpty())
	{
		QString path = dir;
		if (path.right(1) != QDir::separator()) path.append(QDir::separator());
        if (path == parent->repertoiredat) return;
        if (path == parent->repertoirezip) return;
        if (path == parent->repertoirehtml) return;
        if (dir == repertoireicon) return;
        ui.lineEditBackupFolder->setToolTip(path);
		parent->repertoirebackup = path;
        ui.lineEditBackupFolder->setText(dir);
    }
}





void configwindow::HtmlPathClicked()
{
	QString actualdir = ui.lineEditHtmlFolder->text();
	if (QDir::isRelativePath(actualdir)) actualdir = QDir(actualdir).absolutePath();
	QString dir = QFileDialog::getExistingDirectory(this, tr("Html Directory"), actualdir, QFileDialog::ShowDirsOnly);
	if (dir.contains(QDir::currentPath()))
	{
		dir.remove(QDir::currentPath());
		if (!dir.isEmpty()) dir = dir.remove(0, 1);
		else dir = defaultrepertoirehtml;
	}
	if (!dir.isEmpty())
	{
		ui.lineEditHtmlFolder->setText(dir);
		QString path = dir;
		if (path.right(1) != QDir::separator()) path.append(QDir::separator());
		ui.lineEditHtmlFolder->setToolTip(path);
		parent->repertoirehtml = path;
	}
}



void configwindow::DataPathChanged()
{
	if (ui.lineEditDataFolder->text().isEmpty()) ui.lineEditDataFolder->setText(defaultrepertoiredat);
	QString path = ui.lineEditDataFolder->text();
	QDir dir(path);
	path = dir.absolutePath();
	if (path.contains(QDir::currentPath()))
	{
		path.remove(QDir::currentPath());
		if (!path.isEmpty()) path = path.remove(0, 1);
		else path = defaultrepertoiredat;
	}
	if (path.right(1) != QDir::separator()) path.append(QDir::separator());
	parent->repertoiredat = path;
	ui.lineEditDataFolder->setToolTip(path);
}




void configwindow::ZipPathChanged()
{
	QString path = ui.lineEditZipFolder->text();
	if (path.isEmpty()) path = parent->repertoiredat;
	QDir dir(path);
	path = dir.absolutePath();
	if (path.contains(QDir::currentPath()))
	{
		path.remove(QDir::currentPath());
		if (!path.isEmpty()) path = path.remove(0, 1);
		else path = defaultrepertoiredat;
	}
	if (path.right(1) != QDir::separator()) path.append(QDir::separator());
	parent->repertoirezip = path;
	ui.lineEditZipFolder->setToolTip(path);
}




void configwindow::BackupPathChanged()
{
	QString path = ui.lineEditBackupFolder->text();
	if (path.isEmpty()) path = parent->repertoirebackup;
	QDir dir(path);
	path = dir.absolutePath();
	if (path.contains(QDir::currentPath()))
	{
		path.remove(QDir::currentPath());
		if (!path.isEmpty()) path = path.remove(0, 1);
		else path = defaultrepertoirebackup;
	}
	if (path.right(1) != QDir::separator()) path.append(QDir::separator());
	parent->repertoirebackup = path;
	ui.lineEditBackupFolder->setToolTip(path);
}




void configwindow::HtmlPathChanged()
{
	QString path = ui.lineEditHtmlFolder->text();
	if (path.isEmpty()) path = parent->repertoirehtml;
	QDir dir(path);
	path = dir.absolutePath();
	if (path.contains(QDir::currentPath()))
	{
		path.remove(QDir::currentPath());
		if (!path.isEmpty()) path = path.remove(0, 1);
		else path = defaultrepertoirehtml;
	}
	if (path.right(1) != QDir::separator()) path.append(QDir::separator());
	parent->repertoirehtml = path;
	ui.lineEditHtmlFolder->setToolTip(path);
}





void configwindow::readPngConfig(QString &configdata)
{
	bool ok;
	QString TAG_Begin = "PNG_Config";
	QString TAG_End = "PNG_End";
	SearchLoopBegin
	int PngDelay = logisdom::getvalue("PngDelay", strsearch).toInt(&ok);
	if (ok) ui.spinBoxPNGDelay->setValue(PngDelay);
	int PngEnabled = logisdom::getvalue("PngEnabled", strsearch).toInt(&ok);
	if ((ok) && (PngEnabled)) ui.checkBoxGenPng->setCheckState(Qt::Checked);
		else  ui.checkBoxGenPng->setCheckState(Qt::Unchecked);
	ui.lineEditPngPath->setText(logisdom::getvalue("PngFolder", strsearch));
	int PngResizeEnabled = logisdom::getvalue("PngResizeEnabled", strsearch).toInt(&ok);
	if ((ok) && (PngResizeEnabled)) ui.checkBoxSize->setCheckState(Qt::Checked);
	int PngResizeMode = logisdom::getvalue("PngResizeMode", strsearch).toInt(&ok);
	if (ok) ui.comboBoxSize->setCurrentIndex(PngResizeMode);
    int PngResizeSizeH = logisdom::getvalue("PngResizeSizeH", strsearch).toInt(&ok);
    if (ok) ui.spinBoxSizeH->setValue(PngResizeSizeH);
    int PngResizeSizeW = logisdom::getvalue("PngResizeSizeW", strsearch).toInt(&ok);
    if (ok) ui.spinBoxSizeW->setValue(PngResizeSizeW);
    int HtmlResizeEnabled = logisdom::getvalue("HtmlResizeEnabled", strsearch).toInt(&ok);
	if ((ok) && (HtmlResizeEnabled)) ui.checkBoxHtmlSize->setCheckState(Qt::Checked);
	int HtmlResizeMode = logisdom::getvalue("HtmlResizeMode", strsearch).toInt(&ok);
	if (ok) ui.comboBoxHtmlSize->setCurrentIndex(HtmlResizeMode);
	int HtmlResizeSize = logisdom::getvalue("HtmlResizeSize", strsearch).toInt(&ok);
	if (ok) ui.spinBoxHtmlSize->setValue(HtmlResizeSize);
	SearchLoopEnd
}





void configwindow::readHtmlConfig(QString &configdata)
{
	bool ok;
	QString TAG_Begin = "HTML_Config";
	QString TAG_End = "HTML_End";
	SearchLoopBegin
	int HtmlDateEnabled = logisdom::getvalue("HtmlDateEnabled", strsearch).toInt(&ok);
	if ((ok) && (HtmlDateEnabled)) ui.checkBoxHtmlDate->setCheckState(Qt::Checked);
		else  ui.checkBoxHtmlDate->setCheckState(Qt::Unchecked);
	ui.lineEditHtmlDateFormat->setText(logisdom::getvalue("HtmlDateParamters", strsearch));
	int HtmlTimeEnabled = logisdom::getvalue("HtmlTimeEnabled", strsearch).toInt(&ok);
	if ((ok) && (HtmlTimeEnabled)) ui.checkBoxHtmlTime->setCheckState(Qt::Checked);
		else ui.checkBoxHtmlTime->setCheckState(Qt::Unchecked);
	ui.lineEditHtmlTimeFormat->setText(logisdom::getvalue("HtmlTimeParamters", strsearch));
    QString htmlremover = logisdom::getvalue("HtmlLineRemover", strsearch);
    QString str = logisdom::getvalue("HtmlSMTPServer", strsearch);
    if (!str.isEmpty())
    {
        ui.lineEditSMTPServer->setText(str);
        eMailSender.smtpServer = str;
    }
    str = logisdom::getvalue("HtmlSMTPLogin", strsearch);
    if (!str.isEmpty())
    {
        ui.lineEditSMTPLogin->setText(str);
        eMailSender.eMailSender = str;
    }
    str = logisdom::getvalue("HtmlSMTPPort", strsearch);
    if (!str.isEmpty())
    {
        ui.lineEditSMTPPort->setText(str);
        eMailSender.smtpPort = str;
    }
    int HtmlSSLEnabled = logisdom::getvalue("HtmlSMPTSSLEnabled", strsearch).toInt(&ok);
    if ((ok) && (HtmlSSLEnabled)) ui.checkBoxSMTPSecure->setCheckState(Qt::Checked);
        else  ui.checkBoxSMTPSecure->setCheckState(Qt::Unchecked);
    changeSMTPSecure(ui.checkBoxSMTPSecure->checkState());
    str = logisdom::getvalue("HtmlSMTPPassword", strsearch);
    SimpleCrypt crypto(Q_UINT64_C(0x040d52ccb57d64c)); //some random number
    if (!str.isEmpty())
    {
        mailPsw = crypto.decryptToString(str);
        ui.lineEditSMTPPassWord->setText(mailPsw);
        eMailSender.smptPassword = mailPsw;
    }
    int index = 1;
	QString newHtmlMenu = logisdom::getvalue("HtmlMenu0", strsearch);
	while (!newHtmlMenu.isEmpty())
	{
		addHtmlMenu(newHtmlMenu);
		newHtmlMenu = logisdom::getvalue(QString("HtmlMenu%1").arg(index++), strsearch);
	}
	SearchLoopEnd
}





void configwindow::createVirtualDevices(QString &configdata)
{
    QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	QString romid = logisdom::getvalue(RomIDMark, strsearch);
	QString family = romid.right(2);
	if (family == familyVirtual)
		if (!deviceexist(romid))
		{
			onewiredevice *device = NewDevice(romid, nullptr);
			if (device)
			{
				device->setconfig(strsearch);
				emit(DeviceChanged(device));
			}
		}
	SearchLoopEnd
}







void configwindow::setRemoteMode(QString &configdata)
{
	QString TAG_Begin = Remote_Mode;
	QString TAG_End = EndMark;
	SearchLoopBegin
    bool logstate = false;
	QString IP, userName, Password;
	int port;
	if (logisdom::getvalue("Log", strsearch) == "1") logstate = true;
    //if (logisdom::getvalue("LogActivity", strsearch) == "1") logact = true;
	IP = logisdom::getvalue("IPaddress", strsearch);
	port = logisdom::getvalue("Port", strsearch).toInt();
	userName = logisdom::getvalue("userName", strsearch);
	Password = logisdom::getvalue("passWord", strsearch);
	parent->RemoteConnection = new remote(parent);
	if ((parent->isRemoteMode()) and (IP != "") and (!userName.isEmpty()))
		{
			QString rmstr =  tr("Remote Connection");
			parent->RemoteConnection->setAccessibleName(rmstr);
			parent->RemoteConnection->setname(rmstr);
			parent->RemoteConnection->setipaddress(IP);
			parent->RemoteConnection->setport(port);
			parent->RemoteConnection->setlogenabled(logstate);
			parent->RemoteConnection->init(userName, Password);
			ui.tabWidget->addTab(parent->RemoteConnection, rmstr);
            ui.tabWidget->setCurrentIndex(ui.tabWidget->count() - LocalTabsNumber);
			ui.checkBoxServer->setEnabled(false);
		}
	SearchLoopEnd
}





void configwindow::setServerMode(QString &configdata)
{
	bool ok;
	QString Listening;
	int ServerPort = 0;
	ServerPort = default_port_remote;
	QString TAG_Begin = Server_Mode;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ServerPort = logisdom::getvalue(Server_Port, strsearch).toInt(&ok);
	if ((!ok) or (ServerPort == 0)) ServerPort = default_port_remote;
	Listening = logisdom::getvalue(Server_Listening, strsearch);
	SearchLoopEnd
	ui.spinBoxPortServer->setValue(ServerPort);
	parent->configwin->ui.spinBoxPortServer->setValue(ServerPort);
	if (Listening == "1") parent->configwin->ui.checkBoxServer->setChecked(true);
		else parent->configwin->ui.checkBoxServer->setChecked(false);
}






bool configwindow::hasSimulatedMaster()
{
	int count = net1wireList.count();
	for (int n=0; n<count; n++)
		if (net1wirearray[n]->getipaddress() == simulator) return true;
	return false;
}






QString configwindow::getRemoteHost()
{
	if (parent->isRemoteMode()) return parent->RemoteConnection->getipaddress();
	else return "";
}








void configwindow::GetDevicesStr(QString &str)
{
	int count = devicePtArray.count();
	for (int n=0; n<count; n++)
		devicePtArray[n]->getCfgStr(str);
}





void configwindow::SetDevicesScratchpad(const QByteArray &configdata, bool save)
{
	QString ReadRomID, Scratchpad;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	for (int n=0; n<devicePtArray.count(); n++)
	{
		SearchLoopBegin
		ReadRomID = logisdom::getvalue(RomIDTag, strsearch);
		Scratchpad = logisdom::getvalue(ScratchPadMark, strsearch);
		if ((ReadRomID == devicePtArray[n]->getromid()) and (!Scratchpad.isEmpty()))
			devicePtArray[n]->setscratchpad(Scratchpad, save);
		SearchLoopEnd
	}
}






void configwindow::SetDevicesMainValue(const QByteArray &configdata, bool)
{
	bool ok;
	QString ReadRomID, ValueStr;
	double Value;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue(RomIDTag, strsearch);
	ValueStr = logisdom::getvalue(Device_Value_Tag, strsearch);
	Value = ValueStr.toDouble(&ok);
	for (int n=0; n<devicePtArray.count(); n++)
		if (ReadRomID == devicePtArray[n]->getromid())
		{
            parent->setTitle(ReadRomID);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            if (ok) devicePtArray[n]->setMainValue(Value, true);
			else devicePtArray[n]->setMainValue(ValueStr);
		}
	SearchLoopEnd
    parent->setTitle("");
}





void configwindow::GetDevicesScratchpad(QString &str)
{
	int count = devicePtArray.count();
	for (int n=0; n<count; n++)
	{
		QString scratchpad = devicePtArray[n]->getscratchpad();
		if (!scratchpad.isEmpty())
		{
			str +="\n" One_Wire_Device "\n";
            str += logisdom::saveformat(RomIDTag, devicePtArray[n]->getromid());
            str += logisdom::saveformat("Name", devicePtArray[n]->getname());
            str += logisdom::saveformat(ScratchPadMark, scratchpad);
			str += EndMark;
			str += "\n";
		}
	}
}





void configwindow::GetDevicesMainValue(QString &str)
{
	int count = devicePtArray.count();
	for (int n=0; n<count; n++)
	{
		str +="\n" One_Wire_Device "\n";
        str += logisdom::saveformat(RomIDTag, devicePtArray[n]->getromid());
		double v = devicePtArray[n]->getMainValue();
        if (logisdom::isNotNA(v)) str += logisdom::saveformat(Device_Value_Tag, QString("%1").arg(v));
		else str += devicePtArray[n]->MainValueToStr();
		str += EndMark;
		str += "\n";
	}
}







void configwindow::GetMenuHtml(QString *str, QString &ID, int Privilege, QString Menu)
{
    QMutexLocker locker(&ConnectionMutex);
    str->append(htmlFormatBegin);
	if (Menu == NetRequestMsg[MenuConfig])
	{
		str->append(toStr(Config) + "<br><br>");
		parent->GetMenuHtml(str, ID, Privilege);
		str->append("<br>");
	}
	if (Menu == NetRequestMsg[MenuRestart])
	{
		str->append(toStr(Restart) + "<br><br>");
		parent->RestartMenuHtml(str, ID, Privilege);
		str->append("<br>");
		return;
	}
	str->append("<br>");
	GetAllMenuHtml(str, ID, Privilege);
}







void configwindow::GetAllMenuHtml(QString *str, QString &ID, int Privilege)
{
	if (Privilege == Server::FullControl) str->append(logisdom::toHtml(toStr(Config), NetRequestMsg[MenuConfig], ID, logisdom::htmlStyleMenu));
	str->append(htmlFormatEnd);
}





void configwindow::UpdateRemoteDevice(const QString &configdata)
{
	QString ReadRomID;
	onewiredevice *device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue(RomIDTag, strsearch);
    if (parent->isRemoteMode())
    {
        parent->setTitle(ReadRomID);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    device = NewDevice(ReadRomID, parent->RemoteConnection);
    if (device) readconfigfilefordevice(configdata, device);
	SearchLoopEnd
    parent->setTitle("");
}





void configwindow::SaveConfigStr(QString &str)
{
	str += "\nPNG_Config\n";
    if (ui.checkBoxGenPng->isChecked()) str += logisdom::saveformat("PngEnabled", "1"); else str += logisdom::saveformat("PngEnabled", "0");
    str += logisdom::saveformat("PngFolder", ui.lineEditPngPath->text());
    str += logisdom::saveformat("PngDelay", QString("%1").arg(ui.spinBoxPNGDelay->value()));
    if (ui.checkBoxSize->isChecked()) str += logisdom::saveformat("PngResizeEnabled", "1"); else str += logisdom::saveformat("PngResizeEnabled", "0");
	if (ui.comboBoxSize->currentIndex() != -1) str += logisdom::saveformat("PngResizeMode", QString("%1").arg(ui.comboBoxSize->currentIndex()));
    str += logisdom::saveformat("PngResizeSizeW", QString("%1").arg(ui.spinBoxSizeW->value()));
    str += logisdom::saveformat("PngResizeSizeH", QString("%1").arg(ui.spinBoxSizeH->value()));
    if (ui.checkBoxHtmlSize->isChecked()) str += logisdom::saveformat("HtmlResizeEnabled", "1"); else str += logisdom::saveformat("HtmlResizeEnabled", "0");
	if (ui.comboBoxHtmlSize->currentIndex() != -1) str += logisdom::saveformat("HtmlResizeMode", QString("%1").arg(ui.comboBoxHtmlSize->currentIndex()));
	str += logisdom::saveformat("HtmlResizeSize", QString("%1").arg(ui.spinBoxHtmlSize->value()));
    str += "PNG_End\n";
	str += "\nHTML_Config\n";
	if (ui.checkBoxHtmlDate->isChecked()) str += logisdom::saveformat("HtmlDateEnabled", "1"); else str += logisdom::saveformat("HtmlDateEnabled", "0");
	str += logisdom::saveformat("HtmlDateParamters", ui.lineEditHtmlDateFormat->text());
	if (ui.checkBoxHtmlTime->isChecked()) str += logisdom::saveformat("HtmlTimeEnabled", "1"); else str += logisdom::saveformat("HtmlTimeEnabled", "0");
	str += logisdom::saveformat("HtmlTimeParamters", ui.lineEditHtmlTimeFormat->text());
	for (int n=0; n<ui.listWidget->count(); n++)
    str += logisdom::saveformat(QString("HtmlMenu%1").arg(n), ui.listWidget->item(n)->text());
    str += logisdom::saveformat("HtmlSMTPServer", ui.lineEditSMTPServer->text());
    str += logisdom::saveformat("HtmlSMTPLogin", ui.lineEditSMTPLogin->text());
    str += logisdom::saveformat("HtmlSMTPPort", ui.lineEditSMTPPort->text());
    if (ui.checkBoxSMTPSecure->isChecked()) str += logisdom::saveformat("HtmlSMPTSSLEnabled", "1"); else str += logisdom::saveformat("HtmlSMPTSSLEnabled", "0");
    SimpleCrypt crypto(Q_UINT64_C(0x040d52ccb57d64c)); //some random number
    QString passwordEncrypted = crypto.encryptToString(ui.lineEditSMTPPassWord->text());
    //QString decrypted = crypto.decryptToString(result);
    str += logisdom::saveformat("HtmlSMTPPassword", passwordEncrypted, true);
    str += "HTML_End\n";
	str += "\nPATH_Config\n";
	str += logisdom::saveformat("DataFolderPath", ui.lineEditDataFolder->text());
	str += logisdom::saveformat("ZipFolderPath", ui.lineEditZipFolder->text());
	str += logisdom::saveformat("BackupFolderPath", ui.lineEditBackupFolder->text());
	str += logisdom::saveformat("BackupHtmlPath", ui.lineEditHtmlFolder->text());
	if (ui.comboBoxBackupInterval->currentIndex() != -1) str += logisdom::saveformat("BackupInterval", QString("%1").arg(ui.comboBoxBackupInterval->currentIndex()));
	str += logisdom::saveformat("BackupTime", ui.timeEditBackupInterval->time().toString(Qt::ISODate));
	str += "PATH_End\n";
	str += "\nGENERAL_Config\n";
	if (ui.checkBoxSaveQuit->isChecked()) str += logisdom::saveformat("SaveOnQuit", "1"); else str += logisdom::saveformat("SaveOnQuit", "0");
	if (ui.checkBoxAskSave->isChecked()) str += logisdom::saveformat("AskBeforeSave", "1"); else str += logisdom::saveformat("AskBeforeSave", "0");
	if (ui.checkBoxkSaveInterval->isChecked()) str += logisdom::saveformat("AutoSave", "1"); else str += logisdom::saveformat("AutoSave", "0");
    if (ui.checkBoxSaveOnChange->isChecked()) str += logisdom::saveformat("SaveChange", "1"); else str += logisdom::saveformat("SaveChange", "0");
    str += logisdom::saveformat(QString("SaveInterval"), QString("%1").arg(ui.spinBoxSaveInterval->value()));
	if (ui.checkBoxHideHeating->isChecked()) str += logisdom::saveformat("HideHeatingTab", "1"); else str += logisdom::saveformat("HideHeatingTab", "0");
	str += logisdom::saveformat(QString("LogFileSize"), QString("%1").arg(ui.spinBoxLogSize->value()));
	str += logisdom::saveformat(QString("TreeDevivceColumWidth"), QString("%1").arg(OneWireTree.columnWidth(0)));
	parent->alarmwindow->GeneralErrorLog->getCfgStr(str);
    str += logisdom::saveformat("TextCodec", ui.comboBoxCodecs->currentText());
    str += "GENERAL_End\n";
	int count = net1wireList.count();
	for (int n=0; n<count; n++) net1wirearray[n]->getCfgStr(str);
	server->SaveConfigStr(str);
	str += "\nHTMLSetup_Config\n";
	str += logisdom::saveformat("HtmlComboSelection", ui.comboBoxHtml->currentText());
	for (int n=0; n<htmlLayout.count(); n++)
	{
		str += "\nHTMLBlock_Begin\n";
        str += logisdom::saveformat("ComboName", ui.comboBoxHtml->itemText(n));
        str += logisdom::saveformat("HtmlWelcomeMessage", htmlLayout[n]->welcomeMessage);
        str += logisdom::saveformat("HtmlTitle", htmlLayout[n]->htmlTitle);
        str += logisdom::saveformat("HtmlHeader", htmlLayout[n]->htmlHeader);
        str += logisdom::saveformat("HtmlTime", htmlLayout[n]->htmlTime);
        str += logisdom::saveformat("HtmlMenu", htmlLayout[n]->htmlMenu);
        str += logisdom::saveformat("HtmlList", htmlLayout[n]->htmlList);
        str += logisdom::saveformat("HtmlDetector", htmlLayout[n]->htmlDetector);
        str += logisdom::saveformat("HtmlValue", htmlLayout[n]->htmlValue);
        str += logisdom::saveformat("HtmlCommand", htmlLayout[n]->htmlCommand);
        str += "HTMLBlock_End\n";
	}
        str += "HTMLSetup_End\n";
}





void configwindow::readconfigfilefordevice(const QString &configdata, onewiredevice *device)
{
    QMutexLocker locker(&mutexReadConfig);
	QString ReadRomID;
	QString Name;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	if (device == nullptr) return;
	SearchLoopBegin
    ReadRomID = logisdom::getvalue("RomID", strsearch);
	if (ReadRomID == device->getromid())
	{
        device->setCfgStr(strsearch);
        QFile nextSaveFile("next.sav");
        if (ReadRomID.right(2) == "MD") {   // this is only to import Modbus device with modbus plugin
            QString coefstr = logisdom::getvalue("Coef", strsearch);
            bool ok;
            if (!coefstr.isEmpty()) {
                if (coefstr.toDouble(&ok) != 1) {
                device->AXplusB.axbEnabled.setCheckState(Qt::Checked);
                device->AXplusB.A.setText(coefstr); }
            }
        }
		if (nextSaveFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			while(!nextSaveFile.atEnd())
			{
				QString line = nextSaveFile.readLine();
				QStringList split = line.split("#");
				if (split.count() == 2)
				{
					if (split.first() == ReadRomID) device->saveInterval.setNext(QDateTime::fromString (split.at(1), Qt::ISODate));
				}
			}
            nextSaveFile.close();
        }
	}
	SearchLoopEnd
	strsearch = "";
    if (device->getname().isEmpty()) device->setconfig(strsearch);
}


QString configwindow::toStr(int index)
{
	switch (index)
	{
        case Main :         return tr("Main Menu");
        case AllValues :    return tr("All Values");
        case Light :        return tr("Lights");
        case Temperature :  return tr("Temperature");
        case Heating	:   return tr("Heating");
        case Program	:   return tr("Program");
        case Config		:   return tr("Configuration");
        case Restart	:   return tr("Restart");
        default :           return tr("Undefined");
	}
}

