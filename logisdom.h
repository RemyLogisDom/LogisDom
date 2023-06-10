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





#ifndef LOGISDOM_H
#define LOGISDOM_H
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolBar>
#include <QtGui>
#include <QTimer>
#include "iconearea.h"
#include "filesave.h"
#include "ui_mainw.h"
#include "../plugins/interface.h"

class meteo;
class cstr;
class alarmwarn;
class weeklyProgram;
class addDaily;
class ProgramEvent;
class addicons;
class IconeArea;
class graphconfig;
class htmlBinder;
class configwindow;
class energiesolaire;
class iconf;
class onewiredevice;
class ChauffageScrollArea;
class remote;
class tableauconfig;
class treeHtmlWidget;
class SwitchControl;
class SwitchScrollArea;
class weathercom;
class QTable;
class QBuffer;
class QStringListModel;


enum NetType
    {
        NoType, Ha7Net, MBusType, Ezl50_FTS800, Ezl50_X10, Ezl50_PlcBus, MultiGest, RemoteType, TeleInfoType, TCP_RPS2Type, TCP_HA7SType, TCP_ResolType, ModBus_Type, EOceanType, LastType
    };


static const QString NetTypeStr[LastType] =
    {
    "NoType", "Ha7Net", "M-Bus", "Ezl50-FTS800", "Ezl50-X10 (not active)", "Ezl50-PLCBus (not active)", "MultiGest", "Remote Connection", "TeleInfo", "TCP-RPS2", "HA7S", "RESOL", "Modbus", "EOcean"
    };


class srcoolCatchArrow : public QScrollArea
{
   Q_OBJECT
public:
    IconeArea *child;
    srcoolCatchArrow(IconeArea *iconArea)
    {
        child = iconArea;
    }
protected:
   void keyPressEvent(QKeyEvent *event)
   {
       child->sendPressEvent(event);
   }
   void wheelEvent(QWheelEvent *event)
   {
     if(event->modifiers().testFlag(Qt::ControlModifier))
     {
         child->mouseWheel(event);
         //qDebug() << QString("Wheel Control %1").arg(event->delta());
     }
   }
};


