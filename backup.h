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


#ifndef BACKUP_H
#define BACKUP_H

#include <QThread>
#include <QMutex>


class backup : public QThread
{
    Q_OBJECT
#define backupDatFolder "dat"
#define backupZipFolder "zip"
#define backupIconFolder "png"
#define logFileName "backup.log"
public:
    backup();
    ~backup();
    void run();
    void saveFolder(QString source, QString destination, QString extension);
    void saveFile(QString source, QString destination);
    void makeDateIdentical(QString source, QString destination);
    bool abort = false;
    QString datFolder;
    QString zipFolder;
    QString iconFolder;
    QString configFileName;
    QString backupFolder;
    void log(QString logStr);
};

#endif // BACKUP_H
