/****************************************************************************
**
** Copyright (C) 2022 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** LogisDom is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.

** LogisDom is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License
** along with LogisDom.  If not, see <https://www.gnu.org/licenses/>
**
****************************************************************************/





#include "globalvar.h"
#include "onewire.h"
#include "configwindow.h"
#include "weathercom.h"
#include "energiesolaire.h"
#include "meteo.h"
#include "addDaily.h"
#include "addProgram.h"
#include "formula.h"
#include "logisdom.h"
#include "formulasimple.h"
#include "calcthread.h"
#include "calc.h"
#include "mailsender.h"


calcthread::calcthread(formula *Parent)
{
    parent = Parent;
    lastValue = 0;
    integral = 0;
    sendMailRetry = 0;
    previousError = 0;
    threadResult = logisdom::NA;
    lastValueSendMail = logisdom::NA;
    lastValueHysteresis = logisdom::NA;
    calculating = false;
    AutoEnabled = true;
    LinkedOnly = false;
    DSPdevice = nullptr;
    DSPGain = 0;
    DSPpole = 0;
    lastDSPdevice = nullptr;
    lastDSPGain = 0;
    lastDSPpole = 0;
    scroolDone = false;
    restartDSP = true;
    resetDSP();
}




void calcthread::runprocess()
{
    calculate();
	lastValueSendMail = threadResult;
    emit(calcFinished(textBrowserResult));
}




void calcthread::clearAutoEnabled()
{
    AutoEnabled = false;
}



void calcthread::setAutoEnabled()
{
    AutoEnabled = false;
}




void calcthread::clearLinkedEnabled()
{
    LinkedOnly = false;
}



void calcthread::setLinkedEnabled()
{
    LinkedOnly = true;
}




onewiredevice *calcthread::getLinkedDevice()
{
    if (deviceList.count() > 0) return deviceList.first();
    return nullptr;
}




void calcthread::setCalc(const QString &str)
{
    if ((lastCalcStr != str))
    {
        QStringList strlist = str.split("\n");
        Calc.clear();
        for (int c=0; c<strlist.count(); c++)
            if (!strlist.at(c).startsWith("'")) Calc.append(strlist.at(c) + "\n");
        if (AutoEnabled)
        {
            for (int n=0; n<deviceList.count(); n++)
                disconnect(deviceList.at(n), SIGNAL(DeviceValueChanged(onewiredevice*)), parent, SLOT(CalculateThreadRequest(onewiredevice*)));
            deviceList.clear();
        }
        lastCalcStr = str;
        lastCalcClean = Calc;
    }
    else Calc = lastCalcClean;
}



double calcthread::calculate(const QString &str)
{
    setCalc(str);
    return calculate();
}




onewiredevice *calcthread::checkDevice(const QString &RomID)
{
    onewiredevice *device = nullptr;
    device = maison1wirewindow->configwin->DeviceExist(RomID);
    if (!device) device = maison1wirewindow->configwin->Devicenameexist(RomID);
    if (!device) return nullptr;
    if ((!deviceList.contains(device)) && (AutoEnabled || LinkedOnly))
    {
        if (AutoEnabled) connect(device, SIGNAL(DeviceValueChanged(onewiredevice*)), parent, SLOT(CalculateThreadRequest(onewiredevice*)), Qt::QueuedConnection);
        deviceList.append(device);
    }
    return device;
}