class logisdom : public QMainWindow
{
	Q_OBJECT
	Q_ENUMS(InitMode)

public:
#define logfilename "maison"
#define htmlsperarator "|"
#define startdelay 20000
#define defaultrepertoiredat "dat"
#define defaultrepertoirebackup "backup"
#define defaultrepertoirehtml "web"
#define defaultrepertoirepng "png"
const static int NA = -32768;
const static int iconSize = 48;
const static int statusIconSize = 32;
const static int PaletteWidth = 5;
static bool NoHex;
    enum InitMode	{	Normal, RemoteConfig	};
	enum htmlStyle	{	htmlStyleMenu, htmlStyleTime, htmlStyleList, htmlStyleDetector, htmlStyleValue, htmlStyleCommand };
    logisdom(QWidget *parent = nullptr);
	~logisdom();
    QMutex ConnectionMutex;
    QString configfilename;
    QString configData;
    QMutex mutexGetConfig;
    bool paletteOnTop;
    Ui::mainwin ui;
	htmlBinder *htmlBind;
	int logfilesizemax;
	QDialog palette;
	QWidget paletteWidget;
	QString Version, VersionDate, VersionHistory;
	QWidget *widgetTabHeating;
	QString tabHeatingText;
	treeHtmlWidget *mainTreeHtml;
	QGridLayout *paletteLayout;
	QList <QWidget*> paletteHistory;
	int paletteHistoryIndex;
	QWidget *previousSetup;
	QWidget htmlPreview;
	QGridLayout htmlPreviewLayout;
	QPushButton MainToolButton;
	QPushButton NextButton;
    QCheckBox alwaysOnTop;
	QPushButton PreviousButton;
	QIcon greenDotIcon;
	QIcon yellowDotIcon;
	QIcon redDotIcon;
	QIcon lockedIcon;
	QIcon unlockedIcon;
	QIcon paletteIcon;
	QIcon saveIcon;
	QIcon aboutIcon;
	QIcon tableIcon;
	QIcon designIcon;
	QIcon sunIcon;
	QIcon switchIcon;
	QIcon dailyIcon;
	QIcon weeklyIcon;
	QIcon devicesIcon;
	QIcon configIcon;
	QIcon graphicIcon;
	QIcon webHtmlIcon;
// actions
	QAction *statusAction;
	QAction *lockAction;
    QAction *paletteAction;
    QAction *saveAction;
	QAction *aboutAction;
	QAction *tableAction;
	QAction *designAction;
	QAction *sunAction;
	QAction *dailyAction;
	QAction *weeklyAction;
	QAction *devicesAction;
	QAction *configAction;
	QAction *graphicAction;
	QAction *switchAction;
	QAction *webHtmlAction;
	QToolBar statusBar;
// resize
	QDialog resize;
	QGridLayout *resizeLayout;
	QSpinBox resizeIconValue;
	QSpinBox workspaceX, workspaceY;
	QCheckBox resizeIcon, resizeText, resizeValue, resizeAllTab, moveLabels;
	QLabel labelSize, labelWorkSpaceX, labelWorkSpaceY;
	void removeWidget(QWidget *widget);
    void get_ConfigData(QString &data);
    void init(char *argv[]);
    void setTitle(const QString &txt);
    void readconfigfile(const QString &configdata);
    void setTabs(const QString &configdata);
    void readIconTabconfigfile(const QString &configdata);
	void SaveConfigStr(QString &str);
	void SaveIconConfigStr(QString &str);
	void SaveRemoteStatusStr(QString &str);
	QStringListModel *model;
	bool isRemoteMode();
	bool remoteIsAdmin();
    bool isLocked();
	void valueTranslate(QString &txt);
	void htmlTranslate(QString &datHtml, const QString &id);
	void htmlLinkCommand(QString &command);
	QString getHtmlValue(QString &command);
    htmlBinder* getBinder(QString &command);
    void GetMenuHtml(QString *str, QString &ID, int Privilege);
	void RestartMenuHtml(QString *str, QString &ID, int Privilege);
	static QString toHtml(QString Text, QString Link, int style = -1);
	static QString toHtml(QString Text, QString Order, QString &ID, int style = -1);
	static QString spanIt(QString str, int style);
	static QString styleToString(int style);
	static QString saveformat(QString name, QString value, bool hex = false);
	static QString getvalue(QString search, const QString &str);
	static QByteArray getvalue(QString search, QByteArray &str);
	static QString filenameformat(QString RomID, int month, int year);
    static bool AreSame(double a, double b);
    static bool AreNotSame(double a, double b);
    static bool isZero(double a);
    static bool isNotZero(double a);
    static bool isNA(double a);
    static bool isNotNA(double a);
    void showIconTools();
	QToolBar *getToolstatus();
	QDialog iconTools;
	QToolBar iconToolBar;
	QString sessionID;
	QTreeWidgetItem *addHtmlBinder(htmlBinder *binder);
	htmlBinder *getTreeItemBinder(QTreeWidgetItem *item);
	void setBinderCommand(QString command);
	QList <QTreeWidgetItem*> treeItemList;
	QList <htmlBinder*> htmlBinderList;
	QUuid binderID;
	QString getHtmlRequest(QString &Request, QString &WebID, int Privilege);
	void declareHtmlMenu(QString menuString);
	ProgramEvent *getProgEventArea();
	ProgramEvent *ProgEventArea;
	addicons *myicons;
	meteo *MeteoArea;
	weeklyProgram *AddProgwin;
	alarmwarn *alarmwindow;
	graphconfig *graphconfigwin;
	configwindow *configwin;
	SwitchScrollArea *SwitchArea;
	ChauffageScrollArea *ChauffageArea;
	energiesolaire *EnergieSolaire;
	remote *RemoteConnection;
	addDaily *AddDaily;
	tableauconfig *tableauConfig;
	void saveconfig(QString fileName);
    void getSaveStr(QString &str);
	QString DateFormatDetails, TimeFormatDetails; 
	QList<IconeArea*> IconeAreaList;
	bool iconFileCheck;
	void PaletteHide(bool hide);
    void PaletteClear();
    bool isPaletteHidden();
    bool getTabPix(const QString &name, QBuffer &buffer);
    void logthis(const QString &filename, const QString &log, QString S = "");
	void logfile(const QString &log, const QString &S);
	void logfile(const QString &log);
	void GenError(int ErrID, QString Msg);
	void GenMsg(QString Msg);
	QMutex mutexGraph;
    QMutex MutexgetTabPix;
    QLocale LogisDomLocal;
    //QByteArray text_Codec; used to set codec in command line when launching application
	bool TempInCelsius;
	bool remydev;
	bool logTag;
	bool diag;
	QString getrepertoiredat();
	QString repertoiredat;
	QString getrepertoirezip();
	QString repertoirezip;
	QString repertoirebackup;
	QString repertoirehtml;
	QString getrepertoirehtml();
    QString getTabName(int tab);
    void loadRequest(onewiredevice*);
    QList<onewiredevice*> deviceLoadRequest;
    int htmlPageExist(QString name);
    void getTabHtml(int index, QString &html);
    QList<LogisDomInterface*> logisdomInterfaces;
private:
    QDir pluginsDir;
    void loadPlugins();
    int indexupdate;
	QMutex mutexFifoEmpty;
	QMutex mutexLog;
	QMutex MutexGenMsg;
    QTimer timerUpdate;
	void closeEvent(QCloseEvent *event);
	void addIconTab();
    void removeIconTab();
	void renameIconTab();
	bool IconTabisLocked;
	void CompressFile(const QString &filename, bool deletefile);
	void ZipFile(const QString &filename);
	void UnCompressFile(const QString &filename, bool deletefile);
	int lastminute;
	int saveInterval;
	QString strCopyIcon;
    fileSave fileSaver;
    void setTabWidget(int);
    void changePalette(QWidget *setup);
    void checkSetupParent(QWidget *);
private slots:
    void onAllwaysTop(bool checked);
    void nextPalette();
	void previousPalette();
    void MainToolClick();
    void Lock(bool state);
    void swapLock(bool state);
    void paletteShow(bool state);
	void saveShow(bool state);
	void aboutShow(bool state);
	void tableShow(bool state);
	void designShow(bool state);
	void sunShow(bool state);
	void switchShow(bool state);
	void dailyShow(bool state);
	void weeklyShow(bool state);
	void devicesShow(bool state);
	void configShow(bool state);
	void graphicShow(bool state);
    void tabChanged(int);
    void tabPrgChanged(int);
    void tabGraphChanged(int);
    void showgraphconfig();
	void showswitch();
	void showaddProgram();
	void showtableauconfig();
	void showdevices();
	void showenergiesolaire();
	void showresize();
    void exportPage();
    void exportSelection();
    void importPage();
    void importIntoPage();
    void saveHtmlPage();
    void getHtmlLink();
    void setBackGroundColor();
    void AddTextZone();
    void getHtmlHeader(QTextStream &out, QString &title, QString backgroundColorHex);
    void AddDailyPrg();
	void aproposde();
    void deadDevice();
    void setWindowSize();
    void AddIcones();
	void lockClicked(QMouseEvent *);
	void alarmOn(bool);
	void resizeChanged(int size);
	void showTabWidget(int);
	void workSpaceResizeChanged(int size);
	void iconAlignLeft(bool);
	void iconAlignRight(bool);
	void iconAlignTop(bool);
	void iconAlignBottom(bool);
	void iconCopysize(bool);
	void iconCopy(bool);
	void iconPaste(bool);
	void iconUndo(bool);
	void iconRedo(bool);
	void iconAddOne(bool);
	void MainTreeClick(const QPoint &pos);
    void newPluginDevice(LogisDomInterface*, QString);
    void newPluginDeviceValue(QString, QString);
    void pluginDeviceSelected(QString);
    void pluginUpdateNames(LogisDomInterface*, QString);
public slots:
    void showPalette();
    void update(bool force = false);
    void mainStatus();
	void showconfig();
	void restart();
    void restartnoconfirm();
    void setPalette(QWidget *setup);
	void DeviceConfigChanged(onewiredevice *);
    void finishedDataLoading(onewiredevice *);
    void saveconfig();
    void saveconfigtxt();
    void saveDat(QString, QString);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
signals:
	void deviceConfigChanged(onewiredevice *);
};


#endif

