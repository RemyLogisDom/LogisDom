/****************************************************************************
**
** Copyright (C) 2019 Remy CARISIO.
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


/*
http://ws.meteofrance.com/ws/getLieux/Saint%20Alban%20de%20Roche.json
http://ws.meteofrance.com/ws/getDetail/france/383520.json
*/

#include "globalvar.h"
#include "configwindow.h"
#include "weathercom.h"
#include "inputdialog.h"
#include "meteo.h"

// http://api.openweathermap.org/data/2.5/weather?id=3030960&appid=e654a13b2b23a129993c5a4382d4bc78
// http://api.openweathermap.org/data/2.5/forecast?id=3030960&appid=e654a13b2b23a129993c5a4382d4bc78

meteo::meteo(QWidget *Parent, logisdom *application) : QScrollArea(Parent)
{
	QWidget *MeteoWidget = new QWidget(this);
	parent = application;
	setWidgetResizable(true);
	setMinimumSize(MainMinXSize, MainMinYSize);
	gridLayout = new QGridLayout(MeteoWidget);
	gridLayout->setSpacing(6);
	gridLayout->setMargin(9);
	setWidget(MeteoWidget);
	tailleicones = 100;
	minutes = 99999;
	texthtml = new QTextEdit(this);
    for(int j=0; j<JourMax; j++)
	{
		FrameMeteo[j] = new QFrame(this);
		FrameMeteo[j]->hide();
		FrameMeteo[j]->setFrameShape(QFrame::Box);
		FrameMeteo[j]->setFrameShadow(QFrame::Raised);
		FrameMeteo[j]->setMinimumSize(20, 20);
		framegridLayout[j] = new QGridLayout(FrameMeteo[j]);
		FrameMeteo[j]->setLayout(framegridLayout[j]);
	}
	for (int d=0; d<DataMax; d++)
		for(int j=0; j<JourMax; j++)
		{
			IconeMeteo[d][j] = new QLabel(FrameMeteo[j]);
            IconeMeteo[d][j]->setToolTipDuration(10000);
			IconeMeteo[d][j]->setMinimumSize(40, 40);
		}
	QString daystr = " " + tr("days");
	nbjours.addItem("3" + daystr);
	nbjours.addItem("4" + daystr);
	nbjours.addItem("5" + daystr);
    //nbjours.addItem("6" + daystr);
    //nbjours.addItem("7" + daystr);
    //nbjours.addItem("8" + daystr);
    //nbjours.addItem("9" + daystr);
	connect(&nbjours, SIGNAL(currentIndexChanged(int)), this, SLOT(choisirnbjour(int)));
    nbjours.setCurrentIndex(2);
	spinIntervalUpdate.setSuffix(tr("minutes"));
	spinIntervalUpdate.setValue(10);
	updateButton.setText(tr("Update"));
	connect(&updateButton, SIGNAL(clicked()), this, SLOT(sethttprequest()));
	QDateTime Now = QDateTime::currentDateTime();
    IconeMeteo[0][0]->setText(tr("Today"));
    IconeMeteo[0][0]->show();
    framegridLayout[0]->addWidget(IconeMeteo[0][0], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
    //IconeMeteo[0][1]->setText(tr("Tomorrow"));
    //framegridLayout[1]->addWidget(IconeMeteo[0][1], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
    //IconeMeteo[0][2]->setText(tr("Next Tomorrow"));
    //framegridLayout[2]->addWidget(IconeMeteo[0][2], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    IconeMeteo[1][0]->setText(tr("Aujourd'hui"));
    framegridLayout[0]->addWidget(IconeMeteo[1][0], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    IconeMeteo[2][0]->setText(tr("Matin"));
    framegridLayout[0]->addWidget(IconeMeteo[2][0], 3, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    IconeMeteo[3][0]->setText(tr("Après midi"));
    framegridLayout[0]->addWidget(IconeMeteo[3][0], 4, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    IconeMeteo[4][0]->setText(tr("Soir"));
    framegridLayout[0]->addWidget(IconeMeteo[4][0], 5, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    IconeMeteo[5][0]->setText(tr("Nuit"));
    framegridLayout[0]->addWidget(IconeMeteo[5][0], 6, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

    for (int n=1; n<JourMax; n++)
	{
        IconeMeteo[0][n]->setText(Now.addDays(n).toString("dddd d MMM"));
		framegridLayout[n]->addWidget(IconeMeteo[0][n], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
	}
    IconeMeteo[1][0]->setScaledContents(true);
    IconeMeteo[1][0]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
    framegridLayout[0]->addWidget(IconeMeteo[1][0], 1, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
    for (int j=1; j<JourMax; j++)
	{
		IconeMeteo[1][j]->setScaledContents(true);
        IconeMeteo[1][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
        framegridLayout[j]->addWidget(IconeMeteo[1][j], 1, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

        if (j<=PrevisionMax)
        {
            IconeMeteo[3][j]->setScaledContents(true);
            IconeMeteo[3][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[3][j], 3, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

            IconeMeteo[4][j]->setScaledContents(true);
            IconeMeteo[4][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[4][j], 4, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        }
        else
        {
            IconeMeteo[3][j]->setScaledContents(true);
            IconeMeteo[3][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA_Blank.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[3][j], 3, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

            IconeMeteo[4][j]->setScaledContents(true);
            IconeMeteo[4][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA_Blank.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[4][j], 4, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        }

            IconeMeteo[2][j]->setScaledContents(true);
            IconeMeteo[2][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[2][j], 2, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);

            IconeMeteo[5][j]->setScaledContents(true);
            IconeMeteo[5][j]->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA.png")).scaled(tailleicones, tailleicones));
            framegridLayout[j]->addWidget(IconeMeteo[5][j], 5, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
    }
	connect(this, SIGNAL(setupClick(QWidget*)), parent, SLOT(setPalette(QWidget*)));
	Weathercom = new weathercom(this);
    setup.setLayout(&setupLayout);
	spinIntervalUpdate.setRange(1, 60*24);
    spinIconTresNuageux.setRange(0, 100);
    spinIconTresNuageux.setSuffix("%");
    spinIconTresNuageux.setValue(20);
    spinIconNuageux.setRange(0, 100);
    spinIconNuageux.setSuffix("%");
    spinIconNuageux.setValue(40);
    spinIconEnsoleille.setRange(0, 100);
    spinIconEnsoleille.setSuffix("%");
    spinIconEnsoleille.setValue(100);
    spinIconVoile.setRange(0, 100);
    spinIconVoile.setSuffix("%");
    spinIconVoile.setValue(80);
    spinIconAverses.setRange(0, 100);
    spinIconAverses.setSuffix("%");
    spinIconAverses.setValue(40);
    labelIconTresNuageux.setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/28.png")).scaled(tailleicones/3, tailleicones/3));
    labelIconNuageux.setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/30.png")).scaled(tailleicones/3, tailleicones/3));
    labelIconEnsoleille.setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/32.png")).scaled(tailleicones/3, tailleicones/3));
    labelIconVoile.setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/34.png")).scaled(tailleicones/3, tailleicones/3));
    labelIconAverses.setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/39.png")).scaled(tailleicones/3, tailleicones/3));
	int i = 0;
	setupLayout.addWidget(&nbjours, i++, 0, 1, 1);
    setupLayout.addWidget(&labelServer, i, 0, 1, 1);
    labelServer.setText("Meteo Server");
    setupLayout.addWidget(&serverLink, i++, 1, 1, 1);
    serverLink.setText("http://api.openweathermap.org");

    setupLayout.addWidget(&updateButton, i, 0, 1, 1);
	setupLayout.addWidget(&spinIntervalUpdate, i++, 1, 1, 1);
    setupLayout.addWidget(&labelIconTresNuageux, i, 0, 1, 1);
    setupLayout.addWidget(&spinIconTresNuageux, i++, 1, 1, 1);
    setupLayout.addWidget(&labelIconNuageux, i, 0, 1, 1);
    setupLayout.addWidget(&spinIconNuageux, i++, 1, 1, 1);
    setupLayout.addWidget(&labelIconEnsoleille, i, 0, 1, 1);
    setupLayout.addWidget(&spinIconEnsoleille, i++, 1, 1, 1);
    setupLayout.addWidget(&labelIconVoile, i, 0, 1, 1);
    setupLayout.addWidget(&spinIconVoile, i++, 1, 1, 1);
    setupLayout.addWidget(&labelIconAverses, i, 0, 1, 1);
    setupLayout.addWidget(&spinIconAverses, i++, 1, 1, 1);

    icones();
}




meteo::~meteo()
{
	delete gridLayout;
	delete texthtml;
	delete Weathercom;
}





QTreeWidgetItem *meteo::getChildFolderItem(QTreeWidgetItem *folder, QString Name)
{
    for (int n=0; n<folder->childCount(); n++)
    {
        if (folder->child(n)->text(0) == Name) return folder->child(n);
    }
    return nullptr;
}





QTreeWidgetItem *meteo::getFolderItem(QString Name)
{
    QList<QTreeWidgetItem *> found = Weathercom->findItems(Name, Qt::MatchExactly);
	QList<QTreeWidgetItem *>::iterator index = found.begin();
	while (index != found.end())
	{
		if (((*index)->parent() == nullptr) && ((*index)->text(0) == Name))
			return (*index);
		index ++;
    }
	return nullptr;
}






void meteo::clearData(weathercom::WeatherData *data)
{
    data->Icon = "";
    data->IconMain = "";
    data->IconDescr = "";
    data->Info = "";
    data->THi = "";
    data->TLow = "";
    data->TSea = "";
    data->UV = "";
    data->windSpeed = "";
    data->windGust = "";
    data->windDirection = "";
}






QString meteo::getData(QString Name, QTreeWidgetItem * parent)
{
	if (!parent) return "";
	int count = parent->childCount();
	for (int n=0; n<count; n++)
		if (parent->child(n)->text(0) == Name)
			return parent->child(n)->text(1);
	return "";
}




void meteo::readconfigfile(const QString &configdata)
{
    QString server;
    QString Days;
    QString I28, I30, I32, I34, I39, intervalUpdt;
    int i28, i30, i32, i34, i39, interval;
	QString TAG_Begin = "Meteo";
	QString TAG_End = EndMark;
	SearchLoopBegin	
    server = logisdom::getvalue("MeteoServer", strsearch);
    Days = logisdom::getvalue("Days", strsearch);
	I28 = logisdom::getvalue("Icon28", strsearch);
	I30 = logisdom::getvalue("Icon30", strsearch);
	I32 = logisdom::getvalue("Icon32", strsearch);
	I34 = logisdom::getvalue("Icon34", strsearch);
	I39 = logisdom::getvalue("Icon39", strsearch);
	intervalUpdt = logisdom::getvalue("IntervalUpdate", strsearch);
	SearchLoopEnd
	bool ok;
	int jour = Days.toInt(&ok);
    if ((ok) && (jour < 7)) nbjours.setCurrentIndex(jour);
    else nbjours.setCurrentIndex(2);
    if (!server.isEmpty()) serverLink.setText(server);

	interval = intervalUpdt.toInt(&ok);
	if (ok) spinIntervalUpdate.setValue(interval);
	i28 = I28.toInt(&ok);
    if (ok) spinIconTresNuageux.setValue(i28);
	i30 = I30.toInt(&ok);
    if (ok) spinIconNuageux.setValue(i30);
	i32 = I32.toInt(&ok);
    if (ok) spinIconEnsoleille.setValue(i32);
	i34 = I34.toInt(&ok);
    if (ok) spinIconVoile.setValue(i34);
	i39 = I39.toInt(&ok);
    if (ok) spinIconAverses.setValue(i39);
}






void  meteo::SaveConfigStr(QString &str)
{
	str += "\nMeteo\n";
    str += logisdom::saveformat("MeteoServer", serverLink.text());
    str += logisdom::saveformat("Days", QString("%1").arg(nbjours.currentIndex()));
    str += logisdom::saveformat("Icon28", QString("%1").arg(spinIconTresNuageux.value()));
    str += logisdom::saveformat("Icon30", QString("%1").arg(spinIconNuageux.value()));
    str += logisdom::saveformat("Icon32", QString("%1").arg(spinIconEnsoleille.value()));
    str += logisdom::saveformat("Icon34", QString("%1").arg(spinIconVoile.value()));
    str += logisdom::saveformat("Icon39", QString("%1").arg(spinIconAverses.value()));
	str += logisdom::saveformat("IntervalUpdate", QString("%1").arg(spinIntervalUpdate.value()));
	str += EndMark;
	str += "\n";
}




void meteo::timerupdate()
{
	minutes ++;
	if (minutes > spinIntervalUpdate.value())
	{
		minutes = 0;
		sethttprequest();
	}
}





void meteo::codesource()
{
	for(int j=0; j<JourMax; j++)
	{
		gridLayout->removeWidget(FrameMeteo[j]);
		FrameMeteo[j]->hide();
	}
	setMinimumSize(MainMinXSize, MainMinYSize);
	gridLayout->addWidget(texthtml, 0, 0, 1, 1);
	texthtml->show();
    gridLayout->addWidget(Weathercom, 1, 0, 1, 1);
    Weathercom->show();
}








void meteo::icones()
{
	setMinimumSize(MainMinXSize, MainMinYSize);
	gridLayout->removeWidget(texthtml);
	texthtml->hide();
	gridLayout->removeWidget(Weathercom);
	Weathercom->hide();
	int jour = nbjours.currentIndex();
	if (jour < 0) return;
	jour += 3;
	for(int j=0; j<jour; j++)
	{
		gridLayout->addWidget(FrameMeteo[j], 0, j, 1, 1);
		FrameMeteo[j]->show();
	}
}





void meteo::choisirnbjour(int j)
{
	if (j < 0) return;
	int jour = j + 3;
	if (jour > JourMax) return;
    for(int j=0; j<JourMax; j++)
	{
		gridLayout->removeWidget(FrameMeteo[j]);
		FrameMeteo[j]->hide();
	}
    for(int j=0; j<=jour; j++)
	{
		gridLayout->addWidget(FrameMeteo[j], 0, j, 1, 1);
		FrameMeteo[j]->show();
	}
}




QTreeWidgetItem *meteo::setFolder(QString Type, QTreeWidgetItem * parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, Type);
    return item;
}




QTreeWidgetItem *meteo::setData(QString Type, QString Value, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, Type);
    item->setText(1, Value);
    return item;
}



void meteo::parseJsonObject(const QJsonObject &objectList, QTreeWidgetItem *parentItem)
{
    if (!parentItem) return;
    QStringList list = objectList.keys();
    //qDebug() << list;
    foreach (const QString &str, list)
    {
        if (objectList.value(str).isString())
        {
            //qDebug() << str + "->" + objectList.value(str).toString();
            setData(str, objectList.value(str).toString(), parentItem);
        }
        else if (objectList.value(str).isDouble())
        {
            //qDebug() << str + "->" + QString("%1").arg(objectList.value(str).toDouble());
            qint64 value = qint64(objectList.value(str).toDouble());
            setData(str, QString("%1").arg(value), parentItem);
        }
        if (objectList.value(str).isObject())
        {
            //qDebug() << parentItem->text(0) + "->" + str;
            QTreeWidgetItem *item = setFolder(str, parentItem);
            parseJsonObject(objectList.value(str).toObject(), item);
        }
        if (objectList.value(str).isArray())
        {
            //qDebug() << str;
        }
    }
}



void meteo::sethttprequest()
{
    QDateTime Now = QDateTime::currentDateTime();
    for (int n=1; n<JourMax; n++)
    {
        IconeMeteo[0][n]->setText(Now.addDays(n).toString("dddd d MMM"));
        framegridLayout[n]->addWidget(IconeMeteo[0][n], 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
    }
    if (!serverLink.text().isEmpty())
	{
        QString url = serverLink.text();
		texthtml->clear();
		texthtml->insertPlainText(url + "\n");
        updateButton.setToolTip(url);
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        if (reply->error() == QNetworkReply::NoError)
        {
            /*QUrl urlRedirectedTo;
            QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
            urlRedirectedTo = possibleRedirectUrl.toUrl();
            if (!urlRedirectedTo.isEmpty() && urlRedirectedTo != url)
            {
                QString newUrlStr = urlRedirectedTo.toString();
                QStringList list = newUrlStr.split(serverLink.text());
                if (list.count() == 2)
                {
                    //addressPrefix.setText(list.at(0));
                    //addressSuffix.setText(list.at(1));
                }
            }*/
            QByteArray data = reply->readAll();
            texthtml->insertPlainText(data);
            Weathercom->read(data);
            for (int n=0; n<weathercom::weatherdaymax; n++)
            {
                QTreeWidgetItem *item = getFolderItem(QString("%1").arg(n));
                if (item) getPrevisionData(item, &WeathercomData[n]);
                QDate today = QDateTime::currentDateTime().date();
                //qDebug() << WeathercomData[n].date.date().toString();
                if (WeathercomData[n].date.date() == today)
                {
                    int hour = WeathercomData[n].date.time().hour();
                    //qDebug() << QString("Hour = %1").arg(hour);
                    //if (hour <= 3) setWeatherIcon(IconeMeteo[1][0], &WeathercomData[n]);
                    if ((hour == 5)|(hour == 6)|(hour == 7)) setWeatherIcon(IconeMeteo[1][0], &WeathercomData[n]);
                    else if ((hour == 10)|(hour == 11)|(hour == 12)) setWeatherIcon(IconeMeteo[2][0], &WeathercomData[n]);
                    else if ((hour == 16)|(hour == 17)|(hour == 18)) setWeatherIcon(IconeMeteo[3][0], &WeathercomData[n]);
                    else if ((hour == 20)|(hour == 21)|(hour == 22)) setWeatherIcon(IconeMeteo[4][0], &WeathercomData[n]);
                    //else if ((hour == 23)|(hour == 00)) setWeatherIcon(IconeMeteo[4][0], &WeathercomData[n]);
                }
                for (int j=1; j<(nbjours.currentIndex()+4); j++) // JourMax
                {
                    if (WeathercomData[n].date.date() == today.addDays(j))
                    {
                        int hour = WeathercomData[n].date.time().hour();
                        if ((hour <= 3)) setWeatherIcon(IconeMeteo[5][j-1], &WeathercomData[n]);
                        else if ((hour == 5)|(hour == 6)|(hour == 7)) setWeatherIcon(IconeMeteo[1][j], &WeathercomData[n]);
                        else if ((hour == 10)|(hour == 11)|(hour == 12)) setWeatherIcon(IconeMeteo[2][j], &WeathercomData[n]);
                        else if ((hour == 16)|(hour == 17)|(hour == 18)) setWeatherIcon(IconeMeteo[3][j], &WeathercomData[n]);
                        else if ((hour == 20)|(hour == 21)|(hour == 22)) setWeatherIcon(IconeMeteo[4][j], &WeathercomData[n]);
                        //else if ((hour == 20)|(hour == 21)) setWeatherIcon(IconeMeteo[5][j], &WeathercomData[n]);
                        //else if ((hour == 23)|(hour == 00)) setWeatherIcon(IconeMeteo[5][j], &WeathercomData[n]);
                    }
                }
            }
        }
        else
        {
            texthtml->insertPlainText("http download fail\n");
            Weathercom->read("");
        }
        reply->deleteLater();
    }
}



void meteo::getPrevisionData(QTreeWidgetItem *folder, weathercom::WeatherData *data)
{
        for (int n=0; n<folder->childCount(); n++)
        {
            QTreeWidgetItem *currentItem = folder->child(n);
            if (currentItem->text(0) == "dt")
            {
                //qDebug() << "date  : " + currentItem->text(1);
                bool ok;
                qint64 t = currentItem->text(1).toLong(&ok);
                if (ok)
                {
                    data->date.setSecsSinceEpoch(t);
                    data->lastUpdate = QDateTime::currentDateTime();
                }
                //qDebug() << "date  : " + data->date.toString();
            }
            if (currentItem->text(0) == "main")
            {
                for (int c=0; c<currentItem->childCount(); c++)
                {
                    if (currentItem->child(c)->text(0) == "temp_min") data->TLow = currentItem->child(c)->text(1);
                    if (currentItem->child(c)->text(0) == "temp_max") data->THi = currentItem->child(c)->text(1);
                    if (currentItem->child(c)->text(0) == "humidity") data->Humidity = currentItem->child(c)->text(1);
                }
                // "temp": 286.66, "feels_like": 284.75, "pressure": 1023,"sea_level": 1023,"grnd_level": 988, "temp_kf": 0.32
            }
            if (currentItem->text(0) == "weather")
            {
                for (int c=0; c<currentItem->childCount(); c++)
                {
                    if (currentItem->child(c)->text(0) == "main") data->IconMain = currentItem->child(c)->text(1);
                    if (currentItem->child(c)->text(0) == "description") data->IconDescr = currentItem->child(c)->text(1);
                    if (currentItem->child(c)->text(0) == "icon") data->Icon = currentItem->child(c)->text(1);
                }
            }
            if (currentItem->text(0) == "rain")
            {
                for (int c=0; c<currentItem->childCount(); c++)
                {
                    if (currentItem->child(c)->text(0) == "3h") data->Rain = currentItem->child(c)->text(1);
                }
            }
            if (currentItem->text(0) == "clouds")
            {
                //         "all": 0
            }
            if (currentItem->text(0) == "wind")
            {
                for (int c=0; c<currentItem->childCount(); c++)
                {
                    if (currentItem->child(c)->text(0) == "speed") data->windSpeed = currentItem->child(c)->text(1);
                    if (currentItem->child(c)->text(0) == "deg") data->windDirection = currentItem->child(c)->text(1);
                }
            }
            if (currentItem->text(0) == "visibility")
            {
            }
            if (currentItem->text(0) == "dt_txt")
            {
            }
        }
}





void meteo::setWeatherIcon(QLabel *label, weathercom::WeatherData *data)
{
    int s = tailleicones;
    QString txt;
    if (data->date.isValid()) txt.append(data->date.toString("dd MMM yyyy hh") + "\n");
    if (!data->Info.isEmpty()) txt.append(data->Info + "\n");
    if (!data->TLow.isEmpty() && !data->THi.isEmpty()) txt.append(data->TLow + "°C -> " + data->THi + "°C\n");
    if (!data->Humidity.isEmpty()) txt.append(tr("Humidité : ") + data->Humidity + "%\n");
    if (!data->Rain.isEmpty()) txt.append(tr("Pluie : ") + data->Rain + "mm\n");
    if (!data->windSpeed.isEmpty()) txt.append(tr("Vent : ") + data->windSpeed + "Km/h\n");
    if (!data->windDirection.isEmpty() && (data->windDirection != "-1")) txt.append(tr("Dir : ") + data->windDirection + "°\n");
    txt.append(data->IconDescr + " "  + data->Icon);
    label->setToolTip(txt);
    if (data->Icon.isEmpty())
    {
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/NA_Blank.png")).scaled(s, s));
    }
    else
    {
        if (!setIcon(label, data->Icon, tailleicones)) qDebug() << "icon not found : " + data->Icon;
    }
}





bool meteo::setIcon(QLabel *label, QString code, int s)
{
    //qDebug() << code;
    if (code == "01d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/32.png")).scaled(s, s)); return true; } // Ensoleillé
    if (code == "01n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/31.png")).scaled(s, s)); return true;  } // Nuit claire
    //if (code == "01d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/34.png")).scaled(s, s)); return true;  }  // Ciel voilé
    //if (code == "01n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/29.png")).scaled(s, s)); return true;  }  // Nuit voilée
    if (code == "02d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/30.png")).scaled(s, s)); return true;  }  // Eclaicies
    if (code == "02n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/33.png")).scaled(s, s)); return true;  }  // Nuit Eclaircie
    if (code == "03d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/26.png")).scaled(s, s)); return true;  }  // Très Nuageux
    if (code == "03n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/27.png")).scaled(s, s)); return true;  }  // Très Nuageux Nuit
    if (code == "04d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/28.png")).scaled(s, s)); return true;  }  // Nuageux
    if (code == "04n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/29.png")).scaled(s, s)); return true;  }  // Nuageux Nuit
    //if (code == "9_a") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/11.png")).scaled(s, s)); return true;  }  // Pluies Eprases
    //if (code == "9_b") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/11.png")).scaled(s, s)); return true;  }  // Pluies Eprases
    //if (code == "9_c") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/11.png")).scaled(s, s)); return true;  }  // Pluies Eprases Nuit
    if (code == "10d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/39.png")).scaled(s, s)); return true;  }  // Pluies
    if (code == "10n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/11.png")).scaled(s, s)); return true;  }  // Pluies
    if (code == "13d") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/41.png")).scaled(s, s)); return true;  }  // Pluies
    if (code == "13n") { label->setPixmap(QPixmap(QString::fromUtf8(":/images/imagesmeteo/46.png")).scaled(s, s)); return true;  }  // Pluies
    return false;
}


