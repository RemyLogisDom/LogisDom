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



#ifndef devfinder_H
#define devfinder_H
#include "logisdom.h"
#include "net1wire.h"
#include "onewire.h"
#include "ui_deadevice.h"


class deadevice : public QDialog
{
    Q_OBJECT
public:
    deadevice();
    void addOldDevice(const QString dev);
    void addDevice(const QString dev);
    QString *oldDevice, *newDevice;
    QStringList oldDevList, newDevList;
private:
    Ui::deadevicegui ui;
public slots:
    void deviceChanged(QListWidgetItem *);
    void OldDeviceChanged(QListWidgetItem *);
    void cancel();
    void select();
};


#endif
