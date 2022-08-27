#ifndef SENDSMSTHREAD_H
#define SENDSMSTHREAD_H

#include <QThread>
#include <QMutex>
#include "mailsender.h"
#include "mail/sender.h"

class sendSmsThread : public QThread
{
    Q_OBJECT
public:

struct smsContent
{
    QString msg;
    int attempt = 0;
};


sendSmsThread();
    ~sendSmsThread();
    void run();
    void appendSms(QString);
    QList <smsContent*> smsList;
    QList <smsContent*> smsFailed;
    QMutex dataLocker;
    bool send(smsContent*);
signals:
    void logMessage(QString);
};

#endif // SENDSMSTHREAD_H
