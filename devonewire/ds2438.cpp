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
#include "configwindow.h"
#include "net1wire.h"
#include "ds2438.h"



ds2438::ds2438(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
	ui.setupUi(this);
	ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 1);
	QDateTime Now = QDateTime::currentDateTime();
	lastsavevalue = logisdom::NA;
	romid = RomID;
	family = romid.right(4);
	ui.labelromid->setText(romid);
	if (master) ui.labelmaster->setText(master->getipaddress());
	if (family == family2438_T) ui.labelfamily->setText(tr("2438 12bit Temp"));
	if (family == family2438_V) ui.labelfamily->setText(tr("2438 voltage ADC"));
	if (family == family2438_A) ui.labelfamily->setText(tr("2438 current ADC"));
    if (family == family2438_I) ui.labelfamily->setText(tr("2438 ICA"));
    MainValue = logisdom::NA;
	if (family == family2438_T)	Unit.setText("°C");
    if (family == family2438_I)
    {
        LastCounter = logisdom::NA;
        Delta = logisdom::NA;
        counterMode.addItem(tr("Absolute"));
        counterMode.addItem(tr("Relative"));
        counterMode.addItem(tr("Offset"));
        countInit.setName(tr("Offset update"));
        setupLayout.addWidget(&counterMode, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        Coef.setValue(1);
        Coef.setDecimals(10);
        Coef.setPrefix(tr("Coef : "));
        setupLayout.addWidget(&Coef, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        Offset.setRange(-2147483640, +2147483640);
        Offset.setPrefix(tr("Offset : "));
        setupLayout.addWidget(&Offset, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        setupLayout.addWidget(&countInit, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        SaveOnUpdate.setText(tr("Save only on update"));
        setupLayout.addWidget(&SaveOnUpdate,  layoutIndex++, 0, 1, logisdom::PaletteWidth);
        connect(&SaveOnUpdate, SIGNAL(stateChanged(int)), this, SLOT(saveOnUpdateChanged(int)));
        connect(&counterMode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
    }
    else
    {
        setupLayout.addWidget(&AXplusB, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        configBits.setLayout(&configBitsLayout);
        IADButton.setText("IAD");
        IADButton.setToolTip("Current A/D Control Bit. 1 = the current A/D and the ICA are enabled, and current \n\
    measurements will be taken at the rate of 36.41 Hz; 0 = the current A/D and the ICA have been \n\
    disabled. The default value of this bit is a ?1? (current A/D and ICA are enabled).");
        CAButton.setText("CA");
        CAButton.setToolTip("Current Accumulator Configuration. 1 = CCA/DCA is enabled, and data will be stored and can\n\
    be retrieved from page 7, bytes 4-7; 0 = CCA/DCA is disabled, and page 7 can be used for general\n\
    EEPROM storage. The default value of this bit is a 1 (current CCA/DCA are enabled).");
        EEButton.setText("EE");
        EEButton.setToolTip("Current Accumulator Shadow Selector bit. 1 = CCA/DCA counter data will be shadowed to\n\
    EEPROM each time the respective register is incremented; 0= CCA/DCA counter data will not be\n\
    shadowed to EEPROM. The CCA/DCA could be lost as the battery pack becomes discharged. If the CA\n\
    bit in the status/configuration register is set to 0, the EE bit will have no effect on the DS2438\n\
    functionality. The default value of this bit is a 1 (current CCA/DCA data shadowed to EEPROM).");
        ADButton.setText("AD");
        ADButton.setToolTip("Voltage A/D Input Select Bit. 1 = the battery input (VDD) is selected as the input for the\n\
    DS2438 voltage A/D converter; 0 = the general purpose A/D input (VAD) is selected as the voltage\n\
    A/D input. For either setting, a Convert V command will initialize a voltage A/D conversion. The default\n\
    value of this bit is a 1 (VDD is the input to the A/D converter).");
        sendCongif.setText(tr("Write Config Register"));
        configBitsLayout.addWidget(&IADButton, 0, 0, 1, 1);
        configBitsLayout.addWidget(&CAButton, 0, 1, 1, 1);
        configBitsLayout.addWidget(&EEButton, 0, 2, 1, 1);
        configBitsLayout.addWidget(&ADButton, 0, 3, 1, 1);
        if (!parent->isRemoteMode()) configBitsLayout.addWidget(&sendCongif, 1, 0, 1, 1);
        setupLayout.addWidget(&configBits, layoutIndex++, 0, 1, logisdom::PaletteWidth);
        connect(&sendCongif, SIGNAL(clicked()), this, SLOT(writeConfig()));
    }
}





ds2438::~ds2438()
{
}




void ds2438::contextMenuEvent(QContextMenuEvent * event)
{
	QMenu contextualmenu;
	QAction Lecture(tr("&Read"), this);
	QAction Nom(tr("&Name"), this);
	QAction SetTAI8540D(tr("&Set for TAI8540D"), this);
	contextualmenu.addAction(&Lecture);
	if (family == family2438_V)
	{
		contextualmenu.addAction(&SetTAI8540D);
	}
	if (!parent->isRemoteMode())
	{
		contextualmenu.addAction(&Nom);
	}
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Lecture) lecture();
	if (selection == &Nom) changename();
	if (selection == &SetTAI8540D) setTAI8540D();
}





void ds2438::setTAI8540D()
{
	AXplusB.A.setText("32.57329");
	AXplusB.B.setText("-31.205");
	AXplusB.axbEnabled.setCheckState(Qt::Checked);
	Unit.setText(" %");
	emitDeviceValueChanged();
}




QString ds2438::MainValueToStrLocal(const QString &str)
{
	QString S = str;
	ui.valueui->setText(S);
	return S;
}




void ds2438::SetOrder(const QString &)
{
}





void ds2438::writeConfig()
{
	if (parent->isRemoteMode()) return;
	int cfg = 0;
	QString scratchpad; 
	if (!master) return;
	if (IADButton.isChecked()) cfg += 1;
	if (CAButton.isChecked()) cfg += 2;
	if (EEButton.isChecked()) cfg += 4;
	if (ADButton.isChecked()) cfg += 8;
	scratchpad = QString("%1").arg(cfg, 2, 16, QChar('0')).toUpper();
	switch (master->gettype())
	{
		case Ha7Net : 
		case MultiGest :
		case TCP_HA7SType :
				master->addtofifo(WriteScratchpad00, romid, scratchpad);
				master->addtofifo(CopyScratchpad00, romid);
		break;
		case RemoteType : break;
	}
}





void ds2438::lecture()
{
	QString romID_T = romid.left(16) + "_T";
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ConvertTempRomID, romID_T);
			master->addtofifo(ConvertV, romID_T);
			master->addtofifo(RecallMemPage00h, romID_T);
            master->addtofifo(ReadPage00h, romID_T);
            master->addtofifo(RecallMemPage01h, romID_T);
            master->addtofifo(ReadPage01h, romID_T);
            break;
#else
		case Ha7Net :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid));
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1));
            break;
