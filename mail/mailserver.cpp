/*
  Copyright (C) 2019 Daniel Nicoletti <dantti12@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/
#include "mailserver_p.h"
#include "serverreply.h"

#include <QSslSocket>
#include <QTcpSocket>
#include <QHostInfo>
#include <QMessageAuthenticationCode>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(SIMPLEMAIL_Server, "simplemail.Server", QtInfoMsg)

using namespace SimpleMail;

mailServer::mailServer(QObject *parent)
    : QObject(parent)
    , d_ptr(new mailServerPrivate(this))
{
    Q_D(mailServer);
    d->hostname = QHostInfo::localHostName();
}

mailServer::~mailServer()
{
    delete d_ptr;
}

QString mailServer::host() const
{
    Q_D(const mailServer);
    return d->host;
}

void mailServer::setHost(const QString &host)
{
    Q_D(mailServer);
    d->host = host;
}

quint16 mailServer::port() const
{
    Q_D(const mailServer);
    return d->port;
}

void mailServer::setPort(quint16 port)
{
    Q_D(mailServer);
    d->port = port;
}

QString mailServer::hostname() const
{
    Q_D(const mailServer);
    return d->hostname;
}

void mailServer::setHostname(const QString &hostname)
{
    Q_D(mailServer);
    d->hostname = hostname;
}

mailServer::ConnectionType mailServer::connectionType() const
{
    Q_D(const mailServer);
    return d->connectionType;
}

void mailServer::setConnectionType(mailServer::ConnectionType ct)
{
    Q_D(mailServer);
    delete d->socket;
    d->socket = nullptr;
    d->connectionType = ct;
}

QString mailServer::username() const
{
    Q_D(const mailServer);
    return d->username;
}

void mailServer::setUsername(const QString &username)
{
    Q_D(mailServer);
    if (d->authMethod == mailServer::AuthNone) {
        d->authMethod = mailServer::AuthPlain;
    }
    d->username = username;
}

QString mailServer::password() const
{
    Q_D(const mailServer);
    return d->password;
}

void mailServer::setPassword(const QString &password)
{
    Q_D(mailServer);
    d->password = password;
}

mailServer::AuthMethod mailServer::authMethod() const
{
    Q_D(const mailServer);
    return d->authMethod;
}

void mailServer::setAuthMethod(mailServer::AuthMethod method)
{
    Q_D(mailServer);
    d->authMethod = method;
}

ServerReply *mailServer::sendMail(const MimeMessage &email)
{
    Q_D(mailServer);
    ServerReplyContainer cont(email);
    cont.reply = new ServerReply(this);

    // Add to the mail queue
    d->queue.append(cont);

    if (d->state == mailServerPrivate::Disconnected) {
        connectToServer();
    } else if (d->state == mailServerPrivate::Ready) {
        d->processNextMail();
    }

    return cont.reply.data();
}

int mailServer::queueSize() const
{
    Q_D(const mailServer);
    return d->queue.size();
}

void mailServer::connectToServer()
{
    Q_D(mailServer);

    d->createSocket();

    switch (d->connectionType) {
    case mailServer::TlsConnection:
    case mailServer::TcpConnection:
        qCDebug(SIMPLEMAIL_Server) << "Connecting to host" << d->host << d->port;
        d->socket->connectToHost(d->host, d->port);
        d->state = mailServerPrivate::Connecting;
        break;
    case mailServer::SslConnection:
    {
        auto sslSock = qobject_cast<QSslSocket*>(d->socket);
        if (sslSock) {
            qCDebug(SIMPLEMAIL_Server) << "Connecting to host encrypted" << d->host << d->port;
            sslSock->connectToHostEncrypted(d->host, d->port);
            d->state = mailServerPrivate::Connecting;
        } else {
            return /*false*/;
        }
    }
        break;
    }
}

void mailServer::ignoreSslErrors()
{
    Q_D(mailServer);
    auto sslSock = qobject_cast<QSslSocket*>(d->socket);
    if (sslSock) {
        sslSock->ignoreSslErrors();
    }
}