double calcthread::calculate()
{
    V.clear();
    calculating = true;
	QStringList mailTo;
    QString textSubject, textMail, mailAction;
    QString httpSmsRequest, textSms, smsAction;
    bool sendMail = false;
    bool sendSMS = false;
    textBrowserResult.clear();
    webStrResult.clear();
    double R = logisdom::NA;
    deviceLoading = false;
	stopRequest = false;
	originalCalc = Calc;
	bool webParseCalc = false;
	bool webMailCalc = false;
    bool webSmsCalc = false;
    if (lastCalc != Calc) lastCalc = Calc;
    if (Calc.contains("webparse")) webParseCalc = true;
	if (Calc.contains("webmail")) webMailCalc = true;
    if (Calc.contains("websms")) webSmsCalc = true;
restart:
	syntax = true;
	dataValid = true;
	bool check = true;
    QString webParseCalcStr;
    if (webParseCalc)
    {
/*
webparse
webpage=http://particuliers.edf.com/abonnement-et-contrat/les-prix/les-prix-de-l-electricite/option-tempo/la-couleur-du-jour-2585.html&coe_i_id=2585
search=<h4>Demain
search=<span class="period">
end=</span>
filter=Jour non EJP;Jour EJP;séparateur point virgule
Non Déterminé=0
Blanc=1
Bleu=2
Rouge=3
*/
		QStringList list = Calc.split("\n");
		int count = list.count();
        webStrResult = valueOnError;
        Calc = valueOnError;
        QString webpage;
        QStringList searchlist, strs, values, filters, replace, replaceBy, end;
        int n = 0;
        webpage = search(list, "webpage=");
        if (!webpage.isEmpty())
        {
            while (n < count)       // get search list parameter, loop until "search=" is found
            {
                if (list.at(n).startsWith("search=", Qt::CaseInsensitive))
                {
                    QString search = list.at(n);
                    searchlist.append(search.remove(0, 7));
                }
                else
                {
                    if (!searchlist.isEmpty()) break;
                }
                n++;
            }
            if (searchlist.count() > 0)
            {
                textBrowserResult += "\n" + QString("%1 search string defined").arg(searchlist.count());
                while (n < count)   // get end list parameter, loop until "end=" is found
                {
                    if (list.at(n).startsWith("end=", Qt::CaseInsensitive))
                    {
                        QString e = list.at(n);
                        e = e.remove(0, 4);
                        end.append(e);
                        n++;
                        if (!list.at(n).startsWith("end=", Qt::CaseInsensitive)) break;
                    }
                    else n++;
                }
                if (!end.isEmpty())
                {
                    textBrowserResult += "\n" + tr("end defined");
                    while ((list.at(n).startsWith("replace=", Qt::CaseInsensitive)) && (n < count))
                    { // replace= k_BY_000
                        QString str;
                        str = list.at(n);
                        str = str.remove(0, 8);
                        QStringList pair;
                        pair = str.split("_BY_");
                        if (pair.count() == 2)
                        {
                            replace.append(pair.at(0));
                            replaceBy.append(pair.at(1));
                        }
                        n++;
                    }
                    textBrowserResult += "\n" + QString("%1 replacement defined").arg(replace.count());
                    if (list.at(n).startsWith("filter=", Qt::CaseInsensitive))
                    {
                        QString str;
                        str = list.at(n);
                        str = str.remove(0, 7);
                        filters = str.split(";");
                        textBrowserResult += "\n" + QString("%1 filters defined").arg(filters.count());
                        n++;
                    }
                    else textBrowserResult += "\n" + tr("no filter defined");
                    while (n < count) // get user values list, loop until "=" splitter is found
                    {
                        QString val = list.at(n);
                        if (val.isEmpty()) break;
                        QStringList split = val.split("=");
                        if (split.count() == 2)
                        {
                            strs.append(split.at(0));
                            values.append(split.at(1));
                        }
                        n++;
                    }
                    textBrowserResult += "\n" + QString("%1 value defined").arg(values.count());
                    textBrowserResult += "\n" + tr("Open web page : ") + webpage;
                    QString Data;
                    get(webpage, Data);
                    if (Data.count() > 0)
                    {
                        //textBrowserResult += "\n" + QString("web data : ") + Data;
                        //textBrowserResult += "\n" + QString("web data %1").arg(Data.count());
                        int index = 0;
                        int index_end = 0;
                        for (int n=0; n<searchlist.count(); n++)
                        {
                            index = Data.indexOf(searchlist.at(n), index);
                            if (index != -1) textBrowserResult += "\nSearch string" + searchlist.at(n) + QString("   Search string %1, index = %2").arg(n).arg(index);
                            else textBrowserResult += "\nSearch failed   " + searchlist.at(n);
                            if (index == -1) break;
                        }
                        if (index != -1)
                        {
                            index += searchlist.last().length();
                            for (int n=0; n<end.count(); n++)
                            {
                                int i = Data.indexOf(end.at(n), index);
                                if (index_end == 0)
                                {
                                    if (i != -1) index_end = i;
                                }
                                else
                                {
                                    if ((i != -1) && (i < index_end)) index_end = i;
                                }
                            }
                            if (index_end != -1)
                            {
                                QString result = Data.mid(index, index_end-index).remove("\n");
                                while (!result[0].isLetterOrNumber()) result = result.remove(0, 1);
                                while (!result[result.length()-1].isLetterOrNumber())
                                {
                                    result.chop(1);
                                    if (!result.length()) break;
                                }
                                for (int n=0; n<replace.count(); n++)
                                    result = result.replace(replace.at(n), replaceBy.at(n));
                                for (int n=0; n<filters.count(); n++)
                                {
                                    if (result.contains(filters.at(n)))
                                    {
                                        textBrowserResult += "\n" + tr("Result : ") + result + tr(" was filtered");
                                        result = filters.at(n);
                                        break;
                                    }
                                }
                                textBrowserResult += "\n" + tr("Result : ") + result;
                                Calc = result;
                                webStrResult = result;
                                for (int n=0; n<strs.count(); n++)
                                {
                                    if (result == strs.at(n))
                                    {
                                        Calc = values.at(n);
                                        webStrResult.clear();
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                textBrowserResult += "\n" + tr("End search string not found, parsing finished");
                            }
                        }
                        else
                        {
                            textBrowserResult += "\n" + tr("Search did not give any result, parsing finished");
                        }
                    }
                }
                else
                {
                    textBrowserResult += "\n" + tr("end not found, web pasring aborted");
                }
            }
            else
            {
                textBrowserResult += "\n" + tr("No search string found, web pasring aborted");
            }
        }
        else
        {
            textBrowserResult += "\n" + tr("webpage not found, -1 value is returned, web pasring aborted");
        }
        webParseCalcStr = Calc;
	}
    if (webMailCalc)
	{
         Calc = originalCalc;
/*
webmail
mailAddress=remy.carisio@orange.fr
formula=RomID>5.6/76
textSubject=Bonjour, alarme PAC
textMail=file.txt
mailAction=sendifnullr/sendifnotnullr/sendifna (default is sendifnotnullptr)
*/
		mailTo.clear();
		textSubject.clear();
		textMail.clear();
        QStringList list = Calc.split("\n");
        foreach (QString str, list)
        {
            if (str.startsWith("mailAddress=", Qt::CaseInsensitive))
            {
                QString mailAddress = str.remove(0, 12);
                mailTo.append(mailAddress);
            }
            else if (str.startsWith("formula=", Qt::CaseInsensitive))
            {
                Calc = str.remove(0, 8);
                if (webParseCalc) webParseCalcStr = Calc;
            }
            else if (str.startsWith("textSubject=", Qt::CaseInsensitive))
            {
                textSubject = str.remove(0, 12).replace("%", "\n") + QDateTime::currentDateTime().toString("  HH:mm");
            }
            else if (str.startsWith("textMail=", Qt::CaseInsensitive))
            {
                textMail = str.remove(0, 9).replace("%", "\n");
            }
            else if (str.startsWith("mailAction=", Qt::CaseInsensitive))
            {
                mailAction = str.remove(0, 11);
            }
        }
        if (mailTo.count() > 0)
        {
            sendMail = true;
        }
        else
        {
            sendMail = false;
            Calc.clear();
            mailTo.clear();
            textSubject.clear();
            textMail.clear();
        }
        if (webParseCalc) Calc = webParseCalcStr;
	}
    if (webSmsCalc)
    {
         Calc = originalCalc;
/*
websms
smsHttpRequest=https://smsapi.free-mobile.fr/sendmsg?user=10399629&pass=DuQWsfPk7qlSkx&msg=Hello%20World%20!
textSms=file.txt
smsAction=sendifnullr/sendifnotnullr/sendifna (default is sendifnotnullptr)
*/
        QStringList list = Calc.split("\n");
        foreach (QString str, list)
        {
            if (str.startsWith("smsHttpRequest=", Qt::CaseInsensitive))
            {
                httpSmsRequest = str.remove(0,15);
                //https://smsapi.free-mobile.fr/sendmsg?user=10399629&pass=DuQWsfPk7qlSkx&msg=Hello%20World%20!
            }
            else if (str.startsWith("formula=", Qt::CaseInsensitive))
            {
                Calc = str.remove(0, 8);
                if (webParseCalc) webParseCalcStr = Calc;
            }
            else if (str.startsWith("textSms=", Qt::CaseInsensitive))
            {
                textSms.append(str.remove(0, 8));
            }
            else if (str.startsWith("smsAction=", Qt::CaseInsensitive))
            {
                smsAction = str.remove(0, 10);
            }
        }
        if ((!httpSmsRequest.isEmpty()) && (!textSms.isEmpty()) && (!smsAction.isEmpty())) sendSMS = true;
        if (webParseCalc) Calc = webParseCalcStr;
    }
    Calc = Calc.remove(" ");
	Calc = Calc.remove("\n");
    while (resoudreParenthese()) {};
    if (!(syntax && dataValid)) goto error;
    while (pw(Calc)) {};
    if (!(syntax && dataValid)) goto error;
    while (diviser(Calc)) {};
    if (!(syntax && dataValid)) goto error;
    while (multiplier(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (sup(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (inf(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (egal(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (different(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (enlever(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    while (ajouter(Calc)) {};
	if (!(syntax && dataValid)) goto error;
    if (!V.isEmpty()) R = V.last();
	else if (isNumeric(Calc)) R = Calc.toDouble(&check);
	else R = toNumeric(Calc, &check);
    if (restartDSP)
	{
		resetDSP();
		Calc = originalCalc;
		goto restart;
	}
error:
    if (logisdom::isNA(R)) dataValid = false;
	if (!TCalc) textBrowserResult += "\n" + (tr("Result = ") + QString("%1").arg(R, 0, 'e'));
	if (!check) syntax = false;
	if (!TCalc)
	{
		if (syntax) textBrowserResult += "\n" + (tr("No error detected in the formula"));
			else textBrowserResult += "\n" + (tr("Error in the formula"));
	}
	if (!TCalc)
	{
		if (dataValid) textBrowserResult += "\n" + (tr("Variable data were valid"));
			else textBrowserResult += "\n" + (tr("Variable data not valid"));
	}
	if (!TCalc)
	{
		if (deviceLoading)
		{
			textBrowserResult += "\n" + (tr("Result is not consistent some device are loading data"));
			R = logisdom::NA;
		}
		else
		{
			if (syntax && dataValid) textBrowserResult += "\n" + (tr("Result is consistent"));
			else textBrowserResult += "\n" + (tr("Result is not consistent"));
		}
	}
	if (!dataValid) syntax = false;
    threadResult = R;
    if (deviceList.count() == 0) textBrowserResult += "\nNo Connected device :\n";
	else textBrowserResult += "\nConnected devices :\n";
    for (int n=0; n<deviceList.count(); n++)
		textBrowserResult += "  - " + deviceList.at(n)->getromid() + " " + deviceList.at(n)->getname() + "\n";
	calculating = false;

    if (sendMail)
	{
        if (logisdom::AreNotSame(R, lastValueSendMail))
		{
            bool send = false;
            if ((logisdom::isNA(R)) || (!dataValid))
            {
                if (mailAction == "sendifna") send = true;
            }
            else
            {
                if (logisdom::isNA(lastValueSendMail))
                {
                    if (mailAction == "sendifna") send = true;
                }
                else
                {
                    if (mailAction == "sendifnull")
                    {
                        if (logisdom::isZero(R)) send = true;
                    }
                    else if (mailAction != "sendifna")
                    {
                        if (logisdom::isNotZero(R)) send = true;
                    }
                }
            }
			if (send)
            {
                textBrowserResult += "\n" + tr("Ready to Send Mail");
                for (int n=0; n<mailTo.count(); n++) textBrowserResult += "\n" + tr("Recipient") + " : " + mailTo.at(n);
                QString id = "";
                parent->parent->htmlTranslate(textSubject, id);
                textBrowserResult += "\n" + tr("Subject") + " : " + textSubject;
                parent->parent->valueTranslate(textMail);
                textBrowserResult += "\n" + tr("Text") + " : " + textMail;
                QString webFolder = parent->parent->getrepertoirehtml();
                QFile webFile(webFolder + textMail);
                if (!textMail.isEmpty())
                {
                    if (webFile.exists())
                    {
                        textMail.clear();
                        webFile.open(QIODevice::ReadOnly);
                        QString text = webFile.readAll();
                        webFile.close();
                        QString id = "";
                        parent->parent->htmlTranslate(text, id);
                        textMail.append(text);
                    }
                    else
                    {
                        parent->parent->htmlTranslate(textMail, id);
                    }
                }
                sendMailThread::eMailContent *message = new sendMailThread::eMailContent;
                message->destinataire = mailTo.at(0);
                message->nom = mailTo.at(0);
                message->text = textMail;
                message->objet = textSubject;
                maison1wirewindow->configwin->eMailSender.appendeMail(message);
                if (!maison1wirewindow->configwin->eMailSender.isRunning()) maison1wirewindow->configwin->eMailSender.start();
                textBrowserResult += "\n" + tr("Finished email in queue to be sent");
			}
			else
			{
                textBrowserResult += "\n" + tr("Mail not done");
			}
		}
		else
		{
            if (logisdom::isNotNA(R) and logisdom::isNotNA(lastValueSendMail)) textBrowserResult += "\n" + tr("No change with the last result, message is not sent");
            else textBrowserResult += "\n" + tr("result error or unknown last last value , message is not sent");
		}
        if (webParseCalc) R = webParseCalcStr.toInt(&check, 10); // set value if webparsing and mail are use together
	}

// send SMS
    if (sendSMS)
    {
        if (logisdom::AreNotSame(R, lastValueSendMail))
        {
            bool send = false;
            if ((logisdom::isNA(R)) || (!dataValid))
            {
                if (smsAction == "sendifna") send = true;
            }
            else
            {
                if (logisdom::isNA(lastValueSendMail))
                {
                    if (smsAction == "sendifna") send = true;
                }
                else
                {
                    if (smsAction == "sendifnull")
                    {
                        if (logisdom::isZero(R)) send = true;
                    }
                    else if (smsAction != "sendifna")
                    {
                        if (logisdom::isNotZero(R)) send = true;
                    }
                }
            }
            if (send)
            {
                QString webFolder = parent->parent->getrepertoirehtml();
                QFile webFile(webFolder + textSms);
                if (!textSms.isEmpty())
                {
                    if (webFile.exists())
                    {
                        textSms.clear();
                        webFile.open(QIODevice::ReadOnly);
                        QString text = webFile.readAll();
                        webFile.close();
                        parent->parent->htmlTranslate(text, "");
                        textSms.append(text);
                    }
                    else
                    {
                        parent->parent->htmlTranslate(textSms, "");
                    }
                    QString sms = httpSmsRequest + textSms.toLatin1().toPercentEncoding();
                    maison1wirewindow->configwin->smsSender.appendSms(sms);
                    if (!maison1wirewindow->configwin->smsSender.isRunning()) maison1wirewindow->configwin->smsSender.start();
                    textBrowserResult += "\n" + tr("Finished SMS in queue to be sent");
                    textBrowserResult += "\n" + sms;
                }
            }
            else
            {
                textBrowserResult += "\n" + tr("SMS not done");
            }
        }
        else
        {
            if (logisdom::isNotNA(R) and logisdom::isNotNA(lastValueSendMail)) textBrowserResult += "\n" + tr("No change with the last result, message is not sent");
            else textBrowserResult += "\n" + tr("result error or unknown last last value , message is not sent");
        }
        if (webParseCalc) R = webParseCalcStr.toInt(&check, 10); // set value if webparsing and mail are use together
    }
    return R;
}





QString calcthread::search(const QStringList &list, const QString &str)
{
    for (int n=0; n<list.count(); n++)
    {
        if (list.at(n).startsWith(str, Qt::CaseInsensitive))
        {
            QString result = list.at(n);
            return result.remove(0, str.length());
        }
    }
    return "";
}




void calcthread::get(const QString &Request, QString &Data)
{
    Data.clear();
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QUrl urlRequest = QUrl(Request);
    request.setUrl(urlRequest);
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        QUrl urlRedirectedTo;
        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        urlRedirectedTo = possibleRedirectUrl.toUrl();
        if (!urlRedirectedTo.isEmpty() && urlRedirectedTo != urlRequest)
        {
            textBrowserResult += "\n" + tr("Redirection has been received to : ") + urlRedirectedTo.toString();
            QString newUrlStr = urlRedirectedTo.toString();
            emit(redirectHttp(newUrlStr));
        }
        QByteArray data = reply->readAll();
#if QT_VERSION < 0x060000
        QTextCodec *Utf8Codec = QTextCodec::codecForName("utf-8");
        Data = Utf8Codec->toUnicode(data);
#else
        Data.append(data);
#endif
        textBrowserResult += "\n" + QString("%1").arg(Data.count()) + tr(" bytes received without error");
    }
    else textBrowserResult += "\nHttp error " + reply->errorString();
    reply->deleteLater();
}




QString calcthread::op2Function(int index)
{
    QString str = "";
    switch 	(index)
    {
    case ABS : str = tr("ABS(x)"); break;
    case INT : str = tr("INT(x)"); break;
    case MAX : str = tr("Maximum(x ; max)"); break;
    case MIN : str = tr("Minimum(x ; min)"); break;
    case ZeroIfNeg : str = tr("ZeroIfNeg(x)"); break;
    case ZeroIfPos : str = tr("ZeroIfPos(x)"); break;
    case ZeroIfSup : str = tr("ZeroIfSup(x; v)"); break;
    case ZeroIfInf : str = tr("ZeroIfInf(x; v)"); break;
    case BitAnd : str = tr("AND(a; b; format)"); break;
    case BitOr : str = tr("OR(a; b; format)"); break;
    case BitXor : str = tr("XOR(a; b; format)"); break;
    case BitLeft : str = tr("LEFT(a; n; format)"); break;
    case BitRight : str = tr("RIGHT(a; n; format)"); break;
    case EXP : str = tr("exp(x)"); break;
    case LOG : str = tr("log(x)"); break;
    case Hysteresis : str = tr("Hysteresis(x; v1; v2; default)"); break;
    case CurrentSkyClearance : str = tr("CurrentSkyClearance()"); break;
    case MeanSkyClearance : str = tr("MeanSkyClearance(x)"); break;
    case SkyClearance : str = tr("SkyClearance(x)"); break;
    case Rain : str = tr("Rain(x)"); break;
    case TemperatureHi : str = tr("TemperatureHi(x)"); break;
    case TemperatureLow : str = tr("TemperatureLow(x)"); break;
    case Humidity : str = tr("Humidity(x)"); break;
    case WindSpeed : str = tr("WindSpeed(x)"); break;
    case WindDirection : str = tr("WindDirection(x)"); break;
    case ActualSunPower : str = tr("ActualSunPower()"); break;
    case DaySunPower : str = tr("DaySunPower()"); break;
    case ValueRomID : str = tr("ValueRomID(RomID; Minutes; Width)"); break;
    case MeanRomID : str = tr("MeanRomID(RomID; Minutes; Length)"); break;
    case MaxRomID : str = tr("MaxRomID(RomID; Minutes; Length)"); break;
    case MinRomID : str = tr("MinRomID(RomID; Minutes; Length)"); break;
    case SumRomID : str = tr("SumRomID(RomID; Minutes; Length)"); break;
    case LSumRomID : str = tr("LSumRomID(RomID; Minutes; Limit)"); break;
    case CountRomID : str = tr("CountRomID(RomID; Minutes; Level; Length)"); break;
    case DataCount : str = tr("DataCount(RomID; Minutes; Length)"); break;
    case SlopeMeanRomID : str = tr("SlopeMeanRomID(RomID; Minutes)"); break;
    case SlopeSumPosRomID : str = tr("SlopeSumPosRomID(RomID; Minutes; Threshold)"); break;
    case SlopeSumNegRomID : str = tr("SlopeSumNegRomID(RomID; Minutes; Threshold)"); break;
    case Step : str = tr("Step(value; step)"); break;
    case CurrentYear : str =  tr("CurrentYear(starting month)"); break;
    case CurrentMonth : str =  tr("CurrentMonth(starting day)"); break;
    case CurrentWeek : str =  tr("CurrentWeek(starting day)"); break;
    case CurrentDay : str = tr("CurrentDay(starting hour)"); break;
    case YearBefore : str =  tr("YearBefore(number of years before)"); break;
    case MonthBefore : str =  tr("MonthBefore(number of months before)"); break;
    case Minute : str = tr("Minute()"); break;
    case Hour : str = tr("Hour()"); break;
    case DayOfWeek : str = tr("DayOfWeek()"); break;
    case DayOfMonth : str = tr("DayOfMonth()"); break;
    case MonthOfYear : str = tr("MonthOfYear()"); break;
    case Year : str = tr("Year()"); break;
    case SunSet : str = tr("SunSet()"); break;
    case SunRise : str = tr("SunRise()"); break;
    case ProgramValue : str = tr("ProgramValue(program name)"); break;
    case WeekProgValue : str = tr("WeekProgValue(program name)"); break;
    case PID : str = tr("PID(Actual; setPoint; P; I; D)"); break;
    case DSP : str = tr("DSP(RomID; Pole; Gain; Polynome)"); break;
    case webParse : str = tr("webparse\nwebpage=http://bleuciel.edf.com/abonnement-et-contrat/les-prix/les-prix-de-l-electricite/option-tempo/la-couleur-du-jour-2585.html&coe_i_id=2585\nsearch1=<h4>Demain\nsearch2=<span class=\"period\">\nend=</span>\nBlanc=(1)\nBleu=(2)\nRouge=(3)"); break;
    case webMail : str = tr("webmail\nmailAddress=machin@orange.fr\nmailAddress=toto@orange.fr\nmailAddress=bidule@orange.fr\ntextSubject=Alarme Temprature PAC\ntextMail=file.txt\nmailAction=sendifnullr/sendifnotnullr/sendifna\nformula=T_PAC < 1"); break;
    case webSms : str = tr("websms\nsmsHttpRequest=https://smsapi.free-mobile.fr/sendmsg?user=1234567890&pass=xxxxxxxxxx&msg=\ntextSMS=file.txt or any text\nsmsAction=sendifnullr/sendifnotnullr/sendifna\nformula=T_PAC < 1"); break;
    }
    return str;
}


QString calcthread::op2Str(int index)
{
	QString str = "";
	switch 	(index)
	{
		case ABS : str = "ABS"; break;
		case INT : str = "INT"; break;
		case MAX : str = "Max"; break;
		case MIN : str = "Min"; break;
		case ZeroIfNeg : str = "ZeroIfNeg"; break;
		case ZeroIfPos : str = "ZeroIfPos"; break;
		case ZeroIfSup : str = "ZeroIfSup"; break;
		case ZeroIfInf : str = "ZeroIfInf"; break;
        case EXP : str = "exp"; break;
        case LOG : str = "log"; break;
        case BitAnd : str = "AND"; break;
        case BitOr : str = "OR"; break;
        case BitXor : str = "XOR"; break;
        case BitLeft : str = "LEFT"; break;
        case BitRight : str = "RIGHT"; break;
        case Hysteresis : str = "Hysteresis"; break;
        case CurrentSkyClearance : str = "CurrentSkyClearance"; break;
		case MeanSkyClearance : str = "MeanSkyClearance"; break;
		case SkyClearance : str = "SkyClearance"; break;
        case TemperatureHi : str = "TemperatureHi"; break;
        case TemperatureLow : str = "TemperatureLow"; break;
        case Humidity : str = "Humidity"; break;
        case WindSpeed : str = "WindSpeed"; break;
        case WindDirection : str = "WindDirection"; break;
        case ActualSunPower : str = "ActualSunPower"; break;
        case DaySunPower : str = "DaySunPower"; break;
        case Rain : str = "Rain"; break;
        case ValueRomID : str = "ValueRomID"; break;
		case MeanRomID : str = "MeanRomID"; break;
		case MaxRomID : str = "MaxRomID"; break;
		case MinRomID : str = "MinRomID"; break;
		case SumRomID : str = "SumRomID"; break;
        case LSumRomID : str = "LSumRomID"; break;
        case CountRomID : str = "CountRomID"; break;
        case DataCount : str = "DataCount"; break;
		case SlopeMeanRomID : str = "SlopeMeanRomID"; break;
		case SlopeSumPosRomID : str = "SlopeSumPosRomID"; break;
		case SlopeSumNegRomID : str = "SlopeSumNegRomID"; break;
		case Step : str = "Step"; break;
		case CurrentYear : str = "CurrentYear"; break;
		case CurrentMonth : str = "CurrentMonth"; break;
		case CurrentWeek : str = "CurrentWeek"; break;
		case CurrentDay : str = "CurrentDay"; break;
        case YearBefore : str = "YearBefore"; break;
        case MonthBefore : str = "MonthBefore"; break;
        case Minute : str = "Minute"; break;
		case Hour : str = "Hour"; break;
		case DayOfWeek : str = "DayOfWeek"; break;
		case DayOfMonth : str = "DayOfMonth"; break;
		case MonthOfYear : str = "MonthOfYear"; break;
		case Year : str = "Year"; break;
        case SunSet : str = "SunSet"; break;
        case SunRise : str = "Sunrise"; break;
        case ProgramValue : str = "ProgramValue"; break;
        case WeekProgValue : str = "WeekProgValue"; break;
        case PID : str = "PID"; break;
		case DSP : str = "DSP"; break;
		case webParse : str = "Web Parsing"; break;
		case webMail : str = "Web Mail"; break;
        case webSms : str = "Web SMS"; break;
        default : str = "Undefined"; break;
	}
	return str;
}





QString calcthread::family2Str(int index)
{
	QString str = "";
	switch 	(index)
	{
		case famOperator : str = tr("Operator"); break;
		case famDevices : str = tr("Devices"); break;
		case famMath : str = tr("Math"); break;
		case famRomID : str = tr("Devices functions"); break;
		case famTime : str = tr("Time functions"); break;
		case famWeather : str = tr("Weather"); break;
		case webFamily : str = tr("Web"); break;
	}
	return str;
}





int calcthread::op2Family(int index)
{
	int fam = 0;
	switch (index)
	{
		case ABS : fam = famMath; break;
		case INT : fam = famMath; break;
		case MAX : fam = famMath; break;
		case MIN : fam = famMath; break;
		case ZeroIfNeg : fam = famMath; break;
		case ZeroIfPos : fam = famMath; break;
		case ZeroIfSup : fam = famMath; break;
		case ZeroIfInf : fam = famMath; break;
        case BitAnd : fam = famMath; break;
        case BitOr : fam = famMath; break;
        case BitXor : fam = famMath; break;
        case BitLeft : fam = famMath; break;
        case BitRight : fam = famMath; break;
        case EXP : fam = famMath; break;
        case LOG : fam = famMath; break;
        case Hysteresis : fam = famMath; break;
        case CurrentSkyClearance : fam = famWeather; break;
		case MeanSkyClearance : fam = famWeather; break;
		case SkyClearance : fam = famWeather; break;
        case TemperatureHi : fam = famWeather; break;
        case TemperatureLow : fam = famWeather; break;
        case Humidity : fam = famWeather; break;
        case WindSpeed : fam = famWeather; break;
        case WindDirection : fam = famWeather; break;
        case ActualSunPower : fam = famWeather; break;
        case DaySunPower : fam = famWeather; break;
        case Rain : fam = famWeather; break;
        case ValueRomID : fam = famRomID; break;
		case MeanRomID : fam = famRomID; break;
		case MaxRomID : fam = famRomID; break;
		case MinRomID : fam = famRomID; break;
		case SumRomID : fam = famRomID; break;
        case LSumRomID : fam = famRomID; break;
        case CountRomID : fam = famRomID; break;
        case DataCount : fam = famRomID; break;
		case SlopeMeanRomID : fam = famRomID; break;
		case SlopeSumPosRomID : fam = famRomID; break;
		case SlopeSumNegRomID : fam = famRomID; break;
		case Step : fam = famMath; break;
		case CurrentYear : fam = famTime; break;
		case CurrentMonth : fam = famTime; break;
		case CurrentWeek : fam = famTime; break;
		case CurrentDay : fam = famTime; break;
        case YearBefore : fam = famTime; break;
        case MonthBefore : fam = famTime; break;
        case ProgramValue : fam = famTime; break;
        case WeekProgValue : fam = famTime; break;
        case Minute : fam = famTime; break;
		case Hour : fam = famTime; break;
		case DayOfWeek : fam = famTime; break;
		case DayOfMonth : fam = famTime; break;
        case MonthOfYear : fam = famTime; break;
		case Year : fam = famTime; break;
        case SunSet : fam = famTime; break;
        case SunRise : fam = famTime; break;
        case PID : fam = famMath; break;
		case DSP : fam = famMath; break;
		case webParse : fam = webFamily; break;
		case webMail : fam = webFamily; break;
        case webSms : fam = webFamily; break;
    }
	return fam;
}




QString calcthread::op2Description(int index)
{
	QString str = "";
	switch (index)
	{
        case ABS : str = tr("ABS(x) : Absolute"); break;
        case INT : str = tr("INT(x) : Integer value"); break;
		case MAX : str = tr("Maximum(x ; max) : set result to max if x is obove max"); break;
		case MIN : str = tr("Minimum(x ; min) : set result to min if x if below min"); break;
		case ZeroIfNeg : str = tr("ZeroIfNeg(x) : nullptr if x negative"); break;
		case ZeroIfPos : str = tr("ZeroIfPos(x) : nullptr if x positive"); break;
		case ZeroIfSup : str = tr("ZeroIfSup(x; v) : nullptr if x superior at v"); break;
		case ZeroIfInf : str = tr("ZeroIfInf(x; v) : nullptr if x inferior at v"); break;
        case BitAnd : str = tr("AND(a; b; format) : AND logical between a and b, format defines number of bits, it can be 8, 16, 32 or 64, result is unsigned by default, if you want it signed add S before the number of bit ex : S8, S16, S32, S64"); break;
        case BitOr : str = tr("OR(a; b; format) : OR logical between a and b, format defines number of bits, it can be 8, 16, 32 or 64, result is unsigned by default, if you want it signed add S before the number of bit ex : S8, S16, S32, S64"); break;
        case BitXor : str = tr("XOR(a; b; format) : XOR logical between a and b, format defines number of bits, it can be 8, 16, 32 or 64, result is unsigned by default, if you want it signed add S before the number of bit ex : S8, S16, S32, S64"); break;
        case BitLeft : str = tr("LEFT(a; n; format) : LEFT shift logical for a, n is the shift amout, format defines number of bits, it can be 8, 16, 32 or 64, result is unsigned by default, if you want it signed add S before the number of bit ex : S8, S16, S32, S64"); break;
        case BitRight : str = tr("RIGHT(a; n; format) : RIGTH shift logical for a, n is the shitf amount, format defines number of bits, it can be 8, 16, 32 or 64, result is unsigned by default, if you want it signed add S before the number of bit ex : S8, S16, S32, S64"); break;
        case EXP : str = tr("exp(x) : Exponential x"); break;
        case LOG : str = tr("log(x) : Logarithm x"); break;
        case Hysteresis : str = tr("Hysteresis(x; v1; v2; default) : variable is x, hysteresis limits v1 & v2, default value when output is unknowned 0 or 1, any other value will be interpreted as 1"); break;
        case CurrentSkyClearance : str = tr("CurrentSkyClearance() : current sky transmittance\nfrom 0% to 100% and according weather forecast"); break;
		case MeanSkyClearance : str = tr("MeanSkyClearance(x) : Mean of sky transmittance for the number of x next days\nfrom 0% to 100% and according weather forecast"); break;
		case SkyClearance : str = tr("SkyClearance(x) : Sky transmittance day x since today\nfrom 0% to 100% and according weather forecast"); break;
        case Rain : str = tr("Rain(x) : Rain statistic day x from 1 = today to number of days in weather forecast\nfrom 0% to 100% and according weather forecast"); break;
        case TemperatureHi : str = tr("TemperatureHi(x) : Highest temperatre in weather forecast day x from 0 = to number of days in weather configuration (1 today , 2 tomorrow ...)"); break;
        case TemperatureLow : str = tr("TemperatureLow(x) : Lowest temperatre in weather forecast day x from 0 = to number of days in weather configuration (1 today, 2 tomorrow ...)"); break;
        case Humidity : str = tr("Humidity(x) : real time humidity in weather forecast day x from 0 = to number of days in weather configuration (0 real time value, actual day 1, tomorrow 2 ...)"); break;
        case WindSpeed : str = tr("WindSpeed(x) : real time wind speed in weather forecast day x from 0 = to number of days in weather configuration (0 real time value, actual day 1, tomorrow 2 ...)"); break;
        case WindDirection : str = tr("WindDirection(x) : real time wind direction in weather forecast day x from 0 = to number of days in weather configuration (0 real time value, actual day 1, tomorrow 2 ...)"); break;
        case ActualSunPower : str = tr("ActualSunPower() : real time sun power, for given pannel surface, at given angle, for GPS location in watts"); break;
        case DaySunPower : str = tr("DaySunPower() : Total sun power, for given pannel surface, at given angle, for GPS location in watts"); break;
		case ValueRomID : str = tr("ValueRomID(RomID; Minutes; Width) : Value for specified device at Minutes before actual time and search Width if no data available"); break;
        case MeanRomID : str = tr("MeanRomID(RomID; Minutes; Length) : Mean for specified device begining from Minutes before actual time, Length is optional and is used to specify the amount of minute after the begining instead of the actual time"); break;
        case MaxRomID : str = tr("MaxRomID(RomID; Minutes; Length) : Max for specified device begining from Minutes before actual time, Length is optional and is used to specify the amount of minute after the begining instead of the actual time"); break;
        case MinRomID : str = tr("MinRomID(RomID; Minutes; Length) : Min for specified device begining from Minutes before actual time, Length is optional and is used to specify the amount of minute after the begining instead of the actual time"); break;
        case SumRomID : str = tr("SumRomID(RomID; Minutes; Length) : Sum for specified device begining from Minutes before actual time, Length is optional and is used to specify the amount of minute after the begining instead of the actual time"); break;
        case LSumRomID : str = tr("LSumRomID(RomID; Minutes; Limit) : Sum for specified device begining from Minutes before actual time, Limit define maximum RomID value taken  in the calculation, specially for relative counter when there is offset problem after power cutoff"); break;
        case CountRomID : str = tr("CountRomID(RomID; Minutes; Level; Length) : Count the number of values which are higher or equal to Level, begining from Minutes before actual time, Length is optional and is used to specify the amount of minute after the begining instead of the actual time"); break;
        case DataCount : str = tr("DataCount(RomID; Minutes; Length) : Number of data found for specified device for Minutes"); break;
		case SlopeMeanRomID : str = tr("SlopeMeanRomID(RomID; Minutes) : Slope mean for specified device for Minutes"); break;
		case SlopeSumPosRomID : str = tr("SlopeSumPosRomID(RomID; Minutes; Threshold) : Positive slope cumulation for specified device for Minutes, taking minimum shifts of Threshold"); break;
		case SlopeSumNegRomID : str = tr("SlopeSumNegRomID(RomID; Minutes; Threshold) : Negative slope cumulation for specified device for Minutes, taking minimum shifts of Threshold"); break;
		case Step : str = tr("Step(value; step) : return multiple of step"); break;
        case CurrentYear : str =  tr("CurrentYear(starting month) : give the number of minute since de begining of the starting year, or 1st of january if no parameter is given"); break;
        case CurrentMonth : str =  tr("CurrentMonth(starting day) : give the number of minute since de begining of the starting month (limited to 28), or 1st day if no parameter is given"); break;
		case CurrentWeek : str =  tr("CurrentWeek(starting day) : give the number of minute since de begining of the week starting day (1 Monday -> 7 Sunday), or monday if no parameter is given"); break;
		case CurrentDay : str = tr("CurrentDay(starting hour) : give the number of minute since de begining of the starting hour (24h format), or midnigth if no parameter is given"); break;
        case YearBefore : str =  tr("YearBefore(number of years before) : give the number of minute since the number of years before, taking the begning of the year"); break;
        case MonthBefore : str =  tr("MonthBefore(number of months before) : give the number of minute since number of months before, taking the begning of the month"); break;
        case Minute : str = tr("Minute() : give the current minute"); break;
		case Hour : str = tr("Hour() : give the current hour"); break;
		case DayOfWeek : str = tr("DayOfWeek() : give the current day number of the week (1-7)"); break;
		case DayOfMonth : str = tr("DayOfMonth() : give the current day number of the month"); break;
		case MonthOfYear : str = tr("MonthOfYear() : give the number month current"); break;
		case Year : str = tr("Year() : give the current year"); break;
        case SunSet : str = tr("SunSet() : give the sun set time in minute\nfor the given GPS location in the sun configuration in the palette"); break;
        case SunRise : str = tr("SunRise() : give the sun rise time in minute\nfor the given GPS location in the sun configuration in the palette"); break;
        case ProgramValue : str = tr("ProgramValue(program name) : give the value corresponing to the program name\nNote the progam name cannot contain any space character or bracket to work correctly"); break;
        case WeekProgValue : str = tr("WeekProgValue(program name) : give the value corresponing to the week program name\nNote the progam name cannot contain any space character or bracket to work correctly"); break;
        case PID : str = tr("PID(Actual; setPoint; P; I; D) : "); break;
		case DSP : str = tr("DSP(RomID; Pole; Gain; Polynome) : polynomes can be found here :   http://www-users.cs.york.ac.uk/~fisher/mkfilter/"); break;
        case webParse : str = tr("html parsing\nwebparse\nwebpage=http://bleuciel.edf.com/abonnement-et-contrat/les-prix/les-prix-de-l-electricite/option-tempo/la-couleur-du-jour-2585.html&coe_i_id=2585\nsearch1=<h4>Demain\nsearch2=<span class=\"period\">\nend=</span>\nBlanc=(1)\nBleu=(2)\nRouge=(3)"); break;
        case webMail : str = tr("Sending eMail\nwebmail\nmailAddress=machin@orange.fr\nmailAddress=toto@orange.fr\nmailAddress=bidule@orange.fr\ntextSubject=Alarme Temprature PAC\ntextMail=file.txt\nmailAction=sendifnullr/sendifnotnullr/sendifna\nformula=T_PAC < 1"); break;
        case webSms : str = tr("Sending SMS : \nwebsms\nsmsHttpRequest=https://smsapi.free-mobile.fr/sendmsg?user=1234567890&pass=xxxxxxxxxx&msg=\ntextSMS=file.txt or any text\nsmsAction=sendifnullr/sendifnotnullr/sendifna\nformula=T_PAC < 1"); break;
    }
	return str;
}





QString calcthread::getID(QString &str)
{
	int coma = str.indexOf("(");
	if (coma == -1) return "";

	int nextcoma = str.indexOf(")", coma + 1);
	if (nextcoma == -1) nextcoma = str.indexOf("(", coma + 1);
	if (nextcoma == -1) return "";
	QString result = str.mid(coma + 1, nextcoma - coma - 1);
	return result;
}




QDate calcthread::toDate(const QString &S, bool *ok)
{
    *ok = false;
    int year = -1;
    int month = -1;
    if (S.length() != 7) return QDate(year, month, 1);
    // Format must be yyyy_mm or mm_yyyy
    if (S.mid(4,1) == "_")  // yyyy_mm
    {
        bool ok;
        int y = S.left(4).toInt(&ok);
        if (ok) year = y;
        int m = S.right(2).toInt(&ok);
        if (ok) month = m;
    }
    else if (S.mid(2,1) == "_") // mm_yyyy
    {
        bool ok;
        int y = S.right(4).toInt(&ok);
        if (ok) year = y;
        int m = S.left(2).toInt(&ok);
        if (ok) month = m;
    }
    if ((year > 0) && (month > 0) && (month < 13))
    {
        *ok = true;
        textBrowserResult += "\n" + QString("Date is ok 1 ") + QDate(year, month, 1).toString();
        return QDate(year, month, 1);
    }
    return QDate(year, month, 1);
}



double calcthread::toNumeric(const QString &S, bool *ok)
{
    double v = logisdom::NA;
    onewiredevice *device = checkDevice(S);
    *ok = false;
    if (S.left(1) == "V")
    {
        int index = S.right(S.length() - 1).toInt(ok);
        if (*ok) if (index < V.count())
        {
            v = V[index];
            *ok = true;
        }
    }
    else if (S.left(1) == "X")
    {
        int index = S.right(S.length() - 1).toInt(ok);
        if (*ok) if (index < NPOLES)
        {
            scroolDSP();
             v = xv[index];
            *ok = true;
        }
    }
    else if (S.left(1) == "Y")
    {
        int index = S.right(S.length() - 1).toInt(ok);
        if (*ok) if (index < NPOLES)
        {
            scroolDSP();
             v = yv[index];
            *ok = true;
        }
    }
    else if (device)
    {
        if (TCalc) v = device->getMainValue(*TCalc, deviceLoading, parent);
            else v = device->getMainValue();
        if (logisdom::isNA(v)) dataValid = false;
        if (!TCalc) if (!device->isValid()) dataValid = false;
        if (!dataValid) *ok = false; else *ok = true;
    }
    else if (S == "TARGET")
    {
        if (logisdom::isNA(target)) *ok = false;
        else
        {
            v = target;
            *ok = true;
        }
    }
    if (logisdom::isNA(v))
    {
        bool ok_date = false;
        QDate date = toDate(S, &ok_date);
        if (!ok_date)
        {
            syntax = false;
            textBrowserResult += "\n" + (tr("Value error")) + " : " + S;
        }
        else
        {
            *ok = true;
            textBrowserResult += "\n" + QString("Date is ok 2 ") + date.toString();
        }
    }
    return v;
}




void calcthread::scroolDSP()
{
    QDateTime T;
    if (!DSPdevice) return;
    if (!DSPdevice->isValid())
    {
        dataError(tr("Device not ready ") + DSPdevice->getromid());
        return;
    }
    if (logisdom::isZero(DSPpole)) return;
    if (logisdom::isZero(DSPGain)) return;
    if (!(DSPpole < NPOLES)) return;
    if (scroolDone) return;
    double x = logisdom::NA;
    if (TCalc)
    {
        T = *TCalc;
        x = DSPdevice->getMainValue(T, deviceLoading, parent);
    }
    else x = DSPdevice->getMainValue();
    if (logisdom::isNA(x)) return;
    for (int n=0; n<DSPpole; n++)
    {
        xv[n] = xv[n + 1];
        yv[n] = yv[n + 1];
    }
    xv[DSPpole] = x / DSPGain;
    if (!TCalc)
    {
        textBrowserResult += "\n" + ( QString("x = %2").arg(x, 0, 'e'));
        for (int n=0; n<DSPpole+1; n++)
        {
            textBrowserResult += "\n" + ( QString("X%1 = %2").arg(n).arg(xv[n], 0, 'e'));
            textBrowserResult += "\n" + ( QString("Y%1 = %2").arg(n).arg(yv[n], 0, 'e'));
        }
    }
    scroolDone = true;
}





void calcthread::resetDSP()
{
    QDateTime T;
    if (restartDSP)
    {
        restartDSP = false;
        if (!DSPdevice) return;
        if (DSPpole == 0) return;
        if (logisdom::isZero(DSPGain)) return;
        if (!(DSPpole < NPOLES)) return;
        double y = logisdom::NA;
        if (TCalc)
        {
            T = *TCalc;
            y = DSPdevice->getMainValue(T, deviceLoading, parent);
        }
        else y = DSPdevice->getMainValue();
        if (logisdom::isNA(y))
        {
            for (int n=0; n<DSPpole + 1; n++)
            {
                xv[n] = 0;
                yv[n] = 0;
            }
             return;
        }
        else
        {
            double x = y / DSPGain;
            for (int n=0; n<DSPpole + 1; n++)
            {
                xv[n] = x;
                yv[n] = y;
            }
        }
    }
    else
    {
        for (int n=0; n<NPOLES + 1; n++)
        {
            xv[n] = 0;
            yv[n] = 0;
        }
    }
    restartDSP = false;
}




bool calcthread::resoudreParenthese()
{
    int OPindex = -1;
    int Pferm = Calc.indexOf(")");
    QString C;
    QString OP;
    if (Pferm != -1)
    {
        int Pouv = Calc.left(Pferm).lastIndexOf("(");
        if (Pouv == -1) syntaxError(tr("Missing opening bracket"));
        ManageError
        OP = "";
        if (Pouv > 1)
        {
            QString checkOP = Calc.mid(Pouv - 1, 1);
            if (!isOperator(checkOP))
            {
                OPindex = Pouv;
                while (--OPindex >= 0)
                {
                    QString check = Calc.mid(OPindex, 1);
                    if (isOperator(check) or (check == "(") or (check == ";")) break;
                }
                OP = Calc.mid(OPindex + 1, Pouv - 1 - OPindex);
                textBrowserResult += "\n" + ("Try function  : " + OP);
            }
        }
        C = Calc.mid(Pouv + 1, Pferm - Pouv - 1);
        QStringList params = C.split(";");
        C.clear();
        quint16 loop = 65535;
        for (int n=0; n<params.count(); n++)
        {
            if (!C.isEmpty()) C.append(";");
            QString Cc = params.at(n);
            while (pw(Cc) and loop) { loop--; } ManageError
            while (diviser(Cc) and loop) { loop--; } ManageError
            while (multiplier(Cc) and loop) { loop--; } ManageError
            while (sup(Cc) and loop) { loop--; } ManageError
            while (inf(Cc) and loop) { loop--; } ManageError
            while (egal(Cc) and loop) { loop--; } ManageError
            while (different(Cc) and loop) { loop--; } ManageError
            while (enlever(Cc) and loop) { loop--; } ManageError
            while (ajouter(Cc) and loop) { loop--; } ManageError
            C.append(Cc);
        }
        if (OP.isEmpty())
        {
            if (isNumeric(C))
            {
                bool ok;
                double D = C.toDouble(&ok);
                int index = V.count();
                V.append(D);
                QString P = Calc.left(Pouv);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                Calc = P + QString("V%1").arg(index) + S;
                textBrowserResult += "\n" + (QString("V%1 = %2").arg(index).arg(D, 0, 'e'));
            }
            else
            {
                QString P = Calc.left(Pouv);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                Calc = P + C + S;
            }
            textBrowserResult += "\n" + QString::fromLatin1("Done : ") + Calc;
            return true;
        }
        else
        {
            if (isNumeric(C))
            {
                bool ok = false;
                double opD = runOP(OP, C, &ok);
                if (!ok) { syntaxError("Function error : " + OP); ManageError }
                int index = V.count();
                V.append(opD);
                QString P = Calc.left(OPindex + 1);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                Calc = P + QString("V%1").arg(index) + S;
                QString str;
                str += QString("V%1 = ").arg(index);
                str += OP;
                str += "(" + C + ")";
                str += QString(" = %1").arg(opD, 0, 'e');
                textBrowserResult += "\n" + (str);
            }
            else
            {
                bool ok = false;
                double opD = runOP(OP, C, &ok);
                if (!ok) { syntaxError(tr("Function error : ") + OP); ManageError }
                int index = V.count();
                V.append(opD);
                QString P = Calc.left(OPindex + 1);
                QString S = Calc.right(Calc.length() - Pferm - 1);
                QString str;
                str += QString("V%1 = ").arg(index);
                str += OP;
                str += "(" + C + ")";
                str += QString(" = %1").arg(opD, 0, 'e');
                textBrowserResult += "\n" + (str);
                Calc = P + QString("V%1").arg(index) + S;
            }
            textBrowserResult += "\n" + QString::fromLatin1("Function done : ") + Calc;
            return true;
        }
    }
    return false;
}




double calcthread::runOP(const QString &OP ,const QString &C, bool *ok)
{
	double r = logisdom::NA;
    r = 65;
	*ok = true;
	QString Empty = "";
	QStringList P;
	int pos, begin = 0;
	pos = C.indexOf(";");
	if (pos == -1)
	{
		P.append(C);
	}
	else
	{
		while (pos != -1)
		{
			P.append(C.mid(begin, pos - begin));
			begin = pos + 1;
			pos = C.indexOf(";", begin);
		}
		P.append(C.mid(begin, C.length() - begin));
	}
	int count = P.count();
	textBrowserResult += "\n" + (QString("Found %1 parameters").arg(count));
	for (int n=0; n<count; n++)
		textBrowserResult += "\n" + (QString("P%1 = ").arg(n) + P[n]);
	if (count == 0)
		textBrowserResult += "\n" + ("Could not find parameter in function " + OP);
	bool found = false;
	for (int n=0; n<lastOperator; n++)
		if (op2Str(n) == OP)
		{
            found = true;
            switch (n)
			{
                case ABS : if (P.count() == 1) r = Abs(P[0]); break;
				case INT : if (P.count() == 1) r = Entier(P[0]); break;
				case MAX : if (P.count() == 2) r = Max(P[0], P[1]); break;
				case MIN : if (P.count() == 2) r = Min(P[0], P[1]); break;
				case ZeroIfNeg :  if (P.count() == 1) r = ZifNeg(P[0]); break;
				case ZeroIfPos :  if (P.count() == 1) r = ZifPos(P[0]); break;
				case ZeroIfSup :  if (P.count() == 2) r = ZifSup(P[0], P[1]); break;
				case ZeroIfInf :  if (P.count() == 2) r = ZifInf(P[0], P[1]); break;
                case EXP :  if (P.count() == 1) r = Exp(P[0]); break;
                case LOG :  if (P.count() == 1) r = Log(P[0]); break;
                case BitAnd :  if (P.count() == 3) r = getBitAnd(P[0], P[1], P[2]); break;
                case BitOr :  if (P.count() == 3) r = getBitOr(P[0], P[1], P[2]); break;
                case BitXor :  if (P.count() == 3) r = getBitXor(P[0], P[1], P[2]); break;
                case BitLeft :  if (P.count() == 3) r = getBitLeft(P[0], P[1], P[2]); break;
                case BitRight :  if (P.count() == 3) r = getBitRight(P[0], P[1], P[2]); break;
                case Hysteresis :  if (P.count() == 4) r = Hyst(P[0], P[1], P[2], P[3]); break;
                case CurrentSkyClearance : r = getCurrentSkyClearance(); break;
                case MeanSkyClearance : if (P.count() == 1) r = getMeanSkyClearance(P[0]); break;
                case SkyClearance : if (P.count() == 1) r = getSkyClearance(P[0]); break;
                case TemperatureHi : if (P.count() == 1) r = getActualTempHi(P[0]); break;
                case TemperatureLow : if (P.count() == 1) r = getActualTempLow(P[0]); break;
                case Humidity : if (P.count() == 1) r = getActualHumidity(P[0]); break;
                case WindSpeed : if (P.count() == 1) r = getActualWind(P[0]); break;
                case WindDirection : r = getActualWindDir(P[0]); break;
                case ActualSunPower : r = getActualSunPower(); break;
                case DaySunPower : r = getDaySunPower(); break;
                case Rain : r = getRain(P[0]); break;
                case ValueRomID : if (P.count() == 3) r = getValueRomID(P[0], P[1], P[2]);
                            break;
                case MeanRomID :    if (P.count() == 2) r = getMeanRomID(P[0], P[1]);
                                    if (P.count() == 3) r = getMeanRomID(P[0], P[1], P[2]);
                            break;
                case MaxRomID :     if (P.count() == 2) r = getMaxRomID(P[0], P[1]);
                                    if (P.count() == 3) r = getMaxRomID(P[0], P[1], P[2]);
                            break;
                case MinRomID :     if (P.count() == 2) r = getMinRomID(P[0], P[1]);
                                    if (P.count() == 3) r = getMinRomID(P[0], P[1], P[2]);
                            break;
                case SumRomID :     if (P.count() == 2) r = getSumRomID(P[0], P[1]);
                                    if (P.count() == 3) r = getSumRomID(P[0], P[1], P[2]);
                            break;
                case LSumRomID :     if (P.count() == 3) r = getLSumRomID(P[0], P[1], P[2]);
                            break;
                case CountRomID :   if (P.count() == 3) r = getCountRomID(P[0], P[1], P[2]);
                                    if (P.count() == 4) r = getCountRomID(P[0], P[1], P[2], P[3]);
                            break;
                case DataCount :    if (P.count() == 2) r = getDataCount(P[0], P[1]);
                                    if (P.count() == 3) r = getDataCount(P[0], P[1], P[2]);
                            break;
                case SlopeMeanRomID : if (P.count() == 2) r = getSlopeMeanRomID(P[0], P[1]);
                            break;
                case SlopeSumPosRomID : if (P.count() == 3) r = getSlopeSumPosRomID(P[0], P[1], P[2]);
                            break;
                case SlopeSumNegRomID : if (P.count() == 3) r = getSlopeSumNegRomID(P[0], P[1], P[2]);
                            break;
                case Step : if (P.count() == 2) r = getStep(P[0], P[1]);
                            break;
                case CurrentYear : if (P.count() == 1) r = getCurrentYear(P[0]); else r = getCurrentYear(Empty);
                            break;
                case CurrentMonth : if (P.count() == 1) r = getCurrentMonth(P[0]); else r = getCurrentMonth(Empty);
                            break;
                case CurrentWeek :  if (P.count() == 1) r = getCurrentWeek(P[0]); else r = getCurrentWeek(Empty);
                            break;
                case CurrentDay :  if (P.count() == 1) r = getCurrentDay(P[0]); else r = getCurrentDay(Empty);
                            break;
                case YearBefore :  if (P.count() == 1) r = getYearBefore(P[0]);
                            break;
                case MonthBefore :  if (P.count() == 1) r = getMonthBefore(P[0]);
                            break;
                case Minute : r = getMinute();
                            break;
                case Hour : r = getHour();break;
                case DayOfWeek : r = getDayOfWeek(); break;
                case DayOfMonth : r = getDayOfMonth(); break;
                case MonthOfYear : r = getMonthOfYear(); break;
                case Year : r = getYear(); break;
                case SunSet : r = getSunSet(); break;
                case SunRise : r = getSunRise(); break;
                case ProgramValue :  if (P.count() == 1) r = getProgramValue(P[0]); break;
                case WeekProgValue :  if (P.count() == 1) r = getWeekProgValue(P[0]); break;
                case PID :  if (P.count() == 5) r = getPID(P[0], P[1], P[2], P[3], P[4]); break;
                case DSP :  if (P.count() == 4) r = getDSP(P[0], P[1], P[2], P[3]); break;
                default : syntaxError(tr("Function ") + OP + tr(" not defined in library line 378 calcthread.cpp")); found = false; break;
            }
		}
    if (!TCalc) textBrowserResult += "\n" + QString(tr("Function result = %1").arg(r, 0, 'e'));
	if (!found)
	{
		*ok = false;
		 syntaxError(tr("Function not founded ") + OP);
		 return 0;
	}
    if ((logisdom::isNA(r)) or (!syntax) or (!dataValid))
	{
		*ok = false;
		 return 0;
	}
	return r;
}





double calcthread::getValueFromP(const QString &S)
{
    bool ok = false;
	double D = 0;
	if (isNumeric(S))
	{
		D = S.toDouble(&ok);
		if (!ok)
		{
			textBrowserResult += "\n" + ("Unkown value : " + S);
			syntax = false;
			return 0;
		}
	}
	else
	{
		D = toNumeric(S, &ok);
		if (!ok)
		{
			textBrowserResult += "\n" + ("Unkown value : " + S);
			syntax = false;
			return 0;
		}
	}
	return D;
}






double calcthread::Abs(const QString &S)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	D = qAbs(D);
	return D;
}




double calcthread::Entier(const QString &S)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	D = qRound(D);
	return D;
}



double calcthread::Max(const QString &x, const QString &max)
{
	double X = logisdom::NA;
	X = getValueFromP(x);
	if (!syntax) return 0;
	double Mx = getValueFromP(max);
	if (!syntax) return 0;
	if (X > Mx) return Mx;
	return X;
}




double calcthread::Min(const QString &x, const QString &min)
{
	double X = logisdom::NA;
	X = getValueFromP(x);
	if (!syntax) return 0;
	double Mn = getValueFromP(min);
	if (!syntax) return 0;
	if (X < Mn) return Mn;
	return X;
}




double calcthread::ZifNeg(const QString &S)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	if (D > 0) return D;
	return 0;
}




double calcthread::ZifPos(const QString &S)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	if (D < 0) return D;
	return 0;
}






double calcthread::ZifSup(const QString &S, const QString &val)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	double V = getValueFromP(val);
	if (!syntax) return 0;
	if (D <= V) return D;
	return 0;
}




double calcthread::ZifInf(const QString &S, const QString &val)
{
	double D = logisdom::NA;
	D = getValueFromP(S);
	if (!syntax) return 0;
	double V = getValueFromP(val);
	if (!syntax) return 0;
	if (D >= V) return D;
	return 0;
}



double calcthread::Exp(const QString &a)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    return exp(A);
}



double calcthread::Log(const QString &a)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    return log(A);
}



double calcthread::getBitAnd(const QString &a, const QString &b, const QString &f)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    double B = logisdom::NA;
    B = getValueFromP(b);
    if (!syntax) return 0;
    double R = logisdom::NA;
    quint64 a64 = quint64(A);
    quint64 b64 = quint64(B);
    qint64 as64 = qint64(A);
    qint64 bs64 = qint64(B);
    if (f == "S64")
    {
        qint64 inta = as64;
        qint64 intb = bs64;
        qint64 r = inta & intb;
        R = r;
    }
    else if (f == "S32")
    {
        as64 &= 0x00000000FFFFFFFF;
        bs64 &= 0x00000000FFFFFFFF;
        qint32 inta = qint32(as64);
        qint32 intb = qint32(bs64);
        qint32 r = inta & intb;
        R = r;
    }
    else if (f == "S16")
    {
        as64 &= 0x000000000000FFFF;
        bs64 &= 0x000000000000FFFF;
        qint16 inta = qint16(as64);
        qint16 intb = qint16(bs64);
        qint16 r = inta & intb;
        R = r;
    }
    else if (f == "S8")
    {
        as64 &= 0x00000000000000FF;
        bs64 &= 0x00000000000000FF;
        qint8 inta = qint8(as64);
        qint8 intb = qint8(bs64);
        qint8 r = inta & intb;
        R = r;
    }
    else if (f == "64")
    {
        quint64 inta = a64;
        quint64 intb = b64;
        quint64 r = inta & intb;
        R = r;
    }
    else if (f == "32")
    {
        a64 &= 0x00000000FFFFFFFF;
        b64 &= 0x00000000FFFFFFFF;
        quint32 inta = quint32(a64);
        quint32 intb = quint32(b64);
        quint32 r = inta & intb;
        R = r;
    }
    else if (f == "8")
    {
        a64 &= 0x00000000000000FF;
        b64 &= 0x00000000000000FF;
        quint8 inta = quint8(a64);
        quint8 intb = quint8(b64);
        quint8 r = inta & intb;
        R = r;
    }
    else // defaul 16
    {
        a64 &= 0x000000000000FFFF;
        b64 &= 0x000000000000FFFF;
        quint16 inta = quint16(a64);
        quint16 intb = quint16(b64);
        quint16 r = inta & intb;
        R = r;
    }
    return R;
}


double calcthread::getBitOr(const QString &a, const QString &b, const QString &f)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    double B = logisdom::NA;
    B = getValueFromP(b);
    if (!syntax) return 0;
    double R = logisdom::NA;
    quint64 a64 = quint64(A);
    quint64 b64 = quint64(B);
    qint64 as64 = qint64(A);
    qint64 bs64 = qint64(B);
    if (f == "S64")
    {
        qint64 inta = as64;
        qint64 intb = bs64;
        qint64 r = inta | intb;
        R = r;
    }
    else if (f == "S32")
    {
        as64 &= 0x00000000FFFFFFFF;
        bs64 &= 0x00000000FFFFFFFF;
        qint32 inta = qint32(as64);
        qint32 intb = qint32(bs64);
        qint32 r = inta | intb;
        R = r;
    }
    else if (f == "S16")
    {
        as64 &= 0x000000000000FFFF;
        bs64 &= 0x000000000000FFFF;
        qint16 inta = qint16(as64);
        qint16 intb = qint16(bs64);
        qint16 r = inta | intb;
        R = r;
    }
    else if (f == "S8")
    {
        as64 &= 0x00000000000000FF;
        bs64 &= 0x00000000000000FF;
        qint8 inta = qint8(as64);
        qint8 intb = qint8(bs64);
        qint8 r = inta | intb;
        R = r;
    }
    else if (f == "64")
    {
        quint64 inta = a64;
        quint64 intb = b64;
        quint64 r = inta | intb;
        R = r;
    }
    else if (f == "32")
    {
        a64 &= 0x00000000FFFFFFFF;
        b64 &= 0x00000000FFFFFFFF;
        quint32 inta = quint32(a64);
        quint32 intb = quint32(b64);
        quint32 r = inta | intb;
        R = r;
    }
    else if (f == "8")
    {
        a64 &= 0x00000000000000FF;
        b64 &= 0x00000000000000FF;
        quint8 inta = quint8(a64);
        quint8 intb = quint8(b64);
        quint8 r = inta | intb;
        R = r;
    }
    else // defaul 16
    {
        a64 &= 0x000000000000FFFF;
        b64 &= 0x000000000000FFFF;
        quint16 inta = quint16(a64);
        quint16 intb = quint16(b64);
        quint16 r = inta | intb;
        R = r;
    }
    return R;
}

double calcthread::getBitXor(const QString &a, const QString &b, const QString &f)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    double B = logisdom::NA;
    B = getValueFromP(b);
    if (!syntax) return 0;
    double R = logisdom::NA;
    quint64 a64 = quint64(A);
    quint64 b64 = quint64(B);
    qint64 as64 = qint64(A);
    qint64 bs64 = qint64(B);
    if (f == "S64")
    {
        qint64 inta = as64;
        qint64 intb = bs64;
        qint64 r = inta ^ intb;
        R = r;
    }
    else if (f == "S32")
    {
        as64 &= 0x00000000FFFFFFFF;
        bs64 &= 0x00000000FFFFFFFF;
        qint32 inta = qint32(as64);
        qint32 intb = qint32(bs64);
        qint32 r = inta ^ intb;
        R = r;
    }
    else if (f == "S16")
    {
        as64 &= 0x000000000000FFFF;
        bs64 &= 0x000000000000FFFF;
        qint16 inta = qint16(as64);
        qint16 intb = qint16(bs64);
        qint16 r = inta ^ intb;
        R = r;
    }
    else if (f == "S8")
    {
        as64 &= 0x00000000000000FF;
        bs64 &= 0x00000000000000FF;
        qint8 inta = qint8(as64);
        qint8 intb = qint8(bs64);
        qint8 r = inta ^ intb;
        R = r;
    }
    else if (f == "64")
    {
        quint64 inta = a64;
        quint64 intb = b64;
        quint64 r = inta ^ intb;
        R = r;
    }
    else if (f == "32")
    {
        a64 &= 0x00000000FFFFFFFF;
        b64 &= 0x00000000FFFFFFFF;
        quint32 inta = quint32(a64);
        quint32 intb = quint32(b64);
        quint32 r = inta ^ intb;
        R = r;
    }
    else if (f == "8")
    {
        a64 &= 0x00000000000000FF;
        b64 &= 0x00000000000000FF;
        quint8 inta = quint8(a64);
        quint8 intb = quint8(b64);
        quint8 r = inta ^ intb;
        R = r;
    }
    else // defaul 16
    {
        a64 &= 0x000000000000FFFF;
        b64 &= 0x000000000000FFFF;
        quint16 inta = quint16(a64);
        quint16 intb = quint16(b64);
        quint16 r = inta ^ intb;
        R = r;
    }
    return R;
}



double calcthread::getBitLeft(const QString &a, const QString &l, const QString &f)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    double L = logisdom::NA;
    L = getValueFromP(l);
    if (!syntax) return 0;
    double R = logisdom::NA;
    quint64 a64 = quint64(A);
    quint64 l64 = quint64(L);
    qint64 as64 = qint64(A);
    qint64 ls64 = qint64(L);
    if (f == "S64")
    {
        qint64 inta = as64;
        qint64 intl = ls64;
        qint64 r = inta << intl;
        R = r;
    }
    else if (f == "S32")
    {
        as64 &= 0x00000000FFFFFFFF;
        ls64 &= 0x00000000FFFFFFFF;
        qint32 inta = qint32(as64);
        qint32 intl = qint32(ls64);
        qint32 r = inta << intl;
        R = r;
    }
    else if (f == "S16")
    {
        as64 &= 0x000000000000FFFF;
        ls64 &= 0x000000000000FFFF;
        int inta = qint16(as64);
        int intl = qint16(ls64);
        int r = inta << intl;
        R = r;
    }
    else if (f == "S8")
    {
        as64 &= 0x00000000000000FF;
        ls64 &= 0x00000000000000FF;
        int inta = qint8(as64);
        int intl = qint8(ls64);
        int r = inta << intl;
        R = r;
    }
    else if (f == "64")
    {
        quint64 inta = a64;
        quint64 intl = l64;
        quint64 r = inta << intl;
        R = r;
    }
    else if (f == "32")
    {
        a64 &= 0x00000000FFFFFFFF;
        l64 &= 0x00000000FFFFFFFF;
        quint32 inta = quint32(a64);
        quint32 intl = quint32(l64);
        quint32 r = inta << intl;
        R = r;
    }
    else if (f == "8")
    {
        a64 &= 0x00000000000000FF;
        l64 &= 0x00000000000000FF;
        int inta = quint8(a64);
        int intl = quint8(l64);
        int r = inta << intl;
        R = r;
    }
    else // defaul 16
    {
        a64 &= 0x000000000000FFFF;
        l64 &= 0x000000000000FFFF;
        int inta = quint16(a64);
        int intl = quint16(l64);
        int r = inta << intl;
        R = r;
    }
    return R;
}



double calcthread::getBitRight(const QString &a, const QString &l, const QString &f)
{
    double A = logisdom::NA;
    A = getValueFromP(a);
    if (!syntax) return 0;
    double L = logisdom::NA;
    L = getValueFromP(l);
    if (!syntax) return 0;
    double R = logisdom::NA;
    quint64 a64 = quint64(A);
    quint64 l64 = quint64(L);
    qint64 as64 = qint64(A);
    qint64 ls64 = qint64(L);
   if (f == "S64")
    {
        qint64 inta = qint64(as64);
        qint64 intl = qint64(ls64);
        qint64 r = inta >> intl;
        R = r;
    }
    else if (f == "S32")
    {
        as64 &= 0x00000000FFFFFFFF;
        ls64 &= 0x00000000FFFFFFFF;
        qint32 inta = qint32(as64);
        qint32 intl = qint32(ls64);
        qint32 r = inta >> intl;
        R = r;
    }
    else if (f == "S16")
    {
        a64 &= 0x000000000000FFFF;
        l64 &= 0x000000000000FFFF;
        qint16 inta = qint16(as64);
        qint16 intl = qint16(ls64);
        qint16 r = inta >> intl;
        R = r;
    }
    else if (f == "S8")
    {
       a64 &= 0x00000000000000FF;
       l64 &= 0x00000000000000FF;
        qint8 inta = qint8(as64);
        qint8 intl = qint8(ls64);
        qint8 r = inta >> intl;
        R = r;
    }
    else if (f == "64")
    {
        quint64 inta = a64;
        quint64 intl = l64;
        quint64 r = inta >> intl;
        R = r;
    }
    else if (f == "32")
    {
        a64 &= 0x00000000FFFFFFFF;
        l64 &= 0x00000000FFFFFFFF;
        quint32 inta = quint32(a64);
        quint32 intl = quint32(l64);
        quint32 r = inta >> intl;
        R = r;
    }
    else if (f == "8")
    {
        a64 &= 0x00000000000000FF;
        l64 &= 0x00000000000000FF;
        quint8 inta = quint8(a64);
        quint8 intl = quint8(l64);
        quint8 r = inta >> intl;
        R = r;
    }
    else // defaul 16
    {
        a64 &= 0x000000000000FFFF;
        l64 &= 0x000000000000FFFF;
        quint16 inta = quint16(a64);
        quint16 intl = quint16(l64);
        quint16 r = inta >> intl;
        R = r;
    }
    return R;
}




double calcthread::Hyst(const QString &x, const QString &v1, const QString &v2, const QString &def)
{
    double R = logisdom::NA;
    double X = logisdom::NA;
    X = getValueFromP(x);
    if (logisdom::isNA(X)) return lastValueHysteresis;
    double V1 = logisdom::NA;
    V1 = getValueFromP(v1);
    if (logisdom::isNA(V1)) return lastValueHysteresis;
    double V2 = logisdom::NA;
    V2 = getValueFromP(v2);
    if (logisdom::isNA(V2)) return lastValueHysteresis;
    double DEF = logisdom::NA;
    DEF = getValueFromP(def);
    if (int(DEF) != 0) DEF = 1;
    if (logisdom::isNA(DEF)) return lastValueHysteresis;
    if (logisdom::isNA(lastValueHysteresis))
    {
        if (V1 < V2)
        {
            if (X < V1) R = 0;
            else if (X > V2) R = 1;
            else R = DEF;
        }
        else if (V1 > V2)
        {
            if (X < V2) R = 0;
            else if (X > V1) R = 1;
            else R = DEF;
        }
        else
        {
            R = DEF;
        }
    }
    else
    {
        if (V1 < V2)
        {
            if (X < V1) R = 0;
            else if (X > V2) R = 1;
            else R = lastValueHysteresis;
        }
        else if (V1 > V2)
        {
            if (X < V2) R = 0;
            else if (X > V1) R = 1;
            else R = lastValueHysteresis;
        }
        else
        {
            R = lastValueHysteresis;
        }
    }
    lastValueHysteresis = R;
    return R;
}



double calcthread::getSkyClearance(const QString &NbDays)
{
    double Power = 0;
    bool ok = false;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) Power = maison1wirewindow->MeteoArea->getSunPower(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return Power;
}




double calcthread::getCurrentSkyClearance()
{
	double Power = 0;
    bool ok = false;
    if (!syntax) return 0;
	if (maison1wirewindow->MeteoArea) Power = maison1wirewindow->MeteoArea->getCurrentSunPower(&ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return Power;
}





double calcthread::getMeanSkyClearance(const QString &NbDays)
{
	double Power = 0;
    bool ok = false;
    double D = getValueFromP(NbDays);
	if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) Power = maison1wirewindow->MeteoArea->getMeanSunPower(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return Power;
}





double calcthread::getActualTempHi(const QString &NbDays)
{
    bool ok = false;
    double T = logisdom::NA;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) T = maison1wirewindow->MeteoArea->getTempHi(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return T;
}



double calcthread::getActualTempLow(const QString &NbDays)
{
    bool ok = false;
    double T = logisdom::NA;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) T = maison1wirewindow->MeteoArea->getTempLow(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return T;
}



double calcthread::getActualHumidity(const QString &NbDays)
{
    bool ok = false;
    double T = logisdom::NA;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) T = maison1wirewindow->MeteoArea->getHumidity(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return T;
}


double calcthread::getActualWind(const QString &NbDays)
{
    bool ok = false;
    double W = logisdom::NA;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) W = maison1wirewindow->MeteoArea->getWind(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return W;
}



double calcthread::getActualWindDir(const QString &NbDays)
{
    bool ok = false;
    double Wd = logisdom::NA;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) Wd = maison1wirewindow->MeteoArea->getWindDir(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return Wd;
}




double calcthread::getRain(const QString &NbDays)
{
    double Power = 0;
    bool ok = false;
    double D = getValueFromP(NbDays);
    if (!syntax) return 0;
    if (maison1wirewindow->MeteoArea) Power = maison1wirewindow->MeteoArea->getRain(int(D), &ok);
    if (!ok) dataError("Weather data not valid");
    else textBrowserResult += "\nWeather data were valid";
    return Power;
}




double calcthread::getDaySunPower()
{
	double Power = 0;
	if (maison1wirewindow->EnergieSolaire) Power = maison1wirewindow->EnergieSolaire->getSunPower(TCalc);
	return Power;
}





double calcthread::getActualSunPower()
{
	double Power = 0;
	if (maison1wirewindow->EnergieSolaire) Power = maison1wirewindow->EnergieSolaire->getActualSunPower(TCalc);
	return Power;
}




double calcthread::getValueRomID(const QString &RomID, const QString &Minutes, const QString &Width)
{
	double v = logisdom::NA;
    bool ok = false;
	int minutes;
	if (isNumeric(Minutes)) minutes = Minutes.toInt(&ok);
        else minutes = int(toNumeric(Minutes, &ok));
        if (!ok) { syntaxError(tr("Time parameter error ") + RomID); ResultError }
	int width;
	if (isNumeric(Width)) width = Width.toInt(&ok);
        else width = int(toNumeric(Width, &ok));
        if (!ok) { syntaxError(tr("Width parameter error ") + RomID); ResultError }
	onewiredevice *device = checkDevice(RomID);
	QDateTime now = QDateTime::currentDateTime();
    QDateTime T;
    if (!device) { dataError(tr("Device not found ") + RomID); ResultError }
	if (TCalc)
	{
		for (int n=0; (n<width); n++)
		{
			if (stopRequest) break;
			T = TCalc->addSecs((-minutes - n) * 60);
			double V = device->getMainValue(T, deviceLoading, parent);
            if (logisdom::isNotNA(V)) return V;
			T = TCalc->addSecs((-minutes + n) * 60);
			V = device->getMainValue(T, deviceLoading, parent);
            if (logisdom::isNotNA(V)) return V;
		}
	}
	else
	{
		for (int n=0; (n<width); n++)
		{
			if (stopRequest) break;
			T = now.addSecs((-minutes - n) * 60);
			double V = device->getMainValue(T, deviceLoading, parent);
            if (logisdom::isNotNA(V)) return V;
			T = now.addSecs((-minutes + n) * 60);
			V = device->getMainValue(T, deviceLoading, parent);
            if (logisdom::isNotNA(V)) return V;
		}
	}
    if (logisdom::isNA(v)) { dataError(tr("getValueRomID function didn't find enough valid data")); ResultError }
	return v;
}







// C0A80069020RS - ValueRomID(C0A80069020RS; (CurrentDay() + 30); 30)

double calcthread::getMeanRomID(const QString &RomID, const QString &Minutes)
{
    FunctionGetData
	double v = logisdom::NA;
	double sum = 0;
	for (int n=0; n<data.data_Y.count(); n++)
	{
		double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
		{
            if (logisdom::isNA(v)) v = 0;
			v += V;
			sum ++;
		}
	}
	return v / sum;
}



double calcthread::getMeanRomID(const QString &RomID, const QString &Minutes, const QString &Length)
{
    FunctionGetDatawLength
    double v = logisdom::NA;
    double sum = 0;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(v)) v = 0;
            v += V;
            sum ++;
        }
    }
    return v / sum;
}