#endif
		case TCP_HA7SType :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid));
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1));
            break;
		case MultiGest :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid));
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1));
            break;
		case RemoteType : master->getMainValue();
		break;
	}
}






void ds2438::lecturerec()
{
	QString romID_T = romid.left(16) + "_T";
	setValid(ReadRequest);
	if (!master) return;
	switch (master->gettype())
	{
#ifdef HA7Net_No_Thread
		case Ha7Net :
			master->addtofifo(ConvertTempRomID, romID_T);
			master->addtofifo(ConvertV, romID_T);
			master->addtofifo(RecallMemPage00h, romID_T);
            master->addtofifo(ReadPageRec00h, romID_T);
            master->addtofifo(RecallMemPage01h, romID_T);
            master->addtofifo(ReadPageRec01h, romID_T);
            break;
#else
		case Ha7Net :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid), true);
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1), true);
            break;
#endif
		case TCP_HA7SType :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid), true);
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1), true);
            break;
		case MultiGest :
            if ((family == family2438_A) or (family == family2438_T) or (family == family2438_V)) setscratchpad(master->getScratchPad(romid), true);
            if (family == family2438_I) setscratchpad(master->getScratchPad(romid, 1), true);
            break;
		case RemoteType : master->saveMainValue();
		break;
	}
}





