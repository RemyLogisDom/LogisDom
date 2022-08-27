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



#ifndef devteleinfo_H
#define devteleinfo_H
#include "onewire.h"
#include "ui_ds2423.h"

class devteleinfo : public onewiredevice
{
	Q_OBJECT
enum counterMode { NormalMode, TextMode, absoluteMode, relativeMode, offsetMode, RealTime, RecRealTime };
public:
	devteleinfo(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
    QString ValueToStr(double, bool noText = false);
	void lecture();
	void lecturerec();
    void SetOrder(const QString &order);
	QComboBox ParameterList;
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	QComboBox counterMode;
	QSpinBox Offset;
	QDoubleSpinBox Coef;
	interval countInit;
	QCheckBox SaveOnUpdate;
	long int LastCounter, Delta;
	void setLocalMainValue(double value, bool enregistremode);
	QString translateMainValueStr(double value);
	double translateMainValue(QString &str);
    bool readNow();
private:
	Ui::ds2423ui ui;
	QString VStr;
	int validCount;
	bool ReadNow;
	bool ReadRecNow;
	void setResult(long int NewValue);
	QString ADCOstr;
private slots:
	void contextMenuEvent(QContextMenuEvent *event);
	void modeChanged(int);
	void saveOnUpdateChanged(int state);
};

#endif