double calcthread::getMaxRomID(const QString &RomID, const QString &Minutes)
{
	FunctionGetData
	double max = logisdom::NA;
	for (int n=0; n<data.data_Y.count(); n++)
	{
		double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
		{
            if (logisdom::isNA(max)) max = V;
            if (logisdom::isNotNA(max) and (V > max)) max = V;
		}
	}
	return max;
}




double calcthread::getMaxRomID(const QString &RomID, const QString &Minutes, const QString &Length)
{
    FunctionGetDatawLength
    double max = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(max)) max = V;
            if (logisdom::isNotNA(max) and (V > max)) max = V;
        }
    }
    return max;
}




double calcthread::getMinRomID(const QString &RomID, const QString &Minutes)
{
	FunctionGetData
	double min = logisdom::NA;
	for (int n=0; n<data.data_Y.count(); n++)
	{
		double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
		{
            if (logisdom::isNA(min)) min = V;
            if (logisdom::isNotNA(min) and (V < min)) min = V;
		}
	}
	return min;
}





double calcthread::getMinRomID(const QString &RomID, const QString &Minutes, const QString &Length)
{
    FunctionGetDatawLength
    double min = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(min)) min = V;
            if (logisdom::isNotNA(min) and (V < min)) min = V;
        }
    }
    return min;
}





