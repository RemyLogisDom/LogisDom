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
#include "errlog.h"
#include "alarmwarn.h"
#include "onewire.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "teleinfo.h"



teleinfo::teleinfo(logisdom *Parent) : net1wire(Parent)
{
        framelayout = new QGridLayout(ui.frameguinet);
}



void teleinfo::init()
{
	type = NetType(TeleInfoType);
	Buffer = "";
	retry = 0;
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
    connect(&tcp, SIGNAL(readyRead()), this, SLOT(readbuffer()), Qt::QueuedConnection);
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
    TimeOut.setSingleShot(true);
	connecttcp();
    TimeOut.start(DataTimeOut);

	int i = 0;
    ui.fifolist->hide();

// setup CreateDev
	Bouton2.setText(tr("New Device"));
	connect(&Bouton2, SIGNAL(clicked()), this , SLOT(NewDevice()));
	framelayout->addWidget(&Bouton2, i++, 0, 1, 1);

	ui.toolButtonClear->hide();
	ui.EditType->setText("TeleInfo EDF");
    QString configdata;
    parent->get_ConfigData(configdata);
    readConfigFile(configdata);
}






void teleinfo::NewDevice()
{
	bool ok;
	retry:
    QString nom = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal, "", &ok, parent);
	if (!ok) return;
	if (nom.isEmpty()) return;
	if (parent->configwin->devicenameexist(nom))
	{
		messageBox::warningHide(this, cstr::toStr(cstr::MainName), cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
		goto retry;
	}
	addDevice(nom, true);
}





void teleinfo::getConfig(QString &)
{
}




void teleinfo::setConfig(const QString &)
{
}



void teleinfo::readConfigFile(QString &configdata)
{
	QString ReadRomID, MasterIP;
	onewiredevice *device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	MasterIP = logisdom::getvalue("Master", strsearch);
	if ((ReadRomID.left(8) == ip2Hex(moduleipaddress).toUpper()) or (ReadRomID.left(12) == unID + port2Hex(port)))
		if (!parent->configwin->deviceexist(ReadRomID))
		{
			device = parent->configwin->NewDevice(ReadRomID, this);
			if (device)
				if (!localdevice.contains(device)) localdevice.append(device);
			UpdateLocalDeviceList();
		}
    SearchLoopEnd
}





void teleinfo::fifonext()
{
}








QString teleinfo::extractBuffer(const QString &data)
{
    QString extract = "";
	int chdeb, chfin, L;
	Buffer += data;
	chdeb = Buffer.indexOf(STX);
	chfin = Buffer.indexOf(ETX);
	L = Buffer.length();
    if ((chdeb == -1) and (chfin == -1))
    {
        //GenMsg("No begin No end found : ");
        //GenMsg("Buffer : " + Buffer);
        return "";
    }
    if ((chdeb != -1) and (chfin == -1))
    {
        //GenMsg("No end found : ");
        //GenMsg("Buffer : " + Buffer);
        return "";
    }
    if (chdeb > chfin)
    {
        Buffer = Buffer.right(L - chdeb);
        //GenMsg("No begin found : ");
        //GenMsg("Buffer : " + Buffer);
        return "";
    }
    if ((chdeb != -1) and (chfin != -1))
	{
		extract = Buffer.mid(chdeb + 1, chfin - chdeb - 1);
		Buffer = Buffer.right(L - chfin - 1);
        receivedData = true;
        Buffer.clear();
	}
	return extract;
}






void teleinfo::readbuffer()
{
	QByteArray data;
	QString extract;
	QMutexLocker locker(&mutexreadbuffer);
//more:
    data = tcp.readAll();
    QString logtxt;
    for (int n=0; n<data.length(); n++) logtxt += QString(" %1").arg(uchar(data[n]));
    GenMsg(tr("Read Raw : ") + logtxt);
    extract = extractBuffer(data);
    /*if (extract.isEmpty())
    {
        if (tcp.waitForReadyRead(1000))
        {
            data.append(tcp.readAll());
        }
    }*/
    if (extract.isEmpty())
    {
        //tcp.waitForReadyRead(100);
        //if (tcp.bytesAvailable()) goto more;
        // Modif pour Arduino qui pose problème
        //if (ui.checkBoxLog->isChecked()) ui.textBrowser->setPlainText(logtxt);
        return;
    }
	TimeOut.stop();
    //GenMsg(tr("Read : ") + extract);
    if (ui.checkBoxLog->isChecked()) ui.textBrowser->setPlainText(QDateTime::currentDateTime().toString("hh:mm:ss\n") + extract);
    for (int n=0; n<localdevice.count(); n++)
    {
        if (localdevice[n]->readNow())
            if (!localdevice[n]->setscratchpad(extract, true))
                GenError(34, localdevice[n]->getromid());
    }
    //if (tcp.bytesAvailable() > 0) goto more;
    TimeOut.start(DataTimeOut);
}





