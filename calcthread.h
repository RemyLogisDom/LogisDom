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



#ifndef CALCTHREAD_H
#define CALCTHREAD_H
#include <QtGui>
#include "calc.h"

class formula;
class formulasimple;
class onewiredevice;
class MailSender;

class calcthread : public calc
{
    Q_OBJECT
    friend class reprocessthread;
    friend class formula;
public:
    #define DspBufferLengthMax 10
enum oper
{
    ABS, INT, MAX, MIN, ZeroIfNeg, ZeroIfPos, ZeroIfSup, ZeroIfInf, EXP, LOG, Hysteresis, BitAnd, BitOr, BitXor, BitLeft, BitRight,
    CurrentSkyClearance, MeanSkyClearance, SkyClearance, TemperatureHi, TemperatureLow, WindSpeed, Humidity, WindDirection, ActualSunPower, DaySunPower, Rain,
    ValueRomID, MeanRomID, SumRomID, LSumRomID, CountRomID, DataCount, MaxRomID, MinRomID, SlopeMeanRomID, SlopeSumPosRomID, SlopeSumNegRomID, Step, CurrentYear, CurrentMonth, CurrentWeek, CurrentDay,
    YearBefore, MonthBefore,
    Minute, Hour, DayOfWeek, DayOfMonth, MonthOfYear, Year, SunSet, SunRise,
    ProgramValue, WeekProgValue, PID, DSP, webParse, webMail, webSms, lastOperator
};
enum family
{
    famOperator, famDevices, famMath, famRomID, famTime, famWeather, webFamily, lastFamily
};
    calcthread(formula *Parent);
    formula *parent = nullptr;
    QString *logStr = nullptr;
    bool stopRequest;
    bool calculating;
    QString lastCalc, lastCalcStr, lastCalcClean;
    QString originalCalc;
    void get(const QString &Request, QString &Data);
    static QString op2Str(int index);
    QString op2Function(int index);
    int op2Family(int index);
    static QString family2Str(int index);
    QString op2Description(int index);
    QString getID(QString &str);
    double threadResult;
    double lastValueSendMail;
    double lastValueHysteresis;
    void setCalc(const QString &str);
    double calculate(const QString &str);
    double calculate();
    void clearAutoEnabled();
    void setAutoEnabled();
    void clearLinkedEnabled();
    void setLinkedEnabled();
    onewiredevice *getLinkedDevice();
    QList <onewiredevice*> deviceList;
private:
    #define NPOLES 20
    double xv[NPOLES+1], yv[NPOLES+1];
    double DSPGain, lastDSPGain;
    onewiredevice *checkDevice(const QString &RomID);
    int DSPpole, lastDSPpole;
    onewiredevice *DSPdevice, *lastDSPdevice;
    double lastValue, integral, previousError;
    double toNumeric(const QString &S, bool *ok);
    QDate toDate(const QString &S, bool *ok);
    void scroolDSP();
    void resetDSP();
    bool restartDSP;
    QString valueOnError;
    QString search(const QStringList &list, const QString &str);
    int sendMailRetry;
    double runOP(const QString &OP, const QString &C, bool *ok);
    bool resoudreParenthese();
    double getValueFromP(const QString &S);
    double Abs(const QString &S);
    double Entier(const QString &S);
    double Max(const QString &x, const QString &max);
    double Min(const QString &x, const QString &min);
    double ZifNeg(const QString &S);
    double ZifPos(const QString &S);
    double ZifSup(const QString &S, const QString &val);
    double ZifInf(const QString &S, const QString &val);
    double Exp(const QString &a);
    double Log(const QString &a);
    double Hyst(const QString &x, const QString &v1, const QString &v2, const QString &def);
    double getCurrentSkyClearance();
    double getActualTempHi(const QString &NdDays);
    double getActualTempLow(const QString &NdDays);
    double getActualHumidity(const QString &NdDays);
    double getActualWind(const QString &NdDays);
    double getActualWindDir(const QString &NdDays);
    double getMeanSkyClearance(const QString &NdDays);
    double getSkyClearance(const QString &NdDays);
    double getRain(const QString &NdDays);
    double getDaySunPower();
    double getActualSunPower();
    double getValueRomID(const QString &RomID, const QString &Minutes, const QString &Width);
    double getMeanRomID(const QString &RomID, const QString &Minutes);
    double getMeanRomID(const QString &RomID, const QString &Minutes, const QString &Length);
    double getMaxRomID(const QString &RomID, const QString &Minutes);
    double getMaxRomID(const QString &RomID, const QString &Minutes, const QString &Length);
    double getMinRomID(const QString &RomID, const QString &Minutes);
    double getMinRomID(const QString &RomID, const QString &Minutes, const QString &Length);
    double getSumRomID(const QString &RomID, const QString &Minutes);
    double getSumRomID(const QString &RomID, const QString &Minutes, const QString &Length);
    double getLSumRomID(const QString &RomID, const QString &Minutes, const QString &Length);
    double getCountRomID(const QString &RomID, const QString &Minutes, const QString &Level);
    double getCountRomID(const QString &RomID, const QString &Minutes, const QString &Level, const QString &Length);
    double getDataCount(const QString &RomID, const QString &Minutes);
    double getDataCount(const QString &RomID, const QString &Minutes, const QString &Length);
    double getSlopeMeanRomID(const QString &RomID, const QString &Minutes);
    double getSlopeSumPosRomID(const QString &RomID, const QString &Minutes, const QString &Threshold);
    double getSlopeSumNegRomID(const QString &RomID,const  QString &Minutes, const QString &Threshold);
    double getStep(const QString &value, const QString &step);
    double getCurrentMonth(const QString &value);
    double getCurrentYear(const QString &value);
    double getCurrentWeek(const QString &value);
    double getCurrentDay(const QString &value);
    double getYearBefore(const QString &value);
    double getMonthBefore(const QString &value);
    double getMinute();
    double getHour();
    double getDayOfWeek();
    double getDayOfMonth();
    double getMonthOfYear();
    double getYear();
    double getSunSet();
    double getSunRise();
    double getProgramValue(const QString value);
    double getWeekProgValue(const QString value);
    double getPID(const QString actual_Position, const QString setPoint, const QString KP, const QString KI, const QString KD);
    double getDSP(const QString &RomID, const QString &Poles, const QString &Gain, const QString &Polynome);
    double DspBuffer[DspBufferLengthMax];
    double getBitAnd(const QString &,const  QString &, const QString &);
    double getBitOr(const QString &,const  QString &, const QString &);
    double getBitXor(const QString &,const  QString &, const QString &);
    double getBitLeft(const QString &, const QString &, const QString &);
    double getBitRight(const QString &, const QString &, const QString &);
public slots:
    void runprocess();
signals:
    void calcRequest();
    void calcFinished(const QString);
    void redirectHttp(const QString);
};


