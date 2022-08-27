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



#ifndef DEVCHOOSER_H
#define DEVCHOOSER_H
#include "logisdom.h"

class onewiredevice;
class configwindow;




class devchooser : public QWidget
{
	Q_OBJECT
public:	
	devchooser(logisdom *Parent);
	~devchooser();
	logisdom *parent;
	QStringList AcceptedFamily;
	void addFamily(QString Family);
	void setHtmlBindControler(htmlBinder *htmlBind);
    void Lock(bool);
    void updateLock();
    bool lockStatus = true;
// Palette setup
	QWidget setup;
	QGridLayout setupLayout;
    QPushButton chooseButton, ButtonDel;
    void freeDevice();
    void setRomID(QString romid);
	QString getRomID();
	QString getName();
	onewiredevice *device();
	void setStyleSheet(QString style);
private:
	htmlBinder *htmlBindControler;
	QString RomID;
public slots:
    void DeviceConfigChanged(onewiredevice*);
    void newDeviceAdded(onewiredevice*);
    void DeviceValueChanged(onewiredevice*);
public slots:
	void chooseClick();
    void delClick();
signals:
	void deviceSelectionChanged();
    void deleteRequest(devchooser*);
};

#endif
