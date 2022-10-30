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
#include "resol.h"



resol::resol(logisdom *Parent) : net1wire(Parent)
{
	parameterNb = 0;
	waitingEndOfFrame = false;
        framelayout = new QGridLayout(ui.frameguinet);
}



void resol::init()
{
	type = NetType(TCP_ResolType);
	Buffer = "";
	retry = 0;
	connect(&tcp, SIGNAL(connected()), this, SLOT(tcpconnected()));
	connect(&tcp, SIGNAL(disconnected()), this, SLOT(tcpdisconnected()));
	connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpConnectionError(QAbstractSocket::SocketError)));
	connect(&tcp, SIGNAL(readyRead()), this, SLOT(readbuffer()));
	connect(&TimeOut, SIGNAL(timeout()), this, SLOT(timeout()));
	connecttcp();

	int i = 0;
	label_pw.setText(tr("Password : "));
	framelayout->addWidget(&label_pw, i, 0, 1, 1);
	password.setEchoMode(QLineEdit::PasswordEchoOnEdit);
	framelayout->addWidget(&password, i++, 1, 1, 1);


	//Bouton1.setText("Show");
	//connect(&Bouton1, SIGNAL(clicked()), this, SLOT(ShowDevice()));
	//framelayout->addWidget(&Bouton1, i++, 0, 1, 1);

// setup CreateDev
	Bouton2.setText(tr("New Device"));
	connect(&Bouton2, SIGNAL(clicked()), this , SLOT(NewDevice()));
	framelayout->addWidget(&Bouton2, i++, 0, 1, 1);

// setup RemoveDev
	//Bouton3.setText(tr("Remove Device"));
	//connect(&Bouton3, SIGNAL(clicked()), this , SLOT(RemoveModule()));
	//framelayout->addWidget(&Bouton3, i++, 0, 1, 1);

	ui.EditType->setText("RESOL");
        QString configdata;
        parent->get_ConfigData(configdata);
        readConfigFile(configdata);
}






void resol::NewDevice()
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





void resol::getConfig(QString &str)
{
	str += logisdom::saveformat("password", password.text());
}




void resol::setConfig(const QString &strsearch)
{
	password.setText(logisdom::getvalue("password", strsearch));
}





void resol::readConfigFile(QString &configdata)
{
	QString ReadRomID;
	onewiredevice *device = nullptr;
	QString TAG_Begin = One_Wire_Device;
	QString TAG_End = EndMark;
	SearchLoopBegin
	ReadRomID = logisdom::getvalue("RomID", strsearch);
	QString family = ReadRomID.right(2);
	if (ReadRomID.length() == 13) ReadRomID = ReadRomID.left(8) + port2Hex(port) + ReadRomID.right(5);
	if (ReadRomID.left(12) == (ip2Hex(moduleipaddress) + port2Hex(port)) && (family == familyResol))
		if (!parent->configwin->deviceexist(ReadRomID))
		{
			device = parent->configwin->NewDevice(ReadRomID, this);
			if (device)
				if (!localdevice.contains(device)) localdevice.append(device);
			UpdateLocalDeviceList();
		}
    SearchLoopEnd
}





void resol::fifonext()
{
}







