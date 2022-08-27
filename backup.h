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
