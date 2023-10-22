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



#include <QtGui>
#include <QtNetwork>
#include <QtCore>
#include "commonstring.h"
#include "logisdom.h"


#define SecsInDays 86400
#define Minutes2Weeks 1209600
#define dotspercurve 1000
#define simulator "Simulator"
#define repertoirecsv "csv"
#define repertoirelog "log"
#define repertoirepng "png"
#define repertoirewav "wav"
#define repertoirehex QString("..") + QDir::separator() + QString("MultiGest")
#define configfilenamerestart "maison.rst"
#define devicefilename "device.cfg"
#define clientfilename "remote"
#define repertoiresimulator "simulator"
#define repertoireicon "icon"
#define familySTA800 "PV"
#define family1822 "22"
#define family1825 "3B"
#define family1820 "10"
#define family18B20 "28"
#define family2406 "12"
#define family2406_A "12_A"
#define family2406_B "12_B"
#define family2408 "29"
#define family2408_A "29_A"
#define family2408_B "29_B"
#define family2408_C "29_C"
#define family2408_D "29_D"
#define family2408_E "29_E"
#define family2408_F "29_F"
#define family2408_G "29_G"
#define family2408_H "29_H"
#define family2413 "3A"
#define family3A2100H "85"
#define isFamily2413 (family == family2413) || (family == family3A2100H)
#define family2413_A "3A_A"
#define family2413_B "3A_B"
#define family3A2100H_A "85_A"
#define family3A2100H_B "85_B"
#define isFamily2413_A (family == family2413_A) || (family == family3A2100H_A)
#define isFamily2413_B (family == family2413_B) || (family == family3A2100H_B)
#define family2423 "1D"
#define family2423_A "1D_A"
#define family2423_B "1D_B"
#define family2438 "26"
#define family2438_T "26_T"
#define family2438_V "26_V"
#define family2438_A "26_A"
#define family2438_I "26_I"
#define family2450 "20"
#define family2450_A "20_A"
#define family2450_B "20_B"
#define family2450_C "20_C"
#define family2450_D "20_D"
#define familyAM12 "AM12"
#define familyLM12 "LM12"
#define familyLedOW "7F"
#define familyLCDOW "7E"
#define familyLCDOW_A "7E_A"
#define familyLCDOW_B "7E_B"
#define familyLCDOW_C "7E_C"
#define familyLCDOW_D "7E_D"
#define familyPLCBUS "PLCB"
#define familyTeleInfo "TI"
#define familyRps2 "RP"
#define familyResol "RS"
#define familyMBus "MB"
#define familyEOcean "EO"
#define familyeoA52001 "A52001EO"
#define familyeoA502XX "A502XXEO"
#define familyeoA504XX "A504XXEO"
#define familyeoA504XX_T "A504XXEO_T"
#define familyeoA504XX_H "A504XXEO_H"
#define familyeoF60201 "F60201EO"
#define familyeoF60201 "F60201EO"
#define familyeoD2010F "D2010FEO"
#define familyeoD20112 "D20112EO"
#define familyeoD20112_A "D20112EO_A"
#define familyeoD20112_B "D20112EO_B"
#define familyeoF6XXXX "F6XXXXEO"
#define familyModBus "MD"
#define familyMultiGestValve "MV"
#define familyMultiGestSwitch "MS"
#define familyMultiGestWarn "MW"
#define familyMultiGestFlow "MF"
#define familyMultiGestMode "MM"
#define familyVirtual "VD"
#define algomax 100
#define default_port_EZL50 1470
#define default_port_remote 1220
#define defaultTempUnit 1	// 1 = Celsius 0 = Fahrenheit
#define default_wait 500
#define X_default 10
#define Y_default 80
#define Net1Wire_Device "Net1Wire_Device"
#define Remote_Mode "Remote_Mode"
#define Server_Mode "Server_Mode"
#define Main_Window "Main_Window"
#define Icon_Tab_Begin "Icon_Tab_Begin"
#define End_Icon_Tab "End_Icon_Tab"
#define Icon_Begin "Icon_Begin"
#define Icon_Finished "Icon_Finished"
#define Device_Value_Tag "Value"
#define One_Wire_Device "One_Wire_Device"
#define New_Curve_Begin "New_Curve_Begin"
#define New_Curve_Finished "New_Curve_Finished"
#define New_Histo_Begin "New_Histo_Begin"
#define New_Histo_Finished "New_Histo_Finished"
#define Remote_User "Remote_User"
#define Remote_User_Name "Remote_User_Name"
#define Remote_User_Psw "Remote_User_Psw"
#define Remote_User_Privilege "Remote_User_Privilege"
#define Server_Port "Server_Port"
#define Server_Listening "Server_Listening"
#define RomIDMark "RomID"
#define ScratchPadMark "Scratchpad"
#define RedMark "Red"
#define GreenMark "Green"
#define BlueMark "Blue"
#define Window_Width_Mark "Window_Width"
#define Window_Height_Mark "Window_Height"
#define Daily_Unit_Mark "Daily Unit"
#define Solar_Mark "Solar"
#define New_Point_Begin "New_Point_Begin"
#define New_Point_Finished "New_Point_Finished"

