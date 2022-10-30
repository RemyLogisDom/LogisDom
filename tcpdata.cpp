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
#include "tcpdata.h"



tcpData::tcpData()
{
// palette setup icon
//	setLayout(&setupLayout);
//	setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
//	setupLayout.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	Header = "";
	complete = false;
}





tcpData::~tcpData()
{
}




void tcpData::clear()
{
	Data.clear();
	Header.clear();
	complete = false;
}





bool tcpData::isEmpty()
{
	int start = Data.indexOf(headerStart);
	int end = Data.indexOf(headerEnd);
	if ((start != -1) && (end != -1)) return false; else return true;
}



void tcpData::append(QByteArray &data)
{
	bool ok;
	if (complete) return;
	int lengthTag = QString(headerEnd).length();
	Data.append(data);
	int end = 0;
	if (Header.isEmpty())
	{
// Check is header is complete
		int start = Data.indexOf(headerStart);
		end = Data.indexOf(headerEnd);
		if ((start != -1) && (end != -1)) Header = Data.mid(start, end - start + lengthTag); else Header.clear();
		//	GenMsg("Header found" + Header);
	}
	if (!Header.isEmpty())
	{
		int dSize = logisdom::getvalue(DataSize, Header).toInt(&ok);
		if (ok)
		{
			int dataPos = end + lengthTag + 1;
			int lengthData = Data.size() - dataPos;
			if ((lengthData >= dSize) or (dSize == 0))
			{
				// Check according size if data is received completely
				complete = true;
//				if (lengthData > dSize) Data.chop(lengthData - dSize);
			//	GenMsg(QString("Size OK %1").arg(dSize));
			}
		}
	}
}






bool tcpData::isComplete()
{
	return complete;
}




void tcpData::getData(QByteArray &result)
{
	bool ok;
//	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	result.clear();
	QString Compressed = logisdom::getvalue(CompressedData, Header);
	int dSize = logisdom::getvalue(DataSize, Header).toInt(&ok);
	int dataPos = Data.indexOf(headerEnd) + QString(headerEnd).length() + 1;
	if (complete && ok)
	{
		QByteArray targetData = Data.remove(0, dataPos).left(dSize);
//		if (Compressed == "1")  result = codec->fromUnicode(qUncompress(targetData));
//			else result = codec->fromUnicode(targetData);
		if (Compressed == "1")  result = qUncompress(targetData);
					else result = targetData;
		Data.remove(0, dSize);
		Data.clear();
		Header.clear();
		complete = false;
	}
}