#define ManageError			\
        if (!(syntax && dataValid)) return false;

#define ResultError			\
        if (!(syntax && dataValid)) return logisdom::NA;


#if QT_VERSION > 0x050603

#define FunctionGetData		\
qint64 minutes = 0; \
qint64 end = 0; \
qint64 begin = 0; \
QDate Date; \
bool ok = false; \
dataloader::s_Data data; \
if (isNumeric(Minutes)) minutes = Minutes.toInt(&ok); \
    else if (!isDate(Minutes, Date, &ok)) minutes = qint64(toNumeric(Minutes, &ok)); \
if (!ok) minutes = 0; \
if (!ok) { syntaxError(tr("Time parameter error ") + Minutes); ResultError }\
onewiredevice *device = checkDevice(RomID); \
if (!device) { dataError(tr("Device not found ") + RomID); ResultError }\
if (TCalc) \
{ \
    QDateTime T; \
    T = *TCalc; \
    begin = T.toSecsSinceEpoch(); \
} \
else \
{ \
    QDateTime now = QDateTime::currentDateTime(); \
    begin = now.toSecsSinceEpoch(); \
} \
if (Date.isValid()) \
{ \
    QDateTime end_DateTime; \
    end_DateTime.setTime(QTime(0, 0, 0)); \
    end_DateTime.setDate(Date); \
    end = end_DateTime.toSecsSinceEpoch(); \
} \
else \
{ \
    end = begin - (minutes * 60); \
} \
ok = device->getValues(begin, end, deviceLoading, data, parent);\
if (deviceLoading) dataError(tr("Function didn't find data, device is loading data")); ResultError \
if (!ok) \
{ \
    if (Date.isValid()) textBrowserResult += "\n" + QString("Date is ") + Date.toString(); \
    else textBrowserResult += "\n" + QString("Date is not valid"); \
    textBrowserResult += "\n" + QString("begin is %1").arg(begin); \
    textBrowserResult += "\n" + QString("end is %1").arg(end); \
    dataError(tr("Function didn't find data for the gap specified")); ResultError\
} \


#define FunctionGetDatawLength		\
qint64 minutes = 0; \
qint64 end = 0; \
qint64 begin = 0; \
qint64 length = 0; \
QDate Date; \
QDate DateLength; \
bool ok = false; \
dataloader::s_Data data; \
if (isNumeric(Minutes)) minutes = Minutes.toInt(&ok); \
    else if (!isDate(Minutes, Date, &ok)) minutes = qint64(toNumeric(Minutes, &ok)); \
if (!ok) minutes = 0; \
if (!ok) syntaxError(tr("Time parameter error ") + Minutes); ResultError \
if (isNumeric(Length)) length = Length.toInt(&ok); \
    else if (!isDate(Length, DateLength, &ok)) length = qint64(toNumeric(Length, &ok)); \
