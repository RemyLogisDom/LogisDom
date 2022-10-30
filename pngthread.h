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



#ifndef PNGTHREAD_H
#define PNGTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>

class pngthread : public QThread
{
    Q_OBJECT
public:
	pngthread();
	~pngthread();
	void run();
    void appendData(QString, const QPixmap &);
    QMutex dataLocker;
    QList <QPixmap*> pngList;
	QStringList FileNameList;
private:
signals:
private slots:
};

#endif // PNGTHREAD_H