void resol::readbuffer()
{
	settraffic(Connected);
	QByteArray data;
	QMutexLocker locker(&mutexreadbuffer);
//more:
	data = tcp.readAll();
    QString logtxt;
    for (int n=0; n<data.length(); n++) logtxt += QString("[%1]").arg(uchar(data[n]));
	if (data.contains("+HELLO"))
	{
		GenMsg(data + " : Welcome message received");
		writeTCP("PASS " + password.text() + "\n");
		GenMsg("Password sent");
	}
	else if (data.contains("+OK: Password accepted"))
	{
		GenMsg(data + " : Password accepted");
		writeTCP("DATA\n");
		GenMsg("Requesting data");
	}
	else
	{
        GenMsg(tr("Read Raw : ") + logtxt);
		Buffer.append(data);
		int syncIndex = Buffer.indexOf(SYNC);
		int length = Buffer.length();
		QByteArray frameStr;
		if (((syncIndex != -1) && (length > (syncIndex + 9))) or (waitingEndOfFrame))
		{
			int frameCount = Buffer.at(syncIndex + 8);
			waitingEndOfFrame = true;
			if (length >= (syncIndex + 9 + (frameCount * 6)))
			{
				frameStr = Buffer.mid(syncIndex, 10 + (frameCount * 6));
				GenMsg("Destination Address : " + toHex(frameStr.mid(2, 1) + frameStr.mid(1, 1)));
				GenMsg("Source Address : " + toHex(frameStr.mid(4, 1) + frameStr.mid(3, 1)));
				GenMsg("Protocol : " + toHex(frameStr.mid(5, 1)));
				GenMsg("Command : " + toHex(frameStr.mid(7, 1) + frameStr.mid(6, 1)));
				int HeaderCRC = 0;
				for (int n=1; n<9; n++) HeaderCRC += frameStr.at(n);
				HeaderCRC = ~HeaderCRC & 0x7F;
				//GenMsg(QString("Header CRC : %1").arg((unsigned char)frameStr[9]));
				//GenMsg(QString("Header Calculated CRC : %1").arg(HeaderCRC));
                int Adr = frameStr.at(3) + 0x100 * frameStr.at(4);
                int Command = frameStr.at(6) + 0x100 * frameStr.at(7);
                int destAdr = frameStr.at(1) + 0x100 * frameStr.at(2);
                if (frameStr[9] - HeaderCRC)
				{
					GenMsg("Header CRC failed");
				}
                else if ((Command == 0x100) && (destAdr == 0x10))
                {
					GenMsg("Header CRC Ok");
                    {
                        QStringList list;
                        getParameterList(Adr, list);
                        for (int n=0; n<list.count(); n++)
                        {
                            QStringList split;
                            split = list.at(n).split("*");
                            bool ok1, ok2, ok3;
                            int frame = split.at(1).toInt(&ok1);
                            int pos = split.at(2).toInt(&ok2);
                            int size = split.at(3).toInt(&ok3);
                            double v = getParameter(frameStr, frame, pos, size);
                            GenMsg(split.at(0) + QString(" = %1").arg(v));
                        }
                    }
/*
					QString P = getParameterStr(index, frame, pos, size);
					while (!P.isEmpty())
					{
						double v = getParameter(frameStr, frame, pos, size);
						GenMsg(P + QString(" = %1").arg(v));
						index ++;
						P = getParameterStr(index, frame, pos, size);
						if (frame > frameCount) break;
					}*/

                    QString hexFrame = toHex(frameStr);
                    for (int n=0; n<localdevice.count(); n++)
                        if (!localdevice[n]->setscratchpad(hexFrame, true))
                            GenError(34, localdevice[n]->getromid() + "  " + localdevice[n]->getname());
                }
				Buffer.clear();
				waitingEndOfFrame = false;
			}
		}
	}
	TimeOut.stop();
//	if (tcp.bytesAvailable() > 0) goto more;
	TimeOut.start(60000);
}







QString resol::toHex(QByteArray data)
{
	QString hex;
    for (int n=0; n<data.length(); n++) hex += QString("%1").arg(uchar(data[n]), 2, 16, QChar('0'));
	return hex.toUpper();
}




double resol::getParameter(QByteArray &frameStr, int &frame, int &pos, int &size)
{
	QByteArray frameN = getFrame(frameStr, frame);
	QString logtxt;
    for (int n=0; n<frameN.length(); n++) logtxt += QString("[%1]").arg(uchar(frameN[n]));
	double total = logisdom::NA;
	if (frameN.isEmpty()) return total;
	if (frameN.length() != 6) return total;
	// check frame CRC
	int FrameCRC = 0;
	for (int n=0; n<5; n++) FrameCRC += frameN.at(n);
	FrameCRC = ~FrameCRC & 0x7F;
	if (frameN[5] - FrameCRC) return total;
    if (size == 1) { // 8 bits
        total = getByte(frameN, pos);
        return total;
    }
    if (size == 2) { // 16 bits
        total = getByte(frameN, pos) + 256 * getByte(frameN, pos + 1);
        int T = int(total);
        bool s = false;
        if (T & 0x8000)
        {
            T = 0xFFFF - T + 1;
            s = true;
        }
        if (s) total = -double(T);
            else total = double(T);
        return total;
    }
    if (size == 3) {
        total = getByte(frameN, pos) + 256 * getByte(frameN, pos + 1) + 256 * 256 * getByte(frameN, pos + 2);
        return total;
    }
    if (size == 4) { // 32 bits
        total = getByte(frameN, pos) + 256 * getByte(frameN, pos + 1) + 256 * 256 * getByte(frameN, pos + 2) + 256 * 256 * 256 * getByte(frameN, pos + 3);
        return total;
    }
	return total;
}







