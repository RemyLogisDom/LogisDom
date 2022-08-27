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



#ifndef LCDONEWIRE_H
#define LCDONEWIRE_H

#include <QObject>
#include <QGroupBox>

#include "axb.h"
#include "onewire.h"
#include "linecoef.h"
#include "ui_lcdonewire.h"

class devchooser;

#define writeMemoryLCD "F5"
#define writePIO "5A"
#define readMemoryLCD "FA"
#define ADDR_Suffix 32
#define ADDR_Prefix 16
class lcdonewire : public onewiredevice
{
    Q_OBJECT

public:
    lcdonewire(net1wire *Master, logisdom *Parent, QString RomID);
    ~lcdonewire();
	QString MainValueToStrLocal(const QString &str);
    void sendText(double Value);
    void lecture();
	void lecturerec();
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    double calcultemperature(const QString &THex);
    void assignMainValueLocal(double value);
    double getMaxValue();
    bool isSwitchFamily();
    bool isDimmmable();
    void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
    void SetOrder(const QString &order);
    QString toHex(QByteArray data);
    QString fromHex(const QString hexstr);
    void sendWriteValText(const QString hexstr);
    void sendReadMemory(const QString hexstr);
    void setLCDConfig(int Address, const QString Data);
    int assignTry;
    double assignedValue;
    double lastCopyValue;
    QString textDisplay;
    onewiredevice *connectedDevice;
// Palette setup
    //axb AXplusB;
    QPushButton ButtonWrite;
    QSpinBox writeValue;
    QPushButton ButtonWriteValText, ButtonWritePrefix, ButtonWriteSuffix, ButtonWriteConfig, ButtonReadConfig;
    QLineEdit prefixStr, textValue, suffixStr;
    QSpinBox Fontsize, TextFontSize, XPos, YPos, XPosTxt, YPosTxt, Digits, Virgule, CoefMult, CoefDiv;
    QLabel CoefLabel;
    lineCoef Coef;
    QGroupBox boxConfig;
    QGridLayout configLayout;
    QLabel copyValueLabel;
    devchooser *copyValue;
private:
    Ui::lcdonewireui ui;
private slots:
	void contextMenuEvent(QContextMenuEvent * event);
    void writePort();
    void writeValTxt();
    void writePrefix();
    void writeSuffix();
    void writeConfig();
    void readConfig(int ID = 0, int Address = 0);
    void deviceSelectionChanged();
    void DeviceValueChanged(onewiredevice *device);
signals:
	void stateChanged();
};


#endif // LCDONEWIRE_H