double calcthread::getSumRomID(const QString &RomID, const QString &Minutes)
{
    FunctionGetData
    double sum = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(sum)) sum = 0;
            sum += V;
        }
    }
    return sum;
}





double calcthread::getSumRomID(const QString &RomID, const QString &Minutes, const QString &Length)
{
    FunctionGetDatawLength
    double sum = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(sum)) sum = 0;
            sum += V;
        }
    }
    return sum;
}




double calcthread::getLSumRomID(const QString &RomID, const QString &Minutes, const QString &Max)
{
    FunctionGetData
    double sum = logisdom::NA;
    double max = Max.toDouble(&ok);
    if (!ok) { syntaxError(tr("Max parameter error ") + Max); ResultError }
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = data.data_Y.at(n);
        if ((logisdom::isNotNA(V)) && (V < max))
        {
            if (logisdom::isNA(sum)) sum = 0;
            sum += V;
        }
    }
    return sum;
}



double calcthread::getCountRomID(const QString &RomID, const QString &Minutes, const QString &Level)
{
    FunctionGetData
    double level;
    if (isNumeric(Level)) level = Level.toInt(&ok);
    else level = toNumeric(Level, &ok);
    double sum = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = (data.data_Y.at(n) >= level);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(sum)) sum = 0;
            sum += V;
        }
    }
    return sum;
}