QString resol::AddressToName(int address)
{
	QString str;
	switch (address)
	{
		case 0x0000 : str = "Broadcast"; break;
		case 0x0010 : str = "DFA"; break;
		case 0x3011 : str = "WMZ-L10"; break;
		case 0x7731 : str = "SOLTOP DeltaSol S2/S3"; break;
		case 0x7711 : str = "Multitronic [Regler]"; break;
		case 0x7712 : str = "Multitronic [WMZ]"; break;
		case 0x6620 : str = "SunGo XL"; break;
		case 0x0001 : str = "FriWa-Prototyp"; break;
		case 0x3111 : str = "Heizungspumpenregler"; break;
		case 0x7F21 : str = "BV-SOL[10]-Prototyp"; break;
		case 0x7F31 : str = "PER1-SOLEX-Prototyp"; break;
		case 0x4211 : str = "BL-SOL[10]-Prototyp"; break;
		case 0x4241 : str = "REGLOfresh"; break;
		case 0x2121 : str = "DrainBack Remeha 1"; break;
		case 0x7F7E : str = "DrainBack Remeha 2"; break;
		case 0x7F71 : str = "DeltaSol FCS"; break;
		case 0x7331 : str = "SLR"; break;
		case 0x6521 : str = "MSR-65 #"; break;
		case 0x3311 : str = "Diemasol C"; break;
		case 0x7751 : str = "DeDietrich Diemasol C v2007"; break;
		case 0x4311 : str = "Drainback DeDietrich"; break;
		case 0x7511 : str = "Projekt Dr. Schmidt"; break;
		case 0x3241 : str = "DT4 (B)"; break;
		case 0x5221 : str = "DT4 (MS)"; break;
		case 0x3271 : str = "ConergyDT5"; break;
		case 0x3211 : str = "EL1"; break;
		case 0x3221 : str = "DeltaSol Pro"; break;
		case 0x3231 : str = "DeltaSol B"; break;
		case 0x3251 : str = "DeltaSol BS"; break;
		case 0x4010 : str = "WMZ #"; break;
		case 0x4221 : str = "DeltaSol BS Plus"; break;
		case 0x4410 : str = "MSR44"; break;
		case 0x4420 : str = "HKM1 #"; break;
		case 0x5210 : str = "DeltaSol Plus"; break;
		case 0x5510 : str = "EL2/3"; break;
		case 0x6510 : str = "HKM2"; break;
		case 0x6520 : str = "MSR65"; break;
		case 0x6610 : str = "Midi Pro"; break;
		case 0x7311 : str = "DeltaSol M [Regler]"; break;
		case 0x7312 : str = "DeltaSol M [HK1]"; break;
		case 0x7313 : str = "DeltaSol M [HK2]"; break;
		case 0x7316 : str = "DeltaSol M [WMZ1]"; break;
		case 0x7317 : str = "DeltaSol M [WMZ2]"; break;
		case 0x7411 : str = "DeltaSol ES"; break;
		case 0x7611 : str = "Friwa"; break;
		case 0x7621 : str = "SOLEX [Regler]"; break;
		case 0x7622 : str = "SOLEX [WMZ]"; break;
		case 0x7721 : str = "DeltaSol E [Regler]"; break;
		case 0x7722 : str = "DeltaSol E [WMZ]"; break;
		case 0x7F61 : str = "IOC-Modul [Messwerte]"; break;
		case 0x7F62 : str = "IOC-Modul [Tagesbilanz]"; break;
		case 0x7F63 : str = "IOC-Modul [Entnahmekreis]"; break;
		case 0x5111 : str = "DeltaSol D"; break;
		case 0x4278 : str = "DeltaSol BS/DrainBack"; break;
		case 0x4279 : str = "DeltaSol BS/DrainBack (Fahrenheit)"; break;
		case 0x7761 : str = "DeltaSol Pool"; break;
		case 0x7762 : str = "DeltaSol Pool [WMZ]"; break;
		case 0x4212 : str = "DeltaSol C"; break;
		case 0x4223 : str = "DeltaSol BS Plus BTU"; break;
		case 0x5311 : str = "X-Control"; break;
		case 0x7210 : str = "SKSR 1/2/3"; break;
		case 0x7211 : str = "SKSC3 [HK1]"; break;
		case 0x7212 : str = "SKSC3 [HK2]"; break;
		case 0x7213 : str = "SKSC3 [HK3]"; break;
		case 0x4231 : str = "Frista"; break;
		case 0x4251 : str = "DSPlus UMSYS [Regler]"; break;
		case 0x7321 : str = "Vitosolic 200 [Regler]"; break;
		case 0x7326 : str = "Vitosolic 200 [WMZ1]"; break;
		case 0x7327 : str = "Vitosolic 200 [WMZ2]"; break;
        case 0x7E11 : str = "DeltaSol MX [Regler]"; break;
        case 0x7E31 : str = "DeltaSol MX [WMZ1]"; break;
        case 0x7E32 : str = "DeltaSol MX [WMZ2]"; break;
        case 0x7E33 : str = "DeltaSol MX [WMZ3]"; break;
        case 0x7E34 : str = "DeltaSol MX [WMZ4]"; break;
        case 0x7E35 : str = "DeltaSol MX [WMZ5]"; break;
    }
	return str;
}




