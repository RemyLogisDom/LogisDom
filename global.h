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
#include "errlog.h"
#include "alarmwarn.h"
#include "graphconfig.h"
#include "addProgram.h"
#include "messagebox.h"
#include "configwindow.h"





bool checklocaldir(QString dir)
{
	QDir localdirectory = QDir("");
	if (!localdirectory.exists(dir))
	{
		maison1wirewindow->logfile("Try to create local directory  " + dir);
		if (!localdirectory.mkdir(dir)) 
		{
			maison1wirewindow->logfile("Cannot create local directory " + dir);
			return false;
		}
		maison1wirewindow->logfile("Local directory " + dir + " created");
		return true;
	}
	return true;
}








bool isValidIp(QString &ip)
{
	if (ip == simulator) return true;
	bool valid = true;
	int p1 = ip.indexOf(".");			// get first point position
	if (p1 == -1) return false;
	int p2 = ip.indexOf(".", p1 + 1);	// get second point position
	if (p2 == -1) return false;
	int p3 = ip.indexOf(".", p2 + 1);	// get third point position
	if (p3 == -1) return false;
	int l = ip.length();
	QString N1 = ip.mid(0, p1);
	if (N1 == "") return false;
	QString N2 = ip.mid(p1 + 1, p2 - p1 - 1);
	if (N2 == "") return false;
	QString N3 = ip.mid(p2 + 1, p3 - p2 - 1);
	if (N3 == "") return false;
	QString N4 = ip.mid(p3 + 1, l - p3 - 1);
	if (N4 == "") return false;
	int n1 = N1.toInt();
	int n2 = N2.toInt();
	int n3 = N3.toInt();
	int n4 = N4.toInt();
	if (n1 == 0) return false;
	if ((n1>255) or (n2>255) or (n3>255) or (n4>255)) return false;
	if ((n1 + n2 + n3 + n4) == 0) return false;
	ip = QString("%1.%2.%3.%4").arg(n1, 3, 10, QChar('0')).arg(n2, 3, 10, QChar('0')).arg(n3, 3, 10, QChar('0')).arg(n4, 3, 10, QChar('0'));	
	return valid;
}








QString TempConvertF2C(QString TinFahrenheit)	// convert Fahrenheit to Celsius when TempInFahrenheit is set
{
	QString T = cstr::toStr(cstr::NA);
	bool ok = false;
	double t;
	t = TinFahrenheit.toDouble(&ok);
    if (ok) T = QString("%1").arg(round((t - 32.0) * 5.0 / 9.0));
	return T;
}






QString TempConvertC2F(QString TinCelsius)	// convert Celsius to Fahrenheit when TempInFahrenheit is reset
{
	QString T = cstr::toStr(cstr::NA);
	bool ok = false;
	double t;
	t = TinCelsius.toDouble(&ok);
	if (ok) T = QString("%1°F").arg(round((t * 9.0 / 5.0) + 32.0));
	return T;
}





