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



#ifndef ERRORLOG_H
#define ERRORLOG_H
#include <QtGui>
#include <logisdom.h>
#include <ui_errlog.h>


class errlog : public QWidget
{
	Q_OBJECT
enum ErrorLevel
{
    ErrorLogOnly,
    LogOnly,
    LogAndShow,
    ErrorWarn,
    ErrorWarnAlarm,
    ErrorCritical
};
public:
    errlog(logisdom *Parent, QString name);
    ~errlog();
    Ui::errorlog ui;
    logisdom *parent;
    void AddMsg(const QString &Msg);
    QString SetError(int ErrID, const QString &Msg);
    void setLogFileName(const QString &Name);
    void getCfgStr(QString &str);
    void setCfgStr(const QString &strsearch);
    bool isFlagged();
    bool isCritical();
    bool beepOnErrors();
    QMutex logMutex;
    QString soundPath;
private:
    bool AlarmFlag, AlarmCritical;
    QString filename;
    void raiseError();
    QString appendStr;
    QTimer updateTimer;
private slots:
    void effacer();
    void chooseSound();
    void updateMsg();
public slots:
signals:
	void alarmChange(errlog* me);
};

#endif