double calcthread::getCountRomID(const QString &RomID, const QString &Minutes, const QString &Level, const QString &Length)
{
    FunctionGetDatawLength
    double level;
    if (isNumeric(Level)) level = Level.toInt(&ok);
    else level = toNumeric(Level, &ok);
    double sum = logisdom::NA;
    for (int n=0; n<data.data_Y.count(); n++)
    {
        double V = (data.data_Y.at(n) >= level);
        if (logisdom::isNotNA(V))
        {
            if (logisdom::isNA(sum)) sum = 0;
            sum += V;
        }
    }
    return sum;
}



double calcthread::getDataCount(const QString &RomID, const QString &Minutes)
{
	FunctionGetData
	return data.data_Y.count();
}



double calcthread::getDataCount(const QString &RomID, const QString &Minutes, const QString &Length)
{
    FunctionGetDatawLength
    return data.data_Y.count();
}



double calcthread::getSlopeMeanRomID(const QString &RomID, const QString &Minutes)
{
	double v = logisdom::NA;
	bool ok;
	int minutes = Minutes.toInt(&ok);
	if (!ok)
	{
		syntax = false;
		return logisdom::NA;
	}
	onewiredevice *device = checkDevice(RomID);
	QDateTime now = QDateTime::currentDateTime();
	QDateTime T;
	if (!device)
	{
	    syntax = false;
	    return logisdom::NA;
	}
	if (TCalc)
	{
		T = *TCalc;
		double v1 = device->getMainValue(T, deviceLoading, parent);
		T = T.addSecs(- minutes * 60);
		double v2 = device->getMainValue(T, deviceLoading, parent);
        if (logisdom::isNotNA(v1) and logisdom::isNotNA(v2)) v = v1 - v2;
	}
	else 
	{
		double v1 = device->getMainValue();
		T = now.addSecs(- minutes * 60);
		double v2 = device->getMainValue(T, deviceLoading, parent);
        if (logisdom::isNotNA(v1) and logisdom::isNotNA(v2)) v = v1 - v2;
	}
    if (logisdom::isNA(v)) { dataError(tr("SlopeRomID function didn't find enough valid data")); ResultError }
    return v;
}




