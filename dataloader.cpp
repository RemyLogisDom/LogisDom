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





#include "quazip.h"
#include "quazipfile.h"
#include "onewire.h"
#include "dataloader.h"
#include "logisdom.h"




dataloader::dataloader(logisdom *Parent)
{
	parent = Parent;
	begin = 0;
    //Origin.setDate(QDate(yearscalebegin, 1, 1));
    //Origin.setTime(QTime(0, 0));
	check85 = false;
	logGetValue = false;
	logLoadData = false;
    done = false;
	busy = false;
	RemoteConnection = nullptr;
	moveToThread(this);
}




void dataloader::run()
{
	busy = true;
	QTime t;
	t.start();
	logtxt.clear();
	if (!begin)
	{
		if (logLoadData) logtxt += "begin = 0\n";
		Data_Y.clear();
		Data_Time.clear();
        QDateTime now = QDateTime::currentDateTime();
        int year = now.date().year();
        int month = now.date().month();
        QDateTime T;
        T.setDate(QDate(year, month, 1));
        T.setTime(QTime(0, 0));
#if QT_VERSION > 0x050603
        begin = T.toSecsSinceEpoch();
#else
        QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
        begin = origin.secsTo(T);
#endif
        if (begin < 0)
		{
		    begin = 0;
		    busy = false;
            done = true;
            return;
		}
		if (logLoadData) logtxt += QString("Start Load Month Data %1/%2\n").arg(month).arg(year);
		if (!loadData(month, year))
			if (logLoadData) logtxt += QString("File not found ..... %1/%2\n").arg(month).arg(year);
	}
#if QT_VERSION > 0x050603
    while (begin >= (DateRequest.addMonths(-1).toSecsSinceEpoch()))
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    while (begin >= origin.secsTo(DateRequest.addMonths(-1)))
#endif
    {
		//if (logLoadData) logtxt += QString("begin = %1  ").arg(begin) + Origin.addSecs(begin).toString("dd MMM yyyy hh:mm:ss") + "\n";
		QDateTime T;
#if QT_VERSION > 0x050603
        T = QDateTime::fromSecsSinceEpoch(begin).addMonths(-1);
#else
        T = origin.addSecs(begin).addMonths(-1);
#endif
        int year = T.date().year();
		int month = T.date().month();
		T.setDate(QDate(year, month, 1));
		T.setTime(QTime(0, 0));
		if (logLoadData) logtxt += QString("Start Load Month Data %1/%2\n").arg(month).arg(year);
		if (!loadData(month, year))
			if (logLoadData) logtxt += QString("File not found ..... %1/%2\n").arg(month).arg(year);
#if QT_VERSION > 0x050603
        begin = T.toSecsSinceEpoch();
#else
        begin = origin.secsTo(T);
#endif
        emit(beginChanged());
	}
	if (logLoadData)
	{
		logtxt += "Device " + romID + QString("  Finished Loading Data done in %1 ms\n").arg(t.elapsed());
		QFile file(romID + "_LoadData_Log.txt");
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream out(&file);
		out << logtxt;
		file.close();
	}
	busy = false;
    done = true;
}






bool dataloader::isDataReady(const QDateTime &T)
{
	if (busy)
	{
		if ((DateRequest.secsTo(T)) < 0) DateRequest = T;
        done = false;
		return false;
	}
#if QT_VERSION > 0x050603
    if ((T.toSecsSinceEpoch() > begin) && begin && done) return true;
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    if (((origin.secsTo(T)) > begin) && begin && done) return true;
#endif
    DateRequest = T;
	return false;
}






bool dataloader::isDataReady(qint64 T)
{
	if (busy)
	{
#if QT_VERSION > 0x050603
        if (DateRequest.toSecsSinceEpoch() < T) DateRequest = QDateTime::fromSecsSinceEpoch(T);
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    if (DateRequest.secsTo(origin.addSecs(T)) < 0) DateRequest = origin.addSecs(T);
#endif
        done = false;
        return false;
	}
    if ((T > begin) && begin && done) return true;
#if QT_VERSION > 0x050603
    DateRequest = QDateTime::fromSecsSinceEpoch(T);
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    DateRequest = origin.addSecs(T);
#endif
	return false;
}