int meteo::getN(int day)
{
    if (day == 0) return 0;
    for (int n=0; n<weathercom::weatherdaymax; n++)
    {
        QDate DAY = QDateTime::currentDateTime().date().addDays(day);
        if (WeathercomData[n].date.date() == DAY)
        {
            int hour = WeathercomData[n].date.time().hour();
            if ((hour == 11)|(hour == 12)|(hour == 13))
            {
                return n;
            }
        }
    }
    return logisdom::NA;
}


double meteo::getTempHi(int day, bool *ok)
{
    *ok = true;
    if (day < 0)
    {
        *ok = false;
        return 0;
    }
    QDateTime Now = QDateTime::currentDateTime();
    *ok = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].date.secsTo(Now) > 43200) *ok = false;
    //double T = TempConvertF2C(date[day].THi).toDouble(ok);
    double T = WeathercomData[n].THi.toDouble(ok);
    if (logisdom::isNA(T)) *ok = false;
    return T;
}





double meteo::getTempLow(int day, bool *ok)
{
    *ok = true;
    if (day < 0)
    {
        *ok = false;
        return 0;
    }
    QDateTime Now = QDateTime::currentDateTime();
    *ok = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].date.secsTo(Now) > 43200) *ok = false;
    //double T = TempConvertF2C(WeathercomData[day].TLow).toDouble(ok);
    double T = WeathercomData[n].TLow.toDouble(ok);
    if (logisdom::isNA(T)) *ok = false;
    return T;
}