#define EndMark "End"
#define RomIDTag "RomID"
#define File_not_found "File not found"
#define compdat_ext ".zat"
#define dat_ext ".dat"

#define DragIcons "LogisDom/x-newicon"
#define MoveIcons "LogisDom/x-moveicon"
#define htmlFormatBegin "<P align=center><FONT size=2>"
#define htmlFormatEnd "</FONT></P>"

#define MainMinXSize 100
#define MainMinYSize 100
#define ProgMinSize 80




#define tryMacro			\
    QString exeptionLogStr;/*	\
    try				\
    {*/


#define catchException_logstr											/*\
    }														\
    catch(...)													\
    {														\
        QMessageBox::information(0, exeptionLogStr, exeptionLogStr);			\
        QDateTime now = QDateTime::currentDateTime();						\
        QFile file(QString(repertoirelog) + QDir::separator() + "exception.log");	\
        if (file.open(QIODevice::Append | QIODevice::Text))					\
        {													\
            QTextStream out(&file);									\
            out << now.toString() + "  " + exeptionLogStr + "\n";				\
            file.close();										\
        }													\
        throw exeptionLogStr;										\
    }*/



    //	GenMsg("cannot read configuration file  " configfilename);


#define OpenConfigFile												\
    QString configdata;  QFile file(configfilename); \
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) \
    { \
    } \
    else \
    { \
        QTextStream in(&file); \
                configdata.append(in.readAll()); \
        file.close(); \
    }





#define SearchLoopBegin												\
int index_a = 0, index_b = 0, index = 0;									\
QString strsearch;												\
index_a = configdata.indexOf("\n" + TAG_Begin, 0);								\
do															\
    {														\
    if (index_a != -1)											\
        {													\
         index_b = configdata.indexOf("\n" + TAG_End, index_a);					\
         if (index_b != -1)										\
        {													\
            strsearch = configdata.mid(index_a, index_b - index_a);


#define SearchLoopEnd												\
        }													\
            }												\
        index_a = configdata.indexOf("\n" + TAG_Begin, index_b);					\
        index ++;												\
        }													\
    while(index_a != -1);



extern logisdom *maison1wirewindow;

extern bool isValidIp(QString &ip);
extern bool checklocaldir(QString dir);
extern QString toHtml(QString Text, QString Order, QString &ID);
extern QString TempConvertF2C(QString TinFahrenheit);



#ifndef GLOBALVAR
#define GLOBALVAR


enum NetRequest
	{
		None, GetLock, ReleaseLock, NetError, Reset, Search, MatchRom, SkipROM, ConvertTemp, ConvertTempRomID,
        ReadTemp, ReadTempRec, ReadDualSwitch, ReadDualSwitchRec, ReadDualAdrSwitch, ReadDualAdrSwitchRec, WriteScratchpad, WriteScratchpad00, CopyScratchpad00, SendCmd,
        ReadCounter, ReadCounterRec, ReadLCD, ReadPIO, ReadRecPIO, ChannelAccessRead, ChannelAccessWrite, setChannelOn, setChannelOff, WritePIO,
        ConvertADC, ReadADC, ReadADCRec, ReadADCPage01h, ConvertV, ReadPage, ReadPage00h, ReadPage01h, ReadPageRec, ReadPageRec00h, ReadPageRec01h, WriteMemory, WriteMemoryLCD, WriteValText, ReadMemoryLCD, WriteLed,
        RecallMemPage00h, RecallMemPage01h, RecallEEprom, WriteEEprom,
	// FTS800
		Status, Synchro, SendOrder,
	// X10
		X10Address, X10Function, X10Status, SwitchON, SwitchOFF,
	// PCLBUS
		PCLBUSCommand,
	// Remote
		SetUserName, SetPassWord, GetConfigFile, GetDevicesConfig, GetDeviceScratchpad, SaveDeviceScratchpad, SendDeviceScratchpad, GetFile, GetDatFile, setFolder, SendCommand, TransferCommand,
		GetMainValue, SaveMainValue, SendMainValue, GetMainMenu, GetAllValues, MenuLights, MenuTemperatures, MenuHeating, MenuProgram, MenuConfig,
		SetDevice, SwitchProgram,
		LoadConfig, UpdateConfig,
	// EcoGest
		GetScratchPads, SaveScratchPads, GetStatus, SaveStatus, LocalSearch, GlobalSearch, SearchReset, SetMode, SetValue, TRegulChauffage, TReguPanneau, Restart, GetFirmwareVersion,
	// System
		MenuRestart, ConfirmRestart,
		LastRequest
	};


