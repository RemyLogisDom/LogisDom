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



#ifndef FORMULA_H
#define FORMULA_H
#include <QtGui>
#include "calcthread.h"
#include "logisdom.h"
#include "highlighter.h"
#include "reprocessthread.h"
#include "textedit.h"
#include "ui_formula.h"

class onewiredevice;


class formula : public QWidget
{
friend class calcthread;
	Q_OBJECT
public:
	formula(logisdom *Parent);
	~formula();
    reprocessthread *reprossthread = nullptr;
    calcthread *calcth = nullptr;
    QThread *thread = nullptr;
    QThread *threadr = nullptr;
    Highlighter *highlighter = nullptr;
    QTimer progressTimer;
	void stopreProcess();
	void stopAll();
	onewiredevice *deviceParent;
	QElapsedTimer CalcTimer;
    qint64 CalcTime;
	QString getFormula();
	QString startTxt, stopTxt;
	QString startCalc, cancelCalc;
    void setFormula(QString F);
	double Calculate(double target = logisdom::NA);
	double getValue();
	void removeButtons();
	Ui::formula_ui ui;
	QDateTime lastCalc;
	bool reprocessEnabled;
	logisdom *parent;
    double getOnErrorValue();
    QString getOnErrorTxtValue();
private:
	QMutex SenderMutex, MutexThreadRequest;
	QStringList Operator, OpDescription;
	QString Calc;
	int widgetCount;
	QListWidgetItem ** widgetList;
	int onetwo;
	QTimer timerCalc;
public slots:
	void CalculateThreadRequest(onewiredevice*);
	void reprocessEnd();
	void CalculateThread();
	void CalcRequest();
	void ClickClacl();
    void calcFinished(const QString &str);
    void redirectHttp(const QString &str);
    void setName(QString name);
private slots:
	void progressdisplay();
	void reprocess();
	void reProcessEnd();
	void rightclicklList(const QPoint & pos);
	void rowchange(int index);
	void insert(QModelIndex index);
	void showOp();
	void showDev();
    void getDeviceName();
    void getDeviceRomID();
    void changeFamily(int);
signals:
	void reprocessdone();
	void calcdone();
};


#endif