if (!ok) length = 0; \
if (!ok) syntaxError(tr("Length parameter error ") + Length); ResultError \
onewiredevice *device = checkDevice(RomID); \
if (!device) dataError(tr("Device not found ") + RomID); ResultError \
if (Date.isValid()) begin = QDateTime(Date, QTime(0, 0)).toSecsSinceEpoch(); \
else \
{ \
    if (TCalc) \
    { \
        QDateTime T; \
        qint64 delta = minutes - length; \
        if (delta < 0) delta = 0; \
        T = *TCalc; \
        begin = T.toSecsSinceEpoch() - (delta * 60); \
    } \
    else \
    { \
        QDateTime now = QDateTime::currentDateTime(); \
        qint64 delta = minutes - length; \
        if (delta < 0) delta = 0; \
      begin = now.toSecsSinceEpoch() - (delta * 60); \
    } \
} \
if (DateLength.isValid()) \
{ \
    QDateTime end_DateTime; \
    end_DateTime.setTime(QTime(0, 0, 0)); \
    end_DateTime.setDate(DateLength); \
  end = end_DateTime.toSecsSinceEpoch(); \
} \
else \
{ \
    end = begin - (length * 60); \
} \
ok = device->getValues(begin, end, deviceLoading, data, parent);\
if (deviceLoading) { dataError(tr("Function didn't find data, device is loading data")); ResultError }\
if (!ok) { dataError(tr("Function didn't find data for the gap specified")); ResultError }\


#else


#define FunctionGetData		\
QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0)); \
qint32 minutes = 0; \
qint32 end = 0; \
qint32 begin = 0; \
QDate Date; \
bool ok = false; \
dataloader::s_Data data; \
if (isNumeric(Minutes)) minutes = Minutes.toInt(&ok); \
    else if (!isDate(Minutes, Date, &ok)) minutes = (qint32)toNumeric(Minutes, &ok); \
if (!ok) minutes = 0; \
if (!ok) { syntaxError(tr("Time parameter error ") + Minutes); ResultError }\
onewiredevice *device = checkDevice(RomID); \
if (!device) { dataError(tr("Device not found ") + RomID); ResultError }\
if (TCalc) \
{ \
    QDateTime T; \
    T = *TCalc; \
    begin = origin.secsTo(T); \
} \
else \
{ \
    QDateTime now = QDateTime::currentDateTime(); \
    begin = origin.secsTo(now); \
} \
if (Date.isValid()) \
{ \
    QDateTime end_DateTime; \
    end_DateTime.setTime(QTime(0, 0, 0)); \
    end_DateTime.setDate(Date); \
    end = origin.secsTo(end_DateTime); \
} \
else \
{ \
    end = begin - (minutes * 60); \
} \
ok = device->getValues(begin, end, deviceLoading, data, parent);\
if (deviceLoading) { dataError(tr("Function didn't find data, device is loading data")); ResultError }\
if (!ok) \
{ \
    if (Date.isValid()) textBrowserResult += "\n" + QString("Date is ") + Date.toString(); \
    else textBrowserResult += "\n" + QString("Date is not valid"); \
    textBrowserResult += "\n" + QString("begin is %1").arg(begin); \
    textBrowserResult += "\n" + QString("end is %1").arg(end); \
    dataError(tr("Function didn't find data for the gap specified")); ResultError\
} \


#define FunctionGetDatawLength		\
QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0)); \
qint32 minutes = 0; \
qint32 end = 0; \
qint32 begin = 0; \
qint32 length = 0; \
QDate Date; \
QDate DateLength; \
bool ok = false; \
dataloader::s_Data data; \
if (isNumeric(Minutes)) minutes = Minutes.toInt(&ok); \
    else if (!isDate(Minutes, Date, &ok)) minutes = (qint32)toNumeric(Minutes, &ok); \
if (!ok) minutes = 0; \
if (!ok) { syntaxError(tr("Time parameter error ") + Minutes); ResultError }\
if (isNumeric(Length)) length = Length.toInt(&ok); \
    else if (!isDate(Length, DateLength, &ok)) length = (int)toNumeric(Length, &ok); \
if (!ok) length = 0; \
if (!ok) { syntaxError(tr("Length parameter error ") + Length); ResultError }\
onewiredevice *device = checkDevice(RomID); \
if (!device) { dataError(tr("Device not found ") + RomID); ResultError }\
if (Date.isValid()) begin = origin.secsTo(QDateTime(Date, QTime(0, 0))); \
else \
{ \
    if (TCalc) \
    { \
        QDateTime T; \
        qint32 delta = minutes - length; \
        if (delta < 0) delta = 0; \
        T = *TCalc; \
        begin = origin.secsTo(T) - (delta * 60); \
    } \
    else \
    { \
        QDateTime now = QDateTime::currentDateTime(); \
        qint32 delta = minutes - length; \
        if (delta < 0) delta = 0; \
        begin = origin.secsTo(now) - (delta * 60); \
    } \
} \
if (DateLength.isValid()) \
{ \
    QDateTime end_DateTime; \
    end_DateTime.setTime(QTime(0, 0, 0)); \
    end_DateTime.setDate(DateLength); \
    end = origin.secsTo(end_DateTime); \
} \
else \
{ \
    end = begin - (length * 60); \
} \
ok = device->getValues(begin, end, deviceLoading, data, parent);\
if (deviceLoading) { dataError(tr("Function didn't find data, device is loading data")); ResultError }\
if (!ok) { dataError(tr("Function didn't find data for the gap specified")); ResultError }\

#endif


#endif




