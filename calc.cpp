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
#include "onewire.h"
#include "configwindow.h"
#include "weathercom.h"
#include "energiesolaire.h"
#include "meteo.h"
#include "addDaily.h"
#include "addProgram.h"
#include "formula.h"
#include "logisdom.h"
#include "formulasimple.h"
#include "calc.h"
#include "mailsender.h"




calc::calc()
{
    qRegisterMetaType<onewiredevice*>();
    deviceLoading = false;
    target = logisdom::NA;
    syntax = false;
    dataValid = false;
    TCalc = nullptr;
}




bool calc::isNumeric(const QString &S)
{
    bool ok;
    S.toDouble(&ok);
    if (ok)
    {
        onewiredevice *device = maison1wirewindow->configwin->DeviceExist(S);
        if (device) ok = false;
    }
    return ok;
}




double calc::toNumeric(const QString &S, bool *ok)
{
    double v = logisdom::NA;
    onewiredevice *device = checkDevice(S);
    *ok = false;
    if (S.left(1) == "V")
    {
        int index = S.right(S.length() - 1).toInt(ok);
        if (*ok) if (index < V.count())
        {
            v = V[index];
            *ok = true;
        }
    }
    else if (device)
    {
        v = device->getMainValue();
        if (int(v) == logisdom::NA) dataValid = false;
        if (!TCalc) if (!device->isValid()) dataValid = false;
        if (!dataValid) *ok = false; else *ok = true;
    }
    if (v == logisdom::NA)
    {
        syntax = false;
        textBrowserResult += "\n" + (tr("Value error")) + " : " + S;
    }
    return v;
}





bool calc::isDate(const QString &S, QDate &date, bool *ok)
{
    *ok = false;
    int year = -1;
    int month = -1;
    if (S.length() != 7) return false;
    // Format must be yyyy_mm or mm_yyyy
    if (S.mid(4,1) == "_")  // yyyy_mm
    {
        bool ok;
        int y = S.left(4).toInt(&ok);
        if (ok) year = y;
        int m = S.right(2).toInt(&ok);
        if (ok) month = m;
    }
    else if (S.mid(2,1) == "_") // mm_yyyy
    {
        bool ok;
        int y = S.right(4).toInt(&ok);
        if (ok) year = y;
        int m = S.left(2).toInt(&ok);
        if (ok) month = m;
    }
    if ((year > 0) && (month > 0) && (month < 13))
    {
        *ok = true;
        date.setDate(year, month, 1);
        return true;
    }
    return false;
}




onewiredevice *calc::checkDevice(const QString &RomID)
{
    onewiredevice *device = nullptr;
    device = maison1wirewindow->configwin->DeviceExist(RomID);
    if (!device) device = maison1wirewindow->configwin->Devicenameexist(RomID);
    if (!device) return nullptr;
    return device;
}





void calc::dataError(QString msg)
{
    if (!TCalc) textBrowserResult += "\n" + msg;
    dataValid = false;
}




bool calc::ajouter(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("+");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Add " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        if (A.isEmpty()) syntaxError(tr("Missing operand A"));		ManageError
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        ok = true;
        if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        if (!ok) return false;
        double v = a + b;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Add done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}




void calc::syntaxError(QString msg)
{
    if (!TCalc) textBrowserResult += "\n" + msg;
    syntax = false;
}




bool calc::isOperator(const QString &str)
{
    if (str == "*") return true;
    if (str == "/") return true;
    if (str == "+") return true;
    if (str == "-") return true;
    if (str == ">") return true;
    if (str == "<") return true;
    if (str == "=") return true;
    if (str == "!") return true;
    if (str == "^") return true;
    return false;
}



int calc::getNextOp(QString &C, int index)
{
    for (int n=index + 1; n<C.length(); n++)
        if (isOperator(C.mid(n, 1))) return n;
    return -1;
}




int calc::getPreviousOp(QString &C, int index)
{
    for (int n=index - 1; n>=0; n--)
        if (isOperator(C.mid(n, 1))) return n;
    return -1;
}



double calc::calculsimple(const QString &str)
{
    textBrowserResult.clear();
    V.clear();
    double R = logisdom::NA;
    syntax = true;
    dataValid = true;
    bool check = true;
    Calc = str;
    Calc = Calc.remove(" ");
    Calc = Calc.remove("\n");
    while (resoudreParenthese());
    if (!(syntax && dataValid)) goto error;
    while (pw(Calc));
    if (!(syntax && dataValid)) goto error;
    while (diviser(Calc));
    if (!(syntax && dataValid)) goto error;
    while (multiplier(Calc));
    if (!(syntax && dataValid)) goto error;
    while (sup(Calc));
    if (!(syntax && dataValid)) goto error;
    while (inf(Calc));
    if (!(syntax && dataValid)) goto error;
    while (egal(Calc));
    if (!(syntax && dataValid)) goto error;
    while (different(Calc));
    if (!(syntax && dataValid)) goto error;
    while (enlever(Calc));
    if (!(syntax && dataValid)) goto error;
    while (ajouter(Calc));
    if (!(syntax && dataValid)) goto error;
    if (!V.isEmpty()) R = V.last();
    else if (isNumeric(Calc)) R = Calc.toDouble(&check);
    else R = toNumeric(Calc, &check);
error:
    if (int(R) == logisdom::NA) dataValid = false;
    if (!TCalc) textBrowserResult += "\n" + (tr("Result = ") + QString("%1").arg(R, 0, 'e'));
    if (!check) syntax = false;
    if (!TCalc)
    {
        if (syntax) textBrowserResult += "\n" + (tr("No error detected in the formula"));
            else textBrowserResult += "\n" + (tr("Error in the formula"));
    }
    if (!TCalc)
    {
        if (dataValid) textBrowserResult += "\n" + (tr("Variable data were valid"));
            else textBrowserResult += "\n" + (tr("Variable data not valid"));
    }
    if (!TCalc)
    {
        if (deviceLoading)
        {
            textBrowserResult += "\n" + (tr("Result is not consistent some device are loading data"));
            R = logisdom::NA;
        }
        else
        {
            if (syntax && dataValid) textBrowserResult += "\n" + (tr("Result is consistent"));
            else textBrowserResult += "\n" + (tr("Result is not consistent"));
        }
    }
    if (!dataValid) syntax = false;
    //threadResult = R;
    //if (deviceList.count() == 0) textBrowserResult += "\nNo Connected device :\n";
    //else textBrowserResult += "\nConnected devices :\n";
    //for (int n=0; n<deviceList.count(); n++)
    //    textBrowserResult += "  - " + deviceList.at(n)->getromid() + " " + deviceList.at(n)->getname() + "\n";
    return R;
}





