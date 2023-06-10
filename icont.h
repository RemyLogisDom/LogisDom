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



#ifndef ICONT_H
#define ICONT_H
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtCore>
#include <QtGui>
#include "htmlbinder.h"


class onewiredevice;
class devchooser;
class IconeArea;
class formula;


class icont : public QWidget
{
	Q_OBJECT
enum specialValuesIndex
	{	Disabled, Time, Date, UserDateTime	};
#define handle_Size 10
public:	
        icont(QWidget *parent, IconeArea *lparent);
        ~icont();
	IconeArea *Parent;
	void getConfigStr(QString &str);
    void getConfigStrIconPath(QString &str);
    void setConfig(const QString &strsearch);
	void settxt(QString name, QPoint pos);
	void setHighlighted(bool state);
    void bigger();
    bool halfbigger;
    void smaller();
    QLabel *text;
    QColor textColor;
    QString TextStyle, TextStyleHex;
    void setTextStyle(QColor &);
    QString colorToHtmlHex(QColor &);
    bool highlighted;
// palette setup
	QWidget setup;
	QGridLayout setupLayout;
    QTextEdit Text;
    QSpinBox textsize;
	QSpinBox textstretch;
	QPushButton resetPosition;
    QPushButton setAsBgr;
	QPushButton setInFrt;
    QPushButton setTextColor;
    QComboBox fontName;
    QFont fontText;
    static QString colorToHex(QColor &color);
    static bool hexToColor(QString &hex, QColor &color);
public slots:
    void textrezise(int fontsize);
    void setFont(const QString &font);
private slots:
#if QT_VERSION < 0x060000
    void textreweight(int);
#else
    void textreweight(QFont::Weight);
#endif
	void textrestretch(int stretch);
    void changeText();
	void resetPos();
    void lowerIcon();
	void higherIcon();
    void changeTextColor();
    void fontChanged(const QString &);
private:
	QString channel;
signals:
    void setAsBackground(icont*);
    void setInFront(icont*);
    void newFont(QString);
};

#endif
