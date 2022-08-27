#ifndef FILESAVE_H
#define FILESAVE_H

#include <QThread>
#include <QMutex>

class fileSave : public QThread
{
    Q_OBJECT
public:
    fileSave();
    ~fileSave();
    void run();
    void appendData(QString fileName, QString data);
    QMutex dataLocker;
    QString repertoiredat;
private:
    QStringList fileNames, Datas;
};

#endif // FILESAVE_H