void dataloader::clearData()
{
	QMutexLocker locker(&data_Access);
	begin = 0;
    done = false;
    Data_Y.clear();
	Data_Time.clear();
}




void dataloader::appendData(const QDateTime &T, const double &V)
{
	QMutexLocker locker(&data_Access);
	Data_Y.append(V);
#if QT_VERSION > 0x050603
    Data_Time.append(T.toSecsSinceEpoch());
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    Data_Time.append(origin.secsTo(T));
#endif
}




bool dataloader::getValues(qint64 BEGIN, qint64 END, s_Data &data, int minDif)
{
	if (busy) return false;
    if (!done) return false;
    logtxt.clear();
	if (logGetValue) logtxt.append("*********************************\n");
    qint64 begin, end;
	if (BEGIN > END)
	{
		begin = END;
		end = BEGIN;
	}
	else
	{
		begin = BEGIN;
		end = END;
	}
    //if (logGetValue) logtxt += "Get Value Begin  " + Origin.addSecs(begin).toString("dd MMM yyyy hh:mm:ss\n");
    qint64 indexBegin = getIndex(begin, searchAfter, minDif);
    //if (logGetValue) logtxt += "Get Value End  " + Origin.addSecs(end).toString("dd MMM yyyy hh:mm:ss\n");
    qint64 indexEnd = getIndex(end, searchBefore, minDif);
	if ((indexEnd < 0) or (indexBegin < 0))
	{
		if (logGetValue)
		{
			if (indexEnd < 0) logtxt += "Could not locate data for indexEnd\n";
			if (indexBegin < 0) logtxt += "Could not locate data for indexBegin\n";
			QFile file(romID + "_getValues_Log.txt");
			file.open(QIODevice::Append | QIODevice::Text);
			QTextStream out(&file);
			out << logtxt;
			file.close();
		}
		return false;
	}
	if (indexBegin <= indexEnd)
	{
        for (qint64 n=indexBegin; n<=indexEnd; n++)
		{
			data.data_Y.append(Data_Y.at(n));
			data.offset.append(Data_Time.at(n));
		}
	}
	else
	{
        for (qint64 n=indexBegin; n>=indexEnd; n--)
		{
			data.data_Y.append(Data_Y.at(n));
			data.offset.append(Data_Time.at(n));
		}
	}
	if (logGetValue) logtxt += QString("Added %1 values").arg(data.data_Y.count()) + "\n";
    //for (int n=0; n<data.data_Y.count(); n++) logtxt += QString("Values %1 = %2").arg(n).arg(data.data_Y.at(n)) + "\n";
	if (logGetValue)
	{
		QFile file(romID + "_getValues_Log.txt");
		file.open(QIODevice::Append | QIODevice::Text);
		QTextStream out(&file);
		out << logtxt;
		file.close();
	}
	if (data.data_Y.count() == 0) return false;
	return true;
}





double dataloader::getValue(qint64 t, int minDif)
{
	logtxt.clear();
    //if (logGetValue) logtxt.append("*********************************\n");
    //if (logGetValue) logtxt += "Date Request  " + Origin.addSecs(t).toString("dd MMM yyyy hh:mm:ss    "); // + QString("   Time Index = %2").arg(Origin.secsTo(T)) + "\n";
	double v = logisdom::NA;
    qint64 index = getIndex(t, searchAround, minDif);
	if ((index >= 0) && (index < Data_Y.count())) v = Data_Y.at(index);
	if (logGetValue)
	{
		QFile file(romID + "_getValue_t_Log.txt");
		file.open(QIODevice::Append | QIODevice::Text);
		QTextStream out(&file);
		out << logtxt;
		file.close();
	}
	return v;
}




