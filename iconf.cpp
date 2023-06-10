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
#include "iconf.h"



iconf::iconf(QWidget *parent, IconeArea *lparent)
{
    icon = new QLabel(parent);
    text = new QLabel(parent);
    value = new QLabel(parent);
    value->setText(cstr::toStr(logisdom::NA));
    binder = nullptr;
	Parent = lparent;
    halfbigger = false;
	textvisible = true;
	valuevisible = true;
	fileNotFound = false;
    calc_Result = logisdom::NA;
    value->setText(cstr::toStr(cstr::NA));
	highlighted = false;
    Thighlighted = false;
    Vhighlighted = false;
// palette setup icon
	setup.setLayout(&setupLayout);
	int index = 0;
    //setupLayout.setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setupLayout.addWidget(&Text, index++, 0, 1, logisdom::PaletteWidth);
	connect(&Text, SIGNAL(textChanged(const QString&)), this, SLOT(changeText(const QString&)));
	keepRatio.setText(tr("Keep ratio"));
	keepRatio.setChecked(true);
	setupLayout.addWidget(&keepRatio, index, 0, 1, logisdom::PaletteWidth/2);
	connect(&keepRatio, SIGNAL(stateChanged(int)), this, SLOT(keepratiochanged(int)));
	iconsize.setRange(1,999);
	iconsize.setPrefix(tr("Icon Size : "));
	iconsize.setSuffix(" %");
	connect(&iconsize, SIGNAL(valueChanged(int)), this, SLOT(iconrezise(int)));
    connect(&iconsize_X, SIGNAL(valueChanged(int)), this, SLOT(iconrezise(int)));
    connect(&iconsize_Y, SIGNAL(valueChanged(int)), this, SLOT(iconrezise(int)));
    iconsize.setValue(100);
	setupLayout.addWidget(&iconsize, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	iconsize_X.setRange(1, 9999);
	iconsize_X.setPrefix("X = ");
	iconsize_X.setEnabled(false);
	iconsize_Y.setRange(1, 9999);
	iconsize_Y.setPrefix("Y = ");
	iconsize_Y.setEnabled(false);
	setupLayout.addWidget(&iconsize_X, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&iconsize_Y, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	iconRotate.setPrefix(tr("Rotate : "));
	iconRotate.setSuffix("°");
	iconRotate.setRange(-180, 180);
	iconRotate.setSingleStep(5);
	setupLayout.addWidget(&iconRotate, index, 0, 1, logisdom::PaletteWidth/2);
	connect(&iconRotate, SIGNAL(valueChanged(int)), this, SLOT(iconrotate(int)));
	iconOpacity.setPrefix(tr("Opacity : "));
	iconOpacity.setSuffix("%");
	iconOpacity.setRange(0, 100);
	iconOpacity.setValue(100);
	iconOpacity.setSingleStep(5);
	setupLayout.addWidget(&iconOpacity, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	connect(&iconOpacity, SIGNAL(valueChanged(int)), this, SLOT(iconrotate(int)));
	iconchange.setText(tr("Icon change"));
	connect(&iconchange, SIGNAL(stateChanged(int)), this, SLOT(clickiconchange(int)));
	iconchangerange.setRange(-999999,999999);
	iconchangerange.setValue(100);
	iconchangerange.setPrefix(tr("Max : "));
	iconchangestep.setRange(-999999,999999);
	iconchangestep.setValue(50);
	iconchangestep.setPrefix(tr("Step : "));
	iconchangeoffset.setRange(-999999,999999);
	iconchangeoffset.setValue(0);
	iconchangeoffset.setPrefix("Offset : ");
	NoIcon.setText(tr("No Icon"));
	connect(&NoIcon, SIGNAL(stateChanged(int)), this, SLOT(NoITVChange(int)));
	NoText.setText(tr("No Text"));
	connect(&NoText, SIGNAL(stateChanged(int)), this, SLOT(NoITVChange(int)));
	NoValue.setText(tr("No Value"));
	connect(&NoValue, SIGNAL(stateChanged(int)), this, SLOT(NoITVChange(int)));
    textsize.setRange(1, 99);
	textsize.setPrefix(tr("Text Size : "));
    textsize.setValue(QApplication::font().pointSize());
    connect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
	setupLayout.addWidget(&textsize, index, 0, 1, logisdom::PaletteWidth/2);
	textstretch.setRange(1, 200);
	textstretch.setPrefix(tr("Text Stretch : "));
    textstretch.setValue(100);
	connect(&textstretch, SIGNAL(valueChanged(int)), this, SLOT(textrestretch(int)));
	setupLayout.addWidget(&textstretch, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	valuesize.setRange(1, 99);
	valuesize.setPrefix(tr("Value Size : "));
    valuesize.setValue(QApplication::font().pointSize());
    connect(&valuesize, SIGNAL(valueChanged(int)), Parent, SLOT(valuerezise(int)));
	setupLayout.addWidget(&valuesize, index, 0, 1, logisdom::PaletteWidth/2);
	valuestretch.setRange(1, 200);
	valuestretch.setPrefix(tr("Value Stretch : "));
    valuestretch.setValue(100);
	setupLayout.addWidget(&valuestretch, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	connect(&valuestretch, SIGNAL(valueChanged(int)), this, SLOT(valuerestretch(int)));
	specialValue.addItem(tr("Device"));
	specialValue.addItem(tr("Time"));
	specialValue.addItem(tr("Date"));
	specialValue.addItem(tr("User Date & Time"));
	specialValue.setToolTip(maison1wirewindow->TimeFormatDetails + "\n" + maison1wirewindow->DateFormatDetails);
	connect(&specialValue, SIGNAL(currentIndexChanged(int)), this, SLOT(specialValueChanged(int)));
	specialValueParameters.setEnabled(false);
	connect(&specialValueParameters, SIGNAL(textChanged(const QString&)), this, SLOT(specialText(const QString&)));
	resetPosition.setText(tr("Reset position"));
	connect(&resetPosition, SIGNAL(clicked()), this, SLOT(resetPos()));
	setAsBgr.setText(tr("Set as background"));
	connect(&setAsBgr, SIGNAL(clicked()), this, SLOT(lowerIcon()));
	setInFrt.setText(tr("Set in Front"));
	connect(&setInFrt, SIGNAL(clicked()), this, SLOT(higherIcon()));
	chooseDevice.setText(tr("Choose Device"));
	connect(&chooseDevice, SIGNAL(clicked()), this, SLOT(chooseDev()));
    setTextColor.setText(tr("Text Color"));
    connect(&setTextColor, SIGNAL(clicked()), this, SLOT(changeTextColor()));
    setValueColor.setText(tr("Value Color"));
    connect(&setValueColor, SIGNAL(clicked()), this, SLOT(changeValueColor()));
    showDevice.setText(tr("Show Device"));
    connect(&showDevice, SIGNAL(clicked()), this, SLOT(showDev()));
    QFontDatabase database;
    const QStringList fontFamilies = database.families();
    for (const QString &family : fontFamilies)
    {
        const QStringList fontStyles = database.styles(family);
        fontName.addItem(family);
    }
    fontName.setCurrentIndex(fontFamilies.indexOf(QApplication::font().family()));
    connect(&fontName, SIGNAL(currentIndexChanged(const QString)), this, SLOT(fontChanged(const QString)));
    setupLayout.addWidget(&fontName, index++, 0, 1, logisdom::PaletteWidth);
    setupLayout.addWidget(&NoIcon, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&NoText, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&iconchange, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&NoValue, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&setValueColor, index, 0, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&setTextColor, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&iconchangestep, index++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&iconchangeoffset, index++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&iconchangerange, index++, 0, 1, logisdom::PaletteWidth);
	setupLayout.addWidget(&specialValue, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&specialValueParameters, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&actualFileName, index++, 0, 1, logisdom::PaletteWidth);
    htmlCommand.setToolTip("Html Command");
    setupLayout.addWidget(&htmlCommand, index++, 0, 1, logisdom::PaletteWidth);
    connect(&htmlCommand, SIGNAL(editingFinished()), this, SLOT(htmlCommandChanged()));
    setupLayout.addWidget(&chooseDevice, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&resetPosition, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&setAsBgr, index, 0, 1, logisdom::PaletteWidth/2);
	setupLayout.addWidget(&setInFrt, index++, logisdom::PaletteWidth/2, 1, logisdom::PaletteWidth/2);
    setupLayout.addWidget(&showDevice, index, 0, 1, logisdom::PaletteWidth/2);
    clickiconchange(0);
// stat palette setup icon
    Formula = new formula(Parent->parent);
    //Formula->setName(name);
    Formula->removeButtons();
    Formula->calcth->setLinkedEnabled();
    stat_setup.setLayout(&stat_setupLayout);
    index = 0;
    stat_setupLayout.addWidget(&formulaText, index++, 0, 1, logisdom::PaletteWidth);
    Decimal.setRange(0, 10);
    Decimal.setPrefix(tr("Decimal : "));
    stat_setupLayout.addWidget(&Decimal, index++, 1, 1, 1);
    stat_setupLayout.addWidget(Formula, index++, 0, 1, logisdom::PaletteWidth);
    connect(Formula, SIGNAL(calcdone()), this, SLOT(clacdone()));
}





iconf::~iconf()
{
    disconnect();
    icon->deleteLater();
    text->deleteLater();
    value->deleteLater();
}




void iconf::showDev()
{
    Parent->VoirCapteur(romid);
}


void iconf::chooseDev()
{
	Parent->Capteur(this);
}


void iconf::resetPos()
{
	icon->move(QPoint(0, 0));
	text->move(QPoint(0, 0));
	value->move(QPoint(0, 0));
}





void iconf::lowerIcon()
{
	icon->lower();
	text->lower();
	value->lower();
	emit(setAsBackground(this));
}


int iconf::getDisplayMode()
{
    return specialValue.currentIndex();
}


QString iconf::getDisplayModeStr()
{
    return specialValueParameters.text();
}


void iconf::higherIcon()
{
	icon->raise();
	text->raise();
	value->raise();
	emit(setInFront(this));
}




void iconf::changeTextColor()
{
    if (!text) return;
    QColor currentcolor = QColorDialog::getColor(textColor);
    if (!currentcolor.isValid()) return;
    setTextStyle(currentcolor);
}




void iconf::clacdone()
{
    calc_Result = Formula->getValue();
    QString V;
    if (logisdom::isNA(calc_Result)) V = cstr::toStr(cstr::NA);
    else V = QString("%1").arg(calc_Result, 0, 'f', Decimal.value());
    QString tr_txt = formulaText.text().replace("****", V);
    Parent->parent->configwin->valueTranslate(tr_txt);
    value->setText(tr_txt);
    value->resize(value->sizeHint());
}



void iconf::changeValueColor()
{
    if (!value) return;
    QColor currentcolor = QColorDialog::getColor(valueColor);
    if (!currentcolor.isValid()) return;
    setValueStyle(currentcolor);
}


//    QString ValueStyle, TextStyle, ValueStyleHex, TextStyleHex;


void iconf::setValueStyle(QColor &color)
{
    valueColor = color;
    ValueStyle = QString("color:rgb(%1,%2,%3);").arg(valueColor.red()).arg(valueColor.green()).arg(valueColor.blue());
    ValueStyleHex = colorToHtmlHex(color);
    value->setStyleSheet(ValueStyle);
    //qDebug() << ValueStyleHex;
}


void iconf::setTextStyle(QColor &color)
{
    textColor = color;
    TextStyle = QString("color:rgb(%1,%2,%3);").arg(textColor.red()).arg(textColor.green()).arg(textColor.blue());
    text->setStyleSheet(TextStyle);
    TextStyleHex = colorToHtmlHex(color);
}




QString iconf::colorToHex(QColor &color)
{
    QString hex;
    hex += QString("%1").arg(uchar(color.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.blue()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.alpha()), 2, 16, QChar('0'));
    return hex.toUpper();
}



bool iconf::hexToColor(QString &hex, QColor &color)
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



QString iconf::colorToHtmlHex(QColor &color)
{
    QString hex;
    hex += QString("%1").arg(uchar(color.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.blue()), 2, 16, QChar('0'));
    return hex.toUpper();
}



void iconf::valueChanged(QString str)
{
    value->setText(str);
    value->resize(value->sizeHint());
}


// binderici
void iconf::valueBinderChanged()
{
    QString command = htmlCommand.text();
    QString S = maison1wirewindow->getHtmlValue(command);
    value->setText(S);
    value->resize(value->sizeHint());
}


void iconf::htmlCommandChanged()
{
    if (binder) disconnect(binder, SIGNAL(valueChanged()), this, SLOT(valueBinderChanged()));
    QString command = htmlCommand.text();
    binder = maison1wirewindow->getBinder(command);
    if (binder)
    {
        valueBinderChanged();
        connect(binder, SIGNAL(valueChanged()), this, SLOT(valueBinderChanged()));
        //romid.clear();
    }
    else
    {
        if (htmlCommand.text().startsWith("http")) valueChanged(htmlCommand.text());
        else valueChanged("bad link");
    }
}



void iconf::setFont(const QString &font)
{
    fontName.setCurrentText(font);
}



void iconf::fontChanged(const QString &font)
{
    fontText.setFamily(fontName.currentText());
    fontText.setPointSize(textsize.value());
    text->setFont(fontText);
    text->resize(text->sizeHint());
    fontValue.setFamily(fontName.currentText());
    fontValue.setPointSize(valuesize.value());
    value->setFont(fontValue);
    value->resize(value->sizeHint());
    QString f = font;
    emit(newFont(f));
}



void iconf::specialValueChanged(int index)
{
	switch (index)
	{
		case Disabled : specialValueParameters.setEnabled(false); break;
		case Time : 
			specialValueParameters.setText("HH:mm");
			specialValueParameters.setEnabled(false); break;
		case Date : 
			specialValueParameters.setText("dd-MM-yyyy");
			specialValueParameters.setEnabled(false); break;
		case UserDateTime : specialValueParameters.setEnabled(true); break;
	}
	setvalue();
}





void iconf::NoITVChange(int)
{
	if (NoIcon.isChecked() and NoText.isChecked()) NoValue.setEnabled(false);
	else if (NoIcon.isChecked() and NoValue.isChecked()) NoText.setEnabled(false);
	else if (NoText.isChecked() and NoValue.isChecked()) NoIcon.setEnabled(false);
	else
	{
		NoValue.setEnabled(true);
		NoText.setEnabled(true);
		NoIcon.setEnabled(true);
	}
	if (NoIcon.isChecked())
	{
		icon->hide(); 
		iconchange.setEnabled(false);
		iconchange.setCheckState(Qt::Unchecked);
	}
	 else 
	 {
        icon->show();
		iconchange.setEnabled(true);
 	}
	if (NoText.isChecked()) text->hide(); else text->show();
	if (NoValue.isChecked()) value->hide(); else value->show();
}




void iconf::setpict(QString pict, QPoint pos)
{
    path = pict;
	if (path.isEmpty()) path = ":/images/images/kfm_home.png";
    pixmap.load(path);
    icon->setPixmap(pixmap);
    icon->resize(pixmap.size());
	icon->move(pos);
	icon->show();
	icon->setAttribute(Qt::WA_DeleteOnClose);
	icon->setAccessibleName(path);
}



void iconf::setHighlighted(bool state)
{
	reloadpict();
	if (state)
	{
#if QT_VERSION < 0x060000
        QPixmap pixmap = *icon->pixmap();
#else
        QPixmap pixmap = icon->pixmap();
#endif
		QPainter p(&pixmap);
		QSize s = pixmap.size();
		int w = s.width();
		int h = s.height();
		p.setPen(Qt::gray);
		//for (int n=0; n<c; n++) p.drawLine(n, 0, n, c);
		//for (int n=0; n<c; n++) p.drawLine(w-n-1, 0, w-n-1, c);
		//for (int n=0; n<c; n++) p.drawLine(n, h-1, n, h-c-1);
		for (int n=0; n<handle_Size; n++) p.drawLine(w-n-1, h-1, w-n-1, h-handle_Size-1);
        //p.drawRect(icon_rect());
		p.end();
		icon->setPixmap(pixmap);
		highlighted = true;
	}
	else
	{
		highlighted = false;
	}
}


void iconf::setTHighlighted(bool state)
{
    if (state)
    {
        text->setStyleSheet("QLabel { background-color : grey; }");
        Thighlighted = true;
    }
    else
    {
        text->setStyleSheet(TextStyle);
        Thighlighted = false;
    }
}



void iconf::setVHighlighted(bool state)
{
    if (state)
    {
        value->setStyleSheet("QLabel { background-color : grey; }");
        Vhighlighted = true;
    }
    else
    {
        value->setStyleSheet(ValueStyle);
        Vhighlighted = false;
    }
}



void iconf::reloadpict()
{
	if (fileNotFound)
    {
        QString filename = path;
        QFile iconFile(filename);
        if (!iconFile.exists()) return;
        fileNotFound = false;
        pixmap.load(path);
	}
    if (pixmap.isNull()) return;
	if (keepRatio.isChecked())
	{
		int percent = iconsize.value();
		QSize S((pixmap.size().width() * percent) / 100, (pixmap.size().height() * percent) / 100);
		QTransform t;
		t.rotate(iconRotate.value());
        QPixmap temppixmap = pixmap.transformed(t).scaled(S);
        if (iconOpacity.value() < 100)
        {
            QPixmap opaquePix(temppixmap.size());
            opaquePix.fill(Qt::transparent);
            int opacity = iconOpacity.value() * 255 / 100;
            QPainter p(&opaquePix);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawPixmap(0, 0, temppixmap);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(opaquePix.rect(), QColor(0, 0, 0, opacity));
            p.end();
            temppixmap = opaquePix.scaled(S);
        }
        icon->resize(S);
        icon->setPixmap(temppixmap);
	}
	else
	{
		if (iconsize_X.value() && iconsize_Y.value())
		{
			QSize S(iconsize_X.value(), iconsize_Y.value());
			QTransform t;
			t.rotate(iconRotate.value());
            QPixmap temppixmap = pixmap.transformed(t).scaled(S);
            if (iconOpacity.value() < 100)
            {
                QPixmap opaquePix(temppixmap.size());
                opaquePix.fill(Qt::transparent);
                int opacity = iconOpacity.value() * 255 / 100;
                QPainter p(&opaquePix);
                p.setCompositionMode(QPainter::CompositionMode_Source);
                p.drawPixmap(0, 0, temppixmap);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.fillRect(opaquePix.rect(), QColor(0, 0, 0, opacity));
                p.end();
                temppixmap = opaquePix.scaled(S);
            }
            icon->resize(S);
            icon->setPixmap(temppixmap);
		}
	}
}





void iconf::iconrotate(int)
{
	reloadpict();
}



void iconf::iconrezise(int percent)
{
    if (!percent) return;
    reloadpict();
}


#if QT_VERSION < 0x060000
void iconf::textreweight(int fontweight)
{
    fontText.setWeight(fontweight);
    text->setFont(fontText);
    text->resize(text->sizeHint());
}
#else
void iconf::textreweight(QFont::Weight fontweight)
{
    fontText.setWeight(fontweight);
    text->setFont(fontText);
    text->resize(text->sizeHint());
}
#endif




void iconf::textrestretch(int stretch)
{
    fontText.setStretch(stretch);
    text->setFont(fontText);
	text->resize(text->sizeHint());
}




void iconf::valuerezise(int fontsize)
{
    //qDebug() << QString("set Value fontSize %1").arg(fontsize);
    disconnect(&valuesize, SIGNAL(valueChanged(int)), Parent, SLOT(valuerezise(int)));
    valuesize.setValue(fontsize);
    fontValue.setFamily(fontName.currentText());
    fontValue.setPointSize(fontsize);
    value->setFont(fontValue);
    value->resize(value->sizeHint());
    connect(&valuesize, SIGNAL(valueChanged(int)), Parent, SLOT(valuerezise(int)));
}




void iconf::textrezise(int fontsize)
{
    disconnect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
    textsize.setValue(fontsize);
    fontText.setFamily(fontName.currentText());
    fontText.setPointSize(fontsize);
    text->setFont(fontText);
    text->resize(text->sizeHint());
    connect(&textsize, SIGNAL(valueChanged(int)), Parent, SLOT(textrezise(int)));
}



#if QT_VERSION < 0x060000
void iconf::valuereweight(int fontweight)
{
    fontValue.setWeight(fontweight);
    value->setFont(fontValue);
    value->resize(value->sizeHint());
}
#else
void iconf::valuereweight(QFont::Weight fontweight)
{
    fontValue.setWeight(fontweight);
    value->setFont(fontValue);
    value->resize(value->sizeHint());
}
#endif




void iconf::valuerestretch(int stretch)
{
    fontValue.setStretch(stretch);
    value->setFont(fontValue);
	value->resize(value->sizeHint());
}




void iconf::changeText(const QString &txt)
{
    QString tr_txt = txt;
    Parent->parent->configwin->valueTranslate(tr_txt);
    text->setText(tr_txt);
	text->resize(text->sizeHint());
}



void iconf::specialText(const QString&)
{
	setvalue();
}



void iconf::settxt(QString name, QPoint pos)
{
	disconnect(&Text, SIGNAL(textChanged(const QString&)), this, SLOT(changeText(const QString&)));
	if (name.isEmpty()) name = cstr::toStr(cstr::NoName);
	text->setText(name);
	text->resize(text->sizeHint());
	text->move(pos);
	text->show();
	text->setAttribute(Qt::WA_DeleteOnClose);
	Text.setText(name);
	connect(&Text, SIGNAL(textChanged(const QString&)), this, SLOT(changeText(const QString&)));
}




void iconf::keepratiochanged(int state)
{
	if (state)
	{
		iconsize.setEnabled(true);
		iconsize_X.setEnabled(false);
		iconsize_Y.setEnabled(false);
	}
	else
	{
		iconsize.setEnabled(false);
		iconsize_X.setEnabled(true);
		iconsize_Y.setEnabled(true);
	}
    reloadpict();
}



void iconf::clickiconchange(int state)
{
	if (state)
	{
		iconchangerange.setEnabled(true);
		iconchangestep.setEnabled(true);
		iconchangeoffset.setEnabled(true);
	}
	else
	{
		iconchangerange.setEnabled(false);
		iconchangestep.setEnabled(false);
		iconchangeoffset.setEnabled(false);
	}
}




void iconf::setvalue(QString Romid, QPoint pos)
{
	value->move(pos);
    value->resize(value->sizeHint());
    value->show();
	value->setAttribute(Qt::WA_DeleteOnClose);
	romid = Romid;
}



void iconf::setvalue()
{
    switch (specialValue.currentIndex())
    {
        case Date :
        case Time :
        case UserDateTime : value->setText(QDateTime::currentDateTime().toString(specialValueParameters.text())); break;
        case Disabled :
            default :
            break;
    }
    value->resize(value->sizeHint());
    updateText();
}



void iconf::setvalue(onewiredevice *device)
{
    switch (specialValue.currentIndex())
    {
        case Date :
        case Time :
        case UserDateTime : value->setText(QDateTime::currentDateTime().toString(specialValueParameters.text())); break;
        case Disabled :
            default :
            if (device)
            {
                double v = device->getMainValue();
                //lastValue = v;
                if (binder)
                {
                    changeIcon(v);
                    //lastValue = v;
                }
                else
                {
                    if (!htmlCommand.text().isEmpty()) htmlCommandChanged();
                    changeIcon(v);
                    //lastValue = v;
                    value->setText(device->MainValueToStrNoWarning());
                    value->resize(value->sizeHint());
                    if (device->isValid())
                    {
                        value->setStyleSheet(ValueStyle);
                        CurentValueStyleleHex = ValueStyle;
                    }
                    else if (logisdom::isNotNA(v))
                    {
                        value->setStyleSheet("color:red");
                        CurentValueStyleleHex = "FF0000";
                    }
                    else
                    {
                        value->setStyleSheet(ValueStyle);
                        CurentValueStyleleHex = ValueStyle;
                    }
                }
                value->setToolTip(device->getSecondaryValue());
            }
            break;
    }
    value->resize(value->sizeHint());
    updateText();
    if (highlighted) setHighlighted(true);
    if (Thighlighted) setTHighlighted(true);
    if (Vhighlighted) setVHighlighted(true);
}




void iconf::updateText()
{
    QString tr_txt = Text.text();
    Parent->parent->configwin->valueTranslate(tr_txt);
    text->setText(tr_txt);
    text->resize(text->sizeHint());
}



void iconf::changeIcon(double value)
{
	QString file = path;
	if (file.isEmpty()) return;
	double max = iconchangerange.value();
	double step = iconchangestep.value();
	double offset = iconchangeoffset.value();
	int Step;
	if (value > max) Step = qRound((max - offset) / step);
		else Step = qRound((value - offset) / step);
	QString n = QString("_%1").arg(Step);
	if ((Step <= 0) or (iconchange.checkState() == Qt::Unchecked))
	{
        pixmap.load(file);
        if (pixmap.isNull())
		{
			if (maison1wirewindow->isRemoteMode())
			{
				fileNotFound = true;
				QString NameNoDir = file.remove(QString(repertoireicon) + QDir::separator());
				maison1wirewindow->RemoteConnection->addGetFiletoFifo(NameNoDir, repertoireicon);
				actualFileName.setText(file + " " + tr("not found will be downloaded"));
			}
			else actualFileName.setText(file + " " + tr("not found"));
			return;
		}
		actualFileName.setText(file);
		setHighlighted(highlighted);
	}
	else
	{
		QString N = QString("_%1").arg(Step);
		QString NFile = file.insert(path.length() - 4, N);
        pixmap.load(NFile);
        if (pixmap.isNull())
		{
			if (maison1wirewindow->isRemoteMode())
			{
				fileNotFound = true;
				QString NameNoDir = NFile.remove(QString(repertoireicon) + QDir::separator());
				maison1wirewindow->RemoteConnection->addGetFiletoFifo(NameNoDir, repertoireicon);
				actualFileName.setText(NFile + " " + tr("not found will be downloaded"));
			}
			else actualFileName.setText(NFile + " " + tr("not found"));
			return;
		}
		actualFileName.setText(NFile);
		setHighlighted(highlighted);
	}
}




QRect iconf::icon_rect()
{
#if QT_VERSION < 0x060000
    QPixmap pixmap = *icon->pixmap();
#else
    QPixmap pixmap = icon->pixmap();
#endif
	QSize s = pixmap.size();
	int w = s.width();
	int h = s.height();
	QRect rect = QRect(w-handle_Size, h-handle_Size, handle_Size, handle_Size);
	return rect;
}



void iconf::mouseResize(QMouseEvent *event)
{
	iconsize_X.setValue(event->pos().x() - icon->pos().x());
	iconsize_Y.setValue(event->pos().y() - icon->pos().y());
	if (keepRatio.isChecked())
	{
		int p = iconsize_X.value() * 100 / pixmap.size().width();
		iconsize.setValue(p);
	}
	else
	{
		reloadpict();
	}
}




void iconf::checkFile(bool &state)
{
	if (fileNotFound)
	{
		reloadpict();
		if (fileNotFound) state = true;
	}
}


void iconf::bigger()
{
    halfbigger = !halfbigger;
    if (highlighted)
    {
        int s = iconsize.value();
        s++;
        iconsize.setValue(s);
    }
    if (Vhighlighted && halfbigger)
    {
        int s = valuesize.value();
        s++;
        valuerezise(s);
    }
    if (Thighlighted && halfbigger)
    {
        int s = textsize.value();
        s++;
        textrezise(s);
    }
}



void iconf::smaller()
{
    halfbigger = !halfbigger;
    if (highlighted)
    {
        int s = iconsize.value();
        if (s > 0) s--;
        iconsize.setValue(s);
    }
    if (Vhighlighted && halfbigger)
    {
        int s = valuesize.value();
        if (s > 2) s--;
        valuerezise(s);
    }
    if (Thighlighted && halfbigger)
    {
        int s = textsize.value();
        if (s > 2) s--;
        textrezise(s);
    }
}

void iconf::resize(int size)
{
	int percent = iconsize.value();
	iconsize_X.setValue((pixmap.size().width() * size) / 100 * percent / 100);
	iconsize_Y.setValue((pixmap.size().height() * size) / 100 * percent / 100);
	reloadpict();
}



void iconf::getConfigStrIconPath(QString &str)
{
    str += "\n" Icon_Begin "\n";
    QFile iconFile (path);
    QFileInfo iconInfo(iconFile);
    QString iconPath = QString(repertoireicon) + QDir::separator() + iconInfo.fileName();
    str += logisdom::saveformat("IconFileName", iconPath);
    str += logisdom::saveformat("IconPos_X", QString("%1").arg(icon->x()));
    str += logisdom::saveformat("IconPos_Y", QString("%1").arg(icon->y()));
    str += logisdom::saveformat("Text", Text.text(), true);
    str += logisdom::saveformat("TextPos_X", QString("%1").arg(text->x()));
    str += logisdom::saveformat("TextPos_Y", QString("%1").arg(text->y()));
    str += logisdom::saveformat("Text_Red", QString("%1").arg(textColor.red()));
    str += logisdom::saveformat("Text_Green", QString("%1").arg(textColor.green()));
    str += logisdom::saveformat("Text_Blue", QString("%1").arg(textColor.blue()));
    str += logisdom::saveformat("Text_Color", colorToHex(textColor));
    str += logisdom::saveformat("ValuePos_X", QString("%1").arg(value->x()));
    str += logisdom::saveformat("ValuePos_Y", QString("%1").arg(value->y()));
    str += logisdom::saveformat("Value_Red", QString("%1").arg(valueColor.red()));
    str += logisdom::saveformat("Value_Green", QString("%1").arg(valueColor.green()));
    str += logisdom::saveformat("Value_Blue", QString("%1").arg(valueColor.blue()));
    str += logisdom::saveformat("Value_Color", colorToHex(valueColor));
    str += logisdom::saveformat("RomID", romid);
    str += logisdom::saveformat("IconSize", QString("%1").arg(iconsize.value()));
    str += logisdom::saveformat("iconsize_X", QString("%1").arg(iconsize_X.value()));
    str += logisdom::saveformat("iconsize_Y", QString("%1").arg(iconsize_Y.value()));
    str += logisdom::saveformat("iconRotate", QString("%1").arg(iconRotate.value()));
    str += logisdom::saveformat("iconOpacity", QString("%1").arg(iconOpacity.value()));
    str += logisdom::saveformat("TextFontSize", QString("%1").arg(fontText.pointSize()));
    str += logisdom::saveformat("TextWeight", QString("%1").arg(fontText.weight()));
    str += logisdom::saveformat("TextStretch", QString("%1").arg(textstretch.value()));
    str += logisdom::saveformat("ValueFontSize", QString("%1").arg(fontValue.pointSize()));
    str += logisdom::saveformat("ValueWeight", QString("%1").arg(fontValue.weight()));
    str += logisdom::saveformat("ValueStretch", QString("%1").arg(valuestretch.value()));
    str += logisdom::saveformat("FontName", fontName.currentText());
    if (NoIcon.isChecked()) str += logisdom::saveformat("NoIconEnabled", "1"); else str += logisdom::saveformat("NoIconEnabled", "0");
    if (NoText.isChecked()) str += logisdom::saveformat("NoTextEnabled", "1"); else str += logisdom::saveformat("NoTextEnabled", "0");
    if (NoValue.isChecked()) str += logisdom::saveformat("NoValueEnabled", "1"); else str += logisdom::saveformat("NoValueEnabled", "0");
    if (iconchange.isChecked()) str += logisdom::saveformat("IconChangeEnabled", "1"); else str += logisdom::saveformat("IconChangeEnabled", "0");
    if (keepRatio.isChecked()) str += logisdom::saveformat("keepRatioEnabled", "1"); else str += logisdom::saveformat("keepRatioEnabled", "0");
    str += logisdom::saveformat("IconChangeRange", QString("%1").arg(iconchangerange.value()));
    str += logisdom::saveformat("IconChangeStep", QString("%1").arg(iconchangestep.value()));
    str += logisdom::saveformat("IconChangeOffset", QString("%1").arg(iconchangeoffset.value()));
    str += logisdom::saveformat("SpecialIndex", QString("%1").arg(specialValue.currentIndex()));
    str += logisdom::saveformat("SpecialText", specialValueParameters.text());
    str += logisdom::saveformat("HtmlCommand", htmlCommand.text(), true);
    str += logisdom::saveformat("FormulaText", formulaText.text(), true);
    str += logisdom::saveformat("FormulaCalc", Formula->getFormula(), true);
    str += Icon_Finished;
    str += "\n";
}

void iconf::getConfigStr(QString &str)
{
	str += "\n" Icon_Begin "\n";
	str += logisdom::saveformat("IconFileName", path);
	str += logisdom::saveformat("IconPos_X", QString("%1").arg(icon->x()));
	str += logisdom::saveformat("IconPos_Y", QString("%1").arg(icon->y()));
    str += logisdom::saveformat("Text", Text.text(), true);
	str += logisdom::saveformat("TextPos_X", QString("%1").arg(text->x()));
	str += logisdom::saveformat("TextPos_Y", QString("%1").arg(text->y()));
    str += logisdom::saveformat("Text_Red", QString("%1").arg(textColor.red()));
    str += logisdom::saveformat("Text_Green", QString("%1").arg(textColor.green()));
    str += logisdom::saveformat("Text_Blue", QString("%1").arg(textColor.blue()));
    str += logisdom::saveformat("Text_Color", colorToHex(textColor));
    str += logisdom::saveformat("ValuePos_X", QString("%1").arg(value->x()));
	str += logisdom::saveformat("ValuePos_Y", QString("%1").arg(value->y()));
    str += logisdom::saveformat("Value_Red", QString("%1").arg(valueColor.red()));
    str += logisdom::saveformat("Value_Green", QString("%1").arg(valueColor.green()));
    str += logisdom::saveformat("Value_Blue", QString("%1").arg(valueColor.blue()));
    str += logisdom::saveformat("Value_Color", colorToHex(valueColor));
    str += logisdom::saveformat("RomID", romid);
	str += logisdom::saveformat("IconSize", QString("%1").arg(iconsize.value()));
	str += logisdom::saveformat("iconsize_X", QString("%1").arg(iconsize_X.value()));
	str += logisdom::saveformat("iconsize_Y", QString("%1").arg(iconsize_Y.value()));
	str += logisdom::saveformat("iconRotate", QString("%1").arg(iconRotate.value()));
	str += logisdom::saveformat("iconOpacity", QString("%1").arg(iconOpacity.value()));
    str += logisdom::saveformat("TextFontSize", QString("%1").arg(fontText.pointSize()));
    str += logisdom::saveformat("TextWeight", QString("%1").arg(fontText.weight()));
    str += logisdom::saveformat("TextStretch", QString("%1").arg(textstretch.value()));
    str += logisdom::saveformat("ValueFontSize", QString("%1").arg(fontValue.pointSize()));
    str += logisdom::saveformat("ValueWeight", QString("%1").arg(fontValue.weight()));
    str += logisdom::saveformat("ValueStretch", QString("%1").arg(valuestretch.value()));
    str += logisdom::saveformat("FontName", fontName.currentText());
    if (NoIcon.isChecked()) str += logisdom::saveformat("NoIconEnabled", "1"); else str += logisdom::saveformat("NoIconEnabled", "0");
	if (NoText.isChecked()) str += logisdom::saveformat("NoTextEnabled", "1"); else str += logisdom::saveformat("NoTextEnabled", "0");
	if (NoValue.isChecked()) str += logisdom::saveformat("NoValueEnabled", "1"); else str += logisdom::saveformat("NoValueEnabled", "0");
	if (iconchange.isChecked()) str += logisdom::saveformat("IconChangeEnabled", "1"); else str += logisdom::saveformat("IconChangeEnabled", "0");
	if (keepRatio.isChecked()) str += logisdom::saveformat("keepRatioEnabled", "1"); else str += logisdom::saveformat("keepRatioEnabled", "0");
	str += logisdom::saveformat("IconChangeRange", QString("%1").arg(iconchangerange.value()));
	str += logisdom::saveformat("IconChangeStep", QString("%1").arg(iconchangestep.value()));
	str += logisdom::saveformat("IconChangeOffset", QString("%1").arg(iconchangeoffset.value()));
	str += logisdom::saveformat("SpecialIndex", QString("%1").arg(specialValue.currentIndex()));
	str += logisdom::saveformat("SpecialText", specialValueParameters.text());
    str += logisdom::saveformat("HtmlCommand", htmlCommand.text(), true);
    str += logisdom::saveformat("FormulaText", formulaText.text(), true);
    str += logisdom::saveformat("FormulaCalc", Formula->getFormula(), true);
    str += Icon_Finished;
	str += "\n";
}



void iconf::setConfig(const QString &strsearch)
{
	bool ok_x, ok_y, ok;
	int x, y, s;
	double v;
	QString ReadRomID, ReadName, txt;
	if (QDir::separator() == '/') ReadName = logisdom::getvalue("IconFileName", strsearch).replace("\\", QDir::separator());
	else ReadName = logisdom::getvalue("IconFileName", strsearch).replace("/", QDir::separator());
	if (maison1wirewindow->isRemoteMode())
	{
                QString filename = ReadName;
                QFile iconFile(filename);
		QString NameNoDir = filename.remove(QString(repertoireicon) + QDir::separator());
		if (!iconFile.exists())
		{
			fileNotFound = true;
			path = ReadName;
			maison1wirewindow->RemoteConnection->addGetFiletoFifo(NameNoDir, repertoireicon);
		}
	}
	x = logisdom::getvalue("IconPos_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("IconPos_Y", strsearch).toInt(&ok_y, 10);
	if (ok_x && ok_y)
	{
		setpict(ReadName, QPoint(x, y));
	}
	QString name = logisdom::getvalue("Text", strsearch);
	x = logisdom::getvalue("TextPos_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("TextPos_Y", strsearch).toInt(&ok_y, 10);
	if (ok_x && ok_y)
	{
		settxt(name, QPoint(x, y));
	}
	x = logisdom::getvalue("ValuePos_X", strsearch).toInt(&ok_x, 10);
	y = logisdom::getvalue("ValuePos_Y", strsearch).toInt(&ok_y, 10);
	if (ok_x && ok_y)
	{
		ReadRomID = logisdom::getvalue("RomID", strsearch);
		setvalue(ReadRomID, QPoint(x, y));
	}
	s = logisdom::getvalue("IconSize", strsearch).toInt(&ok, 10);
	if (ok) iconsize.setValue(s);
	s = logisdom::getvalue("iconsize_X", strsearch).toInt(&ok, 10);
	if (ok) iconsize_X.setValue(s);
	s = logisdom::getvalue("iconsize_Y", strsearch).toInt(&ok, 10);
	if (ok) iconsize_Y.setValue(s);
	s = logisdom::getvalue("iconRotate", strsearch).toInt(&ok, 10);
	if (ok) iconRotate.setValue(s);
	s = logisdom::getvalue("iconOpacity", strsearch).toInt(&ok, 10);
	if (ok) iconOpacity.setValue(s);
	s = logisdom::getvalue("keepRatioEnabled", strsearch).toInt(&ok, 10);
	if (ok && (s == 0)) keepRatio.setChecked(false); else keepRatio.setChecked(true);
    { int R = 0, G = 0, B = 0;
    s = logisdom::getvalue("Text_Red", strsearch).toInt(&ok, 10);
    if (ok) R = s;
    if (R > 255) R = 255;
    if (R < 0) R = 0;
    textColor.setRed(R);
    s = logisdom::getvalue("Text_Green", strsearch).toInt(&ok, 10);
    if (ok) G = s;
    if (G > 255) G = 255;
    if (G < 0) G = 0;
    textColor.setGreen(G);
    s = logisdom::getvalue("Text_Blue", strsearch).toInt(&ok, 10);
    if (ok) B = s;
    if (B > 255) B = 255;
    if (B < 0) B = 0;
    textColor.setBlue(B);
    }

    QColor color;
    QString hexColor = logisdom::getvalue("Text_Color", strsearch);
    if (!hexColor.isEmpty())
        if (hexToColor(hexColor, color)) textColor = color;
    setTextStyle(textColor);

    { int R = 0, G = 0, B = 0;
    s = logisdom::getvalue("Value_Red", strsearch).toInt(&ok, 10);
    if (ok) R = s;
    if (R > 255) R = 255;
    if (R < 0) R = 0;
    valueColor.setRed(R);
    s = logisdom::getvalue("Value_Green", strsearch).toInt(&ok, 10);
    if (ok) G = s;
    if (G > 255) G = 255;
    if (G < 0) G = 0;
    valueColor.setGreen(G);
    s = logisdom::getvalue("Value_Blue", strsearch).toInt(&ok, 10);
    if (ok) B = s;
    if (B > 255) B = 255;
    if (B < 0) B = 0;
    valueColor.setBlue(B);
    }

    hexColor = logisdom::getvalue("Value_Color", strsearch);
    if (!hexColor.isEmpty())
        if (hexToColor(hexColor, color)) valueColor = color;
    setValueStyle(valueColor);

    s = logisdom::getvalue("ValueFontSize", strsearch).toInt(&ok, 10);
    if (ok) valuerezise(s);
    s = logisdom::getvalue("TextFontSize", strsearch).toInt(&ok, 10);
    if (ok) textrezise(s);

    s = logisdom::getvalue("ValueStretch", strsearch).toInt(&ok, 10);
    if (!ok) s = 100;
    if (s < 2) s = 100;
    valuestretch.setValue(s);
    valuerestretch(s);

    s = logisdom::getvalue("TextStretch", strsearch).toInt(&ok, 10);
    if (!ok) s = 100;
    if (s < 2) s = 100;
    textstretch.setValue(s);
    textrestretch(s);

    s = logisdom::getvalue("NoIconEnabled", strsearch).toInt(&ok, 10);
	if (ok && (s == 1)) NoIcon.setChecked(true); else NoIcon.setChecked(false);
	s = logisdom::getvalue("NoTextEnabled", strsearch).toInt(&ok, 10);
	if (ok && (s == 1)) NoText.setChecked(true); else NoText.setChecked(false);
	s = logisdom::getvalue("NoValueEnabled", strsearch).toInt(&ok, 10);
	if (ok && (s == 1)) NoValue.setChecked(true); else NoValue.setChecked(false);
	s = logisdom::getvalue("IconChangeEnabled", strsearch).toInt(&ok, 10);
	if (ok && (s == 1)) iconchange.setChecked(true); else iconchange.setChecked(false);
	v = logisdom::getvalue("IconChangeRange", strsearch).toDouble(&ok);
	if (ok) iconchangerange.setValue(v);
	v = logisdom::getvalue("IconChangeStep", strsearch).toDouble(&ok);
	if (ok) iconchangestep.setValue(v);
	v = logisdom::getvalue("IconChangeOffset", strsearch).toDouble(&ok);
	if (ok) iconchangeoffset.setValue(v);
	s = logisdom::getvalue("SpecialIndex", strsearch).toInt(&ok);
	if (ok) specialValue.setCurrentIndex(s);
	txt = logisdom::getvalue("SpecialText", strsearch);
	if (!txt.isEmpty()) specialValueParameters.setText(txt);
    txt = logisdom::getvalue("FormulaText", strsearch);
    if (!txt.isEmpty()) formulaText.setText(txt);
    txt = logisdom::getvalue("FormulaCalc", strsearch);
    if (!txt.isEmpty()) Formula->setFormula(txt);
    txt = logisdom::getvalue("HtmlCommand", strsearch);
    if (!txt.isEmpty())
    {
        htmlCommand.setText(txt);
        htmlCommandChanged();
    }
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
    reloadpict();
}


