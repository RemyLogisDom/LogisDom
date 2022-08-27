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



#ifndef devD20112_H
#define devD20112_H
#include "onewire.h"
#include "ui_eoD20112.h"

class eoD20112 : public onewiredevice
{
	Q_OBJECT
#define SwitchON "SwitchON"
#define SwitchOFF "SwitchOFF"
public:
    eoD20112(net1wire *Master, logisdom *Parent, QString RomID);
	void setconfig(const QString &strsearch);
    void GetConfigStr(QString &str);
    QString MainValueToStrLocal(const QString &str);
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    QString getSecondaryValue();
    void remoteCommandExtra(const QString &command);
    void On(bool send);
    void Off(bool send);
// Palette setup
    QPushButton ButtonOn, ButtonOff;
    QCheckBox StringValue;
    QLabel LastRadioMessage;
    QSpinBox timeOut;
private:
        Ui::D20112ui ui;
        bool isSwitchFamily();
        QString lastScratchpad;
        QTimer checkLastMessage;
        void getStatus();
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
    void checkElaspedTime();
};


#endif
