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
#include "pieceslist.h"


PiecesList::PiecesList(QWidget *parent) : QListWidget(parent)
{
    iconPath = repertoireicon;
	setDragEnabled(true);
	setViewMode(QListView::IconMode);
	setIconSize(QSize(50, 50));
	setSpacing(10);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	reload();
}






void PiecesList::reload()
{
    QDir iconDir = QDir(QString(iconPath));
	if (iconDir.exists())
	{
		QFileInfo properties;
		QFile file;
		QStringList fileslist = iconDir.entryList(QDir::Files);
		for (int n = 0; n < fileslist.count(); n ++)
		{
			file.setFileName(fileslist[n]);
			properties.setFile(file);
			if (properties.suffix() == "png") addPiece(fileslist[n]);
		}
	}
}





void PiecesList::addPiece(QString &filename)
{
	QListWidgetItem *pieceItem = new QListWidgetItem(this);
    QPixmap pixmap(QString(iconPath) + QDir::separator() + filename);
	QPoint location(10, 10);
	pieceItem->setIcon(QIcon(pixmap));
	pieceItem->setData(Qt::UserRole, QVariant(pixmap));
	pieceItem->setData(Qt::UserRole+1, location);
	pieceItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	pieceItem->setText(filename);
	pieceItem->setToolTip(filename);
    pieceItem->setWhatsThis(QString(iconPath) + QDir::separator() + filename);
}





void PiecesList::addPiece(QPixmap &pixmap, QPoint &location, QString &filename, QString &name)
{
	QListWidgetItem *pieceItem = new QListWidgetItem(this);
	pieceItem->setIcon(QIcon(pixmap));
	pieceItem->setData(Qt::UserRole, QVariant(pixmap));
	pieceItem->setData(Qt::UserRole+1, location);
	pieceItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	pieceItem->setText(name);
	pieceItem->setToolTip(name);
	pieceItem->setWhatsThis(filename);
}






void PiecesList::startDrag(Qt::DropActions)
{
	QListWidgetItem *item = currentItem();

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QPixmap pixmap = item->data(Qt::UserRole).value<QPixmap>();
	QPoint location = item->data(Qt::UserRole+1).toPoint();
	QString name = item->text();
	QString filename = item->whatsThis();

	dataStream << pixmap << location << name << filename;

	QMimeData *mimeData = new QMimeData;
	mimeData->setData(DragIcons, itemData);

	QDrag *drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setHotSpot(location + QPoint(pixmap.width()/2, pixmap.height()/2));
	drag->setPixmap(pixmap);

#if QT_VERSION < 0x060000
    drag->start(Qt::CopyAction);
#else
    drag->exec(Qt::CopyAction);
#endif

}
