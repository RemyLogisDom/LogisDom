#include <QtCore>
#include "sendmailthread.h"
#include "mail/emailaddress.h"
#include "mail/mimemessage.h"
#include "mail/mimetext.h"

sendMailThread::sendMailThread()
{
}


sendMailThread::~sendMailThread()
{
}



void sendMailThread::appendeMail(eMailContent *eMail)
{
    dataLocker.lock();
    eMailList.append(eMail);
    dataLocker.unlock();
}



void sendMailThread::run()
{
    while(!eMailList.isEmpty())
    {
        dataLocker.lock();
        eMailContent *eMail = eMailList.first();
        eMailList.removeFirst();
        dataLocker.unlock();
        if (send(eMail)) delete eMail;
        else eMailFailed.append(eMail);
        bool sent = false;
        while(!eMailFailed.isEmpty() && eMailList.isEmpty())
        {
            int t = 0;
            while(eMailList.isEmpty() && (t< 60) && (!sent))
            {
                sleep(1);
                t++;
            }
            if (eMailList.isEmpty())
            {
                eMailContent *eMail = eMailFailed.first();
                sent = send(eMail);
                if (sent || (eMail->attempt > 120))
                {
                    eMailFailed.removeFirst();
                    delete eMail;
                }
            }
        }
    }
}




bool sendMailThread::send(eMailContent *eMail)
{
    SimpleMail::MimeMessage message;
    message.setSender(SimpleMail::EmailAddress(eMailSender, "LogisDom"));
    message.addTo(SimpleMail::EmailAddress(eMail->destinataire, eMail->nom));
    message.setSubject(eMail->objet);
    SimpleMail::MimeText text;
    text.setText(eMail->text);
    message.addPart(&text);
    bool sent = false;
    QString error;
    if (useSSL)
    {
        bool ok;
        int port = smtpPort.toInt(&ok);
        if (!ok) port = 465;
        SimpleMail::Sender sender(smtpServer, port, SimpleMail::Sender::SslConnection);
        sender.setUser(eMailSender);
        sender.setPassword(smptPassword);
        sent = sender.sendMail(message);
        if (!sent) error = sender.lastError();
    }
    else
    {
        bool ok;
        int port = smtpPort.toInt(&ok);
        if (!ok) port = 25;
        SimpleMail::Sender sender(smtpServer, port, SimpleMail::Sender::TcpConnection);
        sender.setUser(eMailSender);
        sender.setPassword(smptPassword);
        sent = sender.sendMail(message);
        if (!sent) error = sender.lastError();
    }
    if (!sent)
    {
        error += " " + eMail->objet;
        if (eMail->attempt) error += QString(" retry %1").arg(eMail->attempt);
        eMail->attempt++;
        emit(logMessage(error));
    }
    else if(eMail->attempt)
    {
        QString str = eMail->objet + QString(tr(" message successfully sent after %1 attemps").arg(eMail->attempt));
        emit(logMessage(str));
    }
    return sent;
}






