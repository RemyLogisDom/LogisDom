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




#include <QtWidgets/QMessageBox>
#include "globalvar.h"
#include "graph.h"
#include "plot.h"
#include "energiesolaire.h"
#include "qwt_scale_map.h"


class DateTimeScaleDrawSoleil: public QwtScaleDraw
{
public:
    DateTimeScaleDrawSoleil(const QDateTime &base) :  baseTime(base)
    {
    }
    virtual QwtText label(double v) const
    {
    	v = v * SecsInDays;
        QDateTime Time = baseTime.addSecs(int(v));
        return Time.toString("dd-MMM hh:mm");
    }
private:
    QDateTime baseTime;
};







class DateScaleDrawSoleil: public QwtScaleDraw
{
public:
    DateScaleDrawSoleil(const QDate &base):
        baseTime(base)
    {
    }
    virtual QwtText label(double v) const
    {
        QDate upTime = baseTime.addDays(int(v));
        return upTime.toString("MMM yyyy");
    }
private:
    QDate baseTime;
};









class TimeScaleDrawSoleil : public QwtScaleDraw
{
public:
    TimeScaleDrawSoleil(const QTime &base) :  baseTime(base)
    {
    }
    QwtText label(double v) const
    {
    	v = v * 3600 * 24;
        QTime Time = baseTime.addSecs(int(v));
        return Time.toString("hh:mm");
    }
private:
    QTime baseTime;
};









class BackgroundSoleil1: public QwtPlotItem
{
public:
    BackgroundSoleil1()
    {
        setZ(0.0);
    }

    int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    void draw(QPainter *painter,
        const QwtScaleMap &, const QwtScaleMap &yMap,
        const QRectF &rect) const
    {
        QColor c(Qt::white);
        QRectF r = rect;
        for ( int i = 2000; i > 0; i -= 200 )
        {
            r.setBottom(yMap.transform(i - 200));
            r.setTop(yMap.transform(i));
            painter->fillRect(r, c);
            c = c.darker(110);
        }
    }
};







class BackgroundSoleil2: public QwtPlotItem
{
public:
    BackgroundSoleil2()
    {
        setZ(0.0);
    }

    int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    void draw(QPainter *painter,
        const QwtScaleMap &, const QwtScaleMap &yMap, const QRectF &rect) const
    {
        QColor c(Qt::white);
        QRectF r = rect;
        for ( int i = 8000; i > 0; i -= 800 )
        {
            r.setBottom(yMap.transform(i - 800));
            r.setTop(yMap.transform(i));
            painter->fillRect(r, c);
            c = c.darker(110);
        }
    }
};






energiesolaire::energiesolaire(QWidget*)
{
	ui.setupUi(this);
	plot.setAutoReplot(false);
	plotAn.setAutoReplot(false);
	plot.setWindowIcon(QIcon(QString::fromUtf8(":/images/images/kfm_home.png")));
	plotAn.setWindowIcon(QIcon(QString::fromUtf8(":/images/images/kfm_home.png")));
	for (int i=0; i<475; i+=5) ui.comboBoxVille->addItem(CoordonneePolaires[i] + "  " + CoordonneePolaires[i+1]); 
	
//	BackgroundSoleil1 *bg1 = new BackgroundSoleil1();
//	bg1->attach(&plot);
	plot.setAxisScale(QwtPlot::xBottom, 0, 1);
	plot.setAxisScale(QwtPlot::yLeft, 0, 1100);
	plot.setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDrawSoleil(QTime(0, 0)));
	plot.setWindowTitle("Instant Sun Power received by a 1 square meter pannel during one day");
	plot.setWindowIcon(QIcon(QString::fromUtf8(":/images/images/kfm_home.png")));
	plot.addcurve(tr("Sun Power"), "", Qt::yellow);
	plot.addcurve(tr("Pannel"), "", Qt::blue);
//	plot.addcurve(tr("Moving Max"), "", Qt::red);
//	plot.addcurve(tr("Moving Min"), "", Qt::green);
//	BackgroundSoleil2 *bg2 = new BackgroundSoleil2();
//	bg2->attach(&plotAn);
	plotAn.setAxisScale(QwtPlot::xBottom, 0, 365);
	plotAn.setAxisScale(QwtPlot::yLeft, 0, 5500);
    plotAn.setAxisScaleDraw(QwtPlot::xBottom, new DateScaleDrawSoleil(QDate(1970, 1, 1)));
	plotAn.setWindowTitle("Daily Sun Power received by 1 square meter during 1 year");
	plotAn.setWindowIcon(QIcon(QString::fromUtf8(":/images/images/kfm_home.png")));
	plotAn.addcurve(tr("Pannel"), "", Qt::blue);
//	plotAn.addcurve(tr("Moving Max"), "", Qt::red);
//	plotAn.addcurve(tr("Moving Min"), "", Qt::green);

// Qt::white, Qt::black, Qt::red, Qt::darkRed, Qt::green, Qt::darkGreen, Qt::blue, Qt::darkBlue, Qt::cyan, Qt::darkCyan, Qt::magenta, Qt::darkMagenta, Qt::yellow, Qt::darkYellow, Qt::gray, Qt::darkGray, Qt::lightGray

	TimerRefresh = new QTimer;
	connect(TimerRefresh, SIGNAL(timeout()), this, SLOT(refresh()));
	connect(ui.pushButtonGraphQuot, SIGNAL(clicked()), this, SLOT(GraphQuot()));
	connect(ui.pushButtonGraphAn, SIGNAL(clicked()), this, SLOT(GraphAn()));
	connect(ui.timeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(refreshtextonly(QTime)));
	connect(ui.dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(refreshinclinaison(QDate)));
	connect(ui.checkBoxAuto, SIGNAL(stateChanged(int)), this, SLOT(AutoChange(int)));
	connect(ui.spinBoxInclinaison, SIGNAL(valueChanged(int)), this, SLOT(refreshinclinaison(int)));