void mailServer::ignoreSslErrors(const QList<QSslError> &errors)
{
    Q_D(mailServer);
    auto sslSock = qobject_cast<QSslSocket*>(d->socket);
    if (sslSock) {
        sslSock->ignoreSslErrors(errors);
    }
}

void mailServerPrivate::createSocket()
{
    Q_Q(mailServer);

    if (socket) {
        return;
    }

    switch (connectionType) {
    case mailServer::TcpConnection:
        socket = new QTcpSocket(q);
        break;
    case mailServer::SslConnection:
    case mailServer::TlsConnection:
        socket = new QSslSocket(q);
        setPeerVerificationType(peerVerificationType);
        q->connect(static_cast<QSslSocket*>(socket), static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),
                   q, &mailServer::sslErrors, Qt::DirectConnection);
    }
    q->connect(socket, &QTcpSocket::stateChanged, q, [=] (QAbstractSocket::SocketState sockState) {
        qCDebug(SIMPLEMAIL_Server) << "stateChanged" << sockState << socket->readAll();
        if (sockState == QAbstractSocket::ClosingState) {
            state = Closing;
        } else if (sockState == QAbstractSocket::UnconnectedState) {
            state = Disconnected;
            if (!queue.isEmpty()) {
                q->connectToServer();
            }
        }
    });

    q->connect(socket, &QTcpSocket::connected, q, [=] () {
        qCDebug(SIMPLEMAIL_Server) << "connected" << state << socket->readAll();
        state = WaitingForServiceReady220;
    });

    q->connect(socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error),
               q, [=] (QAbstractSocket::SocketError error) {
        qCDebug(SIMPLEMAIL_Server) << "SocketError" << error << socket->readAll();
    });

    q->connect(socket, &QTcpSocket::readyRead, q, [=] {
        qCDebug(SIMPLEMAIL_Server) << "readyRead" << socket->bytesAvailable();
        switch (state) {
        case SendingMail:
            while (socket->canReadLine()) {
                if (!queue.isEmpty()) {
                    ServerReplyContainer &cont = queue[0];
                    if (cont.state == ServerReplyContainer::SendingCommands) {
                        if (cont.reply.isNull()) {
                            // Abort
                            socket->disconnectFromHost();
                            queue.removeFirst();
                            return;
                        }

                        while (!cont.awaitedCodes.isEmpty() && socket->canReadLine()) {
                            const int awaitedCode = cont.awaitedCodes.takeFirst();

                            QByteArray responseText;
                            const int code = parseResponseCode(&responseText);
                            if (code != awaitedCode) {
                                // Reset connection
                                cont.reply->finish(true, code, QString::fromLatin1(responseText));
                                queue.removeFirst();
                                const QByteArray consume = socket->readAll();
                                qDebug() << "Mail error" << consume;
                                state = Ready;
                                commandReset();
                                return;
                            }

                            if (!capPipelining && !cont.awaitedCodes.isEmpty()) {
                                // Write next command
                                socket->write(cont.commands[cont.commands.size() - cont.awaitedCodes.size()]);
                            }
                        }

                        if (cont.awaitedCodes.isEmpty()) {
                            cont.state = ServerReplyContainer::SendingData;
                            if (cont.msg.write(socket)) {
                                qCDebug(SIMPLEMAIL_Server) << "Mail sent";
                            } else {
                                qCCritical(SIMPLEMAIL_Server) << "Error writing mail";
                                cont.reply->finish(true, -1, q->tr("Error sending mail DATA"));
                                queue.removeFirst();
                                socket->disconnectFromHost();
                                return;
                            }
                        }
                    } else if (cont.state == ServerReplyContainer::SendingData) {
                        QByteArray responseText;
                        const int code = parseResponseCode(&responseText);
                        if (!cont.reply.isNull()) {
                            cont.reply->finish(code != 250, code, QString::fromLatin1(responseText));
                        }
                        qCDebug(SIMPLEMAIL_Server) << "MAIL FINISHED" << code << queue.size() << socket->canReadLine();

                        queue.removeFirst();
                        processNextMail();
                    }
                } else {
                    state = Ready;
                    break;
                }
            }
            break;
        case WaitingForServerCaps250:
            while (socket->canReadLine()) {
                int ret = parseCaps();
                if (ret != 0 && ret == 1) {
                    qCDebug(SIMPLEMAIL_Server) << "CAPS" << caps;
                    capPipelining = caps.contains(QStringLiteral("250-PIPELINING"));
                    if (connectionType == mailServer::TlsConnection) {
                        auto sslSocket = qobject_cast<QSslSocket*>(socket);
                        if (sslSocket) {
                            if (!sslSocket->isEncrypted()) {
                                qCDebug(SIMPLEMAIL_Server) << "Sending STARTTLS";
                                socket->write(QByteArrayLiteral("STARTTLS\r\n"));
                                state = WaitingForServerStartTls_220;
                            } else {
                                login();
                            }
                        }
                    } else {
                        login();
                    }
                    break;
                } else if (ret == -1) {
                    break;
                }
            }
            break;
        case WaitingForServerStartTls_220:
            if (socket->canReadLine()) {
                if (parseResponseCode(220)) {
                    auto sslSock = qobject_cast<QSslSocket *>(socket);
                    if (sslSock) {
                        qCDebug(SIMPLEMAIL_Server) << "Starting client encryption";
                        sslSock->startClientEncryption();

                        // This will be queued and sent once the connection get's encrypted
                        socket->write("EHLO " + hostname.toLatin1() + "\r\n");
                        state = WaitingForServerCaps250;
                        caps.clear();
                    }
                }
            }
            break;
        case Noop_250:
        case Reset_250:
            if (parseResponseCode(250)) {
                qCDebug(SIMPLEMAIL_Server) << "Got NOOP/RSET OK";
                state = Ready;
                processNextMail();
            }
            break;
        case WaitingForAuthPlain235:
        case WaitingForAuthLogin235_step3:
        case WaitingForAuthCramMd5_235_step2:
            if (socket->canReadLine()) {
                if (parseResponseCode(235, mailServer::AuthenticationFailedError)) {
                    state = Ready;
                    processNextMail();
                }
            }
            break;
        case WaitingForAuthLogin334_step1:
            if (socket->canReadLine()) {
                if (parseResponseCode(334, mailServer::AuthenticationFailedError)) {
                    // Send the username in base64
                    qCDebug(SIMPLEMAIL_Server) << "Sending authentication user" << username;
                    socket->write(username.toUtf8().toBase64() + "\r\n");
                    state = WaitingForAuthLogin334_step2;
                }
            }
            break;
        case WaitingForAuthLogin334_step2:
            if (socket->canReadLine()) {
                if (parseResponseCode(334, mailServer::AuthenticationFailedError)) {
                    // Send the password in base64
                    qCDebug(SIMPLEMAIL_Server) << "Sending authentication password";
                    socket->write(password.toUtf8().toBase64() + "\r\n");
                    state = WaitingForAuthLogin235_step3;
                }
            }
            break;
        case WaitingForAuthCramMd5_334_step1:
            if (socket->canReadLine()) {
                QByteArray responseMessage;
                if (parseResponseCode(334, mailServer::AuthenticationFailedError, &responseMessage)) {
                    // Challenge
                    QByteArray ch = QByteArray::fromBase64(responseMessage);

                    // Compute the hash
                    QMessageAuthenticationCode code(QCryptographicHash::Md5);
                    code.setKey(password.toUtf8());
                    code.addData(ch);

                    QByteArray data(username.toUtf8() + " " + code.result().toHex());
                    socket->write(data.toBase64() + "\r\n");
                    state = WaitingForAuthCramMd5_235_step2;
                }
            }
            break;
        case WaitingForServiceReady220:
            if (socket->canReadLine()) {
                if (parseResponseCode(220)) {
                    // The client's first command must be EHLO/HELO
                    socket->write("EHLO " + hostname.toLatin1() + "\r\n");
                    state = WaitingForServerCaps250;
                }
            }
            break;
        default:
            qCDebug(SIMPLEMAIL_Server) << "readyRead unknown state" << socket->readAll() << state;
        }
        qCDebug(SIMPLEMAIL_Server) << "readyRead" << socket->bytesAvailable();
    });
}

