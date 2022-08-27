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




#include <QtGui>
#include <QtCore>

#include "global.h"
#include "weathercom.h"
#include "ecogest.h"
#ifdef Q_OS_LINUX
#include "signal.h"
#endif


logisdom *maison1wirewindow;
void logfile(QString);
bool isValidIp(QString &ip);
static QTranslator tr;
static QTranslator Qtr;


/*
void myMessageOutput(QtMsgType type, const char *msg)
{
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s\n", msg);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s\n", msg);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s\n", msg);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s\n", msg);
		abort();
	}
}
*/



class LogisDomApplication : public QApplication
{
public:
    LogisDomApplication(int& argc, char** argv) : QApplication(argc, argv)
    {}
    bool notify(QObject* receiver, QEvent *event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch(std::bad_alloc const& ex)
        {
            QMessageBox::critical( nullptr, "Memory allocation exception", ex.what(), "Ok" );
            return false;
        }
//		catch(std::bad_cast const& ex)
//		{
//			QMessageBox::critical( nullptr, "Bad cast exception", ex.what(), "Ok" );
//			return false;
//		}
        catch(std::bad_exception const& ex)
        {
            QMessageBox::critical( nullptr, "Bad exception", ex.what(), "Ok" );
            return false;
        }
//		catch(std::bad_typeid const& ex)
//		{
//			QMessageBox::critical( 0, "Bad typeid exception", ex.what(), "Ok" );
//			return false;
//		}
        catch(std::exception const& ex)
        {
            QMessageBox::critical( nullptr, "Exception", ex.what(), "Ok" );
            return false;
        }
        catch(const QString& ex)
        {
            QMessageBox::critical(nullptr, "Exception", ex);
        return false;
        }
        catch(...)
        {
            QMessageBox::critical(nullptr, "Exception", "Erreur inconnue");
            return false;
        }
    return false;
    }
};




#ifdef Q_OS_LINUX
void handleSigpipe(int signum)
{
	QFile file("sigpipe.log");
	QString log;
	log += "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  signal broken pipe %1").arg(signum);
	QTextStream out(&file);
	file.open(QIODevice::Append | QIODevice::Text);
	out << log;
	file.close();
	return;
}
#endif


#ifdef Q_OS_WIN32
#endif



int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(onewire);
	LogisDomApplication appmaison1wire(argc, argv);
	QString locale = QLocale::system().name();
    if (tr.load(QString("logisdom_") + locale, QString("trans"))) tr.load(QString("logisdom_fr") , QString("trans"));
    appmaison1wire.installTranslator(&tr);
    if ((Qtr.load(QDir::separator() + QString("qt_") + locale, QString("trans")))) appmaison1wire.installTranslator(&Qtr);
#ifdef Q_OS_LINUX
	signal(SIGPIPE, handleSigpipe);
#endif
#ifdef Q_OS_WIN32
#endif
    //    QMessageBox::warning(0, QObject::tr("Warning Title"), QObject::tr("Warning Message"), QMessageBox::Ok|QMessageBox::Cancel);
	maison1wirewindow = new logisdom();
	maison1wirewindow->show();
	maison1wirewindow->init(argv);
	appmaison1wire.connect(&appmaison1wire, SIGNAL(lastWindowClosed()), &appmaison1wire, SLOT(quit()));
    appmaison1wire.addLibraryPath("lib");
    qDebug() << QApplication::font();
    try
    {
        return appmaison1wire.exec();
    }
    catch(const QString& ex)
    {
        QMessageBox::critical(nullptr, "Exception", ex);
        return 0;
    }
    catch(...)
    {
        QMessageBox::critical(nullptr, "Exception", "Erreur inconnue");
        return 0;
    }
}






