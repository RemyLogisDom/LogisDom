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



#ifndef LEDONEWIRE_H
#define LEDONEWIRE_H

#include <QObject>

#include "axb.h"
#include "onewire.h"
#include "ui_ledonewire.h"


class ledonewire : public onewiredevice
{
    Q_OBJECT
#define setPower "61"
#define setPowerOn "62"
#define setPowerOff "63"
#define voletOpen "64"
#define voletClose "65"
#define setTTV "6A"
#define setModeBright "setmodebright"
#define setModeDim "setmodedim"
#define setModeOn "setmodeon"
#define setModeOff "setmodeoff"
#define setModeFullOn "setmodefullon"
#define setModeSleep "setmodesleep"

public:
	ledonewire(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	void writescratchpad();
    void assignMainValueLocal(double value);
    double getMaxValue();
    bool isSwitchFamily();
    bool isDimmmable();
    bool isManual();
    bool lastManual;
    void addCRC8(QString &data);
    void sendCommand(const QString hexstr);
    void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
    void SetOrder(const QString &order);
    QString ModeTxt, ModeBrightTxt, ModeDimTxt, ModeOnTxt, ModeOffTxt, ModeFullOnTxt, ModeSleepTxt;
    void remoteCommandExtra(const QString &command);
    bool modSleepEnabled;
    void Dim(int dimValue);
    void setPowerValue(int value);
    int voletState;
    int ledState;
    bool manual;
// Palette setup
    QPushButton ButtonOn;
    QPushButton ButtonFullOn;
    QPushButton ButtonOff;
    QPushButton ButtonDim;
    QPushButton ButtonBright;
    QSpinBox TTV;
    QPushButton ButtonWriteTTV;
private:
    Ui::ledonewireui ui;
	QString zone;
    int lastOnValue;
private slots:
    void clickOn();
    void clickFullOn();
    void clickOff();
    void clickDim();
    void clickBright();
    void clickWriteTTV();
    void contextMenuEvent(QContextMenuEvent * event);
    void On(bool);
    void Off(bool);
    void clickOpen();
    void clickClose();
    void clickSleep();
    void clickSetVolet();
signals:
	void stateChanged();
};


#endif // LEDONEWIRE_H
