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



#ifndef COMMONSTRING_H
#define COMMONSTRING_H

#include <QtWidgets/QDialog>
#include <QtGui>

class cstr : public QDialog
{
	Q_OBJECT
	Q_ENUMS(CommonString)
public:

enum CommonString
	{
		MainName, Yes, No, AlreadyExixts, NoName, Name, NewName, Name2dot, NotValid, Disabled,
		Add, Remove, PassWord, CfrmPassWord, Rename, Lock, UnLock, Sleep, Cancel, NA, ON, OFF, Auto, Lighting, Dark,
        Dim, Brigth, Confort, Nuit, Eco, Manuel, HorsGel, Open, Close, LastCommonString
	};
	cstr();
	static QString toStr(int ID);
};


#endif
