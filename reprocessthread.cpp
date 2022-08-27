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





#include "globalvar.h"
#include "onewire.h"
#include "configwindow.h"
#include "formula.h"
#include "reprocessthread.h"




reprocessthread::reprocessthread(formula *parent) : calcthread(parent)
{
    deviceIndex = nullptr;
}




void reprocessthread::runprocess()
{
	//QString logtxt;
    //Origin.setDate(QDate(yearscalebegin, 1, 1));
    //Origin.setTime(QTime(0, 0));
	//logtxt += "Origin at " + Origin.toString("dd MMM yyyy hh:mm:ss\n");
	QFile file;
	bool loading;
	QTextStream out;
	int lastSaveIndex = -1;
	resetDSP();
	QString F = Calc;
	int lastMonth = -1;
	int decimal = device->Decimal.value();
	if (!QDir().exists(parent->parent->getrepertoiredat())) QDir().mkdir(parent->parent->getrepertoiredat());
	if (!device)
	{
		//logtxt += "Parent is nullptr\n";
		goto Finish;
	}
	stopRequest = false;
	TCalc = &timeIndex;
	//logtxt += "Begin timeIndex at " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
	if (deviceIndex)
	{
	    loading = false;
        qint64 index = deviceIndex->getNextIndex(timeIndex, loading);
	    while (((loading) && (!stopRequest)) || (index < 0))
	    {
            loading = false;
            index = deviceIndex->getNextIndex(timeIndex, loading);
            /*if (loading)
            {
               // logtxt += "Device is loading or index cannot be calculated during initial getNextIndex\n";
                state = "  " + tr("Wait device finished loading");
                msleep(2000);
            }*/
            while (loading)
            {
                state = "  " + tr("Wait device finished loading");
                //msleep(2000);
                QTime dieTime = QTime::currentTime().addSecs(2);
                while( QTime::currentTime() < dieTime )
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                loading = false;
                index = deviceIndex->getNextIndex(timeIndex, loading);
            }
	    }
	    //logtxt += "Get first index " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
        timeIndex = QDateTime::fromMSecsSinceEpoch(index);
	    //logtxt += "First calcul timeIndex at " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
	}
    calculate(F);
	while (deviceLoading)
	{
//	    msleep(2000);
        QTime dieTime = QTime::currentTime().addSecs(2);
        while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        state = "  " + tr("Wait device finished loading");
        Calc = F;
        calculate(F);
	}
	progressIndex = timeIndex.secsTo(QDateTime::currentDateTime());
	progressMax = progressIndex;
	state = timeIndex.toString(statusFormat);
	while ((progressIndex > 0) && (!stopRequest) &&	(!deviceLoading))
	{
		int actualMonth = timeIndex.date().month();
		if (actualMonth != lastMonth)  // delete dat file if already exist
		{
			if (file.isOpen()) file.close();
			QString filename = parent->parent->getrepertoiredat() + device->getromid() + "_" + timeIndex.toString("MM-yyyy") + dat_ext;
			file.setFileName(filename);
			if (file.exists()) file.remove();
			file.open(QIODevice::WriteOnly | QIODevice::Text);
			out.setDevice(&file);
			out << "// Version 1\n";
			lastSaveIndex = -1;
			lastMonth = actualMonth;
			progressIndex = timeIndex.secsTo(QDateTime::currentDateTime());
			//logtxt += "New file " + timeIndex.toString("dd MMM yyyy hh:mm:ss") + "\n";
		}
		state = timeIndex.toString(statusFormat);
        double R = calculate(F);
		//logtxt += "Calculate at " + timeIndex.toString("dd MMM yyyy hh:mm:ss") + QString("   Value = %1").arg(R, 0, 'f', 3) + "\n";
        if (logisdom::isNotNA(R) && (dataValid) && (syntax))
		{
			QString strOut;
			bool checkLastsave = false;
			if (lastSaveIndex < 0) checkLastsave = true;
			int currentIndex = (timeIndex.date().month() * 31 * 24) + (timeIndex.date().day() * 24) + timeIndex.time().hour();
			if (currentIndex != lastSaveIndex) checkLastsave = true;
			if (checkLastsave || (timeIndex.time().minute() == 0))
			{
				strOut = timeIndex.toString("(dd)[HH:mm:ss]") + "'" + QString("%1").arg(R, 0, 'f', decimal) + "'" + "\n";
				lastSaveIndex = currentIndex;
			}
			else
			{
				strOut = timeIndex.toString("[mm:ss]");
                if (logisdom::AreSame(lastsavevalue, R)) strOut += "=";
				else strOut += "'" + QString("%1").arg(R, 0, 'f', decimal) + "'";
				strOut += "\n";
			}
			if (!strOut.isEmpty())
			{
				out << strOut;
				lastSaveIndex = currentIndex;
			}
			lastsavevalue = R;
		}
		if (deviceIndex)
		{
		    loading = false;
		    timeIndex = timeIndex.addSecs(1);
		    //logtxt += "Get Next index for " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
            qint64 index = deviceIndex->getNextIndex(timeIndex, loading);
		    if ((loading))
		    {
			//logtxt += "Device is loading or index cannot be calculated\n";
			goto Finish;
		    }
		    else if (index < 0)
		    {
			//logtxt += "No more Data goto Finish " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
			goto Finish;
		    }
            timeIndex = QDateTime::fromMSecsSinceEpoch(index);
		    //logtxt += "Next index " + timeIndex.toString("dd MMM yyyy hh:mm:ss\n");
		}
                else
                {
                    if (saveInterval > Minutes2Weeks)
                    {
                        timeIndex = timeIndex.addMonths(1);
                    }
                    else
                    {
                        timeIndex = timeIndex.addSecs(saveInterval);
                    }
                }
		progressIndex = timeIndex.secsTo(QDateTime::currentDateTime());
		progress = ((progressMax - progressIndex) * 100) / progressMax;
	}
Finish:
	TCalc = nullptr;
	deviceIndex = nullptr;
	if (file.isOpen()) file.close();
	//if (deviceLoading) logtxt += "Device is loading, reprocess aborted\n";
	//logtxt += "Device " + device->getromid() + "  Finished Reprocessing";
	//QFile logfile(device->getromid() + "_reprocess_Log.txt");
	//logfile.open(QIODevice::WriteOnly | QIODevice::Text);
	//QTextStream logout(&logfile);
	//logout << logtxt;
	//logfile.close();
    emit(finished());
}





