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




#include <QtWidgets/QColorDialog>
#include "globalvar.h"
#include "commonstring.h"
#include "logisdom.h"
#include "configwindow.h"
#include "onewire.h"
#include "devchooser.h"
#include "remote.h"
#include "formula.h"
#include "iconearea.h"
#include "icont.h"



icont::icont(QWidget *parent, IconeArea *lparent)
{
    text = new QLabel(parent);
	Parent = lparent;
    halfbigger = false;
	highlighted = false;
// palette setup icon
	setup.setLayout(&setupLayout);
	int index = 0;
    //setupLayout.setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setupLayout.addWidget(&Text, index++, 0, 1, logisdom::PaletteWidth);
    connect(&Text, SIGNAL(textChanged()), this, SLOT(changeText()));
	QFont textfont = text->font();
    textsize.setRange(1, 99);
	textsize.setPrefix(tr("Text Size : "));
	textsize.setValue(textfont.pointSize());
    connect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
	setupLayout.addWidget(&textsize, index, 0, 1, logisdom::PaletteWidth/2);
	textstretch.setRange(1, 200);
	textstretch.setPrefix(tr("Text Stretch : "));
    textstretch.setValue(100);
	connect(&textstretch, SIGNAL(valueChanged(int)), this, SLOT(textrestretch(int)));
	setupLayout.addWidget(&textstretch, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	resetPosition.setText(tr("Reset position"));
	connect(&resetPosition, SIGNAL(clicked()), this, SLOT(resetPos()));
    setAsBgr.setText(tr("Set as background"));
    connect(&setAsBgr, SIGNAL(clicked()), this, SLOT(lowerIcon()));
    setInFrt.setText(tr("Set in Front"));
    connect(&setInFrt, SIGNAL(clicked()), this, SLOT(higherIcon()));
    setTextColor.setText(tr("Text Color"));
    connect(&setTextColor, SIGNAL(clicked()), this, SLOT(changeTextColor()));
    QFontDatabase database;
    const QStringList fontFamilies = database.families();
    for (const QString &family : fontFamilies)
    {
        const QStringList fontStyles = database.styles(family);
        fontName.addItem(family);
    }
    connect(&fontName, SIGNAL(currentIndexChanged(const QString)), this, SLOT(fontChanged(const QString)));
    setupLayout.addWidget(&fontName, index++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&setTextColor, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&resetPosition, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&setAsBgr, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&setInFrt, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
}





icont::~icont()
{
    disconnect();
    text->deleteLater();
}




void icont::resetPos()
{
	text->move(QPoint(0, 0));
}



void icont::lowerIcon()
{
	text->lower();
	emit(setAsBackground(this));
}




void icont::higherIcon()
{
	text->raise();
	emit(setInFront(this));
}




void icont::changeTextColor()
{
    if (!text) return;
    QColor currentcolor = QColorDialog::getColor(textColor);
    if (!currentcolor.isValid()) return;
    setTextStyle(currentcolor);
}




void icont::setTextStyle(QColor &color)
{
    textColor = color;
    TextStyle = QString("color:rgb(%1,%2,%3);").arg(textColor.red()).arg(textColor.green()).arg(textColor.blue());
    text->setStyleSheet(TextStyle);
    TextStyleHex = colorToHtmlHex(color);
}



QString icont::colorToHtmlHex(QColor &color)
{
    QString hex;
    hex += QString("%1").arg(uchar(color.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.blue()), 2, 16, QChar('0'));
    return hex.toUpper();
}





void icont::setHighlighted(bool state)
{
	if (state)
	{
        text->setStyleSheet("QLabel { background-color : grey; }");
        highlighted = true;
	}
	else
	{
        text->setStyleSheet(TextStyle);
        highlighted = false;
	}
}



void icont::textreweight(int fontweight)
{
    fontText.setWeight(fontweight);
    text->setFont(fontText);
    text->resize(text->sizeHint());
}



void icont::textrestretch(int stretch)
{
    fontText.setStretch(stretch);
    text->setFont(fontText);
    text->resize(text->sizeHint());
}




void icont::textrezise(int fontsize)
{
    disconnect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
    if (textsize.value() != fontsize) textsize.setValue(fontsize);
    fontText.setFamily(fontName.currentText());
    fontText.setPointSize(fontsize);
    text->setFont(fontText);
    text->resize(text->sizeHint());
    connect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
}





void icont::changeText()
{
    QString tr_txt = Text.toPlainText();
    Parent->parent->configwin->valueTranslate(tr_txt);
    text->setText(tr_txt);
	text->resize(text->sizeHint());
}



void icont::settxt(QString name, QPoint pos)
{
    disconnect(&Text, SIGNAL(textChanged()), this, SLOT(changeText()));
	if (name.isEmpty()) name = cstr::toStr(cstr::NoName);
	text->setText(name);
	text->resize(text->sizeHint());
	text->move(pos);
	text->show();
	text->setAttribute(Qt::WA_DeleteOnClose);
	Text.setText(name);
    connect(&Text, SIGNAL(textChanged()), this, SLOT(changeText()));
}




void icont::bigger()
{
    halfbigger = !halfbigger;
    if (highlighted)
    {
        int s = textsize.value();
        s++;
        textrezise(s);
    }
}



void icont::smaller()
{
    halfbigger = !halfbigger;
    if (highlighted)
    {
        int s = textsize.value();
        if (s > 2) s--;
        textrezise(s);
    }
}




void icont::getConfigStrIconPath(QString &str)
{
    str += "\n" Icon_Begin "\n";
    str += logisdom::saveformat("Text", Text.toPlainText(), true);
    str += logisdom::saveformat("TextPos_X", QString("%1").arg(text->x()));
    str += logisdom::saveformat("TextPos_Y", QString("%1").arg(text->y()));
    str += logisdom::saveformat("Text_Red", QString("%1").arg(textColor.red()));
    str += logisdom::saveformat("Text_Green", QString("%1").arg(textColor.green()));
    str += logisdom::saveformat("Text_Blue", QString("%1").arg(textColor.blue()));
    str += logisdom::saveformat("Text_Color", colorToHex(textColor));
    str += logisdom::saveformat("TextFontSize", QString("%1").arg(fontText.pointSize()));
    str += logisdom::saveformat("TextStretch", QString("%1").arg(text->font().stretch()));
    str += logisdom::saveformat("FontName", fontName.currentText());
    str += Icon_Finished;
    str += "\n";
}

void icont::getConfigStr(QString &str)
{
    str += "\n" Icon_Begin "\n";
    str += logisdom::saveformat("Text", Text.toPlainText(), true);
	str += logisdom::saveformat("TextPos_X", QString("%1").arg(text->x()));
	str += logisdom::saveformat("TextPos_Y", QString("%1").arg(text->y()));
    str += logisdom::saveformat("Text_Red", QString("%1").arg(textColor.red()));
    str += logisdom::saveformat("Text_Green", QString("%1").arg(textColor.green()));
    str += logisdom::saveformat("Text_Blue", QString("%1").arg(textColor.blue()));
    str += logisdom::saveformat("Text_Color", colorToHex(textColor));
    str += logisdom::saveformat("TextFontSize", QString("%1").arg(fontText.pointSize()));
    str += logisdom::saveformat("TextStretch", QString("%1").arg(text->font().stretch()));
    str += logisdom::saveformat("FontName", fontName.currentText());
    str += Icon_Finished;
	str += "\n";
}




void icont::setFont(const QString &font)
{
    fontName.setCurrentText(font);
}



void icont::fontChanged(const QString &font)
{
    fontText.setFamily(fontName.currentText());
    fontText.setPointSize(textsize.value());
    text->setFont(fontText);
    text->resize(text->sizeHint());
    QString f = font;
    emit(newFont(f));
}


void icont::setConfig(const QString &strsearch)
{
	bool ok_x, ok_y, ok;
	int x, y, s;
    QString name = logisdom::getvalue("Text", strsearch);
	x = logisdom::getvalue("TextPos_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("TextPos_Y", strsearch).toInt(&ok_y, 10);
	if (ok_x && ok_y)
	{
		settxt(name, QPoint(x, y));
	}
    s = logisdom::getvalue("TextStretch", strsearch).toInt(&ok, 10);
    if (!ok) s = 100;
    if (s < 2) s = 100;
    textstretch.setValue(s);
    textrestretch(s);

    { int R = 0, G = 0, B = 0;
    QColor color;
    s = logisdom::getvalue("Text_Red", strsearch).toInt(&ok, 10);
    if (ok) R = s;
    if (R > 255) R = 255;
    if (R < 0) R = 0;
    color.setRed(R);
    s = logisdom::getvalue("Text_Green", strsearch).toInt(&ok, 10);
    if (ok) G = s;
    if (G > 255) G = 255;
    if (G < 0) G = 0;
    color.setGreen(G);
    s = logisdom::getvalue("Text_Blue", strsearch).toInt(&ok, 10);
    if (ok) B = s;
    if (B > 255) B = 255;
    if (B < 0) B = 0;
    color.setBlue(B);
    setTextStyle(color); }

    QColor color;
    QString hexColor = logisdom::getvalue("Text_Color", strsearch);
    if (!hexColor.isEmpty())
        if (hexToColor(hexColor, color)) textColor = color;
    setTextStyle(textColor);

    QString fontStr = logisdom::getvalue("FontName", strsearch);
    if (fontStr.isEmpty()) fontStr = QApplication::font().family();
    else
    {
        QFontDatabase database;
        const QStringList fontFamilies = database.families();
        if (!fontFamilies.contains(fontStr))
            fontStr = QApplication::font().family();
    }
    fontName.setCurrentText(fontStr);
    s = logisdom::getvalue("TextFontSize", strsearch).toInt(&ok, 10);
    if (ok) textrezise(s);
}



QString icont::colorToHex(QColor &color)
{
    QString hex;
    hex += QString("%1").arg(uchar(color.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.blue()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.alpha()), 2, 16, QChar('0'));
    return hex.toUpper();
}



bool icont::hexToColor(QString &hex, QColor &color)
{
    if (hex.length() != 8) return false;
    bool ok;
    int r = hex.mid(0, 2).toInt(&ok, 16);
    if (!ok) return false;
    int g = hex.mid(2, 2).toInt(&ok, 16);
    if (!ok) return false;
    int b  = hex.mid(4, 2).toInt(&ok, 16);
    if (!ok) return false;
    int a = hex.mid(6, 2).toInt(&ok, 16);
    if (!ok) return false;
    color.setRgba(qRgba(r, g, b, a));
    return true;
}
