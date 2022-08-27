#ifndef SENDMAILTHREAD_H
#define SENDMAILTHREAD_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include "mailsender.h"
#include "mail/sender.h"

class sendMailThread : public QThread
{
    Q_OBJECT
public:

struct eMailContent
{
    QString destinataire;
    QString nom;
    QString objet;
    QString text;
    int attempt = 0;
};

sendMailThread();
    ~sendMailThread();
    void run();
    void appendeMail(eMailContent*);
    QList <eMailContent*> eMailList;
    QList <eMailContent*> eMailFailed;
    bool send(eMailContent*);
    QMutex dataLocker;
    QString eMailSender;
    QString smtpServer;
    QString smtpPort;
    QString smptPassword;
    bool useSSL = false;
private slots:
signals:
    void logMessage(QString);
};

#endif // SENDMAILTHREAD_H