void resol::getParameterList(int address, QStringList &list)
{
	list.clear();
	switch (address)
	{
/*		case 0x0000 : str = "Broadcast"; break;
		case 0x0010 : str = "DFA"; break;
		case 0x3011 : str = "WMZ-L10"; break;
		case 0x7731 : str = "SOLTOP DeltaSol S2/S3"; break;
		case 0x7711 : str = "Multitronic [Regler]"; break;
		case 0x7712 : str = "Multitronic [WMZ]"; break;
		case 0x6620 : str = "SunGo XL"; break;
		case 0x0001 : str = "FriWa-Prototyp"; break;
		case 0x3111 : str = "Heizungspumpenregler"; break;
		case 0x7F21 : str = "BV-SOL[10]-Prototyp"; break;
		case 0x7F31 : str = "PER1-SOLEX-Prototyp"; break;
		case 0x4211 : str = "BL-SOL[10]-Prototyp"; break;
		case 0x4241 : str = "REGLOfresh"; break;
		case 0x2121 : str = "DrainBack Remeha 1"; break;
		case 0x7F7E : str = "DrainBack Remeha 2"; break;
		case 0x7F71 : str = "DeltaSol FCS"; break;
		case 0x7331 : str = "SLR"; break;
		case 0x6521 : str = "MSR-65 #"; break;
		case 0x3311 : str = "Diemasol C"; break;
		case 0x7751 : str = "DeDietrich Diemasol C v2007"; break;
		case 0x4311 : str = "Drainback DeDietrich"; break;
		case 0x7511 : str = "Projekt Dr. Schmidt"; break;
		case 0x3241 : str = "DT4 (B)"; break;
		case 0x5221 : str = "DT4 (MS)"; break;
		case 0x3271 : str = "ConergyDT5"; break;
		case 0x3211 : str = "EL1"; break;
		case 0x3221 : str = "DeltaSol Pro"; break;
		case 0x3231 : str = "DeltaSol B"; break;
		case 0x3251 : str = "DeltaSol BS"; break;
		case 0x4010 : str = "WMZ #"; break;
		case 0x4221 : str = "DeltaSol BS Plus"; break;
		case 0x4410 : str = "MSR44"; break;
		case 0x4420 : str = "HKM1 #"; break;
		case 0x5210 : str = "DeltaSol Plus"; break;
		case 0x5510 : str = "EL2/3"; break;
		case 0x6510 : str = "HKM2"; break;
		case 0x6520 : str = "MSR65"; break;
		case 0x6610 : str = "Midi Pro"; break;*/
		case 0x7311 : list << tr("Sonde 1") + "*0*0*2*10" << tr("Sonde 2") + "*0*2*2*10" << tr("Sonde 3") + "*1*0*2*10" << tr("Sonde 4") + "*1*2*2*10" << tr("Sonde 5") + "*2*0*2*10" << tr("Sonde 6") + "*2*2*2*10" << tr("Sonde 7") + "*3*0*2*10" << tr("Sonde 8") + "*3*2*2*10" << tr("Sonde 9") + "*4*0*2*10" << tr("Sonde 10") + "*4*2*2*10" << tr("Sonde 11") + "*5*0*2*10" << tr("Sonde 12") + "*5*2*2*10" << tr("Capteur solaire (CS)") + "*6*0*2" << tr("Impulse 1 V40") + "*6*2*2" << tr("Impulseingang 1a") + "*7*0*2" << tr("Impulseingang 1b") + "*7*1*2" << tr("Impulseingang 1b") + "*7*2*2" << tr("Impulseingang 1c") + "*7*3*2" << tr("Impulseingang 2a") + "*8*0*2" << tr("Impulseingang 2b") + "*8*1*2" << tr("Impulseingang 2c") + "*8*2*2" << tr("Impulseingang 2d") + "*8*3*2" << tr("Sortie 1") + "*11*0*1" << tr("Sortie 2") + "*11*1*1" << tr("Sortie 3") + "*11*2*1" << tr("Sortie 4") + "*11*3*1" << tr("Sortie 5") + "*12*0*1" << tr("Sortie 6") + "*12*1*1" << tr("Sortie 7") + "*12*2*1"  << tr("Sortie 8") + "*12*3*1" << tr("Sortie 9") + "*13*0*1"; break;
		//<< tr("Code Erreur") + "*9*0*2" << tr("Informations") + "*9*2*2" << tr("Schema") + "*10*2*2" << tr("Sonde aller HK1 Module Sensor 18") + "*11*0*2" << tr("Status HK1 Module") + "*11*2*2" << tr("Sonde aller HK2 Module Sensor 25") + "*12*0*2" << tr("Status HK2 Module") + "*12*2*2" << tr("Sonde aller HK3 Module Sensor 32") + "*13*0*2" << tr("Status HK3 Module") + "*13*2*2" << tr("Sonde aller circuit chauffage Sensor 11") + "*14*0*2" << tr("Status circuit chauffage") + "*14*2*2" << tr("Version soft") + "*15*0*1" << tr("Révision N°") + "*15*1*1" << tr("Heure") + "*15*2*2" << tr("Année") + "*16*0*2" << tr("Mois") + "*16*2*1" << tr("Jour") + "*16*3*1"; break;
/*		case 0x7312 : str = "DeltaSol M [HK1]"; break;
		case 0x7313 : str = "DeltaSol M [HK2]"; break;
		case 0x7316 : str = "DeltaSol M [WMZ1]"; break;
		case 0x7317 : str = "DeltaSol M [WMZ2]"; break;
		case 0x7411 : str = "DeltaSol ES"; break;
		case 0x7611 : str = "Friwa"; break;
		case 0x7621 : str = "SOLEX [Regler]"; break;
		case 0x7622 : str = "SOLEX [WMZ]"; break;*/
// Name * Frame * Position * Length * Coef10
		case 0x7721 : list << tr("Sonde 1") + "*0*0*2*10" << tr("Sonde 2") + "*0*2*2*10" << tr("Sonde 3") + "*1*0*2*10" << tr("Sonde 4") + "*1*2*2*10" << tr("Sonde 5") + "*2*0*2*10" << tr("Sonde 6") + "*2*2*2*10" << tr("Sonde 7") + "*3*0*2*10" << tr("Sonde 8") + "*3*2*2*10" << tr("Sonde 9") + "*4*0*2*10" << tr("Sonde 10") + "*4*2*2*10" << tr("Capteur solaire (CS)") + "*5*0*2" << tr("Sortie 1") + "*6*2*1" << tr("Sortie 2") + "*6*3*1" << tr("Sortie 3") + "*7*0*1" << tr("Sortie 4") + "*7*1*1" << tr("Sortie 5") + "*7*2*1" << tr("Sortie 6") + "*7*3*1" << tr("Sortie 7") + "*8*0*1" << tr("Code Erreur") + "*9*0*2" << tr("Informations") + "*9*2*2" << tr("Schema") + "*10*2*2" << tr("Sonde aller HK1 Module Sensor 18") + "*11*0*2" << tr("Status HK1 Module") + "*11*2*2" << tr("Sonde aller HK2 Module Sensor 25") + "*12*0*2" << tr("Status HK2 Module") + "*12*2*2" << tr("Sonde aller HK3 Module Sensor 32") + "*13*0*2" << tr("Status HK3 Module") + "*13*2*2" << tr("Sonde aller circuit chauffage Sensor 11") + "*14*0*2" << tr("Status circuit chauffage") + "*14*2*2" << tr("Version soft") + "*15*0*1" << tr("Révision N°") + "*15*1*1" << tr("Heure") + "*15*2*2" << tr("Année") + "*16*0*2" << tr("Mois") + "*16*2*1" << tr("Jour") + "*16*3*1"; break;
		case 0x7722 : list << tr("Température Départ") + "*0*0*2*10" << tr("Température Retour") + "*0*2*2*10" << tr("Débit L/h") + "*1*0*2" << tr("Puissance Cumulée W") + "*1*2*2" << tr("Puissance Cumulée Kw") + "*2*0*2" << tr("Puissance Cumulée Mw") + "*2*2*2"; break;
/*		case 0x7F61 : str = "IOC-Modul [Messwerte]"; break;
		case 0x7F62 : str = "IOC-Modul [Tagesbilanz]"; break;
		case 0x7F63 : str = "IOC-Modul [Entnahmekreis]"; break;
		case 0x5111 : str = "DeltaSol D"; break;
		case 0x4278 : str = "DeltaSol BS/DrainBack"; break;
		case 0x4279 : str = "DeltaSol BS/DrainBack (Fahrenheit)"; break;
		case 0x7761 : str = "DeltaSol Pool"; break;
		case 0x7762 : str = "DeltaSol Pool [WMZ]"; break;
		case 0x4212 : str = "DeltaSol C"; break;
		case 0x4223 : str = "DeltaSol BS Plus BTU"; break;
		case 0x5311 : str = "X-Control"; break;
		case 0x7210 : str = "SKSR 1/2/3"; break;
		case 0x7211 : str = "SKSC3 [HK1]"; break;
		case 0x7212 : str = "SKSC3 [HK2]"; break;
		case 0x7213 : str = "SKSC3 [HK3]"; break;
		case 0x4231 : str = "Frista"; break;
		case 0x4251 : str = "DSPlus UMSYS [Regler]"; break;*/
		case 0x7321 : list << tr("Sonde 1") + "*0*0*2*10" << tr("Sonde 2") + "*0*2*2*10" << tr("Sonde 3") + "*1*0*2*10" << tr("Sonde 4") + "*1*2*2*10" << tr("Sonde 5") + "*2*0*2*10" << tr("Sonde 6") + "*2*2*2*10" << tr("Sonde 7") + "*3*0*2*10" << tr("Sonde 8") + "*3*2*2*10" << tr("Sonde 9") + "*4*0*2*10" << tr("Sonde 10") + "*4*2*2*10" << tr("Capteur solaire (CS)") + "*5*0*2" << tr("Sortie 1") + "*6*2*1" << tr("Sortie 2") + "*6*3*1" << tr("Sortie 3") + "*7*0*1" << tr("Sortie 4") + "*7*1*1" << tr("Sortie 5") + "*7*2*1" << tr("Sortie 6") + "*7*3*1" << tr("Sortie 7") + "*8*0*1" << tr("Code Erreur") + "*9*0*2" << tr("Informations") + "*9*2*2" << tr("Schema") + "*10*2*2" << tr("Sonde aller HK1 Module Sensor 18") + "*11*0*2" << tr("Status HK1 Module") + "*11*2*2" << tr("Sonde aller HK2 Module Sensor 25") + "*12*0*2" << tr("Status HK2 Module") + "*12*2*2" << tr("Sonde aller HK3 Module Sensor 32") + "*13*0*2" << tr("Status HK3 Module") + "*13*2*2" << tr("Sonde aller circuit chauffage Sensor 11") + "*14*0*2" << tr("Status circuit chauffage") + "*14*2*2" << tr("Version soft") + "*15*0*1" << tr("Révision N°") + "*15*1*1" << tr("Heure") + "*15*2*2" << tr("Année") + "*16*0*2" << tr("Mois") + "*16*2*1" << tr("Jour") + "*16*3*1"; break;
		case 0x7326 : list << tr("Température Départ") + "*0*0*2*10" << tr("Température Retour") + "*0*2*2*10" << tr("Débit L/h") + "*1*0*2" << tr("Puissance Cumulée W") + "*1*2*2" << tr("Puissance Cumulée Kw") + "*2*0*2" << tr("Puissance Cumulée Mw") + "*2*2*2"; break;
/*		case 0x7321 : str = "Vitosolic 200 [Regler]"; break;
		case 0x7326 : str = "Vitosolic 200 [WMZ1]"; break;
		case 0x7327 : str = "Vitosolic 200 [WMZ2]"; break;*/
        case 0x7E11 : list  << tr("Sonde T°1") + "*0*0*2*10" << tr("Sonde T°2") + "*0*2*2*10" << tr("Sonde T°3") + "*1*0*2*10" \
                            << tr("Sonde T°4") + "*1*2*2*10" << tr("Sonde T°5") + "*2*0*2*10" << tr("Sonde T°6") + "*2*2*2*10" \
                            << tr("Sonde T°7") + "*3*0*2*10" << tr("Sonde T°8") + "*3*2*2*10" << tr("Sonde T°9") + "*4*0*2*10" \
                            << tr("Sonde T°10") + "*4*2*2*10" << tr("Sonde T°11") + "*5*0*2*10" << tr("Sonde T°12") + "*5*2*2*10" \
                            << tr("Sonde T°13") + "*6*0*2*10" << tr("Sonde T°14") + "*6*2*2*10" << tr("Sonde T°15") + "*7*0*2*10" \
                            << tr("Sonde T°16") + "*7*2*2*10" << tr("Sonde T°17") + "*8*0*2*10" << tr("Sonde T°18") + "*8*2*2*10" \
                            << tr("Sonde T°19") + "*9*0*2*10"  << tr("Sonde T°20") + "*9*2*2*10" \
                            << tr("Débitmètre S13") + "*10*0*4" << tr("Débitmètre S14") + "*11*0*4" \
                            << tr("Débitmètre S15") + "*12*0*4" << tr("Débitmètre S17") + "*13*0*4" \
                            << tr("Débitmètre S18") + "*14*0*4" << tr("Débitmètre S19") + "*15*0*4" \
                            << tr("Débitmètre S20") + "*16*0*4" \
                            << tr("Capteur de pression S17") + "*17*0*2*100" << tr("Capteur de pression S18") + "*17*2*2*100" \
                            << tr("Capteur de pression S19") + "*18*0*2*100" << tr("Capteur de pression S20") + "*18*2*2*100" \
                            << tr("Relais sortie 1") + "*19*0*1" << tr("Relais sortie 2") + "*19*1*1" \
                            << tr("Relais sortie 3") + "*19*2*1" << tr("Relais sortie 4") + "*19*3*1" \
                            << tr("Relais sortie 5") + "*20*0*1" << tr("Relais sortie 6") + "*20*1*1" \
                            << tr("Relais sortie 7") + "*20*2*1" << tr("Relais sortie 8") + "*20*3*1" \
                            << tr("Relais sortie 9") + "*21*0*1" << tr("Relais sortie 10") + "*21*1*1" \
                            << tr("Relais sortie 11") + "*21*2*1" << tr("Relais sortie 12") + "*21*3*1" \
                            << tr("Relais sortie 13") + "*22*0*1" << tr("Relais sortie 14") + "*22*1*1" \
                            << tr("Date système") + "*23*0*0" << tr("Code erreur") + "*24*0*4" \
                          ; break;
        case 0x7E31 : list  << tr("Puissance") + "*0*0*4" << tr("Puissance jour") + "*1*0*4" << tr("Puissance semaine") + "*2*0*4"; break;
        case 0x7E32 : list  << tr("Puissance") + "*0*0*4" << tr("Puissance jour") + "*1*0*4" << tr("Puissance semaine") + "*2*0*4"; break;
        case 0x7E33 : list  << tr("Puissance") + "*0*0*4" << tr("Puissance jour") + "*1*0*4" << tr("Puissance semaine") + "*2*0*4"; break;
        case 0x7E34 : list  << tr("Puissance") + "*0*0*4" << tr("Puissance jour") + "*1*0*4" << tr("Puissance semaine") + "*2*0*4"; break;
        case 0x7E35 : list  << tr("Puissance") + "*0*0*4" << tr("Puissance jour") + "*1*0*4" << tr("Puissance semaine") + "*2*0*4"; break;
    }
}