void mailServerPrivate::setPeerVerificationType(const mailServer::PeerVerificationType &type)
{
    peerVerificationType = type;
    if (socket != Q_NULLPTR)
    {
        if (connectionType == mailServer::SslConnection || connectionType == mailServer::TlsConnection)
        {
            switch (type) {
                case mailServer::VerifyNone:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyNone);
                    break;
//                case mailServer::VerifyPeer:
                default:
                    static_cast<QSslSocket*>(socket)->setPeerVerifyMode(QSslSocket::VerifyPeer);
                    break;
            }
        }
    }
}

void mailServerPrivate::login()
{
    qCDebug(SIMPLEMAIL_Server) << "LOGIN" << authMethod;
    if (authMethod == mailServer::AuthPlain) {
        qCDebug(SIMPLEMAIL_Server) << "Sending authentication plain" << state;
        // Sending command: AUTH PLAIN base64('\0' + username + '\0' + password)
        const QByteArray plain = '\0' + username.toUtf8() + '\0' + password.toUtf8();
        socket->write(QByteArrayLiteral("AUTH PLAIN ") + plain.toBase64() + "\r\n");
        state = WaitingForAuthPlain235;
    } else if (authMethod == mailServer::AuthLogin) {
        // Sending command: AUTH LOGIN
        qCDebug(SIMPLEMAIL_Server) << "Sending authentication login";
        socket->write(QByteArrayLiteral("AUTH LOGIN\r\n"));
        state = WaitingForAuthLogin334_step1;
    } else if (authMethod == mailServer::AuthCramMd5) {
        // NOTE Implementando - Ready
        qCDebug(SIMPLEMAIL_Server) << "Sending authentication CRAM-MD5";
        socket->write(QByteArrayLiteral("AUTH CRAM-MD5\r\n"));
        state = WaitingForAuthCramMd5_334_step1;
    } else {
        state = mailServerPrivate::Ready;
        processNextMail();
    }
}