double meteo::getHumidity(int day, bool *ok)
{
    *ok = true;
    if (day < 0)
    {
        *ok = false;
        return 0;
    }
    QDateTime Now = QDateTime::currentDateTime();
    *ok = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].date.secsTo(Now) > 43200) *ok = false;
    double H = WeathercomData[n].Humidity.toInt(ok);
    if (logisdom::isNA(H)) *ok = false;
    return H;
}



double meteo::getWindDir(int day, bool *ok)
{
    *ok = true;
    if (day < 0)
    {
        *ok = false;
        return 0;
    }
    QDateTime Now = QDateTime::currentDateTime();
    *ok = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].lastUpdate.secsTo(Now) > 43200) *ok = false;
    double W = WeathercomData[n].windDirection.toInt(ok);
    if (logisdom::isNA(W)) *ok = false;
    return W;
}



double meteo::getSunPower(int day, bool *ok)
{
    *ok = true;
	QDateTime Now = QDateTime::currentDateTime();
	int Power = 0;
	bool check;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].lastUpdate.secsTo(Now) > 43200) *ok = false;
    Power = iconToPower(WeathercomData[n].Icon.left(2).toInt(&check));
	if (!check) *ok = false;
    return double(Power);
}




double meteo::getWind(int day, bool *ok)
{
    *ok = true;
    QDateTime Now = QDateTime::currentDateTime();
    double W = 0;
    QString wind;
    bool check = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].date.secsTo(Now) > 43200) *ok = false;
    wind = WeathercomData[n].windSpeed;
    if (wind.isEmpty()) return 0;
    W = WeathercomData[n].windSpeed.toDouble(&check);
    if (!check) *ok = false;
    return W;
}



