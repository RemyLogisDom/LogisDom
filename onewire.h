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



#ifndef ONEWIRE_H
#define ONEWIRE_H
#include <QtCore>
#include <QtGui>
#include <QGroupBox>

#include "ui_onewiredevice.h"
#include "axb.h"
#include "interval.h"
#include "dataloader.h"
#include "calcthread.h"
#include "formula.h"
#include "htmlbinder.h"
#include "net1wire.h"
#include "ui_stdui.h"
#include "../plugins/interface.h"

#include "stdint.h"

Q_DECLARE_METATYPE(onewiredevice*)

class net1wire;

class onewiredevice : public QDialog
{
	Q_OBJECT
    friend class devfinder;
    friend class ds1820;
	friend class ds18b20;
	friend class ds1822;
    friend class ds1825;
    friend class ds2406;
	friend class ds2408;
	friend class ds2413;
	friend class ds2423;
	friend class ds2438;
	friend class ds2450;
    friend class max31850;
    friend class devfts800;
	friend class devmbus;
	friend class devmodbus;
	friend class devswbin;
	friend class devvirtual;
	friend class devx10;
    friend class eoA52001;
    friend class eoA50401;
    friend class eoA502xx;
    friend class eoA504xx;
    friend class eoF60201;
    friend class eoD2010F;
    friend class eoD20112;
    friend class eoF6XXXX;
    friend class am12;
    friend class lm12;
	friend class ledonewire;
    friend class lcdonewire;
    friend class devpclbus;
	friend class devteleinfo;
	friend class devrps2;
	friend class devresol;
	friend class valveecogest;
	friend class switchecogest;
	friend class signalecogest;
	friend class flowecogest;
	friend class modecogest;
	friend class ecogest;
    friend class eocean;
    friend class ha7s;
	friend class ha7net;
public:
struct CommonRegStruct
{
	int Reg;
};
	enum DataValidation
    { dataNotValid, dataWaiting, ReadRequest, dataValid, dataLoading, finishedLoadingData };
	enum counter_Mode
	{ relative, absolute, offset };
	onewiredevice(net1wire *Master, logisdom *Parent, QString RomID);
	~onewiredevice();
    Ui::stdui stdui;
    htmlBinder *htmlBind;
	htmlBinder *htmlBindControler;
    LogisDomInterface *plugin_interface;
    void setPluginInterface(LogisDomInterface *);
    virtual void remoteCommandExtra(const QString &);
// Palette setup
    axb AXplusB;
    QWidget setup;
	QGridLayout setupLayout;
	QCheckBox logEnabled;
	QCheckBox WarnEnabled;
	QCheckBox skip85;
	QPushButton RenameButton;
	QPushButton ValueButton;
	QLabel traffic, trafficUi, info;
	QPushButton RomIDButton;
	QPushButton GraphButton;
	QPushButton ShowButton;
	QSpinBox Resolution;
	QSpinBox Decimal;
	interval saveInterval;
	interval calculInterval;
    QLineEdit command;

    QLabel lastValue;
    QSpinBox timeOut;

    QElapsedTimer lastMessage;
    QTimer checkLastMessage;
    QString secondaryValue;