double calcthread::getSlopeSumPosRomID(const QString &RomID, const QString &Minutes, const QString &Threshold)
{
	double sum = logisdom::NA;
	double start = logisdom::NA;
	double next = logisdom::NA;
	double max = logisdom::NA;
	double min = logisdom::NA;
    int increase = logisdom::NA;
	double threshold;
    bool OK = false;
	if (isNumeric(Threshold)) threshold = Threshold.toDouble(&OK);
        else threshold = toNumeric(Threshold, &OK);
        if (!OK) { syntaxError(tr("Threshold parameter error ") + RomID); ResultError }
	FunctionGetData
	for (int n=0; n<data.data_Y.count(); n++)
	{
		double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
		{
            if (logisdom::isNA(start))  start = V;
            else
            {
                next = V;
                if (increase == logisdom::NA)
                {
                    if (next > start) increase = 1; else increase = 0;
                }
                if (increase == 1)
                {
                    if (logisdom::isNA(max)) max = next;
                    else
                    {
                        if (next > max) max = next;
                        else
                        {
                            if (next < (max - threshold))
                            {
                                if (logisdom::isNA(sum)) sum = max - start;
                                    else sum += max - start;
                                start = max;
                                min = max;
                                increase = 0;
                            }
                        }
                    }
                }
                else if (increase == 0)
                {
                    if (logisdom::isNA(min)) min = next;
                    else
                    {
                        if (next < min) min = next;
                        else
                        {
                            if (next > (min + threshold))
                            {
                                start = min;
                                max = min;
                                increase = 1;
                            }
                        }
                    }
                }
            }
		}
	}
    if (logisdom::isNA(sum)) { dataError(tr("getSlopeSumPosRomIDProcess function didn't find enough valid data")); ResultError }
	return sum;
}




