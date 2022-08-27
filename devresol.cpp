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





#include "globalvar.h"
#include "server.h"
#include "resol.h"
#include "devresol.h"




devresol::devresol(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	Address.setReadOnly(true);
	parameterSelected = -1;
	ReadNow = false;
	ReadRecNow = false;
	CPosition.setRange(0, 3);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    setupLayout.addWidget(&AddressListStr, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&Address, layoutIndex++, 1, 1, 1);
    setupLayout.addWidget(&parameterIndex, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	CFrame.setPrefix(tr("Frame : "));
	CFrame.setEnabled(false);
	setupLayout.addWidget(&CFrame, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	CPosition.setPrefix(tr("Position : "));
	CPosition.setEnabled(false);
	setupLayout.addWidget(&CPosition, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	CLength.setPrefix(tr("Length : "));
	CLength.setEnabled(false);
	C10.setEnabled(false);
	setupLayout.addWidget(&CLength, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	C10.setText(tr("Coef 10"));
    setupLayout.addWidget(&C10, layoutIndex, 0, 1, 1);
    //ValueUnsigned.setText("Unsigned");
    //setupLayout.addWidget(&ValueUnsigned, layoutIndex, 1, 1, 1);
    lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("RESOL value"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	MainValue = logisdom::NA;
	Unit.setText("°C");
	connect(&parameterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(paramterChanged(int)));
	connect(&AddressListStr, SIGNAL(currentIndexChanged(int)), this, SLOT(AddressListChanged(int)));
	connect(&AddressListStr, SIGNAL(activated(int)), this, SLOT(AddressListChanged(int)));
}







void devresol::paramterChanged(int index)
{
	if (index == (parameterIndex.count() - 1))
	{
		CFrame.setEnabled(true);
		CPosition.setEnabled(true);
		CLength.setEnabled(true);
		C10.setEnabled(true);
	}
	else
	{
		bool ok;
		CFrame.setEnabled(false);
		int n = parameterFrame.at(index).toInt(&ok);
		if (ok) CFrame.setValue(n);
		CPosition.setEnabled(false);
		n = parameterPosition.at(index).toInt(&ok);
		if (ok) CPosition.setValue(n);
		CLength.setEnabled(false);
		n = parameterLength.at(index).toInt(&ok);
		if (ok) CLength.setValue(n);
		C10.setEnabled(false);
		if (Coef10.at(index) == "Y") C10.setChecked(true); else C10.setChecked(false);
	}
}






void devresol::AddressListChanged(int adr)
{
	Address.setText(QString("0x%1").arg(AddressList.at(adr), 2, 16, QChar('0')));
	setParameter(AddressList.at(adr));
	Address.setEnabled(true);
}




bool devresol::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	bool ok;
	QByteArray scratchpad;
	logMsg("scratchPad : " + devicescratchpad);
	ScratchPad_Show.setText(scratchpad);
	for (int n=0; n<devicescratchpad.count(); n+=2)
	{
		QString byte = devicescratchpad.mid(n, 2);
		scratchpad.append(byte.toInt(&ok, 16));
	}
	int Adr = scratchpad.at(3) + 0x100 * scratchpad.at(4);
	QString adrName = resol::AddressToName(Adr);
	if ((AddressListStr.findText(adrName) == -1) && (!adrName.isEmpty()))
	{
		disconnect(&AddressListStr, SIGNAL(currentIndexChanged(int)), this, SLOT(AddressListChanged(int)));
		AddressListStr.addItem(adrName);
		connect(&AddressListStr, SIGNAL(currentIndexChanged(int)), this, SLOT(AddressListChanged(int)));
		AddressList.append(Adr);
		if (Address.text() == QString("0x%1").arg(Adr, 2, 16, QChar('0')))
		{
			AddressListStr.setCurrentIndex(AddressListStr.count() - 1);
			setParameter(Adr);
			Address.setEnabled(true);
		}
	}

	if (Address.text() != QString("0x%1").arg(Adr, 2, 16, QChar('0'))) return true;
	int frame = CFrame.value();
	int pos = CPosition.value();
	int size = CLength.value();
    double val = resol::getParameter(scratchpad, frame, pos, size);
    if ((C10.isChecked()) and (logisdom::isNotNA(val))) val /= 10;
	if (ReadNow or ReadRecNow) setMainValue(val, enregistremode);
	else setMainValue(val, false);
	ReadNow = false;
	ReadRecNow = false;
	return true;
}






void devresol::setParameter(int Adr)
{
	disconnect(&parameterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(paramterChanged(int)));
	parameterIndex.clear();
	parameterFrame.clear();
	parameterPosition.clear();
	parameterLength.clear();
	QStringList list;
	resol::getParameterList(Adr, list);
	for (int n=0; n<list.count(); n++)
	{
		QStringList split;
		split = list.at(n).split("*");
		parameterIndex.addItem(split.at(0));
		parameterFrame.append(split.at(1));
		parameterPosition.append(split.at(2));
		parameterLength.append(split.at(3));
		if (split.count() == 5) Coef10.append("Y"); else Coef10.append("N");
	}
	parameterIndex.addItem(tr("Custom"));
	parameterFrame.append("");
	parameterPosition.append("");
	parameterLength.append("");
	connect(&parameterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(paramterChanged(int)));
	if ((parameterSelected != -1) && (parameterSelected < parameterIndex.count())) parameterIndex.setCurrentIndex(parameterSelected);
}




void devresol::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case TCP_ResolType : ReadNow = true;
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void devresol::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case TCP_ResolType : ReadRecNow = true;
		break;
		case RemoteType : master->saveMainValue();
		break;
	}

}







void devresol::contextMenuEvent(QContextMenuEvent * event)
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






QString devresol::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}







void devresol::SetOrder(const QString &)
{
}






void devresol::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("parameterIndex", QString("%1").arg(parameterIndex.currentIndex()));
	str += logisdom::saveformat("FrameIndex", QString("%1").arg(CFrame.value()));
	str += logisdom::saveformat("FramePosition", QString("%1").arg(CPosition.value()));
	str += logisdom::saveformat("FrameLength", QString("%1").arg(CLength.value()));
	str += logisdom::saveformat("FrameCoef", QString("%1").arg(C10.isChecked()));
	str += logisdom::saveformat("FrameAddress", Address.text());
    //str += logisdom::saveformat("ValueUnsigned", QString("%1").arg(ValueUnsigned.isChecked()));
}





void devresol::setconfig(const QString &strsearch)
{
	bool ok;
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
	Address.setText(logisdom::getvalue("FrameAddress", strsearch));
	Address.setEnabled(false);
	int PIndex = 0;
	PIndex = logisdom::getvalue("parameterIndex", strsearch).toInt(&ok);
	if ((ok) && (PIndex != -1)) parameterSelected = PIndex; else parameterSelected = -1;
	int FrameIndex = logisdom::getvalue("FrameIndex", strsearch).toInt(&ok);
	if (ok) CFrame.setValue(FrameIndex);
	int FramePosition = logisdom::getvalue("FramePosition", strsearch).toInt(&ok);
	if (ok) CPosition.setValue(FramePosition);
	int FrameLength = logisdom::getvalue("FrameLength", strsearch).toInt(&ok);
	if (ok) CLength.setValue(FrameLength);
	int FrameCoef = logisdom::getvalue("FrameCoef", strsearch).toInt(&ok);
	if (ok) C10.setChecked(FrameCoef);
    //int Vunsigned = logisdom::getvalue("ValueUnsigned", strsearch).toInt(&ok);
    //if (ok) ValueUnsigned.setChecked(Vunsigned);
}





