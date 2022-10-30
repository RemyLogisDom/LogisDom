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



#ifndef devvirtual_H
#define devvirtual_H
#include "interval.h"
#include "formula.h"
#include "onewire.h"
#include "ui_devvirtual.h"



class devvirtual : public onewiredevice
{
	Q_OBJECT
enum intervalTypes
	{ minute, hour, day };
public:
	devvirtual(net1wire *Master, logisdom *Parent, QString RomID);
	~devvirtual();
	QString MainValueToStrLocal(const QString &str);
	bool initialCalcul;
	bool deviceLoading;
	void lecture();
	void lecturerec();
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
	bool isVirtualFamily();
	bool saveLecture;
	formula *FormulCalc;
	void stopAll();
	bool isReprocessing();
	int meantimeCalc;
private:
    bool IncWarning();
    Ui::devvirtualui ui;
    void LectureManual();
    void closeEvent(QCloseEvent *event);
    QCheckBox disable_Device;
    QMutex mutexCalc;
    void saveDeviceConfig();
public slots:
	void contextMenuEvent(QContextMenuEvent *event);
	void setResult();
};


#endif