bool ds2438::setscratchpad(const QString &myscratchpad, bool enregistremode)
{
    QString scratchpad = myscratchpad;
    int page = 0;
    QString str = logisdom::getvalue("page01h", scratchpad);
    if (str.isEmpty())
    {
        ScratchPad_Show.setText(scratchpad + (" page 0"));
    }
    else
    {
        page = 0x01;
        scratchpad = str;
        if (family == family2438_I) ScratchPad_Show.setText(scratchpad + (" page 1"));
    }
    bool ok;
    logMsg(romid + " : " + scratchpad);
	QString errorMsg = scratchpad + "   RomID = " + romid + "  " + name;
	if (scratchpad.isEmpty())
	{
		logMsg(romid + "  scratchpad empty !!!");
		return IncWarning();
	}
	if (scratchpad == "FFFFFFFFFFFFFFFFFF")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(54, errorMsg);
		return IncWarning();		// No answer from device not connected
	}
	if (scratchpad == "000000000000000000")
	{
		if (master && WarnEnabled.isChecked()) master->GenError(56, errorMsg);
		return IncWarning();		// 1 Wire bus shorted
	}
    if (!checkCRC8(scratchpad)) return IncWarning();
	double T = logisdom::NA;
    if (page == 0x00)
    {
        //logMsg("Page 0");
        if (family == family2438_T)
        {
            QString TStr = scratchpad.mid(4,2) + scratchpad.mid(2,2);
            T = calcultemperatureDS2438(TStr);
            if (int(T) == logisdom::NA)
            {
                if (master && WarnEnabled.isChecked()) master->GenError(42, errorMsg + "  T = " + QString("%1").arg(T, 0, 'f', 3));
                return IncWarning();
            }
            if (check85(T))
            {
                if (master && WarnEnabled.isChecked()) master->GenError(36, errorMsg);
                return IncWarning();			// convertion error
            }
            if ((T < -55) or (T > 125))
            {
                if (master && WarnEnabled.isChecked()) master->GenError(44, errorMsg + "  T = " + QString("%1").arg(T, 0, 'f', 3));
                return IncWarning();		// launch reset in case of problem
            }
            QString RomID_V = romid.left(17) + "V";
            QString RomID_A = romid.left(17) + "A";
            onewiredevice *device_V;
            onewiredevice *device_A;
            device_V = parent->configwin->DeviceExist(RomID_V);
            device_A = parent->configwin->DeviceExist(RomID_A);
            if (master)
            switch (master->gettype())
            {
    #ifdef HA7Net_No_Thread
            case Ha7Net :
                if (device_V) device_V->setscratchpad(scratchpad, enregistremode);
                if (device_A) device_A->setscratchpad(scratchpad, enregistremode);
                break;
    #endif
            }
        }
        if (family == family2438_V)
        {
            QString ADCValue;
            ADCValue = scratchpad.mid(8,2) + scratchpad.mid(6,2);
            T = double(ADCValue.toInt(&ok, 16)) / 100.0;
            if (!ok) return IncWarning();
        }
        if (family == family2438_A)
        {
            QString CURRENTValue;
            CURRENTValue = scratchpad.mid(12,2) + scratchpad.mid(10,2);
            T = double(CURRENTValue.toInt(&ok, 16));
            if (!ok) return IncWarning();
        }
        double V = AXplusB.result(T);
        setMainValue(V, enregistremode);
        QString CONFIG = scratchpad.mid(0,2);
        int cfg = CONFIG.toInt(&ok, 16);
        if (ok)
        {
            IADButton.setChecked(cfg & 0x01);
            CAButton.setChecked(cfg & 0x02);
            EEButton.setChecked(cfg & 0x04);
            ADButton.setChecked(cfg & 0x08);
        }
    }
    if (page == 0x01)
    {
        //logMsg("Page 1");
        if (family == family2438_T)
        {
                QString RomID_I = romid.left(17) + "I";
                onewiredevice *device_I;
                device_I = parent->configwin->DeviceExist(RomID_I);
                if (master)
                switch (master->gettype())
                {
            #ifdef HA7Net_No_Thread
                    case Ha7Net :
                        if (device_I) device_I->setscratchpad(myscratchpad, enregistremode);
                        break;
            #endif
                }
                ClearWarning();
        }
        if (family == family2438_I)
        {
            logMsg(romid + " ICA Yoooo " + scratchpad);
            QString ICA;
            ICA = scratchpad.mid(8,2);
            T = double(ICA.toInt(&ok, 16));
            if (!ok) return IncWarning();
            setResult(T, enregistremode);
            setMainValue(MainValue, false);
            //ui.MainText->setText(QString("%1 pulses, Delta = %2").arg(counter).arg(Delta));
            ui.valueui->setText(MainValueToStr());
            bool offsetUpdate = countInit.isitnow();
            if (enregistremode)
            {
                if ((SaveOnUpdate.isChecked()) && (counterMode.currentIndex() == offsetMode))
                {
                    if (offsetUpdate) savevalue(QDateTime::currentDateTime(), MainValue, false);
                }
                else savevalue(QDateTime::currentDateTime(), MainValue);
            }
            if ((counterMode.currentIndex() == offsetMode) && (offsetUpdate)) Offset.setValue(int(T));
        }
    }
	return true;
}



double ds2438::calcultemperatureDS2438(const QString  &THex)
{
	bool ok;
	bool s = false;
    quint16 T = quint16(THex.toInt(&ok, 16));
	if (T & 0x8000)
	{
		T = 0xFFFF - T + 1;
		s = true;
	}
    if (s) return double(-T) / 256.0;
        else return double(T) / 256.0;
}



