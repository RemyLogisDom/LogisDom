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



#ifndef CALC_H
#define CALC_H
#include <QtGui>

class formula;
class formulasimple;
class onewiredevice;


class calc : public QObject
{
	Q_OBJECT
    friend class calcthread;
    friend class devvirtual;
    friend class formula;
    friend class reprocessthread;
public:
    calc();
    double calculsimple(const QString &str);
    QString textBrowserResult;
private:
    QString Calc, webStrResult;
    bool scroolDone;
    virtual double toNumeric(const QString &S, bool *ok);
    bool isDate(const QString &S, QDate &date, bool *ok);
    bool isNumeric(const QString &S);
    virtual onewiredevice *checkDevice(const QString &RomID);
    bool syntax;
    bool dataValid;
    bool deviceLoading;
    QList <double> V;
    QDateTime *TCalc;
    double target;
    void dataError(QString msg);
    bool AutoEnabled;   // used to autocalculate on change when device value used in the formula are changing
    bool LinkedOnly;    // used to only identify device used in the formula
    bool ajouter(QString &C);
    int getNextOp(QString &C, int index);
    int getPreviousOp(QString &C, int index);
    void syntaxError(QString msg);
    bool isOperator(const QString &str);
    virtual bool resoudreParenthese();
    bool diviser(QString &C);
    bool multiplier(QString &C);
    bool enlever(QString &C);
    bool sup(QString &C);
    bool inf(QString &C);
    bool egal(QString &C);
    bool different(QString &C);
    bool pw(QString &C);
};



#endif




