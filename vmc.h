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



#ifndef VMC_H
#define VMC_H
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QButtonGroup>
#include <QtGui>
#include <QDateTime>
#include "ui_switchui.h"
class configwindow;
class ProgramEvent;
class ProgramData;
class logisdom;
class devchooser;
class onewiredevice;
class daily;
class htmlBinder;
class formula;




class SwitchThreshold : public QWidget
{
	Q_OBJECT
enum STModes {ifON, isLower, isEqual, isDifferent, isHigher, formula_Value};
public:
	SwitchThreshold(logisdom *Parent, QString formulaName = "");
	~SwitchThreshold();
	logisdom *parent;
	void SaveConfigStr(QString &str);
	void readconfigfile(QString &strsearch);
    void init();
    void Lock(bool);
    void updateLock();
    bool lockStatus = true;
// Palette
	QGridLayout layout;
    devchooser *switchThreshold = nullptr;
    formula *Switchformula = nullptr;
    QPushButton ButtonFormula, ButtonDel;
    QSpinBox turnOnDelay;
	QSpinBox delay;
	QComboBox mode;
	QSpinBox thresholdValue;
	QLabel status;
	htmlBinder *htmlBind;
    void CheckStatus(double &DeviceON);
	QString getName();
private:
    double lastMainValue, currentValue;
	bool lastStatus;
	QDateTime PushOn;
	QString name;
public slots:
    void delClick();
private slots:
	void configChange(int);
    void showFormula();
protected:
signals:
    void deleteRequest(SwitchThreshold *);
    void configChanged();
};





class SwitchControl : public QWidget
{
#define setModeAuto "setmodeauto"
#define setModeOn "setmodeon"
#define setModeOff "setmodeoff"
#define setModeOpen "setmodeopen"
#define setModeClose "setmodeclose"
#define setModeDisabled "setmodedisabled"
enum SwitchMode
    {	Variable, ONOFF, OPENCLOSE, VariableOPENCLOSE	};
enum ButtonId
	{	ID_Auto = 1, ID_On, ID_Off, ID_Disabled	};
	Q_OBJECT
public:
    SwitchControl(logisdom *Parent);
    QString Button_borderwidth, Button_borderradius, Button_margin, Button_minheight, Button_font;
    QString Button_bckrgd_1, Button_bckrgd_2, Button_bckrgd_3;
    QString ButtonStyle, styleNormal, styleDisabled, stylePressed, styleChecked, styleFocus, styleFocusPressed;
    ~SwitchControl();
    void Lock(bool);
    bool lockStatus = true;
    void Clean();
    logisdom *parent;
    QString ModeTxt, OnModeTxt, OffModeTxt, AutoModeTxt, OpenModeTxt, CloseModeTxt, ManualModeTxt, DisabledModeTxt;
    void SaveConfigStr(QString &);
    void createSwitchDevices(QString &);
    void readconfigfile(QString &);
    void makeConnections();
    QString ID;
    QMutex mutexConfig;
    QWidget *w;
    QGridLayout *timeLayout;
    void removeLayoutThreshold();
    void addLayoutThreshold();
    void removeLayoutDev();
    void addLayoutDev();
// Palette setup
    Ui::switchui ui;
    QGridLayout *thresholdLayout;
    QGridLayout setupLayout;
    daily *dailyprog;
    QWidget setup;
    QList <devchooser*> SwitchDeviceList;
    QList <devchooser*> CloseDeviceList;
    void enableCloseDevices();
    void disableCloseDevices();
    htmlBinder *htmlBind;
    void displayStatus(QString Text);
private:
    devchooser *newSwitchDevice();
    devchooser *newCloseDevice();
    void addFamilies(devchooser *);
    configwindow *configParent;
    ProgramEvent *progParent;
    QDateTime LastTurnOn;
    QGridLayout *SwitchControlLayout;
    QString Status, StatusText;
    QLabel SwitchStatus;
    QPushButton ButtonName, ButtonAuto, ButtonOn, ButtonOff, ButtonDisabled, ButtonDel;
    QSlider SliderStatus;
    QButtonGroup ButtonGroup;
    int volet_status;
    bool sliderHasMoved;
    void sliderActive();
    bool moving;
    QList <SwitchThreshold*> triggerList;
    int lastminute, SwitchDelay, waitMove;
    void setDevicestate(onewiredevice *, double);
    void setDevState(onewiredevice *, double);
public slots:
    void updateStatus();
    void updateJourneyBreaks(ProgramData *prog);
    void modeChanged(int index);
private slots:
    void clickSwitchOn();
    void clickSwitchOff();
    void clickAuto();
    void clickDisabled();
    void sliderReleased();
    void Del();
    void showDelMe();
    void deviceSelectionChanged();
    void deleteMe(devchooser *);
    void deleteThreshold(SwitchThreshold *);
    SwitchThreshold *addTrigger(QString Name = "");
    SwitchThreshold *addFormula();
    void remoteCommand(const QString &command);
    void addDevice();
protected:
    void mousePressEvent(QMouseEvent *event);
signals:
    void deleteRequest(SwitchControl *);
    void setupClick(QWidget *);
    void saveConfig();
};



class SwitchScrollArea : public QWidget
{
	Q_OBJECT
#define Switch_Begin_Mark "New_Switch_Begin"
#define Switch_End_Mark "New_Switch_End"
#define Switch_Trigger_Begin "Switch_Trigger_Begin"
#define Switch_Trigger_End "Switch_Trigger_End"
#define Switch_Config_Begin "Switch_Config_Begin"
#define Switch_Config_End "Switch_Config_End"
#define New_Trigger_Begin "Trigger_Begin"
#define New_Trigger_End "Trigger_End"
public:
    SwitchScrollArea(logisdom *Parent);
    logisdom *parent;
    ~SwitchScrollArea();
    void Lock(bool);
    bool lockStatus = true;
    void updateButton();
    htmlBinder *htmlBindSwitchMenu;
    QGridLayout layout;
    void SaveConfigStr(QString &str);
    void readconfigfile(const QString &configdata);
    void readSwitchConfig(QString &data);
    QTimer UpdateTimer;
// Palette setup
    QWidget setup;
    QGridLayout setupLayout;
    QPushButton AddButton;
    QList <SwitchControl*> Switches;
    QMutex mutexUpdate;
public slots:
    void updateStatus();
    void updateJourneyBreaks(ProgramData *prog);
private slots:
    void setPalette();
    void Add();
    void Del();
    void deleteMe(SwitchControl *);
protected:
    void mousePressEvent(QMouseEvent *event);
signals:
    void setupClick(QWidget*);
};



#endif