double dataloader::getValue(const QDateTime &T, int minDif)
{
	logtxt.clear();
	if (logGetValue) logtxt.append("*********************************\n");
	if (logGetValue) logtxt += "Date Request  " + T.toString("dd MMM yyyy hh:mm:ss    "); // + QString("   Time Index = %2").arg(Origin.secsTo(T)) + "\n";
	double v = logisdom::NA;
#if QT_VERSION > 0x050603
    qint64 index = getIndex(T.toSecsSinceEpoch(), searchAround, minDif);
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    qint64 index = getIndex(origin.secsTo(T), searchAround, minDif);
#endif
	if ((index >= 0) && (index < Data_Y.count())) v = Data_Y.at(index);
	if (logGetValue)
	{
		QFile file(romID + "_getValue_QDateTime_Log.txt");
		file.open(QIODevice::Append | QIODevice::Text);
		QTextStream out(&file);
		out << logtxt;
		file.close();
	}
	return v;
}





qint64 dataloader::getNextIndex(const QDateTime &T)
{
#if QT_VERSION > 0x050603
    qint64 t = T.toSecsSinceEpoch();
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    qint64 t = origin.secsTo(T);
#endif
    //if (logGetValue) logtxt += "Get next index : " + Origin.addSecs(t).toString("dd MMM yyyy hh:mm:ss") + "    ";
	QMutexLocker locker(&data_Access);
    qint64 index = 0;
    qint64 left = 0;
    qint64 right = Data_Time.count() - 1;
	if (Data_Time.count() == 0)
	{
		if (logGetValue) logtxt += "Device has no data\n";
		return -1;
	}
	if (t <= Data_Time.first())
	{
		    if (logGetValue) logtxt += "Return first data index = 0\n";
		    return Data_Time.at(0);
	}
	if (t >= Data_Time.last())
	{
		    if (logGetValue) logtxt += "Search after, but data is only before";
		    return -1;
	}
	while ((right - left) > 1)
	{
		index = (left + right) / 2;
		if (Data_Time.at(index) == t)
		{
            //if (logGetValue) logtxt += QString("Found exact data Data_Y = %1  ").arg(Data_Y.at(index)) + Origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
			return Data_Time.at(index);
		}
		if (Data_Time.at(index) > t) right = index;
		else left = index;
	}
    //if (logGetValue) logtxt += QString("Found next data Data_Y = %1  ").arg(Data_Y.at(index)) + Origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
	return Data_Time.at(right);
}