static const QString NetRequestMsg[LastRequest] = 
	{	"None", "GetLock", "ReleaseLock", "Error", "Reset", "Search", "MatchRom", "SkipROM", "CVT", "ConvertTempRomID",
        "ReadTemp", "ReadTempRec", "ReadDualSwitch", "ReadDualSwitchRec", "ReadDualAdrSwitch", "ReadDualAdrSwitchRec", "WriteScratchpad", "WriteScratchpad00", "CopyScratchpad00", "SendCmd",
        "ReadCounter", "ReadCounterRec", "ReadLCD", "ReadPIO", "ReadRecPIO", "ChannelAccessRead", "ChannelAccessWrite", "setChannelOn", "setChannelOff", "WritePIO",
        "ConvertADC", "ReadADC", "ReadADCRec", "ReadADCPage01h", "ConvertV", "ReadPage", "ReadPage00h", "ReadPage01h", "ReadPageRec", "ReadPageRec00h", "ReadPageRec01h", "WriteMemory", "WriteMemoryLCD", "WriteValText", "ReadMemoryLCD", "WriteLed",
        "RecallMemPage00h", "RecallMemPage01h", "RecallEEprom", "WriteEEprom",
	// FTS800
		"Status", "Synchro", "SendOrder",
	// X10
		"X10Address", "X10Function", "X10Status", "SwitchON", "SwitchOFF",
	// PCLBUS
		"PCLBUSCommand",
	// Remote
		"SetUserName", "SetPassWord", "GetConfigFile", "GetDevicesConfig", "GetDeviceScratchpad", "SaveDeviceScratchpad", "SendDeviceScratchpad", "GetFile", "GetDatFile", "setFolder", "SendCommand", "TransferCommand",
		"GetMainValue", "SaveMainValue", "SendMainValue", "GetMainMenu", "GetAllValues", "MenuLights", "MenuTemperatures", "MenuHeating", "MenuProgram", "MenuConfig",
		"SetDevice", "SwitchProgram",
		"LoadConfig", "UpdateConfig",
	// EcoGest
		"GSP", "SSP", "GST", "SST", "LCS", "GBS", "SRR", "SetMode", "SetValue", "TRC", "TRP", "Restart", "Version",
	// System
		"MenuRestart", "ConfirmRestart"
	};



// CM11 codes

const unsigned char CM11_TX_ACK = 0x00;			// acquittement checksum transmition
const unsigned char CM11_POL  = 0x5A;				// polling
const unsigned char CM11_INIT = 0xA5;					// Demande d'initialisation
const unsigned char CM11_POL_ACK = 0xC3;		// acquittement polling
const unsigned char CM11_READY = 0x55;			// interface prète


// constantes Header
const QString X10_ADDR = "4";
const QString X10_FUNC = "6";
const QString X10_EXTENDED_FUNC = "7";

// Constantes Functions
const QString X10_ALL_UNITS_OFF = "0";
const QString X10_ALL_LIGHTS_ON = "1";
const QString X10_ON = "2";
const QString X10_OFF = "3";
const QString X10_DIM = "4";
const QString X10_BRIGHT = "5";
const QString X10_ALL_LIGHTS_OFF = "6";
const QString X10_EXTENDED = "7";
const QString X10_HAIL_REQ = "8";
const QString X10_HAIL_ACK = "9";
const QString X10_STATUS_ON = "D";
const QString X10_STATUS_OFF = "E";
const QString X10_STATUS_REQUEST = "F";

static const int X10Codes[16] =
	{ 0x6, 0xE, 0x2, 0xA, 0x1, 0x9, 0x5, 0xD, 0x7, 0xF, 0x3, 0xB, 0x0, 0x8, 0x4, 0xC };

static const int InvertX10Codes[16] =
	{ 0xC, 0x4, 0x2, 0xA, 0xE, 0x6, 0x0, 0x8, 0xD, 0x5, 0x3, 0xB, 0xF, 0x7, 0x1, 0x9 };

static const QString X10CodesStr[16] =
	{ "6", "E", "2", "A", "1", "9", "5", "D", "7", "F", "3", "B", "0", "8", "4", "C" };


#endif