QString teleinfo::TeleInfoParamtoStr(int index)
{
	QString str;
	switch(index)
	{
		case ADCO :	str = "ADCO"; break;
		case OPTARIF :	str = "OPTARIF"; break;
		case ISOUSC :	str = "ISOUSC"; break;
		case BASE :	str = "BASE"; break;
		case HCHC :	str = "HCHC"; break;
		case HCHP :	str = "HCHP"; break;
		case EJPHN :	str = "EJPHN"; break;
		case EJPPM :	str = "EJPPM"; break;
		case EJPHPM :	str = "EJPHPM"; break;
		case BBRHCJB :	str = "BBRHCJB"; break;
		case BBRHPJB :	str = "BBRHPJB"; break;
		case BBRHCJW :	str = "BBRHCJW"; break;
		case BBRHPJW :	str = "BBRHPJW"; break;
		case BBRHCJR :	str = "BBRHCJR"; break;
		case BBRHPJR :	str = "BBRHPJR"; break;
		case PEJP :	str = "PEJP"; break;
		case PTEC :	str = "PTEC"; break;
		case DEMAIN :	str = "DEMAIN"; break;
		case IINST :	str = "IINST"; break;
		case IINST1 :	str = "IINST1"; break;
		case IINST2 :	str = "IINST2"; break;
		case IINST3 :	str = "IINST3"; break;
		case ADPS :	str = "ADPS"; break;
		case IMAX :	str = "IMAX"; break;
		case IMAX1 :	str = "IMAX1"; break;
		case IMAX2 :	str = "IMAX2"; break;
		case IMAX3:	str = "IMAX3"; break;
		case PAPP :	str = "PAPP"; break;
		case HHPHC :	str = "HHPHC"; break;
		case MOTDETAT :	str = "MOTDETAT"; break;
        case ADSC :	str = "ADSC"; break;
        case VTIC :	str = "VTIC"; break;
        case DATE :	str = "DATE"; break;
        case NGTF :	str = "NGTF"; break;
        case LTARF :	str = "LTARF"; break;
        case EAST :	str = "EAST"; break;
        case EASF01 :	str = "EASF01"; break;
        case EASF02 :	str = "EASF02"; break;
        case EASF03 :	str = "EASF03"; break;
        case EASF04 :	str = "EASF04"; break;
        case EASF05 :	str = "EASF05"; break;
        case EASF06 :	str = "EASF06"; break;
        case EASF07 :	str = "EASF07"; break;
        case EASF08 :	str = "EASF08"; break;
        case EASF09 :	str = "EASF09"; break;
        case EASF10 :	str = "EASF10"; break;
        case EASD01 :	str = "EASD01"; break;
        case EASD02 :	str = "EASD02"; break;
        case EASD03 :	str = "EASD03"; break;
        case EASD04 :	str = "EASD04"; break;
        case EAIT :	str = "EAIT"; break;
        case ERQ1 :	str = "ERQ1"; break;
        case ERQ2 :	str = "ERQ2"; break;
        case ERQ3 :	str = "ERQ3"; break;
        case ERQ4 :	str = "ERQ4"; break;
        case IRMS1 :	str = "IRMS1"; break;
        case IRMS2 :	str = "IRMS2"; break;
        case IRMS3 :	str = "IRMS3"; break;
        case URMS1 :	str = "URMS1"; break;
        case URMS2 :	str = "URMS2"; break;
        case URMS3 :	str = "URMS3"; break;
        case PREF :	str = "PREF"; break;
        case PCOUP :	str = "PCOUP"; break;
        case SINSTS :	str = "SINSTS"; break;
        case SINSTS1 :	str = "SINSTS1"; break;
        case SINSTS2 :	str = "SINSTS2"; break;
        case SINSTS3 :	str = "SINSTS3"; break;
        case SMAXSN :	str = "SMAXSN"; break;
        case SMAXSN1 :	str = "SMAXSN1"; break;
        case SMAXSN2 :	str = "SMAXSN2"; break;
        case SMAXSN3 :	str = "SMAXSN3"; break;
        case SMAXSN_1 :	str = "SMAXSN-1"; break;
        case SMAXSN1_1 :	str = "SMAXSN1-1"; break;
        case SMAXSN2_1 :	str = "SMAXSN2-1"; break;
        case SMAXSN3_1 :	str = "SMAXSN3-1"; break;
        case SINSTI :	str = "SINSTI"; break;
        case SMAXIN :	str = "SMAXIN"; break;
        case SMAXIN_1 :	str = "SMAXIN-1"; break;
        case CCASN :	str = "CCASN"; break;
        case CCASN_1 :	str = "CCASN-1"; break;
        case CCAIN :	str = "CCAIN"; break;
        case CCAIN_1 :	str = "CCAIN-1"; break;
        case UMOY1 :	str = "UMOY1"; break;
        case UMOY2 :	str = "UMOY2"; break;
        case UMOY3 :	str = "UMOY3"; break;
        case STGE :	str = "STGE"; break;
        case DPM1 :	str = "DPM1"; break;
        case FPM1 :	str = "FPM1"; break;
        case DPM2 :	str = "DPM2"; break;
        case FPM2 :	str = "FPM2"; break;
        case DPM3 :	str = "DPM3"; break;
        case FPM3 :	str = "FPM3"; break;
        case MSG1 :	str = "MSG1"; break;
        case MSG2 :	str = "MSG2"; break;
        case PRM :	str = "PRM"; break;
        case RELAIS :	str = "RELAIS"; break;
        case NTARF :	str = "NTARF"; break;
        case NJOURF :	str = "NJOURF"; break;
        case NJOUR_1 :	str = "NJOURF+1"; break;
        case PJOUR_1 :	str = "PJOURF+1"; break;
        case PPOINTE :	str = "PPOINTE"; break;
        default : 	str = ""; break;
	}
	return str;
}