void mailServerPrivate::processNextMail()
{
     while (!queue.isEmpty()) {
        ServerReplyContainer &cont = queue[0];
        if (cont.reply.isNull()) {
            queue.removeFirst();
            continue;
        }

        if (cont.state == ServerReplyContainer::Initial) {
            // Send the MAIL command with the sender
            cont.commands << "MAIL FROM:<" + cont.msg.sender().address().toLatin1() + ">\r\n";
            cont.awaitedCodes << 250;

            // Send RCPT command for each recipient
            // To (primary recipients)
            const auto toRecipients = cont.msg.toRecipients();
            for (const EmailAddress &rcpt : toRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // Cc (carbon copy)
            const auto ccRecipients = cont.msg.ccRecipients();
            for (const EmailAddress &rcpt : ccRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // Bcc (blind carbon copy)
            const auto bccRecipients = cont.msg.bccRecipients();
            for (const EmailAddress &rcpt : bccRecipients) {
                cont.commands << "RCPT TO:<" + rcpt.address().toLatin1() + ">\r\n";
                cont.awaitedCodes << 250;
            }

            // DATA command
            cont.commands << QByteArrayLiteral("DATA\r\n");
            cont.awaitedCodes << 354;

            qCDebug(SIMPLEMAIL_Server) << "Sending MAIL command" << capPipelining << cont.commands.size() << cont.commands << cont.awaitedCodes;
            if (capPipelining) {
                for (const QByteArray &cmd : cont.commands) {
                    socket->write(cmd);
                }
            } else {
                socket->write(cont.commands.first());
            }

            state = SendingMail;
            cont.state = ServerReplyContainer::SendingCommands;
            return;
        } else {
            return;
        }
    }

     state = Ready;
}

bool mailServerPrivate::parseResponseCode(int expectedCode, mailServer::SmtpError defaultError, QByteArray *responseMessage)
{
    Q_Q(mailServer);

    // Save the mailServer's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_Server) << "Got response" << responseText << "expected" << expectedCode;

    // Extract the respose code from the mailServer's responce (first 3 digits)
    const int responseCode = responseText.left(3).toInt();

    if (responseCode / 100 == 4) {
        //        lastError = QString::fromLatin1(responseText);
        Q_EMIT q->smtpError(mailServer::ServerError, QString::fromLatin1(responseText));
    }

    if (responseCode / 100 == 5) {
        //        lastError = QString::fromLatin1(responseText);
        Q_EMIT q->smtpError(mailServer::ClientError, QString::fromLatin1(responseText));
    }

    if (responseText[3] == ' ') {
        if (responseCode != expectedCode) {
            const QString lastError = QString::fromLatin1(responseText);
            qCWarning(SIMPLEMAIL_Server) << "Unexpected mailServer response" << lastError << expectedCode;
            Q_EMIT q->smtpError(defaultError, lastError);
            return false;
        }
        if (responseMessage) {
            *responseMessage = responseText.mid(4);
        }
        return true;
    }

    const QString lastError = QString::fromLatin1(responseText);
    qCWarning(SIMPLEMAIL_Server) << "Unexpected mailServer response" << lastError << expectedCode;
    Q_EMIT q->smtpError(defaultError, lastError);
    return false;
}

int mailServerPrivate::parseResponseCode(QByteArray *responseMessage)
{
    Q_Q(mailServer);

    // Save the mailServer's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_Server) << "Got response" << responseText;

    // Extract the respose code from the mailServer's responce (first 3 digits)
    const int responseCode = responseText.left(3).toInt();

    if (responseCode / 100 == 4) {
        Q_EMIT q->smtpError(mailServer::ServerError, QString::fromLatin1(responseText));
    }

    if (responseCode / 100 == 5) {
        Q_EMIT q->smtpError(mailServer::ClientError, QString::fromLatin1(responseText));
    }

    if (responseMessage) {
        *responseMessage = responseText.mid(4);
    }

    return responseCode;
}

int mailServerPrivate::parseCaps()
{
    Q_Q(mailServer);

    // Save the mailServer's response
    const QByteArray responseText = socket->readLine().trimmed();
    qCDebug(SIMPLEMAIL_Server) << "Got response" << responseText;

    // Extract the respose code from the mailServer's responce (first 3 digits)
    int responseCode = responseText.left(3).toInt();
    if (responseCode == 250) {
        caps.append(QString::fromLatin1(responseText));
        if (responseText[3] == ' ') {
            return 1;
        } else {
            return 0;
        }
    } else {
        const QString lastError = QString::fromLatin1(responseText);
        qCWarning(SIMPLEMAIL_Server) << "Unexpected mailServer caps" << lastError;
        Q_EMIT q->smtpError(mailServer::ServerError, lastError);
        return -1;
    }
}

void mailServerPrivate::commandReset()
{
    if (state == Ready) {
        qCDebug(SIMPLEMAIL_Server) << "Sending RESET";
        socket->write(QByteArrayLiteral("RSET\r\n"));
        state = Reset_250;
    }
}

void mailServerPrivate::commandNoop()
{
    if (state == Ready) {
        qCDebug(SIMPLEMAIL_Server) << "Sending NOOP";
        socket->write(QByteArrayLiteral("NOOP\r\n"));
        state = Noop_250;
    }
}

void mailServerPrivate::commandQuit()
{
    socket->write(QByteArrayLiteral("QUIT\r\n"));
}

//#include "moc_mailServer.cpp"
