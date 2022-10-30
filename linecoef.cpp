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




#include <QtWidgets/QMessageBox>
#include "logisdom.h"
#include "linecoef.h"



lineCoef::lineCoef(QWidget *parent) : QLineEdit(parent)
{
    //connect(this, SIGNAL(editingFinished()), this, SLOT(checkCoef()));
}




lineCoef::~lineCoef()
{
}




double lineCoef::value()
{
    if (text().isEmpty())
    {
        setToolTip("");
        return 1;
    }
    double R = formul.calculsimple(text());
    setToolTip(formul.textBrowserResult);
    return R;
}



double lineCoef::result(double value)
{
    if (text().isEmpty())
    {
        setToolTip("");
        return value;
    }
    double R = formul.calculsimple(text()) * value;
    setToolTip(formul.textBrowserResult);
    return R;
}






void lineCoef::checkCoef()
{
    bool ok;
    if (text().isEmpty()) return;
    text().toDouble(&ok);
    if (!ok)
    {
        bool alarm = false;
        QString txt = text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (!(okA && okB && (B != 0))) alarm = true;
            }
        }
        else alarm = true;
        if (alarm) QMessageBox::warning(this, tr("Value error"), tr("Value a cannot be interpreted"), QMessageBox::AcceptRole, QMessageBox::NoIcon);
    }
}
