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



#ifndef EOCEAN_H
#define EOCEAN_H

#include <QtWidgets/QButtonGroup>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "net1wire.h"
#include "globalvar.h"
#include "ui_eocean.h"
#include "eoceanthread.h"
#include "ha7s.h"




class eocean : public net1wire
{
    Q_OBJECT
    friend class eoceanthread;
public:
const static int default_port = 1;
    eocean(logisdom *Parent);
    ~eocean();
    Ui::eoceanui uiw;
    void setipaddress();
    void setport(int Port);
    void init();
    void setConfig(const QString &strsearch);
    void getConfig(QString &str);
    void switchOnOff(bool state);
    QString writeScratchPad(const QString&, const QString&);
    void addtofifo(const QString &order);
private:
    eoceanthread eoThread;
    static void getDeviceList(const QString RomID, QStringList &DevList);
    QStringList LocalCatalog;
    bool upgradeCatalogInfo = false;
private slots:
    void logReturn(const QString, const QString);
    void logThis(const QString);
    void changeComPort(const QString& port);
    void eoStatusChange();
    void addNewDevice(const QString &);
    void TeachON();
    void TeachOFF();
    void ClearLog();
    void tcpStatusChange();
protected:
    void mousePressEvent(QMouseEvent *event);
};


#endif	// EOCEAN_H
