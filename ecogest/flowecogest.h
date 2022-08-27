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



#ifndef flowecogest_H
#define flowecogest_H
#include "onewire.h"
#include "linecoef.h"
#include "ui_flowecogest.h"

class flowecogest : public onewiredevice
{
	Q_OBJECT
    enum counterMode { absoluteMode, relativeMode, offsetMode };
public:
	flowecogest(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
    QString ValueToStr(double, bool noText = false);
	QString family4;
	void lecture();
	void lecturerec();
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    void SetOrder(const QString &order);
    QComboBox counterMode;
    QSpinBox Offset;
    QLabel CoefLabel;
    lineCoef Coef;
    interval countInit;
    QCheckBox SaveOnUpdate;
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
    long int LastCounter, Delta;
    double result(double value);
private:
	Ui::flowecogest ui;
    void setResult(long int NewValue, bool enregistremode);
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
    void modeChanged(int);
    void saveOnUpdateChanged(int state);
};


#endif
