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



#ifndef ENERGIESOLAIRE_H
#define ENERGIESOLAIRE_H

#include <QtCore>
#include "weathercom.h"
#include "plot.h"
#include "ui_energiesolaire.h"

class Plot;

class energiesolaire : public QWidget
{
	Q_OBJECT
public:
	QMutex mutexGet;
	QMutex mutexPosition;
	static const int GraphSolaireXMax = 99;
    energiesolaire(QWidget *parent = nullptr);
	~energiesolaire();
	QTime getSunRise();
	QTime getSunSet();
	void SaveConfigStr(QString &str);
    void readconfigfile(const QString &configdata);
	double getSunPower(QDateTime *T);
	double getActualSunPower(QDateTime *T);
private:
	QTimer *TimerRefresh;
	Plot plot;
	Plot plotAn;
	weathercom::CalculSolaire Data;
	Ui::energiesolaire ui;
	double JourMois(double mois);
	void closeEvent(QCloseEvent *event);
    void TimeConvertFromSecond(double TempsSolaireSecondes, QTime &);
	void position(QDateTime now, weathercom::GPS lieu, weathercom::CalculSolaire &data, int resolution, bool calculonly = false);
private slots:
	void GraphQuot();
	void GraphAn();
	void refresh();
	void VilleChange(int);
	void refreshnow(QTime);
	void refreshnow(QDate);
	void refreshnow(int);
	void refreshtextonly(QTime);
	void refreshinclinaison(int);
	void refreshinclinaison(double);
	void refreshinclinaison(QDate);
	void AutoChange(int state);
};



