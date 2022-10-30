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



#ifndef INTERVAL_H
#define INTERVAL_H
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtCore>
#include <QtGui>

class interval : public QWidget
{
	Q_OBJECT
enum mode { M1, M2, M5, M10, M15, M20, M30, H1, H2, H3, H4, H6, H12, D1, D2, D5, D10, W1, W2, MT, AUTO, lastInterval };
public:	
	interval();
	~interval();
	QString getStrMode(int mode);
	QGridLayout setupLayout;
	QCheckBox SaveEnable;
	QComboBox Type;
	QDateTimeEdit nextOne;
	void setName(QString name);
	bool enabled;
	void setEnabled(bool state);
    bool checkOnly();
    bool isitnow();
    bool isAutoSave();
    void setToAuto();
    qint64 getSecs(const QDateTime &T);
    qint64 getSecs();
    void setNext(const QDateTime &T);
	QString getNext();
	void setMode(int index);
	QString getMode();
	void setSaveMode(bool state);
	QString getSaveMode();
    void enableYear();
private slots:
	void changeEnable(int state);
    void stateChanged();
private:
    bool yearEnable = false;
signals:
    void readChanged();
};

#endif
