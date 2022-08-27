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
#include "logisdom.h"
#include "onewire.h"
#include "interval.h"



interval::interval()
{
// palette setup icon
	enabled = true;
	setLayout(&setupLayout);
	setupLayout.addWidget(&SaveEnable);
    nextOne.setDateTime(QDateTime::currentDateTime().addDays(1)); // avoid saving before nextOne is setup by setConfig
	setupLayout.setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	connect(&SaveEnable, SIGNAL(stateChanged(int)), this, SLOT(changeEnable(int)));
	SaveEnable.setCheckState(Qt::Unchecked);
	changeEnable(Qt::Unchecked);
	setupLayout.setAlignment(Qt::AlignLeft | Qt::AlignTop);
    for (int n=0; n<lastInterval; n++) Type.addItem(getStrMode(n));
	setupLayout.addWidget(&Type);
	nextOne.setDisplayFormat("dd-MMM-yyyy  hh:mm");
	nextOne.setDateTime(QDateTime::currentDateTime());
	nextOne.setCalendarPopup(true);
	setupLayout.addWidget(&nextOne);
	SaveEnable.setCheckState(Qt::Checked);
    connect(&Type, SIGNAL(currentIndexChanged(int)), this, SLOT(stateChanged()));
    connect(&nextOne, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(stateChanged()));
}





interval::~interval()
{
}




void interval::setName(QString name)
{
	SaveEnable.setText(name);	
}



void interval::setEnabled(bool state)
{
	enabled = state;
	SaveEnable.setEnabled(state);
	Type.setEnabled(state);
	nextOne.setEnabled(state);
}



void interval::changeEnable(int state)
{
	switch (state)
	{
		case Qt::Unchecked : 
			Type.setEnabled(false);
			nextOne.setEnabled(false);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			Type.setEnabled(true);
			nextOne.setEnabled(true);
			break;
	}
    enabled = state;
    //qDebug() << "emit(readChanged()";
    emit(readChanged());
}


void interval::stateChanged()
{
    //qDebug() << "emit(readChanged()";
    emit(readChanged());
}


qint64 interval::getSecs(const QDateTime &T)
{
	int index = Type.currentIndex();
    qint64 secs = 0;
	switch (index)
	{
		case M1 : secs = 60; break;
		case M2 : secs = 120; break;
		case M5 : secs = 300; break;
		case M10 : secs = 600; break;
		case M15 : secs = 900; break;
		case M20 : secs = 1200; break;
		case M30 : secs = 1800; break;
		case H1 : secs = 3600; break;
		case H2 : secs = 7200; break;
		case H3 : secs = 10800; break;
		case H4 : secs = 14400; break;
		case H6 : secs = 21600; break;
		case H12 : secs = 43200; break;
		case D1 : secs = 86400; break;
		case D2 : secs = 172800; break;
		case D5 : secs = 432000; break;
		case D10 : secs = 864000; break;
		case W1 : secs = 604800; break;
		case W2 : secs = 1209600; break;
        //case MT : secs = T.date().daysInMonth() * SecsInDays; break;
        case MT : secs = T.secsTo(T.addMonths(1)); break;
        case AUTO : if (yearEnable) { secs = T.secsTo(T.addYears(1)); } else { secs = 0; }
            break;
    }
	return secs;
}



void interval::enableYear()
{
    if (yearEnable) return;
    yearEnable = true;
    Type.setItemText(AUTO, tr("One Year"));
}



qint64 interval::getSecs()
{
    QDateTime now = QDateTime::currentDateTime();
    return getSecs(now);
}



bool interval::isAutoSave()
{
	return (Type.currentIndex() == AUTO);
}



void interval::setToAuto()
{
    Type.setCurrentIndex(AUTO);
    Type.setEnabled(false);
    nextOne.hide();
}



bool interval::checkOnly()
{
    if (!enabled) return false;
    if (!SaveEnable.isChecked()) return false;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime next = nextOne.dateTime();
    qint64 n = getSecs(next);
    if (n == 0) return false;
    qint64 gap = next.secsTo(now);
    if (gap >= 0) return true;
    return false;
}


bool interval::isitnow()
{
    if (!enabled) return false;
	if (!SaveEnable.isChecked()) return false;
    QDateTime now = QDateTime::currentDateTime();
	QDateTime next = nextOne.dateTime();
    qint64 n = getSecs(next);
	if (n == 0) return false;
    qint64 gap = next.secsTo(now);
    if (gap >= 0)
	{
        do
		{ next = next.addSecs(n); } 
			while (next.secsTo(now) >= 0);
		int s = next.time().second();		// remove seconds
		if (s != 0) nextOne.setDateTime(next.addSecs(-s));
			else nextOne.setDateTime(next);
		return true;
	}
    return false;
}





QString interval::getNext()
{
	return nextOne.dateTime().toString(Qt::ISODate);
}




void interval::setNext(const QDateTime &T)
{
	nextOne.setDateTime(T);
}



void interval::setMode(int index)
{
	if (index < lastInterval) Type.setCurrentIndex(index);
}




QString interval::getMode()
{
	return QString("%1").arg(Type.currentIndex());
}




void interval::setSaveMode(bool state)
{
	if (state) SaveEnable.setCheckState(Qt::Checked);
	else SaveEnable.setCheckState(Qt::Unchecked);
}




QString interval::getSaveMode()
{
	return QString("%1").arg(SaveEnable.isChecked());
}




QString interval::getStrMode(int mode)
{
	QString str;
	switch (mode)
	{
        case M1 : str = tr("1 Minute"); break;
		case M2 : str = tr("2 Minutes"); break;
		case M5 : str = tr("5 Minutes"); break;
		case M10 : str = tr("10 Minutes"); break;
		case M15 : str = tr("15 Minutes"); break;
		case M20 : str = tr("20 Minutes"); break;
		case M30 : str = tr("30 Minutes"); break;
		case H1 : str = tr("1 Hour"); break;
		case H2 : str = tr("2 Hours"); break;
		case H3 : str = tr("3 Hours"); break;
		case H4 : str = tr("4 Hours"); break;
		case H6 : str = tr("6 Hours"); break;
		case H12 : str = tr("12 Hours"); break;
		case D1 : str = tr("1 Day"); break;
		case D2 : str = tr("2 Days"); break;
		case D5 : str = tr("5 Days"); break;
		case D10 : str = tr("10 Days"); break;
		case W1 : str = tr("1 Weeks"); break;
		case W2 : str = tr("2 Weeks"); break;
		case MT : str = tr("1 Month"); break;
		case AUTO : str = tr("Auto"); break;
    default : str = "";
	}
	return str;
}





