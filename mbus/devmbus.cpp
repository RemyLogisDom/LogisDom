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





#include "globalvar.h"
#include "server.h"
#include "mbus.h"
#include "devmbus.h"




devmbus::devmbus(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    lastsavevalue = logisdom::NA;
    romid = RomID;
    family = romid.right(2);
    ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("M-Bus device"));
    if (master) ui.labelmaster->setText(master->getipaddress());
    setname("");
    MainValue = logisdom::NA;
    AdrLabel.setText(tr("Adress : "));
    setupLayout.addWidget(&AdrLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    AdrId.setReadOnly(true);
    setupLayout.addWidget(&AdrId, layoutIndex++, 1, 1, logisdom::PaletteWidth/2);

    DatIdLabel.setText(tr("Dat ID : "));
    setupLayout.addWidget(&DatIdLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    DatId.setReadOnly(true);
    setupLayout.addWidget(&DatId, layoutIndex++, 1, 1, logisdom::PaletteWidth);

    DatStrLabel.setText(tr("Dat String : "));
    setupLayout.addWidget(&DatStrLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    DatStr.setReadOnly(true);
    setupLayout.addWidget(&DatStr, layoutIndex++, 1, 1, logisdom::PaletteWidth);

    Coef.addItem("1");
    Coef.addItem("1/10");
    Coef.addItem("1/100");
    Coef.addItem("1/1000");
    CoefLabel.setText(tr("Coef : "));
    setupLayout.addWidget(&CoefLabel, layoutIndex, 0, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&Coef, layoutIndex++, 1, 1, logisdom::PaletteWidth/2);

    hexConvert.setText(tr("Hex"));
    setupLayout.addWidget(&hexConvert, layoutIndex++, 0, 1, logisdom::PaletteWidth/2);
}





bool devmbus::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
    scratchpad = devicescratchpad;
    //logMsg(scratchpad);
    ScratchPad_Show.setText(scratchpad);
    QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
    bool ok;
    double val = logisdom::NA;
    if (hexConvert.isChecked())
    {
        int V = scratchpad.toInt(&ok, 16);
        val = V;
    }
    else
    {
        val = scratchpad.toDouble(&ok);
    }
    if ((!ok) && (!hexConvert.isChecked()))
    {
        int V = scratchpad.toInt(&ok, 16);
        val = V;
        if (ok) hexConvert.setChecked(true);
    }
    if (!ok)
    {
        if (master && WarnEnabled.isChecked()) master->GenError(31, errorMsg);
        return IncWarning();
    }
    switch (Coef.currentIndex())
    {
        case 0 : break;
        case 1 : val = val / 10; break;
        case 2 : val = val / 100; break;
        case 3 : val = val / 1000; break;
        default : break;
    }
    setMainValue(val, enregistremode);
    MainValueToStr();
    return true;
}






void devmbus::lecture()
{
    setValid(ReadRequest);
    if (!master) return;
    switch (master->gettype())
    {
        case MBusType : setscratchpad(master->getScratchPad(romid), false);
        break;
        case RemoteType : master->getMainValue();
        break;
    }
}





void devmbus::lecturerec()
{
    return;
    if (!master) return;
    setValid(ReadRequest);
    switch (master->gettype())
    {
        case MBusType : setscratchpad(master->getScratchPad(romid), true);
        break;
        case RemoteType : master->saveMainValue();
        break;
    }

}




void devmbus::masterlecturerec()
{
    if (!master) return;
    setValid(ReadRequest);
    switch (master->gettype())
    {
        case MBusType : setscratchpad(master->getScratchPad(romid), true);
        break;
        case RemoteType : master->saveMainValue();
        break;
    }
}




void devmbus::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu contextualmenu;
    QAction Lecture(tr("&Read"), this);
    QAction Nom(tr("&Name"), this);
    contextualmenu.addAction(&Lecture);
    if (!parent->isRemoteMode())
    {
        contextualmenu.addAction(&Nom);
    }
    QAction *selection;
    selection = contextualmenu.exec(event->globalPos());
    if (selection == &Lecture) lecture();
    if (selection == &Nom) changename();
}





QString devmbus::MainValueToStrLocal(const QString &str)
{
    QString S = str;
    ui.valueui->setText(S);
    return S;
}





void devmbus::SetOrder(const QString &)
{
}





void devmbus::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("MBusAdr", AdrId.text());
    str += logisdom::saveformat("MBusDatId", DatId.text());
    str += logisdom::saveformat("MBusDatStr", DatStr.text().replace('(', "[").replace(')', "]"));
    str += logisdom::saveformat("HexConvert", QString("%1").arg(hexConvert.isChecked()));
    str += logisdom::saveformat("Coef", QString("%1").arg(Coef.currentIndex()));
}




void devmbus::setconfig(const QString &strsearch)
{
    if (name.isEmpty())
    {
        QString Name = logisdom::getvalue("Name", strsearch);
        if (!Name.isEmpty()) setname(assignname(Name));
        else setname(assignname(tr("MBus device ")));
    }
    QString MBusAdr = logisdom::getvalue("MBusAdr", strsearch);
    if (!MBusAdr.isEmpty()) AdrId.setText(MBusAdr);
    QString MBusDatId = logisdom::getvalue("MBusDatId", strsearch);
    if (!MBusDatId.isEmpty()) DatId.setText(MBusDatId);
    //QString MBusDatStr = logisdom::getvalue("MBusDatStr", strsearch).replace('[', "(").replace(']', ")");
    //if (!MBusDatStr.isEmpty()) DatStr.setText(MBusDatStr);
    bool ok;
    int hex = logisdom::getvalue("HexConvert", strsearch).toInt(&ok);
    if (ok)
    {
        if (hex) hexConvert.setCheckState(Qt::Checked);
        else hexConvert.setCheckState(Qt::Unchecked);
    }
    int coef = logisdom::getvalue("Coef", strsearch).toInt(&ok);
    if (ok) Coef.setCurrentIndex(coef);
}





