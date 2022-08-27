/****************************************************************************
**
** Copyright (C) 2018 Remy CARISIO.
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



#ifndef WEATHERCOM_H
#define WEATHERCOM_H
#include <QTreeWidget>
#include <QDateTime>



class weathercom : public QTreeWidget
{
	Q_OBJECT
public:
static const int weatherdaymax = 40;

struct WeatherData
{
    QDateTime date;
    QDateTime lastUpdate;
    QString Icon;
    QString IconMain;
    QString IconDescr;
    QString Info;
	QString THi;
    QString TLow;
    QString TSea;
    QString UV;
    QString windSpeed;
    QString windGust;
    QString windDirection;
    QString Rain;
    QString FreezeProbability;
    QString Humidity;
};

static const int GraphSolaireXMax = 99;
struct CalculSolaire
{
	QTime now;
	double Hauteur;				// Angle du soleil avec l'horizon
	double Azimut;				// Angle du soleil Est-Ouest (positif à l'est)
	double PuissanceSolaire;	// Puissance solaire en watt pour la surface de panneau précisée pour un jour complet
	double PuissanceSolaireNow;	// Puissance solaire en watt pour la surface de panneau précisée pour l'instant donnée
	QTime HeureSolaire;			// Heure cadran Solaire
	QTime HeureLever;			// Heure du lever du soleil
	QTime HeureCoucher;			// Heure du coucher du soleil
	QTime Eclairement;			// Temps d'éclairement
//	QVector <double> GraphHauteur;
//	QVector <double> GraphAzimut;
	QVector <double> GraphPwM2;
	QVector <double> GraphT;
	QVector <double> CapteurFixe;
	double PTotFixDay;
//	QVector <double> CapteurInclinableMax;
//	QVector <double> CapteurInclinableMin;
};
struct GPS
{
	double Latitude;
	double Longitude;
	double Altitude;
};
    bool read(const QByteArray &);
    weathercom(QWidget *parent = nullptr);
    QTreeWidgetItem *setFolder(QString Type, QTreeWidgetItem *parent);
    QTreeWidgetItem *setData(QString Type, QString Value, QTreeWidgetItem *parent);
private:
    //bool dateValid(QString S);
    void parseJsonObject(const QJsonObject &, QTreeWidgetItem *);
};


#endif

