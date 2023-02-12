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
    qDebug() << sms;
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
    if (send(sms)) delete sms;
    else smsFailed.append(sms);
    while((!smsFailed.isEmpty()) || (!smsList.isEmpty()))
    {
        if (!smsList.isEmpty())
        {
            sms = smsList.first();
            if (send(sms) || (sms->attempt > 120)) { smsList.removeFirst(); delete sms; }
        }
        else if (!smsFailed.isEmpty())
        {
            sms = smsFailed.first();
            if (send(sms) || (sms->attempt > 120)) { smsFailed.removeFirst(); delete sms; }
        }
        sleep(1);
    }
}



bool sendSmsThread::send(smsContent *sms)
{
    QNetworkRequest request(sms->msg);
    QEventLoop waitLoop;
    QNetworkAccessManager* connection = new QNetworkAccessManager();
    emit(logMessage("try http request : " + sms->msg));
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
    QString str = sms->msg + tr(" message successfully send");
    emit(logMessage(str));
    return true;
}