//	connect(ui.spinBoxVariation, SIGNAL(valueChanged(int)), this, SLOT(refreshinclinaison(int)));
	connect(ui.comboBoxVille, SIGNAL(currentIndexChanged(int)), this, SLOT(VilleChange(int)));
	connect(ui.spinBoxSurface, SIGNAL(valueChanged(int)), this, SLOT(refreshnow(int)));
	connect(ui.spinBoxLatitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	connect(ui.spinBoxLongitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	connect(ui.spinBoxAltitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	connect(ui.comboBoxCiel, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshinclinaison(int)));
	connect(ui.spinBoxEstOuest, SIGNAL(valueChanged(int)), this, SLOT(refreshinclinaison(int)));
	QDateTime Now = QDateTime::currentDateTime();
	ui.dateEdit->setDate(Now.date());
	if (ui.checkBoxAuto->isChecked() == true)
	{
		ui.dateEdit->setReadOnly(false);
		ui.timeEdit->setReadOnly(false);
		TimerRefresh->start(1000);
	}
	ui.comboBoxCiel->addItem(tr("Clear"));
	ui.comboBoxCiel->addItem(tr("Party cloudy"));
	ui.comboBoxCiel->addItem(tr("Very cloudy"));
	ui.comboBoxCiel->setCurrentIndex(0);
    VilleChange(68);
}








energiesolaire::~energiesolaire()
{
	delete TimerRefresh;
}








void energiesolaire::closeEvent(QCloseEvent*)
{
	plot.close();
	plotAn.close();
}
 
 	
 		


QTime energiesolaire::getSunRise()
{
	QDateTime Now = QDateTime::currentDateTime();
	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	position(Now, Lieu, Data, weathercom::GraphSolaireXMax - 1);
    // UTC to Local time
    return Data.HeureLever;
}





QTime energiesolaire::getSunSet()
{
	QDateTime Now = QDateTime::currentDateTime();
	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	position(Now, Lieu, Data, weathercom::GraphSolaireXMax - 1);
	return Data.HeureCoucher;	
}





void energiesolaire::SaveConfigStr(QString &str)
{
	str += "\n" Solar_Mark "\n";
	int index = ui.comboBoxVille->currentIndex();
	if (index < 0)
	{
		str += logisdom::saveformat("Latitude", QString("%1").arg(ui.spinBoxLatitude->value()));
		str += logisdom::saveformat("Longitude", QString("%1").arg(ui.spinBoxLongitude->value()));
		str += logisdom::saveformat("Altitude", QString("%1").arg(ui.spinBoxAltitude->value()));
	}
	else
	{
		str += logisdom::saveformat("GPSTown", ui.comboBoxVille->currentText());
	}
	str += logisdom::saveformat("EastWest", QString("%1").arg(ui.spinBoxEstOuest->value()));
	str += logisdom::saveformat("Inclinaison", QString("%1").arg(ui.spinBoxInclinaison->value()));
	str += logisdom::saveformat("Surface", QString("%1").arg(ui.spinBoxSurface->value()));
	str += EndMark;
	str +="\n";
}





void energiesolaire::readconfigfile(const QString &configdata)
{
	QString GPSTown;
	bool okLat, okLong, okAlt;
	double Latitude, Longitude, Altitude;
	QString TAG_Begin = Solar_Mark;
	QString TAG_End = EndMark;
	SearchLoopBegin
	GPSTown = logisdom::getvalue("GPSTown", strsearch);
	if (GPSTown.isEmpty())
	{
		Latitude = logisdom::getvalue("Latitude", strsearch).toDouble(&okLat);
		Longitude = logisdom::getvalue("Longitude", strsearch).toDouble(&okLong);
		Altitude = logisdom::getvalue("Altitude", strsearch).toDouble(&okAlt);
		if (okLat && okLong && okAlt)
		{
			ui.spinBoxLatitude->setValue(Latitude);
			ui.spinBoxLongitude->setValue(Longitude);
			ui.spinBoxAltitude->setValue(Altitude);
		}
	}
	else ui.comboBoxVille->setCurrentIndex(ui.comboBoxVille->findText(GPSTown));
	bool ok;
	double V = logisdom::getvalue("EastWest", strsearch).toDouble(&ok);
    if (ok) ui.spinBoxEstOuest->setValue(int(V));
	V = logisdom::getvalue("Inclinaison", strsearch).toDouble(&ok);
    if (ok) ui.spinBoxInclinaison->setValue(int(V));
	V = logisdom::getvalue("Surface", strsearch).toDouble(&ok);
    if (ok) ui.spinBoxSurface->setValue(int(V));
	SearchLoopEnd
}






void energiesolaire::VilleChange(int index)
{
    disconnect(ui.spinBoxLatitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	disconnect(ui.spinBoxLongitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	disconnect(ui.spinBoxAltitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
    bool ok;
    QString L = CoordonneePolaires[index * 5 + 3];
    L = L.replace(QString(","), QString("."));
    double l = L.toDouble(&ok);
    ui.spinBoxLongitude->setValue(l);
    L = CoordonneePolaires[index * 5 + 2];
    L = L.replace(QString(","), QString("."));
    l = L.toDouble(&ok);
    ui.spinBoxLatitude->setValue(l);
    L = CoordonneePolaires[index * 5 + 4];
    L = L.replace(QString(","), QString("."));
	refresh();
	if (plotAn.isHidden() == false) GraphAn();
	connect(ui.spinBoxLatitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	connect(ui.spinBoxLongitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
	connect(ui.spinBoxAltitude, SIGNAL(valueChanged(double)), this, SLOT(refreshinclinaison(double)));
}




void energiesolaire::refreshnow(int)
{
	refresh();
}








void energiesolaire::refreshinclinaison(int)
{
	
	refresh();
	if (plotAn.isHidden() == false) GraphAn();
}








void energiesolaire::refreshinclinaison(double)
{
	disconnect(ui.comboBoxVille, SIGNAL(currentIndexChanged(int)), this, SLOT(VilleChange(int)));
	ui.comboBoxVille->setCurrentIndex(-1);
	connect(ui.comboBoxVille, SIGNAL(currentIndexChanged(int)), this, SLOT(VilleChange(int)));
	refresh();
	if (plotAn.isHidden() == false) GraphAn();
}









void energiesolaire::refreshinclinaison(QDate)
{
	refresh();
	if (plotAn.isHidden() == false) GraphAn();
}







void energiesolaire::refreshnow(QTime)
{
	refresh();
}









void energiesolaire::refreshnow(QDate)
{
	refresh();
}








void energiesolaire::TimeConvertFromSecond(double TempsSolaireSecondes, QTime &T)
{
	int h = int(TempsSolaireSecondes/3600);
	int m = int(TempsSolaireSecondes / 60) % 60;
	int s = int(TempsSolaireSecondes - m * 60 - h * 3600);
	T.setHMS(h, m, s);
}







double energiesolaire::JourMois(double m)
{
    int mois = int(m);
	if (mois == 1) return (0);
	if (mois == 2) return (31);
	if (mois == 3) return (59);
	if (mois == 4) return (90);
	if (mois == 5) return (120);
	if (mois == 6) return (151);
	if (mois == 7) return (181);
	if (mois == 8) return (212);
	if (mois == 9) return (243);
	if (mois == 10) return (273);
	if (mois == 11) return (304);
	if (mois == 12) return (334);
	return (0);
}








void energiesolaire::AutoChange(int)
{
	if (ui.checkBoxAuto->isChecked() == true)
	{
		ui.dateEdit->setReadOnly(true);
		ui.timeEdit->setReadOnly(true);
		TimerRefresh->start(1000);
		refresh();
	}
	else
	{
		ui.dateEdit->setReadOnly(false);
		ui.timeEdit->setReadOnly(false);
		TimerRefresh->stop();
	}
}







void energiesolaire::refreshtextonly(QTime)
{
	QDateTime Now = QDateTime::currentDateTime();
	if (ui.checkBoxAuto->isChecked() == true)
	{
		ui.dateEdit->setDate(Now.date());
		ui.timeEdit->setTime(Now.time());
	}
	else
	{
		Now.setDate(ui.dateEdit->date());
		Now.setTime(ui.timeEdit->time());
	}

	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	position (Now, Lieu, Data, weathercom::GraphSolaireXMax - 1);
}







void energiesolaire::refresh()
{
	QDateTime Now = QDateTime::currentDateTime();
	if (ui.checkBoxAuto->isChecked() == true)
	{
		ui.dateEdit->setDate(Now.date());
		ui.timeEdit->setTime(Now.time());
	}
	else
	{
		Now.setDate(ui.dateEdit->date());
		Now.setTime(ui.timeEdit->time());
	}

	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	position(Now, Lieu, Data, weathercom::GraphSolaireXMax - 1);
	plot.curve[0]->curve->setSamples(Data.GraphT, Data.GraphPwM2);
	plot.curve[1]->curve->setSamples(Data.GraphT, Data.CapteurFixe);
//	plot.curve[2]->curve->setData(Data.GraphT, Data.CapteurInclinableMax, weathercom::GraphSolaireXMax - 1);
//	plot.curve[3]->curve->setData(Data.GraphT, Data.CapteurInclinableMin, weathercom::GraphSolaireXMax - 1);
	plot.replot();
}








double energiesolaire::getSunPower(QDateTime *T)
{
// return calculated sun power for full day today for given square meter of pannels, at given the angle 
	mutexGet.lock();
	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	if (T)
		position (*T, Lieu, Data, weathercom::GraphSolaireXMax - 1, true);  // ne pas supprimer true !!! fonction appelée par un thread
	else
		position (QDateTime::currentDateTime(), Lieu, Data, weathercom::GraphSolaireXMax - 1, true);  // ne pas supprimer true !!! fonction appelée par un thread
	mutexGet.unlock();
	return Data.PuissanceSolaire;
}




double energiesolaire::getActualSunPower(QDateTime *T)
{
// return calculated real time sun power for given square meter of pannels, at given the angle 
	mutexGet.lock();
	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	if (T)
		position(*T, Lieu, Data, weathercom::GraphSolaireXMax - 1, true);  // ne pas supprimer true !!! fonction appelée par un thread
	else
		position(QDateTime::currentDateTime(), Lieu, Data, weathercom::GraphSolaireXMax - 1, true);  // ne pas supprimer true !!! fonction appelée par un thread
	mutexGet.unlock();
	return Data.PuissanceSolaireNow;
}





void energiesolaire::GraphAn()
{
	QVector <double> PAnFixe;
	QVector <double> PAnInclinableMax;
	QVector <double> PAnInclinableMin;
	QVector <double> T;
	weathercom::GPS Lieu;
	Lieu.Latitude = ui.spinBoxLatitude->value();
	Lieu.Longitude = ui.spinBoxLongitude->value();
	Lieu.Altitude = ui.spinBoxAltitude->value();
	QDateTime Now;
	Now.setDate(ui.dateEdit->date());
	Now.setTime(QTime(12, 00));
    qint64 scalestart = - Now.date().daysTo(QDate(1970, 1, 1));
	plotAn.setAxisScale(QwtPlot::xBottom, scalestart, scalestart + 365);
	plotAn.show();
	plotAn.replot();
	for (int j=0; j<365; j+=3)
	{
		Now.setDate(ui.dateEdit->date().addDays(j));
		position(Now, Lieu, Data, weathercom::GraphSolaireXMax - 1, true);
		T.append(scalestart + j);
		PAnFixe.append(Data.PTotFixDay);
//		PAnInclinableMax[index] = Data.CapteurInclinableMax[weathercom::GraphSolaireXMax - 1];
//		PAnInclinableMin[index] = Data.CapteurInclinableMin[weathercom::GraphSolaireXMax - 1];
	}
	plotAn.curve[0]->curve->setSamples(T, PAnFixe);
//	plotAn.curve[1]->curve->setData(T, PAnInclinableMax, index);
//	plotAn.curve[2]->curve->setData(T, PAnInclinableMin, index);
	plotAn.replot();
}







void energiesolaire::GraphQuot()
{
	plot.show();
}







void energiesolaire::position(QDateTime now, weathercom::GPS lieu, weathercom::CalculSolaire &data, int resolution, bool calculonly)
{
	QMutexLocker locker(&mutexPosition);
	double ConversionDegresVersRadians = M_PI / 180.0;
	double ConversionRadiansVersDegres = 180.0 / M_PI;
	double heure = now.time().hour();
	double minute = now.time().minute();
	double seconde = now.time().second();
	double jour = now.date().day();
	double mois = now.date().month();
	double annee = now.date().year();
	double signeLatitude = lieu.Latitude;
	double signeLongitude = -1 * lieu.Longitude;
	double altitude = lieu.Altitude;
    // ici double correctionGMT = lieu.GMT * 60.0;

//	*********	Debut calcul pour l'heure now donnée	*************

// Correction longitude latitude suivant hemispheres ...
        
	double CorrectionLongitude = (4 * (signeLongitude));
    double Fuseau = QTimeZone::systemTimeZone().standardTimeOffset(now) / 60;
    //qDebug() << QString("Fuseau %1").arg(Fuseau);

// Determination heure été hiver
// Le passage a l'heure d'été se fait le dernier dimanche de mars a 2h00 il est 3h00
// Pour l'heure d'hiver c'est le dernier dimanche d'Octobre a 3h00 il est 2h00 (donc une date passée a cette date la entre 2h00 et 3h00 on peut pas savoir...)

	int n = 31;
	while (QDate(int(annee),03,n).dayOfWeek() != Qt::Sunday) n--;
	QDateTime changement_heure_ete = QDateTime(QDate(int(annee),03,n), QTime(2,0,0,0), Qt::LocalTime);
	n = 31;
	while (QDate(int(annee),10,n).dayOfWeek() != Qt::Sunday) n--;
	QDateTime changement_heure_hiver = QDateTime(QDate(int(annee),10,n), QTime(3,0,0,0), Qt::LocalTime);
    /* ici double ete = 0;
	if ((now.secsTo(changement_heure_ete) > 0) or (now.secsTo(changement_heure_hiver) < 0))
	{
		ete = 0;
	}
	if ((now.secsTo(changement_heure_ete) < 0) and (now.secsTo(changement_heure_hiver) > 0))
	{
		ete = -60;
    }*/
    double ete = QTimeZone::systemTimeZone().daylightTimeOffset(now) / 60;


// jour de l'année
	double correctionAnnee;
	double correctionMois;
	if (mois > 2)
		{
			correctionAnnee = annee;
			correctionMois = mois - 3;
		}
		else 
		{
			correctionAnnee = annee - 1;
		correctionMois = mois  + 9;
		}

// Détermination heuresolaire  

	double TempsLocalSecondes = heure * 3600.0 + minute * 60.0 + seconde;
    // ici double HeureTUSecondes = TempsLocalSecondes - correctionGMT * 60.0 + ete * 60.0;
    double HeureTUSecondes = now.toUTC().time().hour() * 3600.0 + now.toUTC().time().minute() * 60.0 + now.toUTC().time().second();
    // ici TempsLocalSecondes = HeureTUSecondes + CorrectionFuseau * 60.0 - ete * 60.0;
	QTime HeureLocaleSecondes;
	TimeConvertFromSecond(TempsLocalSecondes, HeureLocaleSecondes);
    // ici double TempsSolaireSecondes = HeureTUSecondes + CorrectionLongitude * 60.0;
    double TempsSolaireSecondes = HeureTUSecondes + CorrectionLongitude * 60.0;

// Inclinaison Déclinaisnode la terre

	double t = ((TempsSolaireSecondes / 3600.0 / 24.0) + jour + int(30.6 * correctionMois + 0.5) 
				+ int(365.25 * (correctionAnnee - 1976)) - 8707.5) / 36525.0;
	double G = 357.528 + 35999.05 * t;
	G -= int(G / 360) * 360;	// Modulo 360
	double C = (1.915 * sin(G * ConversionDegresVersRadians)) + (0.020 * sin(2.0 * G * ConversionDegresVersRadians));
	double L = 280.460 + (36000.770 * t) + C;
	L -= int(L / 360) * 360;	// Modulo 360
	double alpha = L - 2.466 * sin(2.0 * L * ConversionDegresVersRadians) + 0.053 *  sin(4.0 * L * ConversionDegresVersRadians);
	double inclinaison = 23.4393 - 0.013 * t;
	double declinaison = atan(tan(inclinaison * ConversionDegresVersRadians) * sin(alpha * ConversionDegresVersRadians)) * ConversionRadiansVersDegres;
	double EOT = (L - C - alpha) / 15.0 * 60.0;
	double EOTsec = EOT * 60.0;

// Détermination de l'heure de lever et de coucher

	double Sun = ConversionRadiansVersDegres * acos ( -1.0 *
			(sin (signeLatitude * ConversionDegresVersRadians) *
			sin (declinaison    * ConversionDegresVersRadians) - 
			sin ((-0.8333 - 0.0347 * sqrt (altitude)) * ConversionDegresVersRadians)) /
			cos (signeLatitude * ConversionDegresVersRadians) /
			cos (declinaison    * ConversionDegresVersRadians)) * 4.0;
    double heureleverM = (12.0 * 60.0 - Sun - CorrectionLongitude - EOT  -  ete + Fuseau);
    double heurecoucherM = (12.0 * 60.0 + Sun - CorrectionLongitude - EOT -  ete + Fuseau);
	double eclairement = heurecoucherM - heureleverM;
    QDateTime NowUTC(QDateTime::currentDateTime());
    NowUTC.setTimeZone(QTimeZone::utc());
    QTime T;
    TimeConvertFromSecond(heureleverM * 60.0, T);
    NowUTC.setTime(T);
    data.HeureLever = NowUTC.toLocalTime().time();
	//QTime HeureCoucher;
    TimeConvertFromSecond(heurecoucherM * 60.0, T);
    NowUTC.setTime(T);
    data.HeureCoucher = NowUTC.toLocalTime().time();
    //QTime Eclairement;
	TimeConvertFromSecond(eclairement * 60.0, data.Eclairement);
	plot.setAxisScale(QwtPlot::xBottom, heureleverM / 60 / 24, heurecoucherM / 60 / 24);

	TempsSolaireSecondes = TempsSolaireSecondes + EOTsec;
	if (TempsLocalSecondes > (24 * 3600)) TempsLocalSecondes = TempsLocalSecondes - 24 * 3600;
    //double JourCourant = 0;
	if (TempsSolaireSecondes < 0)
	{           // temps solaire a j+1
		TempsSolaireSecondes += 24 * 3600;
        //JourCourant = -1;
	}
	if (TempsSolaireSecondes >= (24 * 3600))
	{    // temps solaire a j-1
		TempsSolaireSecondes -= 24 * 3600;
        //JourCourant = 1;
	}
	QTime HeureSolaire;
	TimeConvertFromSecond(TempsSolaireSecondes, HeureSolaire);

	double AngleHoraire = (TempsSolaireSecondes / 60.0 - 12.0 * 60.0) / 4.0 * -1;
	double HauteurSoleil = ConversionRadiansVersDegres * asin (
                        (cos(signeLatitude  * ConversionDegresVersRadians)  *
                         cos (declinaison    * ConversionDegresVersRadians)  *
                         cos (AngleHoraire       * ConversionDegresVersRadians)) +
                        (sin (signeLatitude  * ConversionDegresVersRadians)  *
                         sin (declinaison     * ConversionDegresVersRadians)));
	if (HauteurSoleil < 0) HauteurSoleil = 0;

// azimuth angle

	double azimuthAngle = ConversionRadiansVersDegres * acos ((
			(sin (HauteurSoleil  * ConversionDegresVersRadians)  *
			sin (signeLatitude * ConversionDegresVersRadians)) -
			sin (declinaison   * ConversionDegresVersRadians)) /
			(cos (HauteurSoleil  * ConversionDegresVersRadians)  *
			cos (signeLatitude * ConversionDegresVersRadians)));                  
	if (azimuthAngle * AngleHoraire < 0) azimuthAngle *= -1 ;

// Orientation Est Ouest

	double azimuthAngleRelatifPanneau = azimuthAngle - ui.spinBoxEstOuest->value();

//	CalculDistanceTerreSoleil, exprimee en Kilometres

	double Rp = 56.0;
	double J2 = JourMois(mois) + jour - 1.0 + (heure + minute / 60.0 + seconde / 3600.0) / 24.0;
	double aa = annee / 4.0 - round(annee / 4.0);
    if (logisdom::isZero(aa)) aa = 1;
    if ((logisdom::AreSame(aa, 1)) && (mois > 2)) J2 = J2 + 1.0;
    double tt = ((annee - 1970.0) * 365.25 + J2 - aa) / 365250.0;
	double LM = 1.743470314 + 6283.3129663 * tt + 5.3 * pow(10,-4) * tt * tt;
	double KA = -0.003740816 - 0.004793106 * tt + 0.000281 * t * t;
	double HA = 0.016284477 - 0.001532379 * t - 0.00072 * t * t;
	double LP = atan(HA / KA);
	if (KA < 0) LP = LP + M_PI;
	double E = HA / sin(LP);
	double AE = LM - LP;
	Rp = Rp * (1.0 - E * cos(AE));
	double DistanceTS = (1.53 * pow(10,8) / 56.93565) * Rp;

// Calcul de la constante solaire, exprimée en Watts

	double KSoleil = 5500.0; // °C
	double tempsol = KSoleil + 273.0; // °Kelvin
	double dist = DistanceTS * 1000.0;	// Km -> m
	double raysol = 700000000.0;
	double enphoto = 5.67 / 100000000.0 * tempsol * tempsol * tempsol * tempsol * 4.0 * M_PI * raysol * raysol;
	double constante = enphoto / (4.0 * M_PI * dist * dist);

// Calcul l'épaisseur d'atmosphère traversïée, exprimée en Kilomètres

	double tg = tan(HauteurSoleil * ConversionDegresVersRadians);
	double Ccos = cos(HauteurSoleil * ConversionDegresVersRadians);
	double delta = pow((2.0 * 6370.0 * tg), 2.0) - 4.0 * (1.0 + pow(tg, 2.0)) * (pow(6370.0, 2.0) - pow(6870.0, 2.0));
	double x21 = (-(2.0 * 6370.0 * tg) + sqrt(delta)) / (2.0 * (1.0 + pow(tg, 2.0)));
	double epaisseurAtmos = round(x21 / Ccos);

// calcul énergie mesurée

	double CoeffCalc = 3;
	CoeffCalc = (pow(HauteurSoleil, -0.5315) * 12.128);
	if (ui.comboBoxCiel->currentIndex() == 0) CoeffCalc = pow(HauteurSoleil,-0.5315)*12.128;
	if (ui.comboBoxCiel->currentIndex() == 1) CoeffCalc = pow(HauteurSoleil,-0.7853)*46.614;
	if (ui.comboBoxCiel->currentIndex() == 2) CoeffCalc = pow(HauteurSoleil,-0.7918)*58.309;

//	double absorptioncalc = 100 - round((constante / CoeffCalc) / constante * 100.0);
//	double pscalc = round(constante - constante * absorptioncalc / 100.0);

	double absorptioncalc = 100 - ((constante / CoeffCalc) / constante * 100.0);
	double pscalc = (constante - constante * absorptioncalc / 100.0);
// Calcul puissane reçue par panneau solaire

//	double AngleMax = 90 - (ui.spinBoxInclinaison->value());
//	double AngleMin = 90 - (ui.spinBoxInclinaison->value() + ui.spinBoxVariation->value());
	double perteazimut = cos(azimuthAngleRelatifPanneau * ConversionDegresVersRadians);
	double PCapteurFixe = pscalc * sin((HauteurSoleil + ui.spinBoxInclinaison->value()) * ConversionDegresVersRadians) * perteazimut;
	if (PCapteurFixe < 0) PCapteurFixe = 0;
//	double PCapteurInclinableMax = 0;
//	if ((HauteurSoleil >= AngleMin) and (HauteurSoleil <= AngleMax)) PCapteurInclinableMax = pscalc * perteazimut;
//	if (HauteurSoleil < AngleMin) PCapteurInclinableMax = pscalc * sin((90 - (AngleMin - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//	if (HauteurSoleil > AngleMax) PCapteurInclinableMax = pscalc * sin((90 - (AngleMax - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//	double PCapteurInclinableMin = 0;
//	double P1 = pscalc * sin((90 - (AngleMin - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//	double P2 = pscalc * sin((90 - (AngleMax - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//	if (P2 < P1)  PCapteurInclinableMin = P2; else  PCapteurInclinableMin = P1;

// Détermination surfacesol

	double PSufCapteurFixeTot = PCapteurFixe * ui.spinBoxSurface->value();
	double surfacesol = sin(HauteurSoleil * ConversionDegresVersRadians);
	double puissol = pscalc * surfacesol;
	if (HauteurSoleil <= 0)
	{
		puissol = 0;
		pscalc = 0;
		absorptioncalc = 0;
		//constante = 0;
		epaisseurAtmos = 0;
		surfacesol = 0;
		data.PuissanceSolaireNow = 0;
		PCapteurFixe = 0;
		PSufCapteurFixeTot = 0;
	}
	data.PuissanceSolaireNow = PSufCapteurFixeTot;
// Affichage des données
	if (calculonly == false)
	{
		ui.textEdit->clear();
		ui.textEdit->insertPlainText("Calcul instantané ");
		ui.textEdit->insertPlainText(ui.dateEdit->date().toString("ddd d MMM yyyy") + "  ");
		ui.textEdit->insertPlainText(" a " + HeureLocaleSecondes.toString("HH:mm:ss") + "\r\n");
		ui.textEdit->insertPlainText("Heure Solaire = " + HeureSolaire.toString("HH:mm:ss") + "\r\n");
		ui.textEdit->insertPlainText(QString("Déclinaison de la Terre = %1°\r\n").arg(declinaison, 0, 'f', 2));
		ui.textEdit->insertPlainText(QString("Azimut = %1°\r\n").arg(azimuthAngle, 0, 'f', 2));
	 	ui.textEdit->insertPlainText(QString("HauteurSoleil = %1°\r\n").arg(HauteurSoleil, 0, 'f', 2));
		ui.textEdit->insertPlainText("Heure Lever = " + data.HeureLever.toString("HH:mm") + "\r\n");
		ui.textEdit->insertPlainText("Eclairement = " + data.Eclairement.toString("HH:mm") + "\r\n");
		ui.textEdit->insertPlainText("Heure Coucher = " + data.HeureCoucher.toString("HH:mm") + "\r\n");
		ui.textEdit->insertPlainText(QString("Dist. Terre Soleil = %1 Km\r\n").arg(DistanceTS, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Constante Soleil = %1 W/m2\r\n").arg(constante, 0, 'f', 2));
		ui.textEdit->insertPlainText(QString("Epaisseur atmoshpere traversee = %1 Km\r\n").arg(epaisseurAtmos, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Absorption Atmoshphère  = %1%\r\n").arg(absorptioncalc, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Puissance solaire directe = %1 Watts/m2\r\n").arg(pscalc, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Puissance solaire au sol = %1 Watts/m2\r\n").arg(puissol, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Puissance sur panneau incliné à %1° = %2 Watts/m2\r\n").arg(ui.spinBoxInclinaison->value()).arg(PCapteurFixe, 0, 'f', 0));
		ui.textEdit->insertPlainText(QString("Puissance pour %1 m2 = %2 Watts\r\n").arg(ui.spinBoxSurface->value()).arg(PSufCapteurFixeTot, 0, 'f', 0));
//		ui.textEdit->insertPlainText(QString("Puissance Max sur panneau inclinable = %1\r\n").arg(PCapteurInclinableMax, 0, 'f', 0));
//		ui.textEdit->insertPlainText(QString("Puissance Min sur panneau inclinable = %1\r\n").arg(PCapteurInclinableMin, 0, 'f', 0));
//		ui.textEdit->insertPlainText(QString("Surface Sol  = %1\r\n").arg(1 / surfacesol, 0, 'f', 2));
	}

//	**********  fin Calcul pour l'heure donnée	*********





//	**********  Début Calcul pour un jour complet	*********

	double PTotFixe = 0;
//	double PTotInclinableMax = 0;
//	double PTotInclinableMin = 0;
//	data.GraphHauteur.clear();
//	data.GraphAzimut.clear();
	data.GraphPwM2.clear();
	data.GraphT.clear();
	data.CapteurFixe.clear();
	for (int T = 0; T < resolution; T++)
	{
	// Determination heuresolaire
		TempsLocalSecondes = 24.0 * 3600.0 * T / resolution;
        HeureTUSecondes = TempsLocalSecondes - Fuseau * 60.0 + ete * 60.0;
        TempsLocalSecondes = HeureTUSecondes + Fuseau * 60.0 - ete * 60.0;
		TempsSolaireSecondes = HeureTUSecondes + CorrectionLongitude * 60.0;
	// Inclinaison Declinaisno de la terre
		t = ((TempsSolaireSecondes / 3600.0 / 24.0) + jour + int(30.6 * correctionMois + 0.5) 
					+ int(365.25 * (correctionAnnee - 1976)) - 8707.5) / 36525.0;
		G = 357.528 + 35999.05 * t;
		G -= int(G / 360) * 360;	// Modulo 360
		C = (1.915 * sin(G * ConversionDegresVersRadians)) + (0.020 * sin(2.0 * G * ConversionDegresVersRadians));
		L = 280.460 + (36000.770 * t) + C;
		L -= int(L / 360) * 360;	// Modulo 360
		alpha = L - 2.466 * sin(2.0 * L * ConversionDegresVersRadians) + 0.053 *  sin(4.0 * L * ConversionDegresVersRadians);
		inclinaison = 23.4393 - 0.013 * t;
		declinaison = atan(tan(inclinaison * ConversionDegresVersRadians) * sin(alpha * ConversionDegresVersRadians)) * ConversionRadiansVersDegres;
		EOT = (L - C - alpha) / 15.0 * 60.0;
		EOTsec = EOT * 60.0;
		TempsSolaireSecondes = TempsSolaireSecondes + EOTsec;
		if (TempsLocalSecondes > (24 * 3600)) TempsLocalSecondes = TempsLocalSecondes - 24 * 3600;
        //JourCourant = 0;
		if (TempsSolaireSecondes < 0)
		{	// temps solaire ï¿½ j+1
			TempsSolaireSecondes += 24 * 3600;
            //JourCourant = -1;
		}
		if (TempsSolaireSecondes >= (24 * 3600))
		{	// temps solaire ï¿½ j-1
			TempsSolaireSecondes -= 24 * 3600;
            //JourCourant = 1;
		}
	//TimeConvertFromSecond(TempsSolaireSecondes, HeureSolaire);
		AngleHoraire = (TempsSolaireSecondes / 60.0 - 12.0 * 60.0) / 4.0 * -1;
		HauteurSoleil = ConversionRadiansVersDegres * asin (
                        (cos(signeLatitude * ConversionDegresVersRadians) *
	                         cos (declinaison * ConversionDegresVersRadians) *
	                         cos (AngleHoraire * ConversionDegresVersRadians)) +
	                        (sin (signeLatitude * ConversionDegresVersRadians) *
	                         sin (declinaison * ConversionDegresVersRadians)));
		if (HauteurSoleil < 0) HauteurSoleil = 0;
	// azimuth angle
		azimuthAngle = ConversionRadiansVersDegres * acos ((
				(sin (HauteurSoleil * ConversionDegresVersRadians) *
				sin (signeLatitude * ConversionDegresVersRadians)) -
				sin (declinaison * ConversionDegresVersRadians)) /
				(cos (HauteurSoleil * ConversionDegresVersRadians) *
				cos (signeLatitude * ConversionDegresVersRadians)));
		if (azimuthAngle * AngleHoraire < 0) azimuthAngle *= -1 ;
	// Orientation Est Ouest
		azimuthAngleRelatifPanneau = azimuthAngle - ui.spinBoxEstOuest->value();
	//	CalculDistanceTerreSoleil, exprimee en Kilometres
		Rp = 56.0;
		J2 = JourMois(mois) + jour - 1.0 + (heure + minute / 60.0 + seconde / 3600.0) / 24.0;
		aa = annee / 4.0 - round(annee / 4.0);
        if (logisdom::isZero(aa)) aa = 1;
        if ((logisdom::AreSame(aa, 1)) && (mois > 2)) J2 = J2 + 1.0;
        tt = ((annee - 1970.0) * 365.25 + J2 - aa) / 365250.0;
		LM = 1.743470314 + 6283.3129663 * tt + 5.3 * pow(10,-4) * tt * tt;
		KA = -0.003740816 - 0.004793106 * tt + 0.000281 * t * t;
		HA = 0.016284477 - 0.001532379 * t - 0.00072 * t * t;
		LP = atan(HA / KA);
		if (KA < 0) LP = LP + M_PI;
		E = HA / sin(LP);
		AE = LM - LP;
		Rp = Rp * (1.0 - E * cos(AE));
		DistanceTS = (1.53 * pow(10,8) / 56.93565) * Rp;

	// Calcul de la constante solaire, exprimee en Watts
		KSoleil = 5500.0; // °C
		tempsol = KSoleil + 273.0; // °Kelvin
		dist = DistanceTS * 1000.0;	// Km -> m
		raysol = 700000000.0;
		enphoto = 5.67 / 100000000.0 * tempsol * tempsol * tempsol * tempsol * 4.0 * M_PI * raysol * raysol;
		constante = enphoto / (4.0 * M_PI * dist * dist);
	//Calcul épaisseur d'atmosphère traversée, exprimée en Kilomètres
		tg = tan(HauteurSoleil * ConversionDegresVersRadians);
		Ccos = cos(HauteurSoleil * ConversionDegresVersRadians);
		delta=pow((2.0 * 6370.0 * tg), 2.0) - 4.0 * (1.0 + pow(tg, 2.0)) * (pow(6370.0, 2.0) - pow(6870.0, 2.0));
		x21=(-(2.0 * 6370.0 * tg) + sqrt(delta)) / (2.0 * (1.0 + pow(tg, 2.0)));
		epaisseurAtmos = x21 / Ccos;
	//calcul énergie mesurée
		CoeffCalc = (pow(HauteurSoleil, -0.5315) * 12.128);
		if (ui.comboBoxCiel->currentIndex() == 0) CoeffCalc = pow(HauteurSoleil,-0.5315)*12.128;
		if (ui.comboBoxCiel->currentIndex() == 1) CoeffCalc = pow(HauteurSoleil,-0.7853)*46.614;
		if (ui.comboBoxCiel->currentIndex() == 2) CoeffCalc = pow(HauteurSoleil,-0.7918)*58.309;
		absorptioncalc = 100 - ((constante / CoeffCalc) / constante * 100.0);
		pscalc = constante - constante * absorptioncalc / 100.0;
	// Détermination surfacesol
		surfacesol = sin(HauteurSoleil * ConversionDegresVersRadians);
		puissol = pscalc * surfacesol;
		perteazimut = cos(azimuthAngleRelatifPanneau * ConversionDegresVersRadians);
		if (perteazimut < 0) perteazimut = 0;
		PCapteurFixe = qAbs(pscalc * sin((HauteurSoleil + ui.spinBoxInclinaison->value()) * ConversionDegresVersRadians) * perteazimut);
		surfacesol = cos(azimuthAngle * ConversionDegresVersRadians);
//		if ((HauteurSoleil >= AngleMin) and (HauteurSoleil <= AngleMax)) PCapteurInclinableMax = pscalc * perteazimut;
//		if (HauteurSoleil < AngleMin) PCapteurInclinableMax = pscalc * sin((90 - (AngleMin - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//		if (HauteurSoleil > AngleMax) PCapteurInclinableMax = pscalc * sin((90 - (AngleMax - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//		P1 = pscalc * sin((90 - (AngleMin - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//		P2 = pscalc * sin((90 - (AngleMax - HauteurSoleil)) * ConversionDegresVersRadians) * perteazimut;
//		if (P2 < P1)  PCapteurInclinableMin = P2; else  PCapteurInclinableMin = P1;
		if (HauteurSoleil<0)
		{
			HauteurSoleil = 0;
			puissol = 0;
			pscalc = 0;
			absorptioncalc = 0;
			constante = 0;
			epaisseurAtmos = 0;
			surfacesol = 0;
			PCapteurFixe = 0;
//			PCapteurInclinableMax = 0;
//			PCapteurInclinableMin = 0;
		}
//	data.GraphHauteur.append(HauteurSoleil);
//	data.GraphAzimut.append(azimuthAngle);
	data.GraphPwM2.append(pscalc);
	data.GraphT.append(TempsLocalSecondes / 24.0 / 3600.0);
	data.CapteurFixe.append(PCapteurFixe);
//	data.CapteurInclinableMax[T] = PCapteurInclinableMax;
//	data.CapteurInclinableMin[T] = PCapteurInclinableMin;
	PTotFixe += PCapteurFixe;
//	PTotInclinableMax += PCapteurInclinableMax;
//	PTotInclinableMin += PCapteurInclinableMin;
	}
//	double gain = (PTotInclinableMax - PTotFixe) / PTotFixe * 100;
	double PSufCapteurFixeDay = PTotFixe / resolution * 24.0 * ui.spinBoxSurface->value();
	data.PuissanceSolaire = PSufCapteurFixeDay;
//	double PSufCapteurInclinable = PTotInclinableMax / resolution * 24.0 * ui.spinBoxSurface->value();
	if (calculonly == false)
	{
		ui.textEdit->insertPlainText(QString("Cumul sur la journee pour %1 m2 = %2 Watts\r\n").arg(ui.spinBoxSurface->value()).arg(PSufCapteurFixeDay, 0, 'f', 0));
//		ui.textEdit->insertPlainText(QString("Puissance totale Capteur Inclinable = %1 Watts\r\n").arg(PSufCapteurInclinable, 0, 'f', 0));
//		ui.textEdit->insertPlainText(QString("Gain Capteur Inclinable= + %1%\r\n").arg(gain, 0, 'f', 0));
	}
	data.PTotFixDay = PTotFixe / resolution * 24.0;
//	data.CapteurInclinableMax[weathercom::GraphSolaireXMax - 1] = PTotInclinableMax / resolution * 24.0;
//	data.CapteurInclinableMin[weathercom::GraphSolaireXMax - 1] = PTotInclinableMin / resolution * 24.0;
}




