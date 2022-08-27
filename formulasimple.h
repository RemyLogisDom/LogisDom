#ifndef __FORMULASIMPLE_H
#define __FORMULASIMPLE_H
#include <QtGui>
#include "calcthread.h"
#include "logisdom.h"
#include "ui_formula.h"

class onewiredevice;


class formulasimple : public QWidget
{
friend class calcthread;
friend class formula;
        Q_OBJECT
public:
	formulasimple(logisdom *Parent);
        ~formulasimple();
	calcthread *calcth;
	onewiredevice *deviceParent;
	QElapsedTimer CalcTimer;
	int CalcTime;
	QString getFormula();
	QString startTxt, stopTxt;
	QString startCalc, cancelCalc;
	void setFormula(QString F);
	double Calculate(double target = logisdom::NA);
	double getValue();
	void stopAll();
	void removeButtons();
	bool calcRequest;
	Ui::formula_ui ui;
	QDateTime lastCalc;
	bool reprocessEnabled;
public slots:
	void CalculateThread();
	void CalculateThreadRequest();
	void CalcRequest();
	void ClickClacl();
	void CalcEnd();
	void setName(QString name);
private:
	QMutex SenderMutex, MutexThreadRequest;
	QStringList Operator, OpDescription;
	QString Calc;
	int widgetCount;
	QListWidgetItem ** widgetList;
	int onetwo;
	QTimer timerCalc;
	logisdom *parent;
private slots:
	void rightclicklList(const QPoint & pos);
	void rowchange(int index);
	void insert(QModelIndex index);
	void showOp();
	void showDev();
	void changeFamily(int);
signals:
	void calcdone();
};


#endif




