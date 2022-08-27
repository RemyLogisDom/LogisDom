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



#ifndef devfinder_H
#define devfinder_H
#include "logisdom.h"
#include "net1wire.h"
#include "onewire.h"
#include "ui_devfinder.h"


class devfinder : public QDialog
{
    Q_OBJECT
public:
    devfinder(logisdom *Parent, int opt = 0);
    logisdom *parent;
    int options;
    onewiredevice *choosedDevice;
    onewiredevice *originalDevice;
    QList <onewiredevice*> devicesList;
    void sort();
    bool done;
    void closeEvent(QCloseEvent *event);
private:
    Ui::devfindergui ui;
    QList <net1wire*> net1wires;
    QList <onewiredevice*> devices;
    QList <LogisDomInterface*> plugins;
    QList <int> deviceIndex;
public slots:
    void textChanged(const QString &text);
    void masterChanged(QListWidgetItem *);
    void deviceChanged(QListWidgetItem *);
    void select(QModelIndex);
    void cancel();
    void select();
    void clear();
};


#endif