/*
jeudi 31/01/2013 22:01:45:744  :  Command : 0100
jeudi 31/01/2013 22:01:45:858  :  Read Raw : [170][16][0][49]
jeudi 31/01/2013 22:01:45:932  :  Read Raw : [126][16][0][1][4][43][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:45:932  :  Destination Address : 0010
jeudi 31/01/2013 22:01:45:932  :  Source Address : 7E31
jeudi 31/01/2013 22:01:45:932  :  Protocol : 10
jeudi 31/01/2013 22:01:45:932  :  Command : 0100
jeudi 31/01/2013 22:01:45:932  :  Header CRC Ok
jeudi 31/01/2013 22:01:45:932  :  4EE2602D1B8D001RS : scratchPad : AA1000317E100001042B00000000007F00000000007F00000000007F00000000007F
jeudi 31/01/2013 22:01:46:004  :  Read Raw : [170][16][0][50][126][16][0][1][4][42][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:46:004  :  Destination Address : 0010
jeudi 31/01/2013 22:01:46:004  :  Source Address : 7E32
jeudi 31/01/2013 22:01:46:004  :  Protocol : 10
jeudi 31/01/2013 22:01:46:004  :  Command : 0100
jeudi 31/01/2013 22:01:46:004  :  Header CRC Ok
jeudi 31/01/2013 22:01:46:004  :  4EE2602D1B8D001RS : scratchPad : AA1000327E100001042A00000000007F00000000007F00000000007F00000000007F
jeudi 31/01/2013 22:01:46:079  :  Read Raw : [170][16][0][51][126][16][0][1][4][41][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:46:153  :  Read Raw : [0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:46:153  :  Destination Address : 0010
jeudi 31/01/2013 22:01:46:153  :  Source Address : 7E33
jeudi 31/01/2013 22:01:46:153  :  Protocol : 10
jeudi 31/01/2013 22:01:46:153  :  Command : 0100
jeudi 31/01/2013 22:01:46:153  :  Header CRC Ok
jeudi 31/01/2013 22:01:46:153  :  4EE2602D1B8D001RS : scratchPad : AA1000337E100001042900000000007F00000000007F00000000007F00000000007F
jeudi 31/01/2013 22:01:46:228  :  Read Raw : [170][16][0][52][126][16][0][1][4][40][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:46:228  :  Destination Address : 0010
jeudi 31/01/2013 22:01:46:228  :  Source Address : 7E34
jeudi 31/01/2013 22:01:46:228  :  Protocol : 10
jeudi 31/01/2013 22:01:46:228  :  Command : 0100
jeudi 31/01/2013 22:01:46:228  :  Header CRC Ok
jeudi 31/01/2013 22:01:46:228  :  4EE2602D1B8D001RS : scratchPad : AA1000347E100001042800000000007F00000000007F00000000007F00000000007F
jeudi 31/01/2013 22:01:46:482  :  Read Raw : [170][16][0][53][126][16]
jeudi 31/01/2013 22:01:46:556  :  Read Raw : [0][1][4][39][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127][0][0][0][0][0][127]
jeudi 31/01/2013 22:01:46:556  :  Destination Address : 0010
jeudi 31/01/2013 22:01:46:556  :  Source Address : 7E35
jeudi 31/01/2013 22:01:46:556  :  Protocol : 10 *
 **/


