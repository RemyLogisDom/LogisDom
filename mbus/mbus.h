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



#ifndef MBUS_H
#define MBUS_H
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QThread>
#include "mbusthread.h"
#include "ui_mbus.h"
#include "net1wire.h"
#include "onewire.h"



class mbus : public net1wire
{
    Q_OBJECT
    friend class mbusthread;
enum ReadingInterval { Reading1mn, Reading5mn, Reading10mn, Reading15mn, Reading20mn, Reading30mn, Reading1h, Reading1d };
public:
    mbus(logisdom *Parent);
    ~mbus();
    void startTCP();
    void init();
    void setipaddress();
    void getConfig(QString &str);
    void setConfig(const QString &strsearch);
    QStringList devicesScratchPad;
    QString getScratchPad(const QString &RomID, int scratchpad_ID = 0);
    void switchOnOff(bool state);
private:
    Ui::mbus uiw;
    mbusthread mbusThread;
    onewiredevice *addDevice();
    QList <onewiredevice*> devicetoread;
    QTreeWidgetItem *getItem(const QString &adr, const QString &parameter);
    QTreeWidgetItem *getItem(const QString &adr, const QString &record, const QString &parameter);
    QTreeWidgetItem *getTree(const QString &adr);
    QTreeWidgetItem *getTree(const QString &adr, const QString &record);
    void setScratchPad(const QString &RomID, const QString &Value);
private slots:
    void tcpStatusUpdate(const QString&);
    void tcpStatusChange();
    void logEnabledChanged(int);
    void searchMaxChanged(int);
    void searchClicked();
    void readIntervalChanged(int);
    void newTreeAddress(int);
    void setTreeItem(const QString&);
    void setMainTreeItem(const QString&);
    void whatdoUDo(const QString);
    void rightclicklist(const QPoint &pos);
    void ReadingDone();
    void ReadingDevDone();
    void CheckDev();
signals:
    void appendAdr(const QString &str);
};

#endif
