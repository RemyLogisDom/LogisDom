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



#ifndef ICONF_H
#define ICONF_H
#include <QtWidgets/QLineEdit>
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


class iconf : public QWidget
{
	Q_OBJECT
enum specialValuesIndex
	{	Disabled, Time, Date, UserDateTime	};
#define handle_Size 10
public:	
	iconf(QWidget *parent, IconeArea *lparent);
	~iconf();
	IconeArea *Parent;
	void getConfigStr(QString &str);
    void getConfigStrIconPath(QString &str);
    void setConfig(const QString &strsearch);
	void setpict(QString pict, QPoint pos);
	void settxt(QString name, QPoint pos);
	void setvalue(QString Romid, QPoint pos);
    void setvalue(onewiredevice *device);
    void setvalue();
    void updateText();
    void changeIcon(double value);
	void reloadpict();
	void setHighlighted(bool state);
    void setTHighlighted(bool state);
    void setVHighlighted(bool state);
    void showFormula();
	void resize(int size);
    void bigger();
    bool halfbigger;
    void smaller();
    void mouseResize(QMouseEvent *event);
	bool fileNotFound;
	QRect icon_rect();
    QLabel *icon;
    QLabel *text;
    QLabel *value;
    QColor valueColor;
    QColor textColor;
    QString romid;
	QString path;
    QPixmap pixmap;
    htmlBinder *binder;
    QString colorHex(QColor &color);
    static QString colorToHex(QColor &color);
    static bool hexToColor(QString &hex, QColor &color);
    QString ValueStyle, TextStyle, ValueStyleHex, TextStyleHex, CurentValueStyleleHex;
    void setValueStyle(QColor &);
    void setTextStyle(QColor &);
    QString colorToHtmlHex(QColor &);
    bool highlighted, Thighlighted, Vhighlighted;
	void checkFile(bool &state);
    int getDisplayMode();
    QString getDisplayModeStr();
    enum affichageSpecial{ DeviceMode, TimeMode, DateMode, UserDateTimeMode };
// palette setup
	QWidget setup;
	QGridLayout setupLayout;
	QCheckBox keepRatio;
	QSpinBox iconsize;
	QSpinBox iconsize_X;
	QSpinBox iconsize_Y;
	QSpinBox iconRotate;
	QSpinBox iconOpacity;
	QCheckBox NoIcon;
	QCheckBox NoText;
	QCheckBox NoValue;
	QCheckBox iconchange;
	QDoubleSpinBox iconchangerange;
	QDoubleSpinBox iconchangestep;
	QDoubleSpinBox iconchangeoffset;
    QComboBox fontName;
    QFont fontText;
    QFont fontValue;
    QLineEdit Text;
	QLabel actualFileName;
    QSpinBox textsize;
	QSpinBox textstretch;
	QSpinBox valuesize;
	QSpinBox valuestretch;
	QComboBox specialValue;
	QLineEdit specialValueParameters;
    QLineEdit htmlCommand;
	QPushButton chooseDevice;
	QPushButton resetPosition;
	QPushButton setAsBgr;
	QPushButton setInFrt;
    QPushButton setTextColor;
    QPushButton setValueColor;
    QPushButton showDevice;
// palette stat setup
    QWidget stat_setup;
    QGridLayout stat_setupLayout;
    formula *Formula;
    QLineEdit formulaText;
    QSpinBox Decimal;
    double calc_Result;
public slots:
    void valuerezise(int fontsize);
    void textrezise(int fontsize);
    void setFont(const QString &font);
private slots:
	void iconrotate(int angle);
	void iconrezise(int percent);
#if QT_VERSION < 0x060000
    void textreweight(int);
    void valuereweight(int fontweight);
#else
    void textreweight(QFont::Weight);
    void valuereweight(QFont::Weight);
#endif
	void textrestretch(int stretch);
	void changeText(const QString &txt);
    void specialText(const QString &txt);
	void valuerestretch(int stretch);
	void clickiconchange(int state);
	void keepratiochanged(int state);
	void NoITVChange(int);
	void specialValueChanged(int);
    void fontChanged(const QString &);
    void chooseDev();
    void showDev();
    void resetPos();
	void lowerIcon();
	void higherIcon();
    void changeTextColor();
    void changeValueColor();
    void clacdone();
    void valueChanged(QString);
    void valueBinderChanged();
    void htmlCommandChanged();
private:
	QString channel;
	bool textvisible;
	bool valuevisible;
signals:
	void setAsBackground(iconf*);
	void setInFront(iconf*);
    void newFont(QString);
};

#endif
