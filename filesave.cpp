#include <QtCore>
#include "filesave.h"

fileSave::fileSave()
{
}


fileSave::~fileSave()
{
}



void fileSave::appendData(QString fileName, QString data)
{
    dataLocker.lock();
    fileNames.append(fileName);
    Datas.append(data);
    dataLocker.unlock();
}



void fileSave::run()
{
    while(!Datas.isEmpty())
    {
        if (!QDir().exists(repertoiredat))
        {

            //qDebug() << "Try to create Dir " + repertoiredat;
            if (!QDir().mkdir(repertoiredat))
            {
                //qDebug() << "Cannot create Dir " + repertoiredat;
                return;
            }
            //qDebug() << "Dir " + repertoiredat + " created";
        }
        dataLocker.lock();
        QString filename = repertoiredat + QDir::separator() + fileNames.first();
        fileNames.removeFirst();
        QString data = Datas.first();
        Datas.removeFirst();
        dataLocker.unlock();
        QFile file(filename);
        if (!file.exists())
        {
            // create new file
            if(file.open(QIODevice::Append | QIODevice::Text))
            {
                //qDebug() << "Make new file " + filename;
                QTextStream out(&file);
                out << "// Version 1\n";
                out << data;
                file.close();
            }
        }
        else
        {
            if(file.open(QIODevice::Append | QIODevice::Text))
            {
                QTextStream out(&file);
                out << data;
                file.close();
            }
        }
    }
}

