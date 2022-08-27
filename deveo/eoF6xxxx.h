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



#ifndef devF6XXXX_H
#define devF6XXXX_H
#include "onewire.h"
#include "ui_eoF6xxxx.h"

class eoF6XXXX : public onewiredevice
{
	Q_OBJECT
#define SwitchON "SwitchON"
#define SwitchOFF "SwitchOFF"
enum modeF6XXXX{ Switch, DelaiOn, DelaiOff, Toggle, Value };
public:
    eoF6XXXX(net1wire *Master, logisdom *Parent, QString RomID);
	void setconfig(const QString &strsearch);
    void GetConfigStr(QString &str);
    QString MainValueToStrLocal(const QString &str);
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    QString getSecondaryValue();
    void remoteCommandExtra(const QString &command);
    void On(bool send);
    void Off(bool send);
    QTimer delaiTimer;
// Palette setup
    QPushButton ButtonOn, ButtonOff;
    QCheckBox StringValue;
    QComboBox SwitchType;
    QSpinBox delai;
    QSpinBox OnValue;
    QLabel LastRadioMessage;
private:
    Ui::F6XXXXui ui;
    bool isSwitchFamily();
private slots:
	void contextMenuEvent(QContextMenuEvent * event);
    void TimerFinished();
    void changeType(int);
};


#endif
