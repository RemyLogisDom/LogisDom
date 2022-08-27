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



#ifndef valveecogest_H
#define valveecogest_H
#include "onewire.h"
#include "ui_valveecogest.h"

class valveecogest : public onewiredevice
{
	Q_OBJECT
#define More10 "p10"
#define Lest10 "m10"
public:
	valveecogest(net1wire *Master, logisdom *Parent, QString RomID);
// Palette setup
	QSpinBox Min, Max, Value;
	QString MainValueToStrLocal(const QString &str);
    QString ValueToStr(double, bool);
	void lecture();
	void lecturerec();
    bool isVanneFamily();
	void setconfig(const QString &strsearch);
	void setLocalMainValue(double NewValue, bool enregistremode);
    void SetOrder(const QString &order);
	void remoteCommandExtra(const QString &command);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
private:
	Ui::valveecogestui ui;
	QString family4, familyID;
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots :
	void sliderChanged(int);
	void uisliderChanged(int);
	void MinChanged(int);
	void MaxChanged(int);
};


#endif