double meteo::getRain(int day, bool *ok)
{
    *ok = true;
    QDateTime Now = QDateTime::currentDateTime();
    double Rain = 0;
    QString rain;
    bool check = true;
    int n = getN(day);
    if (n < 0) return logisdom::NA;
    if (WeathercomData[n].date.secsTo(Now) > 43200) *ok = false;
    rain = WeathercomData[n].Rain;
    if (rain.isEmpty()) return 0;
    Rain = WeathercomData[n].Rain.toDouble(&check);
    if (!check) *ok = false;
    return Rain;
}



double meteo::getMeanSunPower(int NbDays, bool *ok)
{
	*ok = true;
	if (NbDays < 1) return 0;
	QDateTime Now = QDateTime::currentDateTime();
	int Power = 0;
	if (NbDays > weathercom::weatherdaymax) NbDays = weathercom::weatherdaymax;
	for (int n=1; n<=NbDays; n ++)
	{
        bool check = true;
        int i = getN(n);
        if (i < 0) return logisdom::NA;
        Power += iconToPower(WeathercomData[i].Icon.left(2).toInt(&check));
        if (!check) *ok = false;
        if (WeathercomData[i].date.secsTo(Now) > 43200) *ok = false;
	}
	Power /= NbDays;
    return double(Power);
}





