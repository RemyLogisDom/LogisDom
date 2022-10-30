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



#ifndef METEO_H
#define METEO_H
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtGui>
#include <QtXml>
#include <QtNetwork>
#include "logisdom.h"
#include "weathercom.h"


class QBuffer;
class weathercom;



class meteo : public QScrollArea
{
	Q_OBJECT
#define minH 20
#define JourMax 10
#define PrevisionMax 6
#define DataMax 6
public:
    int tailleicones;
    QTextEdit *texthtml;
    QGridLayout *gridLayout;
    meteo(QWidget *Parent, logisdom *application);
    ~meteo();
    void timerupdate();
    QString xmlsourceID;
    QString xmlsourceNoID;
    QFrame *FrameMeteo[JourMax];
    QLabel *IconeMeteo[DataMax][JourMax];
    QGridLayout *framegridLayout[JourMax];
    void readconfigfile(const QString &configdata);
    void SaveConfigStr(QString &str);
    int getN(int day);
    double getTempHi(int NbDays, bool *ok);
    double getTempLow(int NbDays, bool *ok);
    double getHumidity(int NbDays, bool *ok);
    double getWind(int NbDays, bool *ok);
    double getWindDir(int NbDays, bool *ok);
    double getSunPower(int NbDays, bool *ok);
    double getRain(int NbDays, bool *ok);
    double getMeanSunPower(int day, bool *ok);
    double getCurrentSunPower(bool *ok);
    int iconToPower(int icon);
    QString getData(QString Name, QTreeWidgetItem * parent = 0);
    QTreeWidgetItem *getChildFolderItem(QTreeWidgetItem *, QString);
    QTreeWidgetItem *getFolderItem(QString Name);
    void getPrevisionData(QTreeWidgetItem *, weathercom::WeatherData *);
    void clearData(weathercom::WeatherData *data);
    weathercom::WeatherData WeathercomData[weathercom::weatherdaymax + 1];
// Palette setup
    QWidget setup;
    QGridLayout setupLayout;
    QSpinBox spinIconTresNuageux, spinIconNuageux, spinIconEnsoleille, spinIconVoile, spinIconAverses;
    QLabel labelIconTresNuageux, labelIconNuageux, labelIconEnsoleille, labelIconVoile, labelIconAverses;
    QSpinBox spinIntervalUpdate;
    QComboBox nbjours;
    QPushButton updateButton;
    QLabel labelServer;
    QLineEdit serverLink;
private:
    logisdom *parent;
    int minutes;
    void icones();
    void codesource();
    weathercom *Weathercom;
    void setWeatherIcon(QLabel *, weathercom::WeatherData *);
    bool setIcon(QLabel *label, QString code, int s);
    void parseJsonObject(const QJsonObject &, QTreeWidgetItem *);
    QTreeWidgetItem *setFolder(QString Type, QTreeWidgetItem *parent);
    QTreeWidgetItem *setData(QString Type, QString Value, QTreeWidgetItem *parent);
private slots:
    void choisirnbjour(int);
    void sethttprequest();
protected:
    void mousePressEvent(QMouseEvent *event);
signals:
    void setupClick(QWidget*);
};






#endif

