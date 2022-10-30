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





#include "logisdom.h"
#include "inputdialog.h"


inputDialog::inputDialog(QWidget *parent, Qt::WindowFlags flag) : QInputDialog(parent, flag)
{
}




inputDialog::~inputDialog()
{
}



QString inputDialog::getTextPalette(QWidget *widget, const QString &title, const QString &label, QLineEdit::EchoMode mode, const QString &text, bool *ok, logisdom *parent)
{
	QString str;
	if (!parent) return str;
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	str = getText(widget, title, label, mode, text, ok);
	parent->PaletteHide(hidden);
	return str;
}






QString inputDialog::getItemPalette(QWidget *widget, const QString &title, const QString &label, const QStringList &items, int current, bool editable, bool *ok, Qt::WindowFlags flags, logisdom *parent)
{
	QString str;
	if (!parent) return str;
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	str = getItem(widget, title, label, items, current, editable, ok, flags);
	parent->PaletteHide(hidden);
	return str;
}





int inputDialog::getIntegerPalette(QWidget *widget, const QString &title, const QString &label, int value, int min, int max, int step, bool *ok, Qt::WindowFlags flags, logisdom *parent)
{
	int v = logisdom::NA;
	if (!parent) return v;
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	v = getInt(widget, title, label, value, min, max, step, ok, flags);
	parent->PaletteHide(hidden);
	return v;
}

