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



#ifndef ds2438_H
#define ds2438_H
#include "axb.h"
#include "onewire.h"
#include "ui_ds2438.h"



class ds2438 : public onewiredevice
{
	Q_OBJECT
    enum counterMode { absoluteMode, relativeMode, offsetMode };
enum UseMode
    {
        Basic, HumiditySensor, Last
    };
public:
	ds2438(net1wire *Master, logisdom *Parent, QString RomID);
	~ds2438();
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	double calcultemperatureDS2438(const QString &THex);
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
    void SetOrder(const QString &order);
// Palette setup
    //axb AXplusB;
	QWidget configBits;
	QGridLayout configBitsLayout;
	QCheckBox IADButton, CAButton, EEButton, ADButton;
	QPushButton sendCongif;
// for ICA only
    QComboBox counterMode;
    QSpinBox Offset;
    QDoubleSpinBox Coef;
    interval countInit;
    QCheckBox SaveOnUpdate;
    long int LastCounter, Delta;
private:
	Ui::ds2438ui ui;
	void setTAI8540D();
    void setResult(long int NewValue, bool enregistremode);
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots:
	void writeConfig();
    void modeChanged(int);
    void saveOnUpdateChanged(int state);
};


#endif
