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



#ifndef modecogest_H
#define modecogest_H
#include "onewire.h"
#include "ui_modecogest.h"

class modecogest : public onewiredevice
{
	Q_OBJECT
public:
	modecogest(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
    QString ValueToStr(double, bool noText = false);
	QString family4;
	void lecture();
	void lecturerec();
    void SetOrder(const QString &order);
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
private:
	Ui::modecogest ui;
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
};


#endif
