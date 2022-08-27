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




#include <QtWidgets/QFileDialog>
#include "globalvar.h"
#include "alarmwarn.h"
#include "messagebox.h"
#include "errlog.h"



errlog::errlog(logisdom *Parent, QString name)
{
    parent = Parent;
    ui.setupUi(this);
    AlarmFlag = false;
    AlarmCritical = false;
    filename = name;
    connect(ui.pushButtonClear, SIGNAL(clicked()), this, SLOT(effacer()));
    connect(ui.pushButtonChoose, SIGNAL(clicked()), this, SLOT(chooseSound()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateMsg()), Qt::QueuedConnection);
    updateTimer.setSingleShot(true);
}





errlog::~errlog()
{
}




void errlog::setLogFileName(const QString &Name)
{
	filename = Name;
}




bool errlog::isFlagged()
{
	return AlarmFlag;
}




bool errlog::isCritical()
{
	return AlarmCritical;
}





bool errlog::beepOnErrors()
{
	return ui.checkBoxBeep->isChecked();
}





void errlog::chooseSound()
{
	QString txt = QFileDialog::getOpenFileName(this, tr("Select wav files "), QString(repertoirewav), "wav (*.wav)");
	if (!txt.isEmpty())
	{
		soundPath = txt;
		QFileInfo fileInfo(soundPath);
		ui.pushButtonChoose->setStatusTip(tr("Choose sound") + "  ("+ fileInfo.fileName() + ")");
	}
	else
	{
                if (messageBox::questionHide(this, tr("Default sound"), tr("Do you want to use default sound ?"), parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::Yes)
		soundPath = "";;
		ui.pushButtonChoose->setStatusTip(tr("Choose sound"));
	}
}




void errlog::getCfgStr(QString &str)
{
	if (ui.checkBoxLogtoFile->isChecked()) str += logisdom::saveformat("Log", "1"); else str += logisdom::saveformat("Log", "0");
	if (ui.checkBoxActivity->isChecked()) str += logisdom::saveformat("LogActivity", "1"); else str += logisdom::saveformat("LogActivity", "0");
	if (ui.checkBoxWarning->isChecked()) str += logisdom::saveformat("LogWarning", "1"); else str += logisdom::saveformat("LogWarning", "0");
	if (ui.checkBoxError->isChecked()) str += logisdom::saveformat("LogErrors", "1"); else str += logisdom::saveformat("LogErrors", "0");
	if (ui.checkBoxBeep->isChecked()) str += logisdom::saveformat("BeepOnErrors", "1"); else str += logisdom::saveformat("BeepOnErrors", "0");
	str += logisdom::saveformat("soundFile", soundPath);
}




void errlog::setCfgStr(const QString &strsearch)
{
	bool logstate = false, logact = false, logwarn = false, logerrors = false, beep = false;
	if (logisdom::getvalue("Log", strsearch) == "1") logstate = true;
	if (logisdom::getvalue("LogActivity", strsearch) == "1") logact = true;
	if (logisdom::getvalue("LogWarning", strsearch) == "1") logwarn = true;
	if (logisdom::getvalue("LogErrors", strsearch) == "1") logerrors = true;
	if (logisdom::getvalue("BeepOnErrors", strsearch) == "1") beep = true;
	ui.checkBoxLogtoFile->setChecked(logstate);
	ui.checkBoxActivity->setChecked(logact);
	ui.checkBoxWarning->setChecked(logwarn);
	ui.checkBoxError->setChecked(logerrors);
	ui.checkBoxBeep->setChecked(beep);
	soundPath = logisdom::getvalue("soundFile", strsearch);
	QFileInfo fileInfo(soundPath);
	QString txt = tr("Choose sound");
	if (!soundPath.isEmpty()) txt += "  ("+ fileInfo.fileName() + ")";
	ui.pushButtonChoose->setStatusTip(txt);
}




QString errlog::SetError(int ErrID, const QString &Msg)
{
	QMutexLocker locker(&logMutex);
	QDateTime now = QDateTime::currentDateTime();
	QString ErrorMessage = "";
	int ErrorLevel = ErrorWarn;
	switch (ErrID)
	{
        case 25 : ErrorMessage = tr("Scratchpad AA return error"); ErrorLevel = ErrorWarn; break;
        case 26 : ErrorMessage = tr("Scratchpad too short"); ErrorLevel = ErrorWarn; break;
        case 27 : ErrorMessage = tr("Scratchpad Empty"); ErrorLevel = ErrorWarn; break;
		case 28 : ErrorMessage = tr("Prequest is nullptr"); ErrorLevel = ErrorWarn; break;
		case 29 : ErrorMessage = tr("HA7S setscratchpad error"); ErrorLevel = ErrorWarn;	break;
		case 31 : ErrorMessage = tr("toDouble conversion error"); ErrorLevel = ErrorWarn; break;
		case 32 : ErrorMessage = tr("Rotex Parameter out of range"); ErrorLevel = ErrorWarn; break;
		case 33 : ErrorMessage = tr("Connection timeout no more data received"); ErrorLevel = ErrorWarn;	break;
		case 34 : ErrorMessage = tr("SetScratchPad error"); ErrorLevel = ErrorWarn;	break;
		case 35 : ErrorMessage = tr("LogisDom controler restarted after errors"); ErrorLevel = ErrorWarn;	break;
		case 36 : ErrorMessage = tr("Temperature convertion migh not be done or device has lost power after convertion"); ErrorLevel = ErrorWarn;	break;
		case 37 : ErrorMessage = tr("CRC compute error"); ErrorLevel = ErrorLogOnly;	break;
		case 38 : ErrorMessage = tr("Device pointer not assigned Ha7net"); ErrorLevel = ErrorWarn;	break;
		case 39 : ErrorMessage = tr("Html error Ha7net"); ErrorLevel = ErrorWarn;	break;
		case 40 : ErrorMessage = tr("Formula error"); ErrorLevel = ErrorWarn;	break;
		case 41 : ErrorMessage = tr("Virtual not defined"); ErrorLevel = ErrorWarn;	break;
		case 42 : ErrorMessage = tr("Temperature computation error"); ErrorLevel = ErrorWarn;	break;
		case 43 : ErrorMessage = tr("ScratchPad could not be extracted from html parsing"); ErrorLevel = ErrorWarn;	break;
		case 44 : ErrorMessage = tr("Temperature out of normal limits"); ErrorLevel = ErrorLogOnly;	break;
		case 45 : ErrorMessage = tr("Set scratchpad error"); ErrorLevel = ErrorWarn;	break;
		case 46 : ErrorMessage = tr("Name is required"); ErrorLevel = ErrorLogOnly; break;
		case 47 : ErrorMessage = tr("Cannot write to file"); ErrorLevel = ErrorWarn; break;
		case 48 : ErrorMessage = tr("Microcontroler upload fail"); ErrorLevel = ErrorWarn; break;
		case 49 : ErrorMessage = tr("Remote connection"); ErrorLevel = ErrorLogOnly; break;
		case 51 : ErrorMessage = tr("Http connection problem"); ErrorLevel = ErrorWarn;	break;
		case 52 : ErrorMessage = tr("Read error"); ErrorLevel = ErrorWarn;	break;
		case 53 : ErrorMessage = tr("CRC error"); ErrorLevel = ErrorLogOnly;	break;
		case 54 : ErrorMessage = tr("1 Wire Device not connected"); ErrorLevel = ErrorLogOnly;	break;
		case 55 : ErrorMessage = tr("Problem 1 Wire device read"); ErrorLevel = ErrorWarn;	break;
		case 56 : ErrorMessage = tr("1 Wire Bus shorted"); ErrorLevel = ErrorCritical;	break;
		case 57 : ErrorMessage = tr("Problem reading config file, net1wire device"); ErrorLevel = ErrorWarn;	break;
		case 58 : ErrorMessage = tr("Conversion error Zone String to Int"); ErrorLevel = ErrorWarn;	break;
		case 59 : ErrorMessage = tr("Conversion error, Value String to Int"); ErrorLevel = ErrorWarn;	break;
        case 60 : ErrorMessage = tr("Thread return no value"); ErrorLevel = ErrorWarn;	break;
        case 63 : ErrorMessage = tr("Impossible to find request corresponding answer"); ErrorLevel = ErrorLogOnly;	break;
		case 64 : ErrorMessage = tr("Impossible to find Code1 FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 65 : ErrorMessage = tr("Impossible to find a space chararcter at the end of fin de Code1 FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 66 : ErrorMessage = tr("Conversion error Code1 String to Int FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 67 : ErrorMessage = tr("Impossible to find Code2 FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 68 : ErrorMessage = tr("Impossible to find space character at end of Code2 FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 69 : ErrorMessage = tr("Conversion error Code2 String to Int FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 70 : ErrorMessage = tr("Impossible to find NbZone FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 71 : ErrorMessage = tr("Impossible to find space character at end of NbZone FTS800 configuration"); ErrorLevel = ErrorLogOnly;	break;
		case 72 : ErrorMessage = tr("Conversion error NbZone String to Int FTS800 configuration") ; ErrorLevel = ErrorLogOnly;	break;
		case 73 : ErrorMessage = tr("Impossible to find space character end of Zonexx FTS800 configuration") ; ErrorLevel = ErrorLogOnly;	break;
		case 74 : ErrorMessage = tr("Conversion error Zonexx String to Int FTS800 configuration") ; ErrorLevel = ErrorLogOnly;	break;
		case 76 : ErrorMessage = tr("Answer could not be analysed"); ErrorLevel = ErrorWarn;	break;
		case 77 : ErrorMessage = tr("TcpSocket Disconnected"); ErrorLevel = ErrorWarn; break;
		case 79 : ErrorMessage = tr("pointeur CapteurDevice nul pendant le process"); ErrorLevel = ErrorWarn; break;
		case 80 : ErrorMessage = tr("Erreur connection TCP"); ErrorLevel = ErrorWarn; break;
		case 81 : ErrorMessage = tr("Connection TCP canceled"); ErrorLevel = ErrorWarn; break;
		case 83 : ErrorMessage = tr("FIFO overflow command aborted"); ErrorLevel = ErrorWarn; break;
		case 84 : ErrorMessage = tr("Convert request already in the FIFO"); ErrorLevel = ErrorWarn; break;
		case 85 : ErrorMessage = tr("Time Out"); ErrorLevel = ErrorWarn; break;
		case 86 : ErrorMessage = tr("Scheduler could not calculate time"); ErrorLevel = ErrorWarn; break;
		case 87 : ErrorMessage = tr("Lock ID was not compliant"); ErrorLevel = ErrorLogOnly;	break;
        case 88 : ErrorMessage = tr("toInt conversion error"); ErrorLevel = ErrorWarn; break;
        case 89 : ErrorMessage = tr("Conversion fault"); ErrorLevel = ErrorWarn; break;
        case 90 : ErrorMessage = tr("Send eMail error"); ErrorLevel = ErrorWarn; break;
        case 91 : ErrorMessage = tr("Send SMS error"); ErrorLevel = ErrorWarn; break;
        default : ErrorMessage = QString(tr("Unknown error") + " %1").arg(ErrID); ErrorLevel = ErrorWarn; break;
	}
	ErrorMessage = tr("Error") + QString(" %1 : ").arg(ErrID) + ErrorMessage + "  " + Msg;
	ui.errortext->append(now.toString("dddd dd/MM/yyyy HH:mm:ss:zzz : -> ") + ErrorMessage);
	QString currenttext = ui.errortext->toPlainText();
	if (currenttext.length() > 100000) ui.errortext->setText(currenttext.mid(90000));
	ui.errortext->moveCursor(QTextCursor::End);
	switch (ErrorLevel)
	{
                case LogOnly :		parent->logthis(filename, ErrorMessage, ""); break;
                case LogAndShow :	parent->logthis(filename, ErrorMessage, "");
                                        parent->alarmwindow->show();
					raiseError(); break;
                case ErrorLogOnly :	parent->logthis(filename, ErrorMessage, ""); break;
                case ErrorWarn :	parent->alarmwindow->show();
                                        parent->alarmwindow->raise();
					raiseError(); break;
                case ErrorWarnAlarm :	parent->alarmwindow->show();
                                        parent->alarmwindow->raise();
					raiseError(); break;
                case ErrorCritical :	parent->alarmwindow->show();
                                        parent->alarmwindow->raise();
					raiseError(); break;
	}
	return ErrorMessage;
}






void errlog::raiseError()
{
	if (ui.checkBoxRaise->isChecked())
	{
		AlarmFlag = true;
		emit(alarmChange(this));
	}
}







void errlog::AddMsg(const QString &Msg)
{
	QMutexLocker locker(&logMutex);
        QDateTime now = QDateTime::currentDateTime();
        if (!ui.checkBoxActivity->isChecked()) return;
        if (ui.checkBoxLogtoFile->isChecked()) parent->logthis(filename, Msg);
        appendStr.append(now.toString("dddd dd/MM/yyyy HH:mm:ss:zzz  :  ") + Msg + "\n");
        //ui.errortext->append(now.toString("dddd dd/MM/yyyy HH:mm:ss:zzz  :  ") + Msg);
        //QString currenttext = ui.errortext->toPlainText();
        //if (currenttext.length() > 100000) ui.errortext->setText(currenttext.mid(90000));
        //ui.errortext->moveCursor(QTextCursor::End);
        updateTimer.start(200);
}





void errlog::updateMsg()
{
    QMutexLocker locker(&logMutex);
    //ui.errortext->append(appendStr);
    //QString currenttext = ui.errortext->toPlainText();
    if (appendStr.length() > 100000) appendStr = appendStr.mid(90000);
    ui.errortext->setText(appendStr);
    ui.errortext->moveCursor(QTextCursor::End);
}




void errlog::effacer()
{
	ui.errortext->clear();
	AlarmFlag = false;
	emit(alarmChange(this));
}