    QPushButton SendButton;
	int layoutIndex;
	int VD_Rank;
	QLabel UnitText;
	QLineEdit Unit;
    QLineEdit ScratchPad_Show;
	QCheckBox UsetextValues;
	QComboBox TextValues;
    bool UsetextValuesChanged;
	QString getromid();
	QString getname();
	QString getfamily();
	QString getunit();
    QTreeWidgetItem *listTreeItem = nullptr;
    void setListTreeItem(QTreeWidgetItem*);
    void clearListTreeItem();
    double getMainValue();
    bool getValues(qint64 begin, qint64 end, bool &loading, dataloader::s_Data &data, formula *sender = nullptr);
    double getMainValue(qint64 t, bool &loading, formula *sender = nullptr);
	double getMainValue(const QDateTime &T, bool &loading, formula *sender = nullptr);
    qint64 getNextIndex(const QDateTime &T, bool &loading);
	QList <formula*> senderList;
	int lastSaveIndex;
    QString lastCommand;
    int lastCommandCount = 0;
	QMutex mutexGet;
	QFile getdatfile(QDateTime &T);
	net1wire *getMaster();
    QString getMasterName();
    void getCfgStr(QString &str, bool All = true);
	void setCfgStr(QString &strsearch);
	QString getscratchpad();
	int getHouseCode();
	int getModuleCode();
	void emitDeviceValueChanged();
    void setPluginMainValue(double value);
    virtual void setMainValue(double value, bool enregistremode);
    virtual void setMainValue(QString &txt);
    void assignMainValue(double value);
    virtual void assignMainValueLocal(double value);
    virtual bool readNow();
    virtual double getMaxValue();
    QString MainValueToStr();
	virtual QString MainValueToStrLocal(const QString &str);
    virtual QString ValueToStr(double, bool noText = false);
	QString MainValueToStrNoWarning();
	virtual bool isTempFamily();
	virtual bool isX10Family();
    virtual bool isVanneFamily();
	virtual bool isSwitchFamily();
	virtual bool isVirtualFamily();
	virtual bool isDimmmable();
	virtual bool IncWarning();
	virtual bool isReprocessing();
	bool isValid();
    int readCounter;
	void setname(const QString &Name);
	void setValid(int datavalid);
	virtual void GetConfigStr(QString &str);
	virtual void setconfig(const QString &configdata);
	virtual void send_Value(double val);
	virtual void setHouseCode(int House, int Module);
	virtual void setHouseCode(QString House, QString Module);
	virtual bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    virtual void SetOrder(const QString &order);
	virtual void stopAll();
	virtual QString getOffCommand();
	virtual QString getOnCommand();
    virtual QString getSecondaryValue();
    void savevalue(const QDateTime &now, const double &V, bool intervalCheck = true);
    QDateTime lastTimeSetValue;
	bool isAutoSave();
    bool hastoread();
    void clearData();
	void setHtmlMenulist(QListWidget *List);
	void removeHtmlMenulist(QString name);
	bool hasPreviousDatFile();
	bool zipPreviousDatFile();
	bool fileAlreadyInsideZip(QString datfileName, QString zipFileName);
	bool copyZipWithoutFile(QString datfileName, QString zipFileName);
	volatile CommonRegStruct *commonReg;
	void setMasternullptr();
	int meanTimeCalculation();
	bool isDataLoading();
	dataloader *getdataloader();
    static uint8_t calcCRC8(QVector <uint8_t> s);
    static uint16_t calcCRC16(QVector <uint8_t> s);
    bool checkCRC8(QString);
    bool checkCRC16(QString);
	static const uint8_t  crc8Table[256];   /**< Preinitialize CRC-8 table. */
	static const uint16_t crc16Table[256];  /**< Preinitialize CRC-16 table. */
    void startLoading();
    void setTobeDeleted();
private:
    bool tobeDeleted;
    dataloader *dataLoader;
	bool logisdomReading;		// choose if device his controled by MultiGest or LogisDom for reading request default is true;
	void convertDatToV2(QString fileName);
	QDate lastFreeMem;
	bool check85(double V);
	QList <double> check85Values;
	enum NBits	{ R9bits, R10bits, R11bits, R12bits, maxbits };
	QString NbitsStr(int nbit);
	void logMsg(QString msg);
	int RetryWarning;
	int ReadRetry;
	void ClearWarning();
	QString assignname(const QString Name);
	QString name, romid;
	int resolution, channel[4];
	int valid;
	double MainValue, lastMainValue, TalarmH, TalarmB, lastsavevalue;
	net1wire *master;
    logisdom *parent;
	net1wire *uploader;
	QString scratchpad, family ;
	int X10SleepCount;
	bool firstsave;
	virtual void writescratchpad();
	virtual void changresoultion();
	virtual void changealarmehaute();
	virtual void changealarmebasse();
    virtual void saveDeviceConfig();
    virtual double calcultemperature(const QString &THex);
	QGridLayout *framelayout;
	int HouseCode, ModuleCode;
	int getresolution(QString scratch);
	static double calcultemperaturealarmeH(const QString  &scratch);
	static double calcultemperaturealarmeB(const QString  &scratch);
	void addStdActions(QMenu *contextualmenu);
	void mousePressEvent(QMouseEvent *event);
	QString checkValid(const QString str);
	QList <double> textValList;
	QStringList textStrList;
private slots:
    void sendCommand();
    void checkElaspedTime();
    void openGraph();
    void changename();
    void removeme();
    void unitChanged(const QString &U);
	void DecimalChanged(int d);
	void remoteCommand(const QString &);
	void finishedLoading();
	void logEnabledChanged(int);
	void dataLoaderBeginChanged();
	void RomID_rigthClick(const QPoint &pos);
	void UserText_rigthClick(const QPoint &pos);
    void UserText_Changed(const QString &);
    void setDataLoading();
    void setTraffic(int);
public slots:
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void setdualchannelAOn();
	virtual void setdualchannelAOff();
	void set_On();
	void set_Off();
    virtual void On(bool);
	virtual void Off(bool);
	virtual void Dim(bool);
    virtual void Sleep();
	virtual void SleepStep();
	virtual void Bright(bool emitX10);
	virtual void Bright(int Power, bool emitX10);	// Power = 0 -> 22
	virtual void lecture();
	virtual void lecturerec();
    virtual void masterlecturerec();
    virtual bool isManual();
    void skip85Changed(int);
signals:
	void DeviceValueChanged(onewiredevice*);
	void DeviceConfigChanged(onewiredevice*);
	void setupClick(QWidget*);
    void finishedDataLoading(onewiredevice*);
    void LoadRequest();
    void trafficsig(int);
    void saveDat(QString, QString);
};

#endif
