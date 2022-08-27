#include <QtCore>
#include "backup.h"
#include "logisdom.h"
#ifdef Q_OS_LINUX
    #include <utime.h>
    #include <sys/stat.h>
#endif

backup::backup()
{
}


backup::~backup()
{
}




void backup::run()
{
    quit();
    abort = false;
    int delay = 20;
    while(delay-- > 0)
    {
        msleep(100);
        if (abort) return;
    }
    if (!QDir().exists(backupFolder))
        if (!QDir().mkdir(backupFolder)) return;
    QFile::remove(backupFolder + logFileName);
    if (!datFolder.isEmpty()) saveFolder(datFolder, backupFolder + backupDatFolder, "dat");
    if (!zipFolder.isEmpty())
    {
        if (zipFolder == datFolder) saveFolder(datFolder, backupFolder + backupZipFolder, "zip");
        else saveFolder(zipFolder, backupFolder + backupZipFolder, "zip");
    }
    if (!iconFolder.isEmpty()) saveFolder(iconFolder, backupFolder + backupIconFolder, "png");
    saveFile(configFileName, backupFolder + configFileName);
    datFolder.clear();
    zipFolder.clear();
    iconFolder.clear();
}



void backup::saveFile(QString sourceFileName, QString destinationFileName)
{
    if (abort) return;
    QFile sourceFile(sourceFileName);
    QFileInfo sourceInfo(sourceFile);
    QFile destinationFile(destinationFileName);
    if (destinationFile.exists())
    {
        QFileInfo destinationInfo(destinationFile);
        if (sourceInfo.fileTime(QFileDevice::FileModificationTime).secsTo(destinationInfo.fileTime(QFileDevice::FileModificationTime)) < 0)
        {
            QFile::remove(destinationFileName);
            if (sourceFile.copy(destinationFileName))
            {
                makeDateIdentical(sourceFileName, destinationFileName);
                log(sourceFileName + " overwrite " + destinationFileName);
            }
            else log(sourceFileName + " not possible to copy to " + destinationFileName);
        }
        else
        {
            if (sourceInfo.fileTime(QFileDevice::FileModificationTime).secsTo(destinationInfo.fileTime(QFileDevice::FileModificationTime)) == 0)
                log(sourceFileName + " skipped " + destinationFileName + " no change");
            else
                log(sourceFileName + " skipped " + destinationFileName + " is newer");
        }
    }
    else
    {
        if (sourceFile.copy(destinationFileName))
        {
            makeDateIdentical(sourceFileName, destinationFileName);
            log(sourceFileName + " copied to " + destinationFileName);
        }
        else log(sourceFileName + " not possible to copy to " + destinationFileName);
    }
}




void backup::saveFolder(QString source, QString destination, QString extension)
{
    if (abort) return;
    QDir datDir = QDir(source);
    QDir backupDir = QDir(destination);
    if (!QDir().exists(destination))
        if (!QDir().mkdir(destination)) return;
    if (datDir.exists())
    {
        QStringList datfileslist = datDir.entryList(QDir::Files);
        for (int n=0; n<datfileslist.count(); n++)
        {
            QString sourceFileName = source + datfileslist[n];
            QFile sourceFile(sourceFileName);
            QFileInfo sourceInfo(sourceFile);
            if (sourceInfo.suffix() == extension)
            {
                QString destinationFileName = destination + QDir::separator() + datfileslist[n];
                QFile destinationFile(destinationFileName);
                if (destinationFile.exists())
                {
                    QFileInfo destinationInfo(destinationFile);
                    if (sourceInfo.fileTime(QFileDevice::FileModificationTime).secsTo(destinationInfo.fileTime(QFileDevice::FileModificationTime)) < 0)
                    {
                        QFile::remove(destinationFileName);
                        if (sourceFile.copy(destinationFileName))
                        {
                            makeDateIdentical(sourceFileName, destinationFileName);
                            log(sourceFileName + " overwrite " + destinationFileName);
                        }
                        else log(sourceFileName + " not possible to copy to " + destinationFileName);
                    }
                    else
                    {
                        if (sourceInfo.fileTime(QFileDevice::FileModificationTime).secsTo(destinationInfo.fileTime(QFileDevice::FileModificationTime)) == 0)
                            log(sourceFileName + " skipped " + destinationFileName + " no change");
                        else
                            log(sourceFileName + " skipped " + destinationFileName + " is newer");
                    }
                }
                else
                {
                    if (sourceFile.copy(destinationFileName))
                    {
                        makeDateIdentical(sourceFileName, destinationFileName);
                        log(sourceFileName + " copied to " + destinationFileName);
                    }
                    else log(sourceFileName + " not possible to copy to " + destinationFileName);
                }
            }
            if (abort) break;
        }
    }
}




void backup::makeDateIdentical(QString sourceFileName, QString destinationFileName)
{
#ifdef Q_OS_LINUX       // make file date identical
    struct utimbuf timebuf;
    //stat(sourceFileName.toLocal8Bit(), &statbuf);
    QFileInfo info(sourceFileName);
    timebuf.actime = info.lastRead().toSecsSinceEpoch();
    timebuf.modtime = info.lastModified().toSecsSinceEpoch();
    utime(destinationFileName.toLocal8Bit(), &timebuf);
#endif
}


void backup::log(QString logStr)
{
    if (!logStr.isEmpty())
    {
        QFile logFile(backupFolder + logFileName);
        if(logFile.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&logFile);
            out << logStr + "\n";
            logFile.close();
        }
    }
}