double calcthread::getSlopeSumNegRomID(const QString &RomID, const QString &Minutes, const QString &Threshold)
{
	double sum = logisdom::NA;
	double start = logisdom::NA;
	double next = logisdom::NA;
	double max = logisdom::NA;
	double min = logisdom::NA;
    int increase = logisdom::NA;
	double threshold;
	bool OK;
	if (isNumeric(Threshold)) threshold = Threshold.toDouble(&OK);
        else threshold = toNumeric(Threshold, &OK);
        if (!OK) { syntaxError(tr("Threshold parameter error ") + RomID); ResultError }
	FunctionGetData
	for (int n=0; n<data.data_Y.count(); n++)
	{
		double V = data.data_Y.at(n);
        if (logisdom::isNotNA(V))
		{
            if (logisdom::isNA(start))  start = V;
			else
			{
				next = V;
                if (logisdom::isNA(increase))
				{
					if (next > start) increase = 1; else increase = 0;
				}
				if (increase == 1)
				{
                    if (logisdom::isNA(max)) max = next;
					else
					{
						if (next > max) max = next;
						else
						{
							if (next < (max - threshold))
							{
								start = max;
								min = max;
								increase = 0;
							}
						}
					}
				}
				else if (increase == 0)
				{
                    if (logisdom::isNA(min)) min = next;
					else
					{
						if (next < min) min = next;
						else
						{
							if (next > (min + threshold))
							{
                                if (logisdom::isNA(sum)) sum = min - start;
									else sum += min - start;
								start = min;
								max = min;
								increase = 1;
							}
						}
					}
				}
			}
		}
	}
    if (logisdom::isNA(sum)) { dataError(tr("getSlopeSumNegRomIDProcess function didn't find enough valid data")); ResultError }
	return sum;
}





