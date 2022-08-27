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
#include "rps2.h"
#include "devrps2.h"




devrps2::devrps2(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ReadNow = false;
	ReadRecNow = false;
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
	for (int n=0; n<lastRps2Parameter; n++)
		parameterIndex.addItem(Rps2ParamtoStr(n));
	setupLayout.addWidget(&parameterIndex, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	textValue.setText(tr("Text Value"));
	setupLayout.addWidget(&textValue, layoutIndex++, 0, 1, logisdom::PaletteWidth);
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(2);
	ui.labelromid->setText("RomID : " + romid);
	ui.labelfamily->setText(tr("Rotex value"));
	if (master) ui.labelmaster->setText(master->getipaddress());
	setname("");
	MainValue = logisdom::NA;
	connect(&parameterIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(paramterChanged(int)));
}






void devrps2::paramterChanged(int index)
{
	switch (index)
	{
		case HAp :
        case BKp :
		case P1p :
		case P2p :
			textValue.setEnabled(true);
			break;
		case TKp :
		case TRp :
		case TSp :
		case TVp :
		case Debp :
			textValue.setEnabled(false);
			break;
	}
}





bool devrps2::setscratchpad(const QString &devicescratchpad, bool enregistremode)
{
//	QMutexLocker locker(&parent->MutexSetScratchPad);
	scratchpad = devicescratchpad;
	ScratchPad_Show.setText(scratchpad);
//	if (!(ReadNow or ReadRecNow)) return true;
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name + "  ";
	int index = parameterIndex.currentIndex();
	QStringList param = devicescratchpad.split(rps2::SEP);
	if (index >= param.count())
	{
		if (master && WarnEnabled.isChecked()) master->GenError(32, errorMsg);
		return IncWarning();
	}
	bool ok;
    double val = param[index].toDouble(&ok);
    if (!ok)
    {
        QString strReplaceLocalDecPoint = param[index].replace(QChar(','), QChar('.'));
        val = strReplaceLocalDecPoint.toDouble(&ok);
    }
	if (index == Statusp)
	{
		if (param[index].isEmpty()) setMainValue(StatusOk, enregistremode);
		else setMainValue(val, enregistremode);
	}
	else
	{
		if (!ok)
		{
			if (master && WarnEnabled.isChecked()) master->GenError(31, errorMsg);
			return IncWarning();
		}
		if (ReadNow or ReadRecNow) setMainValue(val, enregistremode);
		else setMainValue(val, false);
	}
	ReadNow = false;
	ReadRecNow = false;
	return true;
}







void devrps2::lecture()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case TCP_RPS2Type : ReadNow = true;
		break;
		case RemoteType : master->getMainValue();
		break;
	}
}





void devrps2::lecturerec()
{
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
		case TCP_RPS2Type : ReadRecNow = true;
		break;
		case RemoteType : master->saveMainValue();
		break;
	}

}







void devrps2::contextMenuEvent(QContextMenuEvent * event)
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






QString devrps2::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}





QString devrps2::ValueToStr(double Value, bool noText)
{
	QString str;
	if (Value == logisdom::NA) str = cstr::toStr(cstr::NA);
	else
	{
		str = QString("%L1 ").arg(Value, 0, 'f', Decimal.value()) + Unit.text();
		str = parent->LogisDomLocal.toString(Value, 'f', Decimal.value()) + Unit.text();
        if (textValue.isChecked() and (!noText))
        {
            switch (parameterIndex.currentIndex())
            {
                case HAp :
                            switch (int(Value))
                            {
                                case 0 : str = tr("Automatic"); break;
                                case 1 : str = tr("Manual"); break;
                                default : str = tr("Not defined"); break;
                            }
                        break;
                case BKp :
                case P1p :
                case P2p :
                            switch (int(Value))
                            {
                                case 0 : str = cstr::toStr(cstr::OFF); break;
                                case 1 : str = cstr::toStr(cstr::ON); break;
                                default : str = str = cstr::toStr(cstr::NA); break;
                            }
                        break;
                case Statusp :
                                if (int(Value) == StatusOk) str = tr("Status OK");
                                else str = tr("Error code ") + QString("%1").arg(int(Value));
                            break;

            }
        }
	}
	return str;
}




void devrps2::SetOrder(const QString &)
{
}






void devrps2::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("parameterIndex", QString("%1").arg(parameterIndex.currentIndex()));
    str += logisdom::saveformat("rps2ValueAsText", QString("%1").arg(textValue.isChecked()));
}





void devrps2::setconfig(const QString &strsearch)
{
	bool ok;
	QString Name = logisdom::getvalue("Name", strsearch);
	int PIndex = 0;
	PIndex = logisdom::getvalue("parameterIndex", strsearch).toInt(&ok);
	if ((ok) && (PIndex != -1))
	{
		parameterIndex.setCurrentIndex(PIndex);
		if (!Name.isEmpty()) setname(assignname(Name));
		else setname(assignname(Rps2ParamtoStr(PIndex)));
	}
	else
	{
		if (!Name.isEmpty()) setname(assignname(Name));
		else setname(assignname("RPS2"));
	}
    int textV = logisdom::getvalue("rps2ValueAsText", strsearch).toInt(&ok);
	if (ok)
	{
		if (textV) textValue.setCheckState(Qt::Checked);
		else textValue.setCheckState(Qt::Unchecked);
	}
}







QString devrps2::Rps2ParamtoStr(int index)
{
	QString str;
	switch(index)
	{
		case HAp :		str = tr("Auto/Man"); break;
		case BKp :		str = tr("Chauffage"); break;
		case P1p :		str = tr("Pompe 1"); break;
		case P2p :		str = tr("Pompe 2"); break;
		case TKp :		str = tr("T° Capteur"); break;
		case TRp :		str = tr("T° Retour"); break;
		case TSp :		str = tr("T° Ballon"); break;
		case TVp :		str = tr("T° Départ"); break;
		case Debp	:	str = tr("Débit"); break;
		case Statusp :	str = tr("Status"); break;
		case Powerp :	str = tr("Power"); break;
		default : 	str = tr("RPS2 Device"); break;
	}
	return str;
}

/*
HA État de la régulation : pilotage automatique = 0; pilotage manuel = 1
BK État du contact de coupure du r échauffage : off = 0; on = 1
P1 État de la pompe de service P1 : off = 0; on = 1
P2 État de la pompe de remplissage P2 : off = 0; on = 1
TK/°C Température de capteur en °C
TR/°C Température de retour en °C
TS/°C Température de ballon en °C
TV/°C Température de départ en °C
V/l/min Débit en l/min
*/