/*

/dev $ cat ttyUSBP
EP*PREF 03      B
PCOUP   03      \
SINSTS  00000   F
SMAXSN  E180716063417   00021   9
SMAXSN-1        E180715064903   00023   Y
SINSTI  01258   L
SMAXIN  E180716103316   01423   /
SMAXIN-1        E180715144229   01971   \
CCASN   E180716100000   00000   0
CCASN-1 E180716090000   00000   V
UMOY1   E180716103000   230     +
STGE    002A0301        <
MSG1    PAS DE          MESSAGE                 <
PRM     22246454325444  4
RELAIS  000     B
NTARF   01      N
NJOURF  00      &
NJOURF+1        00      B
PJOURF+1        00008001 NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE   9
ADSC    031864661664    @
VTIC    02      J
DATE    E180716103423           B
NGTF       PRODUCTEUR           .
LTARF    INDEX NON CONSO        0
EAST    000000000       O
EASF01  000000000       "
EASF02  000000000       #
EASF03  000000000       $
EASF04  000000000       %
EASF05  000000000       &
EASF06  000000000       '
EASF07  000000000       (
EASF08  000000000       )
EASF09  000000000       *
EASF10  000000000       "
EASD01  000000000
EASD02  000000000       !
EASD03  000000000       "
EASD04  000000000       #
EAIT    000032781       Z
ERQ1    000000000       ;
ERQ2    000000890       M
ERQ3    000002546       N
ERQ4    000000000       >
IRMS1   005     3
URMS1   228     F
PREF    03      B
PCOUP   03      \
SINSTS  00000   F
SMAXSN  E180716063417   00021   9
SMAXSN-1        E180715064903   00023   Y
SINSTI  01253   G
SMAXIN  E180716103316   01423   /
SMAXIN-1        E180715144229   01971   \
CCASN   E180716100000   00000   0
CCASN-1 E180716090000   00000   V
UMOY1   E180716103000   230     +
STGE    002A0301        <
MSG1    PAS DE          MESSAGE                 <
PRM     22246454325444  4
RELAIS  000     B
NTARF   01      N
NJOURF  00      &
NJOURF+1        00      B
PJOURF+1        00008001 NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE   9
ADSC    031864661664    @
VTIC    02      J
DATE    E180716103424           C
NGTF       PRODUCTEUR           .
LTARF    INDEX NON CONSO        0
EAST    000000000       O
EASF01  000000000       "
EASF02  000000000       #
EASF03  000000000       $
EASF04  000000000       %
EASF05  000000000       &
EASF06  000000000       '
EASF07  000000000       (
EASF08  000000000       )
EASF09  000000000       *
EASF10  000000000       "
EASD01  000000000
EASD02  000000000       !
EASD03  000000000       "
EASD04  000000000       #
EAIT    000032781       Z
ERQ1    000000000       ;
ERQ2    000000890       M
ERQ3    000002546       N
ERQ4    000000000       >
IRMS1   005     3
ok URMS1   228     F
PREF    03      B
PCOUP   03      \
SINSTS  00000   F
SMAXSN  E180716063417   00021   9
SMAXSN-1        E180715064903   00023   Y
SINSTI  01249   L
SMAXIN  E180716103316   01423   /
SMAXIN-1        E180715144229   01971   \
CCASN   E180716100000   00000   0
CCASN-1 E180716090000   00000   V
UMOY1   E180716103000   230     +
STGE    002A0301        <
MSG1    PAS DE          MESSAGE                 <
PRM     22246454325444  4
RELAIS  000     B
NTARF   01      N
NJOURF  00      &
NJOURF+1        00      B
PJOURF+1        00008001 NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE   9
ADSC    031864661664    @
VTIC    02      J
DATE    E180716103426           E
NGTF       PRODUCTEUR           .
LTARF    INDEX NON CONSO        0
EAST    000000000       O
EASF01  000000000       "
EASF02  000000000       #
EASF03  000000000       $
EASF04  000000000       %
EASF05  000000000       &
EASF06  000000000       '
EASF07  000000000       (
EASF08  000000000       )
EASF09  000000000       *
EASF10  000000000       "
EASD01  000000000
EASD02  000000000       !
EASD03  000000000       "
EASD04  000000000       #
EAIT    000032782       [
ERQ1    000000000       ;
ERQ2    000000890       M
ERQ3    000002546       N
ERQ4    000000000       >
IRMS1   005     3
URMS1   229     G
PREF    03      B
PCOUP   03      \
SINSTS  00000   F
SMAXSN  E180716063417   00021   9
SMAXSN-1        E180715064903   00023   Y
SINSTI  01237   I
SMAXIN  E180716103316   01423   /
SMAXIN-1        E180715144229   01971   \
CCASN   E180716100000   00000   0
CCASN-1 E180716090000   00000   V
UMOY1   E180716103000   230     +
STGE    002A0301        <
MSG1    PAS DE          MESSAGE                 <
PRM     22246454325444  4
RELAIS  000     B
NTARF   01      N
NJOURF  00      &
NJOURF+1        00      B
PJOURF+1        00008001 NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE NONUTILE   9
ADSC    031864661664    @
VTIC    02      J
DATE    E180716103427           F
NGTF       PRODUCTEUR           .
LTARF    INDEX NON CONSO        0
EAST    000000000       O
EASF01  000000000       "
EASF02  000000000       #
EASF03  000000000       $
EASF04  000000000       %
EASF05  000000000       &
EASF06  000000000       '
EASF07  000000000       (
EASF08  000000000       )
EASF09  000000000       *
EASF10  000000000       "
EASD01  000000000
EASD02  000000000       !
EASD03  000000000       "
EASD04  000000000       #
EAIT    000032782       [
ERQ1    000000000       ;
ERQ2    000000890       M



  */