double calcthread::getStep(const QString &value, const QString &step)
{
	double Value = getValueFromP(value);
	if (!syntax) return 0;
	double Step = getValueFromP(step);
	if (!syntax) return 0;
	Value += Step / 2;
    int v = int(Value / Step);
	Value = v * Step;
	return Value;
}



double calcthread::getCurrentYear(const QString &value)
{
    qint64 result = logisdom::NA;
	QDateTime T;
	QDateTime now = QDateTime::currentDateTime();
	int month = 0;
	if (value.isEmpty()) month = 0;
    else month = int(getValueFromP(value));
	if (!syntax) return 0;
	if (month < 1) month = 1;
	if (month > 12) month = 1;
	if (TCalc)
	{
		T = *TCalc;
		QDateTime start;
		start.time().setHMS(0, 0, 0);
		start.setDate(QDate(T.date().year(), month, 1));
        qint64 dif = start.secsTo(T);
		if (dif < 0) result = start.addYears(-1).secsTo(T) / 60;
		else result = start.secsTo(T) / 60;
	}
	else
	{
		QDateTime start;
		start.time().setHMS(0, 0, 0);
		start.setDate(QDate(now.date().year(), month, 1));
        qint64 dif = start.secsTo(now);
		if (dif < 0) result = start.addYears(-1).secsTo(now) / 60;
		else result = start.secsTo(now) / 60;
	}
    return double(result);
}




double calcthread::getCurrentMonth(const QString &value)
{
    qint64 result = logisdom::NA;
	QDateTime T;
	QDateTime now = QDateTime::currentDateTime();
	int day;
	if (value.isEmpty()) day = 0;
    else day = int(getValueFromP(value));
	if (!syntax) return 0;
	if (day < 1) day = 1;
	if (day > 28) day = 28;
	if (TCalc)
	{
		T = *TCalc;
		QDateTime start;
		start.time().setHMS(0, 0, 0);
		start.setDate(QDate(T.date().year(), T.date().month(), day));
        qint64 dif = start.secsTo(T);
		if (dif < 0) result = start.addMonths(-1).secsTo(T) / 60;
		else result = start.secsTo(T) / 60;
	}
	else
	{
		QDateTime start;
		start.time().setHMS(0, 0, 0);
		start.setDate(QDate(now.date().year(), now.date().month(), day));
        qint64 dif = start.secsTo(now);
		if (dif < 0) result = start.addMonths(-1).secsTo(now) / 60;
		else result = start.secsTo(now) / 60;
	}
    return double(result);
}





double calcthread::getCurrentWeek(const QString &value)
{
    qint64 result = logisdom::NA;
	QDateTime T;
	QDateTime now = QDateTime::currentDateTime();
	int day;
	if (value.isEmpty()) day = 0;
    else day = int(getValueFromP(value));
	if (!syntax) return 0;
	if (day < 1) day = 1;
	if (day > 7) day = 1;
	if (TCalc)
	{
		T = *TCalc;
		int dif = 0;
		int weekDayNow = T.date().dayOfWeek();
		if (weekDayNow < day) dif = weekDayNow + 7 - day;
			else dif = weekDayNow - day;
		QDateTime start;
		start.setDate(T.date().addDays(-dif));
		start.setTime(QTime(0, 0, 0));
		result = start.secsTo(T) / 60;
	}
	else
	{
		int dif = 0;
		int weekDayNow = now.date().dayOfWeek();
		if (weekDayNow < day) dif = weekDayNow + 7 - day;
			else dif = weekDayNow - day;
		QDateTime start;
		start.setDate(now.date().addDays(-dif));
		start.setTime(QTime(0, 0, 0));
		result = start.secsTo(now) / 60;
	}
    return double(result);
}




double calcthread::getMinute()
{
    if (TCalc)
    {
        return TCalc->time().minute();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.time().minute();
    }
}



double calcthread::getHour()
{
    if (TCalc)
    {
        return TCalc->time().hour();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.time().hour();
    }
}



double calcthread::getDayOfWeek()
{
    if (TCalc)
    {
        return TCalc->date().dayOfWeek();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.date().dayOfWeek();
    }
}



double calcthread::getDayOfMonth()
{
    if (TCalc)
    {
        return TCalc->date().day();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.date().day();
    }
}



double calcthread::getMonthOfYear()
{
    if (TCalc)
    {
        return TCalc->date().month();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.date().month();
    }
}



double calcthread::getYear()
{
    if (TCalc)
    {
        return TCalc->date().year();
    }
    else
    {
        QDateTime now = QDateTime::currentDateTime();
        return now.date().year();
    }
}




double calcthread::getSunSet()
{
    int result = logisdom::NA;
    QTime sunset;
    if (maison1wirewindow->EnergieSolaire)
    {
        sunset = maison1wirewindow->EnergieSolaire->getSunSet();
        result = sunset.hour() * 60 + sunset.minute();
    }
    return result;
}




double calcthread::getSunRise()
{
    int result = logisdom::NA;
    QTime sunrise;
    if (maison1wirewindow->EnergieSolaire)
    {
        sunrise = maison1wirewindow->EnergieSolaire->getSunRise();
        result = sunrise.hour() * 60 + sunrise.minute();
    }
    return result;
}


double calcthread::getCurrentDay(const QString &value)
{
    qint64 result = logisdom::NA;
	QDateTime T;
	QDateTime now = QDateTime::currentDateTime();
	int hour;
	if (value.isEmpty()) hour = 0;
    else hour = int(getValueFromP(value));
	if (!syntax) return 0;
	if (hour < 0) hour = 0;
	if (hour > 23) hour = 0;
	if (TCalc)
	{
		T = *TCalc;
		QDateTime start;
		start.setDate(T.date());
		start.setTime(QTime(hour, 0, 0));
        qint64 dif = start.secsTo(T);
		if (dif < 0) result = start.addDays(-1).secsTo(T) / 60;
		else result = start.secsTo(T) / 60;
	}
	else
	{
		QDateTime start;
		start.setDate(now.date());
		start.setTime(QTime(hour, 0, 0));
        qint64 dif = start.secsTo(now);
		if (dif < 0) result = start.addDays(-1).secsTo(now) / 60;
		else result = start.secsTo(now) / 60;
	}
    return double(result);
}



double calcthread::getYearBefore(const QString &value)
{
    qint64 result = logisdom::NA;
    QDateTime T;
    QDateTime now = QDateTime::currentDateTime();
    int year;
    if (value.isEmpty()) year = 0;
    else year = int(getValueFromP(value));
    if (!syntax) return 0;
    if (year < 0) year = 0;
    if (TCalc)
    {
        T = *TCalc;
        QDateTime start;
        QDate month_Date = T.addYears(-year).date();
        start.setDate(QDate(month_Date.year(), 1, 1));
        start.setTime(QTime(0, 0, 0));
        result = start.secsTo(T) / 60;
    }
    else
    {
        QDateTime start;
        QDate month_Date = now.addYears(-year).date();
        start.setDate(QDate(month_Date.year(), 1, 1));
        start.setTime(QTime(0, 0, 0));
        result = start.secsTo(now) / 60;
    }
    return double(result);
}




double calcthread::getMonthBefore(const QString &value)
{
    qint64 result = logisdom::NA;
    QDateTime T;
    QDateTime now = QDateTime::currentDateTime();
    int month;
    if (value.isEmpty()) month = 0;
    else month = int(getValueFromP(value));
    if (!syntax) return 0;
    if (month < 0) month = 0;
    if (TCalc)
    {
        T = *TCalc;
        QDateTime start;
        QDate month_Date = T.addMonths(-month).date();
        start.setDate(QDate(month_Date.year(), month_Date.month(), 1));
        start.setTime(QTime(0, 0, 0));
        result = start.secsTo(T) / 60;
    }
    else
    {
        QDateTime start;
        QDate month_Date = now.addMonths(-month).date();
        start.setDate(QDate(month_Date.year(), month_Date.month(), 1));
        start.setTime(QTime(0, 0, 0));
        result = start.secsTo(now) / 60;
    }
    return double(result);
}




double calcthread::getProgramValue(const QString value)
{
	double result = logisdom::NA;
	QDateTime T;
	QDateTime now = QDateTime::currentDateTime();
	if (TCalc)
	{
		T = *TCalc;
		result = maison1wirewindow->AddDaily->getDailyProgValue(value, T);
	}
	else
	{
		result = maison1wirewindow->AddDaily->getDailyProgValue(value, now);	
	}
    if (logisdom::isNA(result)) dataValid = false;
	return result;
}





double calcthread::getWeekProgValue(const QString value)
{
    double result = logisdom::NA;
    QDateTime T;
    QDateTime now = QDateTime::currentDateTime();
    if (TCalc)
    {
        T = *TCalc;
        int indexNow = maison1wirewindow->AddProgwin->getActualProgram(value, T);
        int indexPrevious = maison1wirewindow->AddProgwin->getPreviousProgram(value, T);
        result = maison1wirewindow->AddDaily->getValue(indexNow, indexPrevious, T);
   }
    else
    {
        int indexNow = maison1wirewindow->AddProgwin->getActualProgram(value, now);
        int indexPrevious = maison1wirewindow->AddProgwin->getPreviousProgram(value, now);
        result = maison1wirewindow->AddDaily->getValue(indexNow, indexPrevious, now);
    }
    if (logisdom::isNA(result)) dataValid = false;
    return result;
}





double calcthread::getPID(const QString actual_Position, const QString setPoint, const QString KP, const QString KI, const QString KD)
{
	double actual_position = getValueFromP(actual_Position);
	if (!syntax) return 0;
	double setpoint = getValueFromP(setPoint);
	if (!syntax) return 0;
	double Kp = getValueFromP(KP);
	double Ki = getValueFromP(KI);
	double Kd = getValueFromP(KD);
	double error = setpoint - actual_position;
	integral = integral + error;
	double derivative = (error - previousError);
	double output = Kp*error + Ki*integral + Kd*derivative;
	previousError = error;
	return output;
}





double calcthread::getDSP(const QString &RomID, const QString &Poles, const QString &Gain, const QString &Polynome)
{
	double result = logisdom::NA;
	QDateTime T;
    DSPdevice = maison1wirewindow->configwin->DeviceExist(RomID);
    if (!DSPdevice) { dataError(tr("Device not found ") + RomID); ResultError }
	double x = logisdom::NA;
	if (TCalc)
	{
		T = *TCalc;
		x = DSPdevice->getMainValue(T, deviceLoading, parent);
	}
	else x = DSPdevice->getMainValue();
    if (logisdom::isNA(x)) { dataError(tr("Device not ready ") + RomID); ResultError }
    DSPpole = int(getValueFromP(Poles));
    if (!(DSPpole < NPOLES)) { syntaxError(tr("Pole number too high")); ResultError }
	DSPGain = getValueFromP(Gain); ResultError
    if ((DSPdevice != lastDSPdevice) or (logisdom::AreNotSame(DSPGain, lastDSPGain)) or (DSPpole != lastDSPpole)) restartDSP = true;
	lastDSPdevice = DSPdevice;
	lastDSPGain = DSPGain;
	lastDSPpole = DSPpole;
    result = getValueFromP(Polynome); ResultError
        if (logisdom::isNA(result)) { dataValid = false; ResultError }
	scroolDone = false;
	yv[DSPpole] = result;
	return result;
}





