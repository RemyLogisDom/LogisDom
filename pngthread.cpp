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





#include <QPixmap>
#include "pngthread.h"



pngthread::pngthread()
{
}




pngthread::~pngthread()
{
}



void pngthread::appendData(QString fileName, const QPixmap &png)
{
    dataLocker.lock();
    int i = FileNameList.indexOf(fileName);
    if (i != -1)
    {
        QPixmap *oldpict = pngList.at(i);
        QPixmap *pict = new QPixmap(png);
        pngList.replace(i, pict);
        delete oldpict;
    }
    else
    {
        FileNameList.append(fileName);
        QPixmap *pict = new QPixmap(png);
        pngList.append(pict);
    }
    //qDebug() << "append : " + fileName;
    dataLocker.unlock();
}


void pngthread::run()
{
	while (!FileNameList.isEmpty())
	{
        dataLocker.lock();
        QString filePathName = FileNameList.first();
        QPixmap *pict = pngList.first();
        FileNameList.removeFirst();
        pngList.removeFirst();
        dataLocker.unlock();
        pict->save(filePathName, "PNG");
        //qDebug() << "save : " + filePathName;
        delete pict;
	}
}