double meteo::getCurrentSunPower(bool *ok)
{
	*ok = true;
	bool check;
	QDateTime Now = QDateTime::currentDateTime();
	int Power = 0;
    Power = iconToPower(WeathercomData[0].Icon.left(2).toInt(&check));
	if (!check) *ok = false;
    if (WeathercomData[0].date.secsTo(Now) > 7200) *ok = false;
    return double(Power);
}



int meteo::iconToPower(int icon)
{
	int P = 0;
	switch (icon)
	{
        case 0 : P = spinIconEnsoleille.value(); break;
        case 1 : P = spinIconVoile.value(); break;
        case 2 : P = spinIconAverses.value(); break;
        case 3 :
        case 10 : P = spinIconTresNuageux.value(); break;
        case 4 :
        case 9 : P = spinIconNuageux.value(); break;
		default : P = 0; break;
	}
	return P;
}



void meteo::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
        QMenu contextualmenu;
        QAction source(tr("show code source"), this);
        QAction icone(tr("show icones"), this);
        contextualmenu.addAction(&source);
        contextualmenu.addAction(&icone);
        QAction *selection;
		selection = contextualmenu.exec(event->globalPos());
        if (selection == &source) codesource();
        if (selection == &icone) icones();
    }
	else if (event->button() == Qt::LeftButton)  parent->setPalette(&setup);
}

