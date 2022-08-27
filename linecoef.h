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



#ifndef LINECOEF_H
#define LINECOEF_H
#include <QtWidgets/QLineEdit>
#include <QtCore>
#include <QtGui>
#include "calc.h"


class logisdom;


class lineCoef : public QLineEdit
{
	Q_OBJECT
public:
    lineCoef(QWidget *parent = 0);
    ~lineCoef();
    double result(double value);
    double value();
    calc formul;
private slots:
    void checkCoef();
};

#endif


