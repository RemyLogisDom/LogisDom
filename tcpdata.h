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



#ifndef TCPDATA_H
#define TCPDATA_H
#include <QtCore>
#include <QtWidgets/QGridLayout>
#include <QtGui>

class tcpData
{
#define headerStart "{HEADER_START}"
#define headerEnd "{HEADER_END}"
#define DataTypeStr "DataType"
#define DataSize "DataSize"
#define DataCheckSum "DataCheckSum"
#define RequestStr "Request"
#define LogginTypeStr "Loggin"
#define RigthsTypeStr "Rigths"
#define RigthsLimitedStr "Limited"
#define RigthsAdminStr "Admin"
#define DataFileTypeStr "File"
#define DataZipFileTypeStr "ZipFile"
#define DataDeviceTypeStr "Device"
#define DataBinderTypeStr "Binder"
#define FileNameStr "FileName"
#define FolderNameStr "FolderName"
#define CompressedData "DataCompressed"
public:	
	tcpData();
	~tcpData();
	QGridLayout setupLayout;
	void append(QByteArray &data);
	bool isComplete();
	void clear();
	bool complete;
	bool isEmpty();
	QByteArray Data;
	QByteArray Header;
	void getData(QByteArray &result);
private slots:
private:
signals:
};

#endif
