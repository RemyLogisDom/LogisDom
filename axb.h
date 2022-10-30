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



#ifndef AXB_H
#define AXB_H
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtCore>
#include <QtGui>

class axb : public QWidget
{
	Q_OBJECT
public:	
	axb();
	~axb();
	QGridLayout setupLayout;
	QCheckBox axbEnabled;
	QLabel AText;
	QLabel BText;
	QLineEdit A;
	QLineEdit B;
	QPushButton RawValue;
	void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
	double result(double value);
    double fromResult(double value);
private slots:
	void checkA();
	void checkB();
	void changeEnable(int state);
private:
signals:
};

#endif
