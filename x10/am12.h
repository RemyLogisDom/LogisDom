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




#ifndef am12_H
#define am12_H
#include "devx10.h"
#include "ui_am12.h"

#include "devx10.h"
#include "interval.h"


class am12 : public devx10
{
	Q_OBJECT
#define LM12ON "LM12ON"
#define LM12OFF "LM12OFF"
public:
	am12(net1wire *Master, logisdom *Parent, QString RomID);
	void setHMCaption();
	QPushButton ButtonOn, ButtonOff;
    void SetOrder(const QString &order);
	void remoteCommandExtra(const QString &command);
private:
	Ui::am12ui ui;
public slots:
	virtual void contextMenuEvent(QContextMenuEvent * event);
private slots:
	void clickX10On();
	void clickX10Off();
};


#endif
