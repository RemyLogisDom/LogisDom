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



#ifndef devfts800_H
#define devfts800_H
#include "onewire.h"
#include "ui_valvefts800.h"

class devfts800 : public onewiredevice
{
	Q_OBJECT
public:
	devfts800(net1wire *Master, logisdom *Parent, QString RomID);
	void lecture();
	void lecturerec();
    bool isVanneFamily();
	void setconfig(const QString &strsearch);
    void SetOrder(const QString &order);
    QString MainValueToStrLocal(const QString &str);
    void assignMainValueLocal(double value);
private:
	Ui::valvefts800ui ui;
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
};


#endif
