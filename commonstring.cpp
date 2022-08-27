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
#include "globalvar.h"
#include "commonstring.h"

cstr::cstr()
{
}



QString cstr::toStr(int ID)
{
	QString Str = "";
	switch (ID)
	{
// CStr->toStr(cstr::MainName)
		case MainName : Str = "LogisDom"; break;
// CStr->toStr(cstr::Yes)
		case Yes : Str = tr("&Yes"); break;
// CStr->toStr(cstr::No)
		case No : Str = tr("&No"); break;
// CStr->toStr(cstr::AlreadyExixts)
		case AlreadyExixts : Str = tr("This name already exists\nPlease choose a different one"); break;
// CStr->toStr(cstr::NoName)
		case NoName : Str = tr("No Name"); break;
// CStr->toStr(cstr::Name)
		case Name : Str = tr("Name"); break;
// CStr->toStr(cstr::Name2dot)
		case Name2dot : Str = tr("Name :"); break;
// CStr->toStr(cstr::NewName)
		case NewName : Str = tr("New Name"); break;
// CStr->toStr(cstr::NotValid)
		case NotValid : Str = tr("Not Valid"); break;
// CStr->toStr(cstr::Disabled)
		case Disabled : Str = tr("Disabled"); break;
// CStr->toStr(cstr::Add)
		case Add : Str = tr("Add"); break;
// CStr->toStr(cstr::Remove)
		case Remove : Str = tr("Remove"); break;
// CStr->toStr(cstr::PassWord)
		case PassWord : Str = tr("Password"); break;
// CStr->toStr(cstr::CfrmPassWord)
		case CfrmPassWord : Str = tr("Confirm password"); break;
// CStr->toStr(cstr::Rename)
		case Rename : Str = tr("Rename"); break;
// CStr->toStr(cstr::Lock)
		case Lock : Str = tr("Lock"); break;
// CStr->toStr(cstr::Unlock)
		case UnLock : Str = tr("Unlock"); break;
// CStr->toStr(cstr::Sleep)
		case Sleep : Str = tr("Sleep"); break;
// CStr->toStr(cstr::Cancel)
		case Cancel : Str = tr("Cancel"); break;
// CStr->toStr(cstr::NA)
		case NA : Str = tr("N/A"); break;
// CStr->toStr(cstr::ON)
		case ON : Str = tr("ON"); break;
// CStr->toStr(cstr::OFF)
		case OFF : Str = tr("OFF"); break;
// CStr->toStr(cstr::Auto)
		case Auto : Str = tr("Auto"); break;
// CStr->toStr(cstr::Lighting)
		case Lighting : Str = tr("Lighting"); break;
// CStr->toStr(cstr::Dark)
		case Dark : Str = tr("Dark"); break;
// CStr->toStr(cstr::Dim)
		case Dim : Str = tr("Dim"); break;
// CStr->toStr(cstr::Brigth)
		case Brigth : Str = tr("Brigth"); break;
// CStr->toStr(cstr::Confort)
		case Confort : Str = tr("Confort"); break;
// CStr->toStr(cstr::Nuit)
        case Nuit : Str = tr("Nuit"); break;
// CStr->toStr(cstr::Eco)
		case Eco : Str = tr("Eco"); break;
// CStr->toStr(cstr::Manuel)
        case Manuel : Str = tr("Manual"); break;
// CStr->toStr(cstr::HorsGel)
        case HorsGel : Str = tr("No frost"); break;
// CStr->toStr(cstr::Open)
        case Open : Str = tr("Open"); break;
// CStr->toStr(cstr::Close)
        case Close : Str = tr("Close"); break;
    }
	return Str;
}