bool calc::resoudreParenthese()
{
    //int OPindex = -1;
    int Pferm = Calc.indexOf(")");
    if (Pferm != -1)
    {
        int Pouv = Calc.left(Pferm).lastIndexOf("(");
        if (Pouv == -1) syntaxError(tr("Missing opening bracket"));
        ManageError
        QString OP = "";
        if (Pouv > 1)
        {
            QString checkOP = Calc.mid(Pouv - 1, 1);
            if (!isOperator(checkOP))
            {
                return false;
            }
        }
        QString C = Calc.mid(Pouv + 1, Pferm - Pouv - 1);
        quint16 loop = 65535;
        while (pw(C) and loop) { loop--; } ManageError
        while (diviser(C) and loop) { loop--; } ManageError
        while (multiplier(C) and loop) { loop--; } ManageError
        while (sup(C) and loop) { loop--; } ManageError
        while (inf(C) and loop) { loop--; } ManageError
        while (egal(C) and loop) { loop--; } ManageError
        while (different(C) and loop) { loop--; } ManageError
        while (enlever(C) and loop) { loop--; } ManageError
        while (ajouter(C) and loop) { loop--; } ManageError
        if (OP.isEmpty())
        {
            if (isNumeric(C))
            {
                bool ok;
                double D = C.toDouble(&ok);
                int index = V.count();
                V.append(D);
                QString P = Calc.left(Pouv);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                Calc = P + QString("V%1").arg(index) + S;
                textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(D, 0, 'e'));
            }
            else
            {
                QString P = Calc.left(Pouv);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                Calc = P + C + S;
            }
            textBrowserResult += "\n" + QString::fromLatin1("Done : ") + Calc;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}







bool calc::multiplier(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("*");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Multiplier " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        if (A.isEmpty()) syntaxError(tr("Missing operand A"));		ManageError
        int Next = getNextOp(C, posx);
        if (Next == posx + 1) Next = getNextOp(C, posx + 1);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v = a * b;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Multiplication done : " + C + QString(" = %1").arg(v, 0, 'e'));
        return true;
    }
    return false;
}







bool calc::diviser(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("/");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Divide " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        if (A.isEmpty()) syntaxError(tr("Missing operand A"));		ManageError
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        ok = true;
        if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v = a / b;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Division done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}





bool calc::enlever(QString &C)
{
    QString A, B;
    double a, b;
    bool ok = true;
    int posx = C.indexOf("-");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Substract " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (A.isEmpty()) a = 0;
        else if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v = a - b;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Substract done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}






bool calc::sup(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf(">");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Higher " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v;
        if (a > b) v = 1; else v = 0;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Higher done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}







bool calc::inf(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("<");
    if (posx != -1)
    {
        textBrowserResult += "\nLower " + C;
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\nA = " + A;
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (isNumeric(A)) a = A.toDouble(&ok); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(&ok); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v;
        if (a < b) v = 1; else v = 0;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Lower done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}







bool calc::egal(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("=");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Equal " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (isNumeric(A)) a = A.toDouble(); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v;
        if (logisdom::AreSame(a,b)) v = 1; else v = 0;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("EQual done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}




bool calc::different(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("!");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Different " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (isNumeric(A)) a = A.toDouble(); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v;
        if (logisdom::AreNotSame(a, b)) v = 1; else v = 0;
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Different done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}



bool calc::pw(QString &C)
{
    QString A, B;
    double a, b;
    bool ok;
    int posx = C.indexOf("^");
    if (posx != -1)
    {
        textBrowserResult += "\n" + ("Puissance " + C);
        int Prev = getPreviousOp(C, posx);
        if (Prev == -1) Prev = 0; else Prev++;
        A = C.mid(Prev, posx - Prev);
        textBrowserResult += "\n" + ("A = " + A);
        int Next = getNextOp(C, posx);
        if (Next == -1) Next = C.length() - 1; else Next--;
        B = C.mid(posx + 1, Next - posx);
        textBrowserResult += "\n" + ("B = " + B);
        ok = true;
        if (isNumeric(A)) a = A.toDouble(); else a = toNumeric(A, &ok);
        if (!ok) ManageError("Operand A error : " + A); ManageError
        if (B.isEmpty()) syntaxError(tr("Missing operand B"));		ManageError
        if (isNumeric(B)) b = B.toDouble(); else b = toNumeric(B, &ok);
        if (!ok) ManageError("Operand B error : " + B); ManageError
        double v;
        v = pow(a, b);
        int index = V.count();
        V.append(v);
        QString P = C.left(Prev);
        QString S = C.right(C.length() - Next - 1);
        C = P + QString("V%1").arg(index) + S;
        textBrowserResult += "\n" + ("Puissance done : " + C);
        textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(v, 0, 'e'));
        return true;
    }
    return false;
}