qint64 dataloader::getIndex(qint64 t, int searchMode, int minDif)
{
    //if (logGetValue) logtxt += "Get index : " + Origin.addSecs(t).toString("dd MMM yyyy hh:mm:ss") + "    ";
	QMutexLocker locker(&data_Access);
    qint64 index = 0;
    qint64 left = 0;
    qint64 right = Data_Time.count();
	if (Data_Time.count() == 0)
	{
		if (logGetValue) logtxt += "Device has no data\n";
		return -1;
	}
	if (t <= Data_Time.first())
	{
		if (searchMode == searchBefore)
		{
		    if (logGetValue) logtxt += "Search before, but data is only after\n";
		    return -1;
		}
		else
		{
		    if (logGetValue) logtxt += "Return first data index = 0\n";
		    return 0;
		}
	}
	if (t >= Data_Time.last())
	{
		if (searchMode == searchAfter)
		{
		    if (logGetValue) logtxt += "Search after, but data is only before";
		    return -1;
		}
		else
		{
		    if (logGetValue) logtxt += QString("Return last data index = %1\n").arg(Data_Time.count() - 1);
		    return (Data_Time.count() - 1);
		}
	}
	while ((right - left) > 1)
	{
		index = (left + right) / 2;
		if (Data_Time.at(index) == t)
		{
            //if (logGetValue) logtxt += QString("Found exact data Data_Y = %1  ").arg(Data_Y.at(index)) + Origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
			return index;
		}
		if (Data_Time.at(index) > t) right = index;
		else left = index;
	}
    QList <qint64> search;
	bool plus, minus;
	plus = false;
	minus = false;
    qint64 cursor = index - 10;
	if (cursor < 0) cursor = 0;
	while ((cursor < (index + 10)) && (cursor < Data_Time.count()))
	{
        qint64 dif = Data_Time.at(cursor) - t;
		if (dif > 0) plus = true;
		if (dif < 0) minus = true;
		search.append(dif);
		cursor ++;
	}
	cursor = index - 10;
	if (cursor < 0) cursor = 0;
    qint64 lastDif = Data_Time.at(cursor) - t;
	if (plus && minus)
	{
        qint64 minIndex = 0, minValue = -1;
		for (int n=0; n<search.count(); n++)
		{
            qint64 dif = Data_Time.at(cursor + n) - t;
            qint64 difAbs = qAbs(Data_Time.at(cursor + n) - t);
			if ((minValue < 0) or (difAbs < minValue))
			{
				minValue = difAbs;
				minIndex = cursor + n;
			}
			if ((dif > 0) && (lastDif < 0))
			{
				if (searchMode == searchBefore)
				{
					index = cursor + n - 1;
					if (logGetValue)
					{
#if QT_VERSION > 0x050603
                        logtxt += QString("Search Mode Before Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#else
                        QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
                        logtxt += QString("Search Mode Before Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + origin.addSecs(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + origin.addSecs(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#endif
					}
					return index;
				}
				else if (searchMode == searchAfter)
				{
					index = cursor + n;
					if (logGetValue)
					{
#if QT_VERSION > 0x050603
                        logtxt += QString("Search Mode After Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#else
                        QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
                        logtxt += QString("Search Mode After Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + origin.addSecs(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                        if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + origin.addSecs(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#endif
					}
					return index;
				}
			}
			lastDif = dif;
		}
		index = minIndex;
		if ((minValue <= minDif) || (minDif == 0))
		{
			if (logGetValue)
			{
#if QT_VERSION > 0x050603
                logtxt += QString("Search Mode Around Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#else
                QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
                logtxt += QString("Search Mode Around Found closest Index = %1 Data_Y = %2  ").arg(index).arg(Data_Y.at(index)) + origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                if (index > 0) logtxt += QString("Previous One Index = %1 Data_Y = %2  ").arg(index - 1).arg(Data_Y.at(index - 1)) + origin.addSecs(Data_Time.at(index - 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
                if (index < (Data_Y.count() - 1)) logtxt += QString("Next One Index = %1 Data_Y = %2  ").arg(index + 1).arg(Data_Y.at(index + 1)) + origin.addSecs(Data_Time.at(index + 1)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#endif
			}
			return index;
		}
		else
		{
#if QT_VERSION > 0x050603
            if (logGetValue) logtxt += QString("Found data was too far Data_Y = %1  ").arg(Data_Y.at(index)) + QDateTime::fromSecsSinceEpoch(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#else
                QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
                if (logGetValue) logtxt += QString("Found data was too far Data_Y = %1  ").arg(Data_Y.at(index)) + origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
#endif
			return -1;
		}
	}
	if (logGetValue) logtxt += "Did not Find closest Data_Y\n";
	return -1;
}





bool dataloader::loadData(int month, int year)
{
	QTime t;
	t.start();
	bool dataLoaded = false;
	QDateTime T;
	T.setDate(QDate(year, month, 1));
	T.setTime(QTime(0, 0));
#if QT_VERSION > 0x050603
    qint64 offset = T.toSecsSinceEpoch();
#else
    QDateTime origin(QDate(1970, 1, 1), QTime(0, 0, 0));
    qint64 offset = origin.secsTo(T);
#endif
	QByteArray cmpdata;
	QString filename = logisdom::filenameformat(romID, month, year);
	if (logLoadData) logtxt += "filename = " + filename + "\n";
	QFile file(parent->getrepertoiredat() + filename + dat_ext);
	if (logLoadData) logtxt += "file = " + file.fileName() + "\n";
	QFile zatfile(parent->getrepertoirezip() + filename + compdat_ext);
	//if (logLoadData) logtxt += "zatfile = " + zatfile.fileName() + "\n";
	QString zipFileName = parent->getrepertoirezip() + romID + "_" + QString("%1").arg(year) + ".zip";
	if (logLoadData) logtxt += "zipfilename = " + zipFileName + "\n";
	if (file.exists())
	{
		if (file.open(QIODevice::ReadOnly))
		{
			QTextStream in;
			in.setDevice(&file);
			extractdata(in, offset);
			file.close();
			dataLoaded = true;
			if (logLoadData) logtxt += "file " + file.fileName() + " OK\n";
		}
	}
	else if (zatfile.exists())
	{
		if (zatfile.open(QIODevice::ReadOnly))
		{
			QByteArray compresseddata = zatfile.readAll();
			cmpdata = qUncompress(compresseddata);
			QBuffer data;
			data.open(QIODevice::ReadWrite);
			data.write(cmpdata);
			data.reset();
			QTextStream in;
			in.setDevice(&data);
			extractdata(in, offset);
			zatfile.close();
			dataLoaded = true;
		}
	}
	else
	{
		QuaZip zip(zipFileName);
		if(zip.open(QuaZip::mdUnzip))
		{
            QTextCodec *codec = zip.getFileNameCodec();
            zip.setFileNameCodec(codec);
            //zip.setFileNameCodec("IBM866");
			QuaZipFileInfo info;
			QuaZipFile zipFile(&zip);
			QString name;
			for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
			{
				if (zip.getCurrentFileInfo(&info))
				{
					if(zipFile.getZipError() == UNZ_OK)
					{
						name = zipFile.getActualFileName();
						if (name == filename + dat_ext)
						{
							if(zipFile.open(QIODevice::ReadOnly))
							{
								QBuffer data;
								data.open(QIODevice::ReadWrite);
								data.write(zipFile.readAll());
								data.reset();
								QTextStream in;
								//in.setCodec(QTextCodec::codecForLocale());
								in.setDevice(&data);
								extractdata(in, offset);
								dataLoaded = true;
								zipFile.close();
							}
							else logtxt += ("Zip file open error : " + name);
						}
					}
				}
			}
			zip.close();
			if(zip.getZipError() != UNZ_OK) logtxt += ("Zip file close error : " + zipFileName);
		}
		else if (RemoteConnection)
		{
			QString filename = logisdom::filenameformat(romID, month, year);
			if (!getFileList.contains(filename))
			{
                RemoteConnection->addGetDatFiletoFifo(filename);
				getFileList.append(filename);
				logtxt += QString("File not loaded, added to download list ... %2/%3").arg(month).arg(year);
				return false;
			}
		}
	}
	if (!dataLoaded)
	{
		logtxt += QString("File not loaded !!!!  %1 ms  %2/%3").arg(t.elapsed()).arg(month).arg(year);
		return false;
	}
	logtxt += "Device " + romID + QString("  Load Month Data done in %1 ms %2/%3\n").arg(t.elapsed()).arg(month).arg(year);
	return true;
}







void dataloader::extractdata(QTextStream &in, qint64 offset_Time)
{
	Extract_Data_Y.clear();
	Extract_Data_Time.clear();
	QString dataRead;
	double lastreadvalue = logisdom::NA;
	if (in.atEnd()) return;
	dataRead = in.readLine();
	int version = 0;
    qint64 Xpoint = 0;
	int dPoint = -1;
	int hPoint = -1;
	int mPoint = -1;
	int sPoint = -1;
	if (logLoadData) logtxt += QString("extractdata Data \n");
	if (dataRead.contains("Version 1")) version = 1;
        else if (dataRead.contains("Version 2")) version = 2;
        while (!in.atEnd())
	{
		QString d, v, t, dataLeft;
		int shift = 0;
		double value;
		dataRead = in.readLine();
		if (logLoadData) logtxt += "line = " + dataRead + "\n";
		bool ok;
		qreal val = dataRead.toDouble(&ok);
		switch (version)
		{
			case 0 :	break;
                        case 1 :
                        case 2 :        if (dataRead.isEmpty()) break;
					shift = 0;
					dataLeft = dataRead.left(2);
					if (dataLeft == QString("°=")) shift = 1;
					else if (dataLeft == QString("¹=")) shift = 1;
					else if (dataLeft == QString("²=")) shift = 2;
					else if (dataLeft == QString("³=")) shift = 3;
					if (shift)
					{
                        if (logisdom::isNotNA(lastreadvalue))
						{
							Xpoint += shift * 60;
                            if ((check85 && (logisdom::AreNotSame(lastreadvalue, 85))) or (!check85)) addData((qreal)lastreadvalue, Xpoint + offset_Time);
						}
					}
					else if (dataRead.right(2) == "]=")
					{// [59:00]=
						if (dataRead.left(1) == "[")
						{
							if ((dPoint >= 0) && (hPoint >= 0))
							{
								bool okm, oks;
								mPoint = dataRead.mid(1,2).toInt(&okm);
								sPoint = dataRead.mid(4,2).toInt(&oks);
								Xpoint = (dPoint - 1)  * SecsInDays + hPoint * 3600 + mPoint * 60 + sPoint;
                                if ((check85 && (logisdom::AreNotSame(lastreadvalue, 85))) or (!check85)) addData((qreal)lastreadvalue, Xpoint + offset_Time);
							}
						}
					}
					else if (dataRead.left(1) == "=")
					{
                        if (logisdom::isNotNA(lastreadvalue))
						{
							Xpoint += 60;
                            if ((check85 && (logisdom::AreNotSame(lastreadvalue, 85))) or (!check85)) addData((qreal)lastreadvalue, Xpoint + offset_Time);
						}
					}
					else if (ok)
					{
						Xpoint += 60;
						lastreadvalue = val;
                        if ((check85 && (logisdom::AreNotSame(val, 85))) or (!check85)) addData(val, Xpoint + offset_Time);
					}
					if (dataRead.left(1) == "(")   // (01)[12:59:00]'123.456' complete format
					{
						bool ok, okj, okh, okm, oks;
						dPoint = -1;
						hPoint = -1;
						mPoint = -1;
						sPoint = -1;

						int par = dataRead.indexOf("(");
					if (par == -1) break;
						int nextpar = dataRead.indexOf(")", par + 1);
						if (nextpar == -1) break;
						d = dataRead.mid(par + 1, nextpar - par - 1);
//'123.456'
						int coma = dataRead.indexOf("'");
						if (coma == -1) break;
						int nextcoma = dataRead.indexOf("'", coma + 1);
						if (nextcoma == -1) break;
						v = dataRead.mid(coma + 1, nextcoma - coma - 1);
						value = v.toDouble(&ok);
//[12:59:00]
						int bracket = dataRead.indexOf("[");
						if (bracket == -1) break;
						int nextbracket = dataRead.indexOf("]");
						if (nextbracket == -1) break;
						t = dataRead.mid(bracket + 1, nextbracket - bracket - 1);
						int L = t.length();
						if (L == 5)	// [12:59]
						{
							dPoint = d.toInt(&okj);
							hPoint = t.mid(0, 2).toInt(&okh);
							mPoint = t.mid(3,2).toInt(&okm);
							Xpoint = (dPoint - 1)  * SecsInDays + hPoint * 3600 + mPoint * 60;
							if (ok && okj && okh && okm && (Xpoint  >= 0))
							{	// add to newData array
                                if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85))
								{
									addData((qreal)value, Xpoint + offset_Time);
									lastreadvalue = value;
								}
							}
						}
						else if (L == 8)	// [12:59:00]
						{
							dPoint = d.toInt(&okj);
							hPoint = t.mid(0, 2).toInt(&okh);
							mPoint = t.mid(3, 2).toInt(&okm);
							sPoint = t.mid(6, 2).toInt(&oks);
							if (ok && okj && okh && okm && oks)
							{
								Xpoint = (dPoint - 1)  * SecsInDays + hPoint * 3600 + mPoint * 60 + sPoint;
								if (Xpoint  >= 0)
								{	// add to newData array
                                    if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85))
									{
										addData((qreal)value, Xpoint + offset_Time);
										lastreadvalue = value;
									}
								}
							}
						}
					}
					else if (dataRead.left(1) == "[")   // [59:00]'123.456' reduced format
					{
						if (dataRead.mid(6, 1) == "]")
						{
							//[12:59:00]'132.123'
							if ((dPoint > 0) && (hPoint >= 0))
							{
								bool ok, okm, oks;
								int coma = dataRead.indexOf("'");
								if (coma == -1) break;
								int nextcoma = dataRead.indexOf("'", coma + 1);
								if (nextcoma == -1) break;
								v = dataRead.mid(coma + 1, nextcoma - coma - 1);
								value = v.toDouble(&ok);
								lastreadvalue = value;
								mPoint = dataRead.mid(1,2).toInt(&okm);
									sPoint = dataRead.mid(4,2).toInt(&oks);
									if (ok && okm && oks)
									{
										Xpoint = (dPoint - 1)  * SecsInDays + hPoint * 3600 + mPoint * 60 + sPoint;
                                        if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85)) addData((qreal)value, Xpoint + offset_Time);
									}
							}
						}
					}
					else if (dataRead.left(1) == QString("°"))
					{
						dataRead.remove(QString("°"));
						value = dataRead.toDouble(&ok);
						if (ok)
						{
							lastreadvalue = value;
							Xpoint += 60;
                            if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85)) addData((qreal)value, Xpoint + offset_Time);
						}
					}
					else if (dataRead.left(1) == QString("¹"))
					{
						dataRead.remove(QString("¹"));
						value = dataRead.toDouble(&ok);
						if (ok)
						{
							lastreadvalue = value;
							Xpoint += 60;
                            if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85)) addData((qreal)value, Xpoint + offset_Time);
						}
					}
					else if (dataRead.left(1) == QString("²"))
					{
						dataRead.remove(QString("²"));
						value = dataRead.toDouble(&ok);
						if (ok)
						{
							lastreadvalue = value;
							Xpoint += 120;
                            if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85)) addData((qreal)value, Xpoint + offset_Time);
						}
					}
					else if (dataRead.left(1) == QString("³"))
					{
						dataRead.remove(QString("³"));
						value = dataRead.toDouble(&ok);
						if (ok)
						{
							lastreadvalue = value;
							Xpoint += 180;
                            if ((check85 && (!logisdom::AreSame(value, 85))) or (!check85)) addData((qreal)value, Xpoint + offset_Time);
						}
					}
				break;
			default :
			break;
		}
	}
	QVector <qreal> Temp_Data_Y;
    QVector <qint64> Temp_Data_Time;
	data_Access.lock();