static const QString CoordonneePolaires[500] =
{
    "01", "BOURG", "46,12", "-5,13", "1",
	"02", "LAON", "49,34", "-3,37", "1",
	"03", "MOULINS", "46,34", "-3,20", "1",
	"04", "DIGNE", "44,05", "-6,14", "1",
	"05", "GAP", "44,33", "-6,05", "1",
	"06", "NICE", "43,42", "-7,16", "1",
	"07", "PRIVAS", "44,44", "-4,36", "1",
	"08", "MEZIERES", "49,46", "-4,44", "1",
	"09", "FOIX", "42,57", "-1,35", "1",
	"10", "TROYES", "48,18", "-4,05", "1",
	"11", "CARCASSONNE", "43,13", "-2,21", "1",
	"12", "RODEZ", "44,21", "-2,34", "1",
	"13", "MARSEILLE", "43,18", "-5,22", "1",
	"14", "CAEN","49,11", "0.62", "1",
	"15", "AURILLAC", "44,56", "-2,26", "1",
	"16", "ANGOULEME", "45,40", "-0,10", "1",
	"17", "LA ROCHELLE", "46,10", "1,50", "1",
	"18", "BOURGES", "47,05", "-2,23", "1",
	"19", "TULLE", "45,16", "-1,46", "1",
	"20", "AJACCIO", "41,55", "-8,43", "1",
	"21", "DIJON", "47,20", "-5,02", "1",
	"22", "ST-BRIEUX", "48,31", "2,85", "1",
	"23", "GUERET", "46,10", "-1,52", "1",
	"24", "PERIGUEUX", "45,12", "-0,44", "1",
	"25", "BESANCON", "47,14", "-6,12", "1",
	"26", "VALENCE", "44,56", "-4,54", "1",
	"27", "EVREUX", "49,03", "-1,11", "1",
	"28", "CHARTRES", "48,27", "-1,30", "1",
	"29", "QUIMPER", "48,00", "4.46", "1",
	"30", "NIMES", "43,50", "-4,21", "1",
	"31", "TOULOUSE", "43,37", "-1,27", "1",
	"32", "AUCH", "43,30", "-0,36", "1",
	"33", "BORDEAUX", "44,50", "0.74", "1",
	"34", "MONTPELLIER", "43,36", "-3,53", "1",
	"35", "RENNES", "48,06", "1,80", "1",
	"36", "CHATEAUROUX", "46,49", "-1,41", "1",
	"37", "TOURS", "47,23", "-0,42",  "1",
	"38", "GRENOBLE", "45,11", "-5,43", "1",
	"39", "LONS-LE-SAUNIER", "46,41", "-5,33", "1",
	"40", "MONT-DE-MARSAN", "43,54", "0,70", "1",
	"41", "BLOIS", "47,36", "-1,20", "1",
	"42", "ST-ETIENNE", "45,26", "-4,23", "1",
	"43", "LE PUY", "45,03", "-3,53", "1",
	"44", "NANTES", "47,14", "1,75", "1",
	"45", "ORLEANS", "47,54", "-1,54", "1",
	"46", "CAHORS", "44,28", "-0,26", "1",
	"47", "AGEN", "44,12", "-0,38", "1",
	"48", "MENDE", "44,32", "-3,30", "1",
	"49", "ANGERS", "47,29", "0,72", "1",
	"50", "ST-LO", "49,07", "1,45", "1",
	"51", "CHALONS-S-MARNE", "48,58", "-4,22", "1",
	"52", "CHAUMONT", "48,07", "-5,08", "1",
	"53", "LAVAL", "48,04", "0,85", "1",
	"54", "NANCY", "48,42", "-6,12", "1",
	"55", "BAR-LE-DUC", "48,46", "-5,10", "1",
	"56", "VANNES", "47,40", "2,84", "1",
	"57", "METZ", "49,07", "-6,11", "1",
	"58", "NEVERS", "47,00", "-3,09", "1",
	"59", "LILLE", "50,39", "-3,05", "1",
	"60", "BEAUVAIS", "49,26", "-2,05", "1",
	"61", "ALENCON", "48,25", "-0,05", "1",
	"62", "ARRAS", "50,17", "-2,46", "1",
	"63", "CLERMONT-FERRAND", "45,47", "-3,05", "1",
	"64", "PAU", "43,18", "0,62", "1",
	"65", "TARBES", "43,14", "-0,05", "1",
	"66", "PERPIGNAN", "42,42", "-2,54", "1",
	"67", "STRASBOURG", "48,35", "-7,45", "1",
	"68", "COLMAR", "48,05", "-7,21", "1",
//	"69", "LYON", "45,46", "-4,50", "1",
	"69", "LYON", "45,75", "-4,85", "1",
	"70", "VESOUL", "47,38", "-6,09", "1",
	"71", "MACON", "46,18", "-4,50", "1",
	"72", "LE MANS", "48,00", "-0,12", "1",
	"73", "CHAMBERY", "45,34", "-5,55", "1",
	"74", "ANNECY", "45,54", "-6,07", "1",
	"75", "PARIS", "48,52", "-2,20", "1",
	"76", "ROUEN", "49,26", "-1,05", "1",
	"77", "MELUN", "48,32", "-2,40", "1",
	"78", "VERSAILLES", "48,48", "-2,08", "1",
	"79", "NIORT", "46,19", "0,77", "1",
	"80", "AMIENS", "49,54", "-2,18", "1",
	"81", "ALBI", "43,56", "-2,08", "1",
	"82", "MONTAUBAN", "44,01", "-1,20", "1",
	"83", "TOULON", "43,07", "-5,55",  "1",
	"84", "AVIGNON", "43,56", "-4,48", "1",
	"85", "LA-ROCHE-SUR-YON", "46,38", "1,70", "1",
	"86", "POITIERS", "46,35", "-0,20", "1",
	"87", "LIMOGES", "45,50", "-1,15", "1",
	"88", "EPINAL", "48,10", "-6,28", "1",
	"89", "AUXERRE", "47,48", "-3,35", "1",
	"90", "BELFORT", "47,38", "-6,52", "1",
	"91", "EVRY", "48,38", "-2,34", "1",
	"92", "NANTERRE", "48,53", "-2,13", "1",
	"93", "BOBIGNY", "48,55", "-2,27", "1",
	"94", "CRETEIL", "48,47", "-2,28", "1",
	"95", "PONTOISE", "49,03", "-2,05", "1"
};


#endif
