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
#include "messagebox.h"



messageBox::messageBox(QWidget *parent) : QMessageBox(parent)
{
}




messageBox::~messageBox()
{
}




QMessageBox::StandardButton messageBox::questionHide(QWidget*, const QString &title, const QString &text, logisdom *parent, QFlags<QMessageBox::StandardButton> buttons)
{
	StandardButton button;
	if (!parent) return question(parent, title, text, buttons);
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	button = question(parent, title, text, buttons);
	parent->PaletteHide(hidden);
	return button;
}




QMessageBox::StandardButton messageBox::criticalHide(QWidget*, const QString &title, const QString &text, logisdom *parent)
{
	StandardButton button;
	if (!parent) return critical(parent, title, text);
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
	button = critical(parent, title, text);
	parent->PaletteHide(hidden);
	return button;
}




QMessageBox::StandardButton messageBox::warningHide(QWidget*, const QString &title, const QString &text, logisdom *parent, QFlags<QMessageBox::StandardButton>)
{
	StandardButton button;
	if (!parent) return critical(parent, title, text);
	bool hidden = parent->isPaletteHidden();
	parent->PaletteHide(true);
    //button = critical(parent, title, text);
    button = warning(parent, title, text);
    parent->PaletteHide(hidden);
	return button;
}





void messageBox::aboutHide(QWidget*, const QString &title, const QString &text, logisdom *parent)
{
	if (!parent)
	{
		about(parent, title, text);
	}
	else
	{
		bool hidden = parent->isPaletteHidden();
		parent->PaletteHide(true);
		about(parent, title, text);
		parent->PaletteHide(hidden);
	}
}