QString teleinfo::TeleInfoValeurtoStr(int index)
{
	QString str;
	switch(index)
	{
        case ADCO :	str = "Adresse du compteur (ADCO)";break;
        case OPTARIF :	str = "Option tarifaire choisie (OPTARIF)";break;
        case ISOUSC :	str = "Intensité souscrite (ISOUSC)";break;
        case BASE :	str = "Index option Base (BASE)";break;
        case HCHC :	str = "Heures Creuses (HCHC)";break;
        case HCHP :	str = "Heures Pleines (HCHP)";break;
        case EJPHN :	str = "Heures Normales (EJPHN)";break;
        case EJPPM :	str = "Heures de Pointes EJP (EJPPM)";break;
        case EJPHPM :	str = "Heures de Pointes EJP (EJPHPM)";break;
        case BBRHCJB :	str = "Heures Creuses Jours Bleus (BBRHCJB)";break;
        case BBRHPJB :	str = "Heures Pleines Jours Bleus (BBRHPJB)";break;
        case BBRHCJW :	str = "Heures Creuses Jours Blancs (BBRHCJW)";break;
        case BBRHPJW :	str = "Heures Pleines Jours Blancs (BBRHPJW)";break;
        case BBRHCJR :	str = "Heures Creuses Jours Rouges (BBRHCJR)";break;
        case BBRHPJR :	str = "Heures Pleines Jours Rouges (BBRHPJR)";break;
        case PEJP :	str = "Préavis Début EJP (30 min) (PEJP)";break;
        case PTEC :	str = "Période Tarifaire en cours (PTEC)";break;
        case DEMAIN :	str = "Couleur du lendemain (DEMAIN)";break;
        case IINST :	str = "Intensité Instantanée (IINST)";break;
        case ADPS :	str = "Avertissement de Dépassement de Puissance Souscrite (ADPS)";break;
        case IMAX :	str = "Intensité maximale appelée (IMAX)";break;
        case PAPP :	str = "Puissance apparente (PAPP)";break;
        case HHPHC :	str = "Horaire Heures Pleines Heures Creuses (HHPHC)";break;
        case MOTDETAT :	str = "Mot d'état du compteur (MOTDETAT)";break;
        case IINST1 :	str = "Intensité Instantanée Phase 1 (IINST1)";break;
        case IINST2 :	str = "Intensité Instantanée Phase 2 (IINST2)";break;
        case IINST3 :	str = "Intensité Instantanée Phase 3 (IINST3)";break;
        case IMAX1 :	str = "Intensité maximale appelée Phase 1 (IMAX1)";break;
        case IMAX2 :	str = "Intensité maximale appelée Phase 2 (IMAX2)";break;
        case IMAX3 :	str = "Intensité maximale appelée Phase 3 (IMAX3)";break;
        case ADSC :	str = "Adresse Secondaire du Compteur (ADSC)";break;
        case VTIC :	str = "Version de la TIC (VTIC)";break;
        case DATE :	str = "Date et heure courante (DATE)";break;
        case NGTF :	str = "Nom du calendrier tarifaire fournisseur (NGTF)";break;
        case LTARF :	str = "Libellé tarif fournisseur en cours (LTARF)";break;
        case EAST :	str = "Energie active soutirée totale (EAST)";break;
        case EASF01 :	str = "Energie active soutirée Fournisseur, index 01 (EASF01)";break;
        case EASF02 :	str = "Energie active soutirée Fournisseur, index 02 (EASF02)";break;
        case EASF03 :	str = "Energie active soutirée Fournisseur, index 03 (EASF03)";break;
        case EASF04 :	str = "Energie active soutirée Fournisseur, index 04 (EASF04)";break;
        case EASF05 :	str = "Energie active soutirée Fournisseur, index 05 (EASF05)";break;
        case EASF06 :	str = "Energie active soutirée Fournisseur, index 06 (EASF06)";break;
        case EASF07 :	str = "Energie active soutirée Fournisseur, index 07 (EASF07)";break;
        case EASF08 :	str = "Energie active soutirée Fournisseur, index 08 (EASF08)";break;
        case EASF09 :	str = "Energie active soutirée Fournisseur, index 09 (EASF09)";break;
        case EASF10 :	str = "Energie active soutirée Fournisseur, index 10 (EASF10)";break;
        case EASD01 :	str = "Energie active soutirée Distributeur, index 01 (EASD01)";break;
        case EASD02 :	str = "Energie active soutirée Distributeur, index 02 (EASD02)";break;
        case EASD03 :	str = "Energie active soutirée Distributeur, index 03 (EASD03)";break;
        case EASD04 :	str = "Energie active soutirée Distributeur, index 04 (EASD04)";break;
        case EAIT :	str = "Energie active injectée totale (EAIT)";break;
        case ERQ1 :	str = "Energie réactive Q1 totale (ERQ1)";break;
        case ERQ2 :	str = "Energie réactive Q2 totale (ERQ2)";break;
        case ERQ3 :	str = "Energie réactive Q3 totale (ERQ3)";break;
        case ERQ4 :	str = "Energie réactive Q4 totale (ERQ4)";break;
        case IRMS1 :	str = "Courant efficace, phase 1 (IRMS1)";break;
        case IRMS2 :	str = "Courant efficace, phase 2 (IRMS2)";break;
        case IRMS3 :	str = "Courant efficace, phase 3 (IRMS3)";break;
        case URMS1 :	str = "Tension efficace, phase 1 (URMS1)";break;
        case URMS2 :	str = "Tension efficace, phase 2 (URMS2)";break;
        case URMS3 :	str = "Tension efficace, phase 3 (URMS3)";break;
        case PREF :	str = "Puissance app. de référence (PREF)";break;
        case PCOUP :	str = "Puissance app. de coupure (PCOUP)";break;
        case SINSTS :	str = "Puissance app. Instantanée soutirée (SINSTS)";break;
        case SINSTS1 :	str = "Puissance app. Instantanée soutirée phase 1 (SINSTS1)";break;
        case SINSTS2 :	str = "Puissance app. Instantanée soutirée phase 2 (SINSTS2)";break;
        case SINSTS3 :	str = "Puissance app. Instantanée soutirée phase 3 (SINSTS3)";break;
        case SMAXSN :	str = "Puissance app. max. soutirée n (SMAXSN)";break;
        case SMAXSN1 :	str = "Puissance app. max. soutirée n phase 1 (SMAXSN1)";break;
        case SMAXSN2 :	str = "Puissance app. max. soutirée n phase 2 (SMAXSN2)";break;
        case SMAXSN3 :	str = "Puissance app. max. soutirée n phase 3 (SMAXSN3)";break;
        case SMAXSN_1 :	str = "Puissance app max. soutirée n-1 (SMAXSN-1)";break;
        case SMAXSN1_1 :	str = "Puissance app max. soutirée n-1 phase 1 (SMAXSN1-1)";break;
        case SMAXSN2_1 :	str = "Puissance app max. soutirée n-1 phase 2 (SMAXSN2-1)";break;
        case SMAXSN3_1 :	str = "Puissance app max. soutirée n-1 phase 3 (SMAXSN3-1)";break;
        case SINSTI :	str = "Puissance app. Instantanée injectée (SINSTI)";break;
        case SMAXIN :	str = "Puissance app. max. injectée n (SMAXIN)";break;
        case SMAXIN_1 :	str = "Puissance app max. injectée n-1 (SMAXIN-1)";break;
        case CCASN :	str = "Point n de la courbe de charge active soutirée (CCASN)";break;
        case CCASN_1 :	str = "Point n-1 de la courbe de charge active soutirée (CCASN_1)";break;
        case CCAIN :	str = "Point n de la courbe de charge active injectée (CCAIN)";break;
        case CCAIN_1 :	str = "Point n-1 de la courbe de charge active injectée (CCAIN_1)";break;
        case UMOY1 :	str = "Tension moy. ph. 1 (UMOY1)";break;
        case UMOY2 :	str = "Tension moy. ph. 2 (UMOY2)";break;
        case UMOY3 :	str = "Tension moy. ph. 3 (UMOY3)";break;
        case STGE :	str = "Registre de Statuts (STGE)";break;
        case DPM1 :	str = "Début Pointe Mobile 1 (DPM1)";break;
        case FPM1 :	str = "Fin Pointe Mobile 1 (FPM1)";break;
        case DPM2 :	str = "Début Pointe Mobile 2 (DPM2)";break;
        case FPM2 :	str = "Fin Pointe Mobile 2 (FPM2)";break;
        case DPM3 :	str = "Début Pointe Mobile 3 (DPM3)";break;
        case FPM3 :	str = "Fin Pointe Mobile 3 (FPM3)";break;
        case MSG1 :	str = "Message court (MSG1)";break;
        case MSG2 :	str = "Message Ultra court (MSG2)";break;
        case PRM :	str = "PRM (PRM)";break;
        case RELAIS :	str = "Relais (RELAIS)";break;
        case NTARF :	str = "Numéro de l’index tarifaire en cours (NTARF)";break;
        case NJOURF :	str = "Numéro du jour en cours calendrier fournisseur (NJOURF)";break;
        case NJOUR_1 :	str = "Numéro du prochain jour calendrier fournisseur (NJOUR+1)";break;
        case PJOUR_1 :	str = "Profil du prochain jour calendrier fournisseur (PJOUR+1)";break;
        case PPOINTE :	str = "Profil du prochain jour de pointe (PPOINTE)";break;
        case UserDefined :	str = "User Defined";break;
        default : 	str = ""; break;
	}
	return str;
}



