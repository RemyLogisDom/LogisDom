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



#ifndef ICONEAREA_H
#define ICONEAREA_H
#include <QFrame>
#include <QLabel>
#include <QRubberBand>
#include <QTextStream>

class QDragEnterEvent;
class QDropEvent;
class onewiredevice;
class logisdom;
class htmlBinder;
class iconf;
class icont;
#define extrapix 10

class IconeArea : public QWidget
{
	Q_OBJECT
public:
	IconeArea(logisdom *Parent);
	~IconeArea();
	logisdom *parent;
	QList <iconf*> IconList;
    QList <icont*> TextList;
    void readconfigfile(QByteArray &configdata);
    void appendconfigfile(QByteArray &configdata);
    void SaveConfigStr(QString &str);
    void SaveConfigStrIconPath(QString &str);
    void SaveSelectionConfigStr(QString &str);
    void getHtml(QString &html);
    void getHtml(QTextStream &out);
    void getIconHtml(QTextStream &, iconf *, int);
    void getIconHtmlLink(QTextStream &, iconf *, int);
    void getIconHtmlCommand(QTextStream &, iconf *, int);
    void getTextHtml(QTextStream &, iconf *, int);
    void getValueHtml(QTextStream &, iconf *, int);
    void iconmenu(int areaindex, QMouseEvent *event);
	void textmenu(QLabel *label, QMouseEvent *event);
	iconf *newIcon(QWidget *parent);
    icont *newText(QWidget *parent);
    void checkFile(bool &state);
	void setLockedState(bool state);
	void Capteur(iconf *icon);
	void iconAlignLeft();
	void iconAlignRight();
	void iconAlignTop();
	void iconAlignBottom();
	void iconCopysize();
	void iconCopy(QString &str);
	void iconPaste(const QString &str);
	void iconUndo();
	void iconRedo();
	void iconAddOne();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void paste(QKeyEvent *);
    void selectAll();
    void setHtmlEnabled(bool state);
    bool isHtmlEnabled();
    bool hasSelection();
    void clearSelection();
    void sendPressEvent(QKeyEvent *event);
    void mouseWheel(QWheelEvent *event);
    int mouseWheelDelta;
    void setValue();
    void AddTextZone();
    void setHighlighted(QRect &area);
    QColor backgroundColor;
    static QString colorToHex(QColor &color);
    static bool hexToColor(QString &hex, QColor &color);
    QString getBackGroundColor();
    void setBackGroundColor(QColor);
    void VoirCapteur(QString romID);
private:
	bool locked;
    bool htmlEnabled;
	int resize_Icon;
	QPoint startSelectionPoint;
	QPoint endSelectionPoint;
    QPoint click;
    QRubberBand *rubberband;
	bool rubberBandActive;
	QLabel *childClicked;
	void VoirCapteur(int areaindex);
	void Capteur(int areaindex);
    void Dupliquer(int);
    void DupliquerwFormula(int);
    void Renommer(int areaindex);
    void ShowFormula(int areaindex);
    void Supprimer(int areaindex);
    void changeIcon(int areaindex);
	iconf *lastClicked;
	QStringList undo;
	int undoIndex;
	bool captureUndo;
private slots:
	void PushUndo();
protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
public slots:
    void setValue(onewiredevice *device);
	void setAsBackground(iconf *icon);
	void setInFront(iconf *icon);
    void setInFront(icont *icon);
    void newFont(QString font);
    void valuerezise(int fontsize);
    void textrezise(int fontsize);
};


#endif