// Keep existing data
	if (logLoadData) logtxt += QString("Keep existing data , Data_Y count = %1\n").arg(Data_Y.count());
	Temp_Data_Y = Data_Y;
	Temp_Data_Time = Data_Time;
	if (logLoadData) logtxt += QString("Temp_Data_Y count = %1\n").arg(Temp_Data_Y.count());

//  Clear Data_Y
	if (logLoadData) logtxt += "Clear Data_Y\n";
	Data_Y.clear();
	Data_Time.clear();
// Add extracted data
	if (logLoadData) logtxt += QString("Extract_Data_Y count = %1\n").arg(Extract_Data_Y.count());
	Data_Y = Extract_Data_Y;
	Data_Time = Extract_Data_Time;
	if (logLoadData) logtxt += QString("Now Add Extract to Data_Y count = %1\n").arg(Extract_Data_Y.count());
// Add existing Data_T
	for (int n=0; n<Temp_Data_Y.count(); n++)
	{
	    Data_Y.append(Temp_Data_Y.at(n));
	    Data_Time.append(Temp_Data_Time.at(n));
	}
	data_Access.unlock();
	if (logLoadData) logtxt += QString("Now Add Existing data Data_Y count = %1\n").arg(Temp_Data_Y.count());
}





void dataloader::addData(qreal Y, qint64 T)
{
	if (logLoadData) logtxt += QString("addData %1 at %2\n").arg(Y).arg(T);
    if (int(Y) == logisdom::NA) return;
    //if (logLoadData) logtxt += "addData = " + Origin.addSecs(T).toString() + "\n";
	if (Extract_Data_Time.count() == 0)
	{
		Extract_Data_Y.append(Y);
		Extract_Data_Time.append(T);
	}
	else
	{
		if (Extract_Data_Time.last() < T)
		{
			Extract_Data_Y.append(Y);
			Extract_Data_Time.append(T);
		}
		else if (Extract_Data_Time.first() > T)
		{
			Extract_Data_Y.prepend(Y);
			Extract_Data_Time.prepend(T);
		}
		else
		{
			if (logGetValue) logtxt += QString("Data not added need to sort Data_Y = %1  Time index = %2  "); //.arg(Data_Y.at(index)).arg(Data_Time.at(index)) + "   " + Origin.addSecs(Data_Time.at(index)).toString("dd MMM yyyy hh:mm:ss") + "\n";
		}
	}
}