QString teleinfo::TeleInfoUnit(int index)
{
	QString str;
	switch(index)
	{
		case ADCO :	str = ""; break;
		case OPTARIF :	str = ""; break;
		case ISOUSC :	str = "A"; break;
		case BASE :	str = "Wh"; break;
		case HCHC :	str = "Wh"; break;
		case HCHP :	str = "Wh"; break;
		case EJPHN :	str = "Wh"; break;
		case EJPPM :	str = "Wh"; break;
		case EJPHPM :	str = "Wh"; break;
		case BBRHCJB :	str = "Wh"; break;
		case BBRHPJB :	str = "Wh"; break;
		case BBRHCJW :	str = "Wh"; break;
		case BBRHPJW :	str = "Wh"; break;
		case BBRHCJR :	str = "Wh"; break;
		case BBRHPJR :	str = "Wh"; break;
		case PEJP :	str = "min"; break;
		case PTEC :	str = ""; break;
		case DEMAIN :	str = ""; break;
		case IINST1 :
		case IINST2 :
		case IINST3 :
		case IINST :	str = "A"; break;
		case ADPS :	str = "A"; break;
		case IMAX1 :
		case IMAX2 :
		case IMAX3 :
		case IMAX :	str = "A"; break;
		case PAPP :	str = "VA"; break;
		case HHPHC :	str = ""; break;
		case MOTDETAT :	str = ""; break;
        case ADSC :	str = ""; break;
        case VTIC :	str = ""; break;
        case DATE :	str = ""; break;
        case NGTF :	str = ""; break;
        case LTARF :	str = ""; break;
        case EAST :	str = "Wh"; break;
        case EASF01 :	str = "Wh"; break;
        case EASF02 :	str = "Wh"; break;
        case EASF03 :	str = "Wh"; break;
        case EASF04 :	str = "Wh"; break;
        case EASF05 :	str = "Wh"; break;
        case EASF06 :	str = "Wh"; break;
        case EASF07 :	str = "Wh"; break;
        case EASF08 :	str = "Wh"; break;
        case EASF09 :	str = "Wh"; break;
        case EASF10 :	str = "Wh"; break;
        case EASD01 :	str = "Wh"; break;
        case EASD02 :	str = "Wh"; break;
        case EASD03 :	str = "Wh"; break;
        case EASD04 :	str = "Wh"; break;
        case EAIT :	str = "Wh"; break;
        case ERQ1 :	str = "VArh"; break;
        case ERQ2 :	str = "VArh"; break;
        case ERQ3 :	str = "VArh"; break;
        case ERQ4 :	str = "VArh"; break;
        case IRMS1 :	str = "A"; break;
        case IRMS2 :	str = "A"; break;
        case IRMS3 :	str = "A"; break;
        case URMS1 :	str = "V"; break;
        case URMS2 :	str = "V"; break;
        case URMS3 :	str = "V"; break;
        case PREF :	str = "kVA"; break;
        case PCOUP :	str = "kVA"; break;
        case SINSTS :	str = "VA"; break;
        case SINSTS1 :	str = "VA"; break;
        case SINSTS2 :	str = "VA"; break;
        case SINSTS3 :	str = "VA"; break;
        case SMAXSN :	str = "VA"; break;
        case SMAXSN1 :	str = "VA"; break;
        case SMAXSN2 :	str = "VA"; break;
        case SMAXSN3 :	str = "VA"; break;
        case SMAXSN_1 :	str = "VA"; break;
        case SMAXSN1_1 :	str = "VA"; break;
        case SMAXSN2_1 :	str = "VA"; break;
        case SMAXSN3_1 :	str = "VA"; break;
        case SINSTI :	str = "VA"; break;
        case SMAXIN :	str = "VA"; break;
        case SMAXIN_1 :	str = "VA"; break;
        case CCASN :	str = "W"; break;
        case CCASN_1 :	str = "W"; break;
        case CCAIN :	str = "W"; break;
        case CCAIN_1 :	str = "W"; break;
        case UMOY1 :	str = "V"; break;
        case UMOY2 :	str = "V"; break;
        case UMOY3 :	str = "V"; break;
        case STGE :	str = ""; break;
        case DPM1 :	str = ""; break;
        case FPM1 :	str = ""; break;
        case DPM2 :	str = ""; break;
        case FPM2 :	str = ""; break;
        case DPM3 :	str = ""; break;
        case FPM3 :	str = ""; break;
        case MSG1 :	str = ""; break;
        case MSG2 :	str = ""; break;
        case PRM :	str = ""; break;
        case RELAIS :	str = ""; break;
        case NTARF :	str = ""; break;
        case NJOURF :	str = ""; break;
        case NJOUR_1 :	str = ""; break;
        case PJOUR_1 :	str = ""; break;
        case PPOINTE :	str = ""; break;
        case UserDefined :	str = ""; break;
        default : 	str = ""; break;
	}
	return str;
}




