#include <QtCore>
#include <QDesktopServices>
#include <QtNetwork>
#include "sendsmsthread.h"

sendSmsThread::sendSmsThread()
{
    //QSslConfiguration sslConfiguration(QSslConfiguration::defaultConfiguration());
    //sslConfiguration.setProtocol(QSsl::AnyProtocol);
}


sendSmsThread::~sendSmsThread()
{
}





void sendSmsThread::appendSms(QString sms)
{
    dataLocker.lock();
    smsContent *newSMS = new smsContent;
    newSMS->msg = sms;
    smsList.append(newSMS);
    dataLocker.unlock();
}


void sendSmsThread::run()
{
    dataLocker.lock();
    smsContent *sms = smsList.first();
    smsList.removeFirst();
    dataLocker.unlock();
    if (!send(sms)) smsFailed.append(sms);
    //QDesktopServices::openUrl(QUrl(sms));
    bool sent = false;
    while(!smsFailed.isEmpty() && smsList.isEmpty())
    {
        int t = 0;
        while(smsList.isEmpty() && (t< 60) && (!sent))
        {
            sleep(1);
            t++;
        }
        if (smsList.isEmpty())
        {
            sms = smsFailed.first();
            sent = send(sms);
            if (sent || (sms->attempt > 120))
            {
                smsFailed.removeFirst();
            }
        }
    }
}



bool sendSmsThread::send(smsContent *sms)
{
    QNetworkRequest request(sms->msg);
    QEventLoop waitLoop;
    QNetworkAccessManager* connection = new QNetworkAccessManager();
    QNetworkReply* reply = connection->get(request);
    QObject::connect(reply, SIGNAL(finished()), &waitLoop, SLOT(quit()));
    waitLoop.exec();
    int errorCode = reply->error();
    QString error = reply->errorString();
    delete reply;
    delete connection;
    if (errorCode != 0)
    {
        error += " " + sms->msg;
        if (sms->attempt) error += QString(" retry %1").arg(sms->attempt);
        sms->attempt++;
        emit(logMessage(error));
        return false;
    }
    else if (sms->attempt)
    {
        QString str = sms->msg + QString(tr(" message successfully send after %1 attemps").arg(sms->attempt));
        emit(logMessage(str));
        return true;
    }
}

