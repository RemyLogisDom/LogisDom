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



#ifndef CHAUFFAGEUNIT_H
#define CHAUFFAGEUNIT_H

#include <QtGui>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>

class configwindow;
class weeklyProgram;
class logisdom;
class onewiredevice;
class ProgramEvent;
class daily;
class configwindow;
class formula;
class devchooser;
class htmlBinder;


class ChauffageUnit : public QWidget
{
	Q_OBJECT
#define minH 20
#define setValveCommand "setValve"
#define modeValveCommand "modeValve"
#define setTargetCommand "setTarget"
#define modeTargetCommand "modeTarget"
#define modeValveAuto "valveauto"
#define modeValveManual "valvemanual"
#define modeTargetAuto "targetauto"
#define modeTargetManual "targetmanual"
#define targetPlus "targetplus"
#define targetMinus "targetminus"
#define valvePlus "valveplus"
#define valveMinus "valveminus"
#define setmodeAuto "setmodeauto"
#define setmodeOn "setmodeon"
#define setmodeOff "setmodeoff"
#define setmodeConfort "setmodeconfort"
#define setmodeNuit "setmodenuit"
#define setmodeEco "setmodeeco"
#define setmodeManuel "setmodemanual"
#define setmodeHorsGel "setmodehorsgel"
#define setmodeSolaireOn "setmodesolaireon"
#define setmodeSolaireOff "setmodesolaireoff"
#define setmodeSolaireToggle "setmodesolairetoggle"
public:
enum modeChauffageUnit{ modeAuto, modeOn, modeOff, modeConfort, modeNuit, modeEco, modeManuel, modeHorsGel };
	ChauffageUnit(logisdom *parent, QString name);
	~ChauffageUnit();
	logisdom *parent;
	htmlBinder *htmlBind;
// Display setup
	QGridLayout displayLayout;
	QPushButton *Name;
    QDoubleSpinBox Consigne;
    QSpinBox Status;
    QLabel ActualDevValue;
    QCheckBox vanneManual;
    QComboBox ModeList;
    QCheckBox solarEnable;
	bool solarEnabled();
	int Result;
	int ComboIndex;
	QString ProgramName;
    QString valveModeTxt, targetModeTxt, autoTxt, manualTxt, targetTxt, valveTxt, remoteDisTxt;
    void connectcombo();
    void disconnectcombo();
    QTimer update;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QPushButton RenameButton, CalculButton;
	formula *Formula;
    QLabel labelVanne, labelRemoteDisplay;
	devchooser *VanneDevice;
    devchooser *RemoteDisplay;
    QLabel labelProgram;
	QComboBox Program;
	QDoubleSpinBox TConfort;
    QDoubleSpinBox TNuit;
    QDoubleSpinBox TEco;
    QCheckBox minEnabled, maxEnabled, passiveMode, autonomousValve, proportionalMinMax;
	QSpinBox Vmin, Vmax;
	int mode;
	QString modeToStr(int Mode);
    void setValue(int value);
    int checkValveAutonomus();
private:
	QList <onewiredevice*> CaptTempPtArray;
	QList <onewiredevice*> VannePtArray;
	double Integral;
	int I_Counter;
    bool master;
public slots:
	int process();
    void setMode(int Mode, bool update = true);
    void clickMode(int Mode);
    void AddProgram(QString);
	void RemoveProgram(QString);
private slots:
	void applyConfigStr(QString command);
	void remoteCommand(const QString &command);
	void changename();
	void emitSetupClick();
	void consigneManualChange(int state);
	void consigneChange(double value);
	void vanneChange(int value);
	void vanneManualChange(int state);
    void MinEnChanged(int state);
	void minChanged(int value);
	void MaxEnChanged(int state);
    void maxChanged(int value);
    void AutonomusChanged(int state);
    void solarChanged(int);
    void startupdate();
signals:
	void setupClick(QWidget*);
};







class ChauffageBlockHeader : public QWidget
{
	Q_OBJECT
#define minH 20
public:
	ChauffageBlockHeader(QWidget *parent);
	~ChauffageBlockHeader();
	QGridLayout *gridLayout;
	QLineEdit *NameDesignation;
	QLineEdit *ConsigneDesignation;
	QLineEdit *StatusDesignation;
	QLineEdit *ProgDesignation;
	QLineEdit *VanneDesignation;
};






class ChauffageScrollArea : public QWidget
{
	Q_OBJECT
public:
	ChauffageScrollArea(logisdom *Parent);
	logisdom *parent;
	~ChauffageScrollArea();
	QGridLayout displayLayout;
    int displayLayoutIndex;
	htmlBinder *htmlBind;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QLabel labelCirculator;
	QLabel labelCirculatorStatus;
	devchooser *CirculatorDevice;
	QLabel labelTankEnabler;
	QLabel labelSolarEnabler;
	QLabel labelSolarDischarge;
	QLabel labelTankHigh;
	devchooser *TankEnablerDevice;
	devchooser *SolarEnablerDevice;
	devchooser *SolarDischargeDevice;
	devchooser *TankHighDevice;
	QSpinBox TankEnablerThreshold;
	QSpinBox SolarEnablerThreshold;
	QSpinBox SolarDischargeThreshold;
	QToolButton ChauffageTool;
	QPushButton AddButton;
	QPushButton RemoveButton;
	QPushButton AllOpened, AllClosed, Process;
//	ChauffageBlockHeader *Header;
	bool Locked;
	ChauffageUnit *AddEvent(QString Name);
    ChauffageUnit *AddEvent(QString Name, QString Formula, QString ReadRomIDVanne, QString ReadRomIDRemote, QString ReadProgName, double consigne, double conf, double nuit, double eco);
	void verrouiller();
	void deverrouiller();
	void SaveConfigStr(QString &str);
    void readconfigfile(const QString &configdata);
    void readchauffagconfig(const QString &configdata);
	void setProgramName(int index, QString programName);
private:
	QList <ChauffageUnit*> chauffage;
	QComboBox chauffageList;
	configwindow *configParent;
	ProgramEvent *progParent;
	QString textProcessButton;
private slots:
	void Add();
	void supprimer();
	void remoteCommand(QString command);
	void AllOpenedClick(bool checked);
	void AllClosedClick(bool checked);
public slots:
	void process();
protected:
	void mousePressEvent(QMouseEvent *event);
};


#endif