void ds2438::GetConfigStr(QString &str)
{
	AXplusB.GetConfigStr(str);
    str += logisdom::saveformat("Coef", QString("%1").arg(Coef.value(), 0, 'f', 10));
    str += logisdom::saveformat("counterMode", QString("%1").arg(counterMode.currentIndex()));
    str += logisdom::saveformat("counterOffset", QString("%1").arg(Offset.value()));
    str += logisdom::saveformat("NextCountInit", countInit.getNext());
    str += logisdom::saveformat("CountInitMode", countInit.getMode());
    str += logisdom::saveformat("CountInitEnabled", countInit.getSaveMode());
    str += logisdom::saveformat("SaveOnUpdate", QString("%1").arg(SaveOnUpdate.isChecked()));
}




void ds2438::setconfig(const QString &strsearch)
{
	QString Name = logisdom::getvalue("Name", strsearch);
	if (!Name.isEmpty()) setname(assignname(Name));
		else if (family == family2438_T) setname(assignname(tr("Detector T° ")));
		else if (family == family2438_V) setname(assignname(tr("Volt ADC ")));
        else if (family == family2438_A) setname(assignname(tr("Volt ADC ")));
        else setname(assignname(tr("ICA ")));
	AXplusB.setconfig(strsearch);
    bool ok = false;
    QString coefstr = logisdom::getvalue("Coef", strsearch);
    double coef = 1;
    if (!coefstr.isEmpty()) coef = coefstr.toDouble(&ok);
    if (ok) Coef.setValue(coef);
    Offset.setEnabled(false);
    countInit.setEnabled(false);
    SaveOnUpdate.setEnabled(false);
    int mode = 0;
    mode = logisdom::getvalue("counterMode", strsearch).toInt(&ok);
    if (ok) counterMode.setCurrentIndex(mode);
        else counterMode.setCurrentIndex(absoluteMode);
    int offset = 0;
    offset = logisdom::getvalue("counterOffset", strsearch).toInt(&ok);
    if (ok) Offset.setValue(offset);
    QString next = logisdom::getvalue("NextCountInit", strsearch);
    if (next.isEmpty()) countInit.setNext(QDateTime::currentDateTime());
        else countInit.setNext(QDateTime::fromString (next, Qt::ISODate));
    int savemode = logisdom::getvalue("CountInitMode", strsearch).toInt(&ok);
    if (ok) countInit.setMode(savemode);
    int saveen = logisdom::getvalue("CountInitEnabled", strsearch).toInt(&ok);
    if (!ok) countInit.setSaveMode(true);
        else if (saveen) countInit.setSaveMode(true);
            else  countInit.setSaveMode(false);
    int saveupdate = logisdom::getvalue("SaveOnUpdate", strsearch).toInt(&ok);
    if (ok)
    {
        if (saveupdate) SaveOnUpdate.setCheckState(Qt::Checked);
        else SaveOnUpdate.setCheckState(Qt::Unchecked);
    }
}





void ds2438::saveOnUpdateChanged(int state)
{
    switch (state)
    {
        case Qt::Unchecked :
            saveInterval.setEnabled(true);
            break;
        case Qt::PartiallyChecked :
        case Qt::Checked :
            saveInterval.SaveEnable.setCheckState(Qt::Unchecked);
            saveInterval.setEnabled(false);
            break;
    }
}




void ds2438::modeChanged(int)
{
    QDateTime now = QDateTime::currentDateTime();
    switch (counterMode.currentIndex())
    {
        case absoluteMode :
            Offset.setEnabled(false);
            countInit.setEnabled(false);
            SaveOnUpdate.setEnabled(false);
            break;
        case relativeMode :
            Offset.setEnabled(false);
            countInit.setEnabled(false);
            SaveOnUpdate.setEnabled(false);
            break;
        case offsetMode  :
            Offset.setEnabled(true);
            countInit.setEnabled(true);
            countInit.setNext(now);
            SaveOnUpdate.setEnabled(true);
            break;
    }
}




void ds2438::setResult(long int NewValue, bool enregistremode)
{
    switch (counterMode.currentIndex())
    {
        case absoluteMode :
            if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
            if (enregistremode) LastCounter = NewValue;
            MainValue = NewValue * Coef.value();
        break;
        case relativeMode :
            if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
            if (enregistremode)
            {
                LastCounter = NewValue;
                MainValue = Delta * Coef.value();
            }
        break;
        case offsetMode :
            if ((NewValue < LastCounter) or (LastCounter == logisdom::NA)) Delta = 0; else Delta = NewValue - LastCounter;
            if (enregistremode) LastCounter = NewValue;
            MainValue = (NewValue - Offset.value()) * Coef.value();

        break;
    }
}

