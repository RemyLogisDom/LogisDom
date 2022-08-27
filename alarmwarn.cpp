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





//#include <QSound>
#include <QStringList>
#include <QStringListModel>
#include "errlog.h"
#include "globalvar.h"
#include "logisdom.h"
#include "alarmwarn.h"
#include <QTimer>


alarmwarn::alarmwarn(logisdom *Parent)
{
	ui.setupUi(this);
	parent = Parent;
	ismuet = false;
	newAlarm = false;
	soundtimer = new QTimer(this);
//	alarmwav = new QSound("wav/newemail.wav", this);
	connect(soundtimer, SIGNAL(timeout()), this, SLOT(playsound()));
	connect(ui.boutonsilence, SIGNAL(clicked()), this, SLOT(silence()));
	ui.errortab->removeTab(0);
	GeneralErrorLog = newTab(tr("General"));
}





alarmwarn::~alarmwarn()
{
	delete soundtimer;
//	delete alarmwav;
}





errlog *alarmwarn::newTab(const QString &name)
{
    errlog *ErrorLog = nullptr;
        ErrorLog = new errlog(parent, name);
	ui.errortab->addTab(ErrorLog, name);
	logTabList.append(ErrorLog);
	connect(ErrorLog, SIGNAL(alarmChange(errlog*)), this, SLOT(alarmChanged(errlog*)));
	return ErrorLog;
}





void alarmwarn::setTabName(errlog *ErrorLog, const QString &name)
{
	int index = ui.errortab->indexOf(ErrorLog);
	if (index != -1) ui.errortab->setTabText(index, name);
}




void alarmwarn::alarmChanged(errlog* errorTab)
{
	bool generalFlag = false;
	int tab = logTabList.indexOf(errorTab);
//	qDebug() <<  QString("tab : %1").arg(tab);
	if (tab != -1)
	{
		if (errorTab->isFlagged())
		{
			ui.errortab->setTabIcon(tab, QIcon(QPixmap(QString::fromUtf8(":/images/images/warning.png"))));
			newAlarm = true;
		}
		else  ui.errortab->setTabIcon(tab, QIcon());
		if (errorTab->beepOnErrors()) playsound(errorTab->soundPath);
	}
	for (int n=0; n<logTabList.count(); n++)
		if (logTabList[n]->isFlagged()) generalFlag = true;
	if (generalFlag) emit(alarmOn(true));
	else emit(alarmOn(false));
}





void alarmwarn::playsound(QString)
{
	this->show();
	this->raise();
/*	if (alarmwav->isFinished() and !ismuet)
	{
		if (soundPath.isEmpty()) alarmwav->play();
		else alarmwav->play(soundPath);
    }*/
}





void alarmwarn::addtoalarm(const QString &text)
{
	parent->logfile(text);
}






void alarmwarn::config()
{
	parent->showconfig();
}







void alarmwarn::getCfgStr(QString&)
{
//	if (ui.checkBoxLogtoFile->isChecked()) str += logisdom::saveformat("Log", "1"); else str += logisdom::saveformat("Log", "0");
}





void alarmwarn::setCfgStr(QString)
{
//	bool logstate = false, logact = false, logwarn = false, logerrors = false; beep = false; silence = false;
}






void alarmwarn::silence()
{
	soundtimer->stop();
	if (ui.boutonsilence->isChecked()) ismuet = true;
	else ismuet = false;
}




