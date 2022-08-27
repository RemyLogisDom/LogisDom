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





#include "addicons.h"
#include <QDir>
#include <QFileDialog>


addicons::addicons(QWidget*, const char*)
{
	setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
	ui.setupUi(this);
    piecesList = new PiecesList;
    ui.gridLayout->addWidget(piecesList);
    QDir iconDir(piecesList->iconPath);
    if (iconDir.exists())
        ui.lineEditPath->setText(iconDir.absolutePath());
    ui.pushButtonChange->hide();
}


void addicons::on_pushButtonChange_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), piecesList->iconPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui.lineEditPath->setText(dir);
    piecesList->iconPath = dir;
    piecesList->clear();
    piecesList->reload();
}

void addicons::on_pushButtonUpdate_clicked()
{
    piecesList->clear();
    piecesList->reload();
}
