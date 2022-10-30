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
#include "globalvar.h"
#include "axb.h"



axb::axb()
{
// palette setup icon
	setLayout(&setupLayout);
	setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	setupLayout.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	int layoutIndex = 0;
	axbEnabled.setText(tr("ax+b correction"));
	setupLayout.addWidget(&axbEnabled, layoutIndex, 0, 1, 2 );
	axbEnabled.setCheckState(Qt::Unchecked);
	connect(&axbEnabled, SIGNAL(stateChanged(int)), this, SLOT(changeEnable(int)));
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	A.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	B.setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	RawValue.setText("...");
	setupLayout.addWidget(&RawValue, layoutIndex++, 2, 1, 2 );
	A.setText("1");
	B.setText("0");
	AText.setText("a = ");
	BText.setText("b = ");
	setupLayout.addWidget(&AText, layoutIndex, 0, 1, 1 );
	setupLayout.addWidget(&A, layoutIndex, 1, 1, 1 );
	setupLayout.addWidget(&BText, layoutIndex, 2, 1, 1 );
	setupLayout.addWidget(&B,  layoutIndex++, 3, 1, 1);
	changeEnable(Qt::Unchecked);
	connect(&A, SIGNAL(editingFinished()), this, SLOT(checkA()));
	connect(&B, SIGNAL(editingFinished()), this, SLOT(checkB()));
}





axb::~axb()
{
}




void axb::checkA()
{
	bool ok;
	if (A.text().isEmpty()) return;
	A.text().toDouble(&ok);
    if (!ok)
    {
        bool alarm = false;
        QString txt = A.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (!(okA && okB && (logisdom::isNotZero(B)))) alarm = true;
            }
        }
        else alarm = true;
        if (alarm) QMessageBox::warning(this, tr("Value error"), tr("Value a cannot be interpreted"), QMessageBox::AcceptRole, QMessageBox::NoIcon);
    }
}




void axb::checkB()
{
	bool ok;
	if (B.text().isEmpty()) return;
	B.text().toDouble(&ok);
    if (!ok)
    {
        bool alarm = false;
        QString txt = A.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (!(okA && okB && (logisdom::isNotZero(B)))) alarm = true;
            }
        }
        else alarm = true;
        if (alarm) QMessageBox::warning(this, tr("Value error"), tr("Value b cannot be interpreted"), QMessageBox::AcceptRole, QMessageBox::NoIcon);
    }
}



void axb::GetConfigStr(QString &str)
{
	str += logisdom::saveformat("axbEnabled", QString("%1").arg(axbEnabled.isChecked()));
	str += logisdom::saveformat("AValue", A.text());
	str += logisdom::saveformat("BValue", B.text());
}





void axb::setconfig(const QString &strsearch)
{
	bool ok;
	int en = logisdom::getvalue("axbEnabled", strsearch).toInt(&ok);
	if (!ok) 	axbEnabled.setCheckState(Qt::Unchecked);
		else if (en) 	axbEnabled.setCheckState(Qt::Checked);
			else axbEnabled.setCheckState(Qt::Unchecked);
	QString a = logisdom::getvalue("AValue", strsearch);
	if (!a.isEmpty()) A.setText(a);
	QString b = logisdom::getvalue("BValue", strsearch);
	if (!b.isEmpty()) B.setText(b);
}





double axb::result(double value)
{
	bool ok;
	if (!axbEnabled.isChecked()) return value;
	if (A.text().isEmpty()) return value;
    bool a_Valid = true;
    bool b_Valid = true;
    double a = A.text().toDouble(&ok);
    if (!ok)
    {
        QString txt = A.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) a = A/B; else a_Valid = false;
            }
        }
        else a_Valid = false;
    }
    double b = B.text().toDouble(&ok);
    if (!ok)
    {
        QString txt = B.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) b = A/B; else b_Valid = false;
            }
        }
        else b_Valid = false;
    }
    RawValue.setText("x = " + QString("%1").arg(value, 0, 'g'));
    if (!a_Valid) return value;
    if (!b_Valid) return value;
	double R = a * value + b;
	return R;
}



double axb::fromResult(double r)
{
    bool ok;
    if (!axbEnabled.isChecked()) return r;
    if (A.text().isEmpty()) return r;
    bool a_Valid = true;
    bool b_Valid = true;
    double a = A.text().toDouble(&ok);
    if (!ok)
    {
        QString txt = A.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) a = A/B; else a_Valid = false;
            }
        }
        else a_Valid = false;
    }
    double b = B.text().toDouble(&ok);
    if (!ok)
    {
        QString txt = B.text();
        int pos = txt.indexOf("/");
        if (pos != -1)
        {
            QStringList p = txt.split("/");
            if (p.count() == 2)
            {
                bool okA, okB;
                double A = p.at(0).toDouble(&okA);
                double B = p.at(1).toDouble(&okB);
                if (okA && okB && (logisdom::isNotZero(B))) b = A/B; else b_Valid = false;
            }
        }
        else b_Valid = false;
    }
    if (!a_Valid) return r;
    if (!b_Valid) return r;
    double x = (r - b) / a;
    return x;
}


void axb::changeEnable(int state)
{
	switch (state)
	{
		case Qt::Unchecked : 
			A.setEnabled(false);
			B.setEnabled(false);
			AText.setEnabled(false);
			BText.setEnabled(false);
			RawValue.setEnabled(false);
			break;
		case Qt::PartiallyChecked :
		case Qt::Checked :
			A.setEnabled(true);
			B.setEnabled(true);
			AText.setEnabled(true);
			BText.setEnabled(true);
			RawValue.setEnabled(true);
			break;
	}
}