QByteArray resol::getFrame(QByteArray &frame, int index)
{
	int L = frame.length();
	if (L < (((index + 1) * 6) + 10)) return "";
	return frame.mid(10 + (index * 6), 6);
}




int resol::getByte(QByteArray &frame, int position)
{
	int result = 0;
	// check frame CRC

	int MSB = frame.at(4);
	int MSB1 = (MSB & 0x01) * 128;
	int MSB2 = (MSB & 0x02) * 64;
	int MSB3 = (MSB & 0x04) * 32;
	int MSB4 = (MSB & 0x08) * 16;
	switch (position)
	{
		case 0 : result = frame.at(0) + MSB1; break;
		case 1 : result = frame.at(1) + MSB2; break;
		case 2 : result = frame.at(2) + MSB3; break;
		case 3 : result = frame.at(3) + MSB4; break;
	}
//	GenMsg(QString("Get Byte position %1 = %2").arg(position).arg(result));
	return result;
}






void resol::writeTCP(QString Req)
{
/*	if (tcp.isOpen())
	{
		for (int n = 0 ; n < Req.length(); n ++) writeTcp(Req[n].unicode());
		settraffic(Waitingforanswer);
	}
	else
	{
		settraffic(Disabled);
	}*/
	switch (tcp.state())
	{
		case QTcpSocket::UnconnectedState : settraffic(Disabled); connecttcp(); break;
		case QTcpSocket::HostLookupState : break;
		case QTcpSocket::ConnectingState : break;
        case QTcpSocket::ConnectedState : tcp.write(Req.toLatin1());
			settraffic(Waitingforanswer);
			if (!tcp.waitForBytesWritten(1000)) GenMsg("Resol waitForBytesWritten failed" + Req);
			return;
		case QTcpSocket::BoundState : break;
		case QTcpSocket::ClosingState : break;
		default : break;
	}
	GenMsg(tr("Write : ") + Req);
}



void resol::timeout()
{
	TimeOut.stop();
	GenError(33, name);
	reconnecttcp();
}




onewiredevice *resol::addDevice(QString name, bool show)
{
	onewiredevice * device;
	for (int id=1; id<99; id++)
	{
		QString RomID = (ip2Hex(moduleipaddress) + port2Hex(port) + QString("%1").arg(id, 3, 10, QChar('0')) + familyResol);
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



