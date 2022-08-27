#ifndef ECOGESTNOTHREAD_H
#define ECOGESTNOTHREAD_H
#include "net1wire.h"

#ifdef MultiGest_No_Thread

#include "globalvar.h"
#include "ui_multigest.h"
#include "uploaderwindow.h"


class formula;



class ecogest : public net1wire
{
	Q_OBJECT
#define familyMultiGestValve "MV"
#define familyMultiGestValve1 "V1MV"
#define familyMultiGestValve2 "V2MV"
#define familyMultiGestServo "MS"
#define familyMultiGestServo1 "S1"
#define familyMultiGestServo2 "S2"
#define familyMultiGestServo3 "S3"
#define familyMultiGestServo4 "S4"
#define familyMultiGestSwitchHP "HPMS"
#define familyMultiGestSwitchSP "SPMS"
#define familyMultiGestSwitchCR "CRMS"
#define familyMultiGestWarn "MW"
#define familyMultiGestFlow "MF"
#define familyMultiGestFlowA "FAMF"
#define familyMultiGestFlowB "FBMF"
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
			devSwitchA, devSwitchB, devSwitchC, devSwitchD, devSwitchE, devSwitchF, devFluidicMode, devSolarMode, allDetector
		};
	ecogest(logisdom *Parent);
	~ecogest();
	QString getStr(int index);
	void init();
	void createonewiredevices();
	void getConfig(QString &str);
	void setConfig(const QString &strsearch);
	bool isUploading();
	bool uploadStarted;
	bool isGlobalConvert();
	QStringList RomIDs, Channels;
	QTimer converttimer;
	void pausefifo(int delay);
public slots:
	void convert();
	void getScratchPads();
	void saveScratchPads();
	void initialsearchreq();
	void setDefaultNames();
private:
	Ui::multigest uiw;
	void getMainValue();
	void setGUImode(int mode);
	void setGUIdefaultMode();
	void setGUIAutoVidangeMode_A();
	void catchLoaclStrings();
	void removeDuplicates();
	void LocalSearchAnalysis(QString &data);
	onewiredevice *devices[allDetector];
	QTimer TimeOut;
	QTimer TimerPause;
	bool busy;
	onewiredevice *Prequest;
	QString RomIDstr[allDetector];
	QString buttonstext[LastDetector];
	QString buttonFlowAText, buttonFlowBText, buttonHeaterPumpText, buttonSolarPumpText, buttonCirculatorText;
	QString buttonFillText, buttonKeepFilledText, buttonInterlockText, buttonSolarVirtualText;
	void updatebuttonsvalues();
	int wait;
	void SetStatusValues(QString &data, bool enregistremode);
	void saveMainValue();
	void appendConfigStr(QString *str, QString RomID, QString Value);
	bool Interlock;
	bool SolarVirtualStart;
	bool MSDEnable;
	void connectAll();
	void disconnectAll();
	QButtonGroup fluidicGroup, SolarGroup;
	QString version;
	void restart();
	void endGetVersion(QString &data);
	QMutex mutexFifonext;
	QMutex mutexReadBuffer;
	QString currentRequest;
	int request;
	int unkownCommandRetry, busyRetryCount, retry;
	QString Buffer;
	int modeProcess;
	void simpleend();
	void commandRetry();
	void busyRetry();
	QString extractBuffer(const QString &data);
	void checkFamily(const QString &RomID, const QString &Channel);
	void createDevice(const QString &RomID, const QString &Channel);
	void foundDevice(onewiredevice *device, const QString Channel);
	void endconvert();
	void GlobalSearchAnalysis(QString &data);
	uploaderwindow *picuploader;
private slots:
	void remoteCommand(const QString &command);
	void setHeaterPump();
	void setSolarPump();
	void setCirculator();
	void setFill();
	void setKeepFilled();
	void setSwitchF();
	void search();
	void status();
	void clearRomIDEEprom();
	void readExtrabuffer();
	void OpenUpload();
	void CloseUpload();
	void timeout();
	void rightclicklList(const QPoint &pos);
	void timerconvertout();
	void solarModeClicked(int);
	void fluidicModeClicked(int);
	void virtualStartClicked();
	void MSDEnableChanged();
	void FillValueChanged(int);
	void SolarValueChanged(int);
	void TankValueChanged(int);
	void HeatingValueChanged(int);
	void SolarDelayONChanged(int);
	void FillDelayValueChanged(int);
	void DegazDelayValueChanged(int);
	void KeepFilledDelayValueChanged(int);
	void TRCValueChanged(int);
	void TRPValueChanged(int);
	void fifonext();
	void FIFOnext();
	void readbuffer();
protected:
	void mousePressEvent(QMouseEvent *event);
};

#endif // MultiGest_No_Thread
#endif	// ECOGESTNOTHREAD_H
