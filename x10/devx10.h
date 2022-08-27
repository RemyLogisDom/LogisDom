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



#ifndef devx10_H
#define devx10_H
#include "interval.h"
#include "onewire.h"


class devx10 : public onewiredevice
{
	Q_OBJECT
	friend class am12;
	friend class lm12;
public:
	devx10(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
    QString ValueToStr(double, bool noText = false);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode);
	bool isX10Family();
	bool isSwitchFamily();
	void setHouseCode(int House, int Module);
	void setHouseCode(QString House, QString Module);
	virtual void setHMCaption();
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
    void SetOrder(const QString &order);
	void setMainValue(double value, bool enregistremode);
// Palette setup
    QPushButton ButtonOn, ButtonOff;
    QCheckBox StringValue;
private:
	bool ExtendedMode;
	QString dailyPrg;
	void setCode();
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots:
	void On(bool emitX10);
	void Off(bool emitX10);
	void Dim(bool emitX10);
	void Sleep();
	void Bright(bool emitX10);
	void Bright(int Power, bool emitX10);	// Power = 0 -> 22
    void ON_rigthClick(const QPoint &pos);
    void OFF_rigthClick(const QPoint &pos);
signals:
	void stateChanged();
};


#endif