bool teleinfo::horodatage(int index)
{
    bool r = false;
    switch(index)
    {
        case DATE :	r = true; break;
        case SMAXSN :	r = true; break;
        case SMAXSN1 :	r = true; break;
        case SMAXSN2 :	r = true; break;
        case SMAXSN3 :	r = true; break;
        case SMAXSN_1 :	r = true; break;
        case SMAXSN1_1 :	r = true; break;
        case SMAXSN2_1 :	r = true; break;
        case SMAXSN3_1 :	r = true; break;
        case SMAXIN :	r = true; break;
        case SMAXIN_1 :	r = true; break;
        case CCASN :	r = true; break;
        case CCASN_1 :	r = true; break;
        case CCAIN :	r = true; break;
        case CCAIN_1 :	r = true; break;
        case UMOY1 :	r = true; break;
        case UMOY2 :	r = true; break;
        case UMOY3 :	r = true; break;
        case DPM1 :	r = true; break;
        case FPM1 :	r = true; break;
        case DPM2 :	r = true; break;
        case FPM2 :	r = true; break;
        case DPM3 :	r = true; break;
        case FPM3 :	r = true; break;
    }
    return r;
}



int teleinfo::TeleInfoValeurLength(int index)
{
	int L;
	switch(index)
	{
		case ADCO :	 L = 12; break;
		case OPTARIF :	 L = 4; break;
		case ISOUSC :	 L = 2; break;
		case BASE :	 L = 9; break;
		case HCHC :	 L = 9; break;
		case HCHP :	 L = 9; break;
		case EJPHN :	 L = 9; break;
		case EJPPM :	 L = 9; break;
		case EJPHPM :	 L = 9; break;
		case BBRHCJB :	 L = 9; break;
		case BBRHPJB :	 L = 9; break;
		case BBRHCJW :	 L = 9; break;
		case BBRHPJW :	 L = 9; break;
		case BBRHCJR :	 L = 9; break;
		case BBRHPJR :	 L = 9; break;
		case PEJP :	 L = 2; break;
		case PTEC :	 L = 4; break;
		case DEMAIN :	 L = 4; break;
		case IINST :	 L = 3; break;
		case IINST1 :	 L = 3; break;
		case IINST2 :	 L = 3; break;
		case IINST3 :	 L = 3; break;
		case ADPS :	 L = 3; break;
		case IMAX1 :	 L = 3; break;
		case IMAX2 :	 L = 3; break;
		case IMAX3 :	 L = 3; break;
		case IMAX :	 L = 3; break;
		case PAPP :	 L = 5; break;
		case HHPHC :	 L = 1; break;
		case MOTDETAT :	 L = 6; break;
        case ADSC :	L = 12; break;
        case VTIC :	L = 2; break;
        case DATE :	L = 0; break;
        case NGTF :	L =16; break;
        case LTARF :	L = 16; break;
        case EAST :	L = 9; break;
        case EASF01 :	L = 9; break;
        case EASF02 :	L = 9; break;
        case EASF03 :	L = 9; break;
        case EASF04 :	L = 9; break;
        case EASF05 :	L = 9; break;
        case EASF06 :	L = 9; break;
        case EASF07 :	L = 9; break;
        case EASF08 :	L = 9; break;
        case EASF09 :	L = 9; break;
        case EASF10 :	L = 9; break;
        case EASD01 :	L = 9; break;
        case EASD02 :	L = 9; break;
        case EASD03 :	L = 9; break;
        case EASD04 :	L = 9; break;
        case EAIT :	L = 9; break;
        case ERQ1 :	L = 9; break;
        case ERQ2 :	L = 9; break;
        case ERQ3 :	L = 9; break;
        case ERQ4 :	L = 9; break;
        case IRMS1 :	L = 3; break;
        case IRMS2 :	L = 3; break;
        case IRMS3 :	L = 3; break;
        case URMS1 :	L = 3; break;
        case URMS2 :	L = 3; break;
        case URMS3 :	L = 3; break;
        case PREF :	L = 2; break;
        case PCOUP :	L = 2; break;
        case SINSTS :	L = 5; break;
        case SINSTS1 :	L = 5; break;
        case SINSTS2 :	L = 5; break;
        case SINSTS3 :	L = 5; break;
        case SMAXSN :	L = 5; break;
        case SMAXSN1 :	L = 5; break;
        case SMAXSN2 :	L = 5; break;
        case SMAXSN3 :	L = 5; break;
        case SMAXSN_1 :	L = 5; break;
        case SMAXSN1_1 :	L = 5; break;
        case SMAXSN2_1 :	L = 5; break;
        case SMAXSN3_1 :	L = 5; break;
        case SINSTI :	L = 5; break;
        case SMAXIN :	L = 5; break;
        case SMAXIN_1 :	L = 5; break;
        case CCASN :	L = 5; break;
        case CCASN_1 :	L = 5; break;
        case CCAIN :	L = 5; break;
        case CCAIN_1 :	L = 5; break;
        case UMOY1 :	L = 3; break;
        case UMOY2 :	L = 3; break;
        case UMOY3 :	L = 3; break;
        case STGE :	L = 8; break;
        case DPM1 :	L = 2; break;
        case FPM1 :	L = 2; break;
        case DPM2 :	L = 2; break;
        case FPM2 :	L = 2; break;
        case DPM3 :	L = 2; break;
        case FPM3 :	L = 2; break;
        case MSG1 :	L = 32; break;
        case MSG2 :	L = 16; break;
        case PRM :	L = 14; break;
        case RELAIS :	L = 3; break;
        case NTARF :	L = 2; break;
        case NJOURF :	L = 2; break;
        case NJOUR_1 :	L = 2; break;
        case PJOUR_1 :	L = 98; break;
        case PPOINTE :	L = 98; break;
        default : 		L = 0; break;
	}
	return L;
}






void teleinfo::timeout()
{
	TimeOut.stop();
	GenError(33, name);
    for (int n=0; n<localdevice.count(); n++)
    {
        localdevice[n]->setValid(onewiredevice::dataNotValid);
    }
    receivedData = false;
    reconnecttcp();
    TimeOut.start(DataTimeOut);
}



bool teleinfo::receiveData()
{
    return receivedData;
}



onewiredevice *teleinfo::addDevice(QString name, bool show)
{
	onewiredevice *device;
	for (int id=1; id<99; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + QString("%1").arg(id, 3, 10, QChar('0')) + familyTeleInfo);
		if (!parent->configwin->deviceexist(RomID))
		{
			device = parent->configwin->NewDevice(RomID, this);
			if (device)
			{
				device->setname(name);
				if (!localdevice.contains(device)) localdevice.append(device);
				UpdateLocalDeviceList();
				if (show) device->show();
				return device;
			}
		}
	}
	return nullptr;
}



