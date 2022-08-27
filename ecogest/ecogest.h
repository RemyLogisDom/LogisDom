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



#ifndef ECOGEST_H
#define ECOGEST_H

#include <QtWidgets/QButtonGroup>

#include "net1wire.h"
#include "globalvar.h"
#include "ui_multigest.h"
#include "masterthread.h"
#include "uploaderwindow.h"


class formula;


class ecogest : public net1wire
{
    Q_OBJECT
enum SearchIntervals { searchOnce, searchConstant, search5mn, search10mn, search30mn, search1hr, search6hr, search1day};
#define familyMultiGestServo "MV"
#define familyMultiGestServo1 "S1"
#define familyMultiGestServo2 "S2"
#define familyMultiGestServo3 "S3"
#define familyMultiGestServo4 "S4"
#define familyMultiGestServo5 "S5"
#define familyMultiGestSwitchA "SAMS"
#define familyMultiGestSwitchB "SBMS"
#define familyMultiGestSwitchC "SCMS"
#define familyMultiGestSwitchD "SDMS"
#define familyMultiGestSwitchE "SEMS"
#define familyMultiGestSwitchF "SFMS"
#define familyMultiGestSwitchN "SNMS"
#define familyMultiGestWarn "MW"
#define familyMultiGestFlow "MF"
#define familyMultiGestFlowA "FAMF"
#define familyMultiGestFlowB "FBMF"
#define familyMultiGestFlowC "FCMF"
#define familyMultiGestFluidicMode "FL"
#define familyMultiGestSolarMode "SL"



// Solar Mode
#define Solar_Pump_Enable 1
#define Circulator_Enabled 2
#define servoTankMode 4
#define servoHeatingMode 8
#define servoSolarMode 16

#define Mode_NoSun 		servoTankMode
#define Mode_ECS 		Solar_Pump_Enable + servoTankMode + servoSolarMode
#define Mode_MSD 		Solar_Pump_Enable + Circulator_Enabled + servoTankMode + servoSolarMode

// Fluidic Mode
#define servoFillMode 32
#define switchFillMode 64
#define switchKeepFilledMode 128

#define Mode_Solar_Fill 		Solar_Pump_Enable + servoFillMode + switchFillMode
#define Mode_Solar_Run_Degaz 	Solar_Pump_Enable + switchFillMode
#define Mode_Solar_Run 			Solar_Pump_Enable
#define Mode_Solar_KeepFilled 	switchKeepFilledMode + Solar_Pump_Enable
#define Mode_Solar_Off 			0

#define TpRegulPanneauMax 60
#define TpRegulPanneauMin 40
#define TpRegulChauffageMax 50
#define TpRegulChauffageMin 30

#define setFluidicStr "SFL"
#define setSolarStr "SSL"


public:
	enum ButtonIDs
		{
			ButtonIDOff = 1, ButtonIDFill, ButtonIDRun, ButtonIDDegaz, ButtonIDKeepFiled,
			ButtonIDECS, ButtonIDMSD, ButtonIDMSDOnly, ButtonIDNoSun
		};
	enum EcoGestDetectors
		{
			TankLow = 0, TankHigh, HeaterIn, HeaterOut, HeatingOut, SolarIn, SolarOut, LastDetector,
			devServo1, devServo2, devServo3, devServo4, devServo5, devFlowA, devFlowB, devFlowC, devInterlock,
            devSwitchA, devSwitchB, devSwitchC, devSwitchD, devSwitchE, devSwitchF, devSwitchN, devFluidicMode, devSolarMode, allDetector
		};
	ecogest(logisdom *Parent);
	~ecogest();
    void startTCP();
    QString getStr(int index);
	QMutex mutexFifonext;
    void setipaddress();
    void init();
	void createdevices();
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	bool isUploading();
	bool uploadStarted;
	bool isGlobalConvert();
	QStringList RomIDs;
	QTimer statusTimer;
	QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
	void addtofifo(const QString &order);
	void addtofifo(int order, const QString &RomID, bool priority = false);
	void addtofifo(int order);
	void addtofifo(int order, const QString &RomID, const QString &Data, bool priority = false);
public slots:
	void setDefaultNames();
private:
	Ui::multigest uiw;
    masterthread TcpThread;
	void switchOnOff(bool state);
	void setGUImode(int mode);
	void setGUIdefaultMode();
	void setGUIAutoVidangeMode_A();
	void catchLocalStrings();
	onewiredevice *devices[allDetector];
	QString RomIDstr[allDetector];
	QString buttonstext[LastDetector];
	QString buttonFlowAText, buttonFlowBText, buttonFlowCText, buttonSwitchAText, buttonSwitchBText, buttonSwitchCText;
	QString buttonSwitchDText, buttonSwitchEText, buttonSwitchFText, buttonInterlockText, buttonSolarVirtualText;
    void SetDone(const QString &data);
    void SetStatusValues(const QString &data, bool enregistremode);
	bool SolarVirtualStart;
	bool MSDEnable;
	QButtonGroup fluidicGroup, SolarGroup;
	QString version;
	void restart();
	QString Buffer;
	int modeProcess;
	void foundDevice(onewiredevice *device);
	uploaderwindow *picuploader;
private slots:
    void updatebuttonsvalues(const QString&);
	void remoteCommand(const QString &command);
	void setSwitchA();
	void setSwitchB();
	void setSwitchC();
	void setSwitchD();
	void setSwitchE();
	void setSwitchF();
    void setSwitchN();
    void clearRomIDEEprom();
	void OpenUpload();
	void CloseUpload();
	void rightclicklList(const QPoint &pos);
	void solarModeClicked(int);
	void fluidicModeClicked(int);
	void virtualStartClicked();
	void MSDEnableChanged();
	void ServoAValueChanged(int);
	void ServoBValueChanged(int);
	void ServoCValueChanged(int);
	void ServoDValueChanged(int);
	void ServoEValueChanged(int);
	void SolarDelayONChanged(int);
	void FillDelayValueChanged(int);
	void DegazDelayValueChanged(int);
	void KeepFilledDelayValueChanged(int);
	void TRCValueChanged(int);
	void TRPValueChanged(int);
	void HTMValueChanged(int);
	void SOTValueChanged(int);
	void SOLOStateChanged(int);
    void newDeviceSlot(const QString&, const QString&);
	void logEnabledChanged(int);
	void modeChanged(int);
    void tcpStatusUpdate(const QString&);
	void tcpStatusChange();
	void searchIntervalChanged(int);
	void Global_ConvertChanged(bool);
    void searchFinished();
protected:
	void mousePressEvent(QMouseEvent *event);
};


#define try_ecogest			\
	QString exeptionLogStr;	\
	try				\
	{


#define catch_ecogest								\
	}									\
	catch(...)								\
	{									\
		QDateTime now = QDateTime::currentDateTime();			\
		QFile file("exception.log");					\
		if (file.open(QIODevice::Append | QIODevice::Text))		\
		{								\
			QTextStream out(&file);					\
			out << now.toString() + "  " + exeptionLogStr + "\n";	\
			file.close();						\
		}								\
		newThread();							\
	}


#endif	// ECOGEST_H
