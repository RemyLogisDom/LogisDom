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



#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H
#include <QtWidgets/QInputDialog>
#include <QtCore>
#include <QtGui>


class logisdom;


class inputDialog : public QInputDialog
{
	Q_OBJECT
public:
	inputDialog(QWidget *parent, Qt::WindowFlags flag);
	~inputDialog();
    static QString getTextPalette(QWidget *widget, const QString &title, const QString &label, QLineEdit::EchoMode mode = QLineEdit::Normal, const QString &text = QString(), bool *ok = 0, logisdom *parent = nullptr);
    static QString getItemPalette(QWidget *widget, const QString &title, const QString &label, const QStringList &items, int current = 0, bool editable = true, bool *ok = 0, logisdom *parent = nullptr);
    static int getIntegerPalette(QWidget *widget, const QString &title, const QString &label, int value = 0, int min = -2147483647, int max = 2147483647, int step = 1, bool *ok = 0, logisdom *parent = nullptr);
};

#endif

