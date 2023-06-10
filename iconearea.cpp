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




#include <QtWidgets/QFormLayout>
#include "globalvar.h"
#include "iconf.h"
#include "icont.h"
#include "configwindow.h"
#include "onewire.h"
#include "logisdom.h"
#include "iconearea.h"
#include "devfinder.h"
#include "inputdialog.h"
#include "messagebox.h"
#include "htmlbinder.h"



IconeArea::IconeArea(logisdom *Parent)
{
    qRegisterMetaType<onewiredevice*>();
    parent = Parent;
    resize_Icon = -1;
    mouseWheelDelta = 0;
    rubberBandActive = false;
    rubberband = nullptr;
    childClicked = nullptr;
    lastClicked = nullptr;
    captureUndo = false;
    htmlEnabled = false;
    undoIndex = -1;
    setMinimumSize(MainMinXSize, MainMinYSize);
    setAcceptDrops(true);
    locked = false;
    connect(parent->configwin, SIGNAL(DeviceChanged(onewiredevice *)), this, SLOT(setValue(onewiredevice *)), Qt::QueuedConnection);
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);
    backgroundColor = QWidget::palette().color(QWidget::backgroundRole());
}



IconeArea::~IconeArea()
{
}


QString IconeArea::colorToHex(QColor &color)
{
    QString hex;
    hex += QString("%1").arg(uchar(color.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.blue()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(color.alpha()), 2, 16, QChar('0'));
    return hex.toUpper();
}



bool IconeArea::hexToColor(QString &hex, QColor &color)
{
    if (hex.length() != 8) return false;
    bool ok;
    int r = hex.mid(0, 2).toInt(&ok, 16);
    if (!ok) return false;
    int g = hex.mid(2, 2).toInt(&ok, 16);
    if (!ok) return false;
    int b  = hex.mid(4, 2).toInt(&ok, 16);
    if (!ok) return false;
    int a = hex.mid(6, 2).toInt(&ok, 16);
    if (!ok) return false;
    color.setRgba(qRgba(r, g, b, a));
    return true;
}



QString IconeArea::getBackGroundColor()
{
    QString hex;
    hex += QString("%1").arg(uchar(backgroundColor.red()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(backgroundColor.green()), 2, 16, QChar('0'));
    hex += QString("%1").arg(uchar(backgroundColor.blue()), 2, 16, QChar('0'));
    return hex.toUpper();
}


void IconeArea::setBackGroundColor(QColor color)
{
    backgroundColor = color;
    QPalette pal = palette();
#if QT_VERSION < 0x060000
    pal.setColor(QPalette::Background, backgroundColor);
#else
    pal.setColor(QPalette::Base, backgroundColor);
#endif
    setAutoFillBackground(true);
    setPalette(pal);
}





void IconeArea::setValue(onewiredevice *device)
{
    if (!device) return;
	QString RomID;
    RomID = device->getromid();
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->romid == RomID) IconList.at(index)->setvalue(device);
	}
}



void IconeArea::AddTextZone()
{
    icont *text = newText(this);
    QPoint TextPos = QPoint(50, 50);
    text->settxt("New Text", TextPos);
}



void IconeArea::setValue()
{
    for (int index=0; index<IconList.count(); index++)
    {
        onewiredevice *device = parent->configwin->DeviceExist(IconList.at(index)->romid);
        if (IconList.at(index)->highlighted or IconList.at(index)->Thighlighted or IconList.at(index)->Vhighlighted) return;
        if (device) IconList.at(index)->setvalue(device); else IconList.at(index)->setvalue();
    }
}



void IconeArea::setLockedState(bool state)
{
	locked = state;
    if (state)
    {
        for (int index=0; index<IconList.count(); index++)
        {
            IconList.at(index)->setHighlighted(false);
            IconList.at(index)->setTHighlighted(false);
            IconList.at(index)->setVHighlighted(false);
            mouseWheelDelta = 0;
        }
    }
}



void IconeArea::appendconfigfile(QString &configdata)
{
    QString ReadRomID, ReadName;
    QString TAG_Begin = Icon_Begin;
    QString TAG_End = Icon_Finished;
    SearchLoopBegin
    ReadName = logisdom::getvalue("IconFileName", strsearch);
    if (ReadName != "")
    {
        iconf *icon = newIcon(this);
        icon->setConfig(strsearch);
        icon->setHighlighted(true);
        icon->setTHighlighted(true);
        icon->setVHighlighted(true);
    }
    SearchLoopEnd
    PushUndo();
}



void IconeArea::readconfigfile(const QString &configdata)
{
    QColor color;
    QString hexColor = logisdom::getvalue("Background_Color", configdata);
    if (!hexColor.isEmpty())
        if (hexToColor(hexColor, color))
        {
            backgroundColor = color;
            QPalette pal = palette();
#if QT_VERSION < 0x060000
            pal.setColor(QPalette::Background, backgroundColor);
#else
            pal.setColor(QPalette::Base, backgroundColor);
#endif
            setAutoFillBackground(true);
            setPalette(pal);
        }
    QString ReadRomID, ReadName, Text;
    QString TAG_Begin = Icon_Begin;
    QString TAG_End = Icon_Finished;
    QString htmlEn = logisdom::getvalue("HtmlEnabled", configdata);
    if (htmlEn == "1") htmlEnabled = true;
    SearchLoopBegin
    ReadName = logisdom::getvalue("IconFileName", strsearch);
    if (ReadName.isEmpty())
    {   // Text Only
        Text = logisdom::getvalue("Text", strsearch);
        if (!Text.isEmpty())
        {
            icont *text = newText(this);
            text->setConfig(strsearch);
        }
    }
    else
    {
        iconf *icon = newIcon(this);
        icon->setConfig(strsearch);
    }
    SearchLoopEnd
    PushUndo();
}





void  IconeArea::SaveConfigStr(QString &str)
{
    if (htmlEnabled) str += logisdom::saveformat("HtmlEnabled", "1"); else str += logisdom::saveformat("HtmlEnabled", "0");
    for (int index=0; index<IconList.count(); index++)
        IconList.at(index)->getConfigStr(str);
    for (int index=0; index<TextList.count(); index++)
        TextList[index]->getConfigStr(str);
    str += logisdom::saveformat("Background_Color", colorToHex(backgroundColor));
}



void  IconeArea::SaveConfigStrIconPath(QString &str)
{
    if (htmlEnabled) str += logisdom::saveformat("HtmlEnabled", "1"); else str += logisdom::saveformat("HtmlEnabled", "0");
    for (int index=0; index<IconList.count(); index++)
        IconList.at(index)->getConfigStrIconPath(str);
    for (int index=0; index<TextList.count(); index++)
        TextList[index]->getConfigStr(str);
    str += logisdom::saveformat("Background_Color", colorToHex(backgroundColor));
}


void  IconeArea::SaveSelectionConfigStr(QString &str)
{
    for (int index=0; index<IconList.count(); index++)
        if ((IconList.at(index)->highlighted) or (IconList.at(index)->Thighlighted) or (IconList.at(index)->Vhighlighted))
            IconList.at(index)->getConfigStrIconPath(str);
    for (int index=0; index<TextList.count(); index++)
        if (TextList.at(index)->highlighted)
            TextList[index]->getConfigStr(str);
}



void IconeArea::setAsBackground(iconf *icon)
{
	int index = IconList.indexOf(icon);
	if (index != -1) IconList.move(index, 0);
}



void IconeArea::newFont(QString font)
{
    if (locked) return;
    for (int i=0; i<IconList.count(); i++)
    {
        if (IconList.at(i)->highlighted)
        {
            disconnect(IconList.at(i), SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
            IconList.at(i)->setFont(font);
            connect(IconList.at(i), SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
        }
    }
    for (int i=0; i<TextList.count(); i++)
    {
        if (TextList.at(i)->highlighted)
        {
            disconnect(TextList.at(i), SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
            TextList.at(i)->setFont(font);
            connect(TextList.at(i), SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
        }
    }
}



void IconeArea::setInFront(iconf *icon)
{
	int index = IconList.indexOf(icon);
	if (index != -1) IconList.move(index, IconList.size() - 1);
}



void IconeArea::setInFront(icont *icon)
{
    int index = TextList.indexOf(icon);
    if (index != -1) TextList.move(index, TextList.size() - 1);
}


void IconeArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (locked) return;
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat(MoveIcons) or mimeData->hasFormat(DragIcons))
	{
        if (event->source() == this)
		{
            event->setDropAction(Qt::MoveAction);
			event->accept();
		}
		else
		{
			event->acceptProposedAction();
        }
	}
    else if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        QString name;
        for (int i = 0; i < urlList.size() and i < 32; ++i)
            name += urlList.at(i).path();
        QFile icon(name);
        if (name.right(4) == ".png")
            if (icon.exists())
            event->acceptProposedAction();
            //qDebug() << "Drag enter " + name;
    }
    else
    {
		event->ignore();
	}
}





void IconeArea::dropEvent(QDropEvent *event)
{
    if (locked) return;
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasFormat(DragIcons))
    {
        QByteArray itemData = event->mimeData()->data(DragIcons);
		QDataStream dataStream(&itemData, QIODevice::ReadOnly);
		QPixmap pixmap;
		QPoint location;
		QString name;
		QString filename;
		dataStream >> pixmap >> location >> name >> filename;
		iconf *icon = newIcon(this);
		QPoint IconPos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2));
		icon->setpict(filename, IconPos);
		QPoint TextPos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(pixmap.width(), pixmap.height()));
		icon->settxt(name, TextPos);
		QPoint ValuePos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(0, pixmap.height()));
		icon->setvalue("", ValuePos);
		event->setDropAction(Qt::CopyAction);
		event->accept();
		PushUndo();
	}
    else if (mimeData->hasFormat(MoveIcons))
    {
    }
    else if (mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        QString name;
        for (int i = 0; i < urlList.size() && i < 32; ++i)
            name += urlList.at(i).path();
        //qDebug() << "Drag drop " + name;
        QFile icon(name);
        if (name.right(4) == ".png")
            if (icon.exists())
            {
                QPixmap pixmap;
                iconf *icon = newIcon(this);
#if QT_VERSION < 0x060000
                QPoint IconPos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2));
                icon->setpict(name, IconPos);
                QPoint TextPos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(pixmap.width(), pixmap.height()));
                icon->settxt(name, TextPos);
                QPoint ValuePos = QPoint(mapFromParent(event->pos()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(0, pixmap.height()));
#else
                QPoint IconPos = QPoint(mapFromParent(event->position().toPoint()) - QPoint(pixmap.width()/2, pixmap.height()/2));
                icon->setpict(name, IconPos);
                QPoint TextPos = QPoint(mapFromParent(event->position().toPoint()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(pixmap.width(), pixmap.height()));
                icon->settxt(name, TextPos);
                QPoint ValuePos = QPoint(mapFromParent(event->position().toPoint()) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(0, pixmap.height()));
#endif
                icon->setvalue("", ValuePos);
                event->setDropAction(Qt::CopyAction);
                event->accept();
                PushUndo();
            }
     }
    else
    {
        event->ignore();
    }
}




void IconeArea::mouseDoubleClickEvent(QMouseEvent *)
{
    if (locked) return;
    int n = 0;
    int index = -1;
    for (int i=0; i<IconList.count(); i++)
    {
        if (IconList.at(i)->highlighted)
        {
            n++;
            index = i;
        }
    }
    if (n == 1)
    {
        IconList.at(index)->setVHighlighted(true);
        IconList.at(index)->setTHighlighted(true);
    }
    else if (n > 1)
    {
        parent->showIconTools();
    }
}



void IconeArea::mouseMoveEvent(QMouseEvent *event)
{
	if (resize_Icon != -1)
	{
		IconList[resize_Icon]->mouseResize(event);
		IconList[resize_Icon]->setHighlighted(true);
		captureUndo = true;
		return;
	}
	if(rubberBandActive)
	{
		rubberband->setGeometry(QRect(startSelectionPoint,event->pos()).normalized());
        endSelectionPoint = event->pos();
        QRect area = QRect(startSelectionPoint, endSelectionPoint);
        setHighlighted(area);
    }
    if (!childClicked)
    {
        mouseWheelDelta = 0;
        return;
    }
#if QT_VERSION < 0x060000
    if (childClicked->pixmap() != nullptr)
#else
    if (!childClicked->pixmap().isNull())
#endif
	{
		// only if clicked on pixmap and not Qlabel
		// define drag size if multiple selection
		int xMin = -1;
		int xMax = -1;
		int yMin = -1;
		int yMax = -1;
		for (int index=0; index<IconList.count(); index++)
        {   // icon select drag size
            if (IconList.at(index)->highlighted)
			{
				int xmin = IconList.at(index)->icon->x();
                int xmax = IconList.at(index)->icon->x() + IconList.at(index)->icon->width();
				int ymin = IconList.at(index)->icon->y();
                int ymax = IconList.at(index)->icon->y() + IconList.at(index)->icon->height();
                if ((xMin == -1) or (xmin < xMin)) xMin = xmin;
                if ((xMax == -1) or (xmax > xMax)) xMax = xmax;
                if ((yMin == -1) or (ymin < yMin)) yMin = ymin;
                if ((yMax == -1) or (ymax > yMax)) yMax = ymax;
            }
            if (IconList.at(index)->Thighlighted && !IconList.at(index)->NoText.isChecked())
            {
                int xmin = IconList.at(index)->text->x();
                int xmax = IconList.at(index)->text->x() + IconList.at(index)->text->width();
                int ymin = IconList.at(index)->text->y();
                int ymax = IconList.at(index)->text->y() + IconList.at(index)->text->height();
                if ((xMin == -1) or (xmin < xMin)) xMin = xmin;
                if ((xMax == -1) or (xmax > xMax)) xMax = xmax;
                if ((yMin == -1) or (ymin < yMin)) yMin = ymin;
                if ((yMax == -1) or (ymax > yMax)) yMax = ymax;
            }
            if (IconList.at(index)->Vhighlighted && !IconList.at(index)->NoValue.isChecked())
            {
                int xmin = IconList.at(index)->value->x();
                int xmax = IconList.at(index)->value->x() + IconList.at(index)->value->width();
                int ymin = IconList.at(index)->value->y();
                int ymax = IconList.at(index)->value->y() + IconList.at(index)->value->height();
                if ((xMin == -1) or (xmin < xMin)) xMin = xmin;
                if ((xMax == -1) or (xmax > xMax)) xMax = xmax;
                if ((yMin == -1) or (ymin < yMin)) yMin = ymin;
                if ((yMax == -1) or (ymax > yMax)) yMax = ymax;
            }
        }
        for (int index=0; index<TextList.count(); index++)
        {   // text selected drag size
            if (TextList.at(index)->highlighted)
            {
                int xmin = TextList.at(index)->text->x();
                int xmax = TextList.at(index)->text->x() + TextList.at(index)->text->width();
                int ymin = TextList.at(index)->text->y();
                int ymax = TextList.at(index)->text->y() + TextList.at(index)->text->height();
                if ((xMin == -1) or (xmin < xMin)) xMin = xmin;
                if ((xMax == -1) or (xmax > xMax)) xMax = xmax;
                if ((yMin == -1) or (ymin < yMin)) yMin = ymin;
                if ((yMax == -1) or (ymax > yMax)) yMax = ymax;
            }
        }
        QByteArray itemData;
		QDataStream dataStream(&itemData, QIODevice::WriteOnly);
		dataStream << QPoint(event->pos() - childClicked->pos());
		QMimeData *mimeData = new QMimeData;
		mimeData->setData(MoveIcons, itemData);
		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
// Create drag pixmap
		QPixmap dragpixmap = QPixmap(xMax - xMin, yMax - yMin);
        dragpixmap.fill(QColor::fromRgb(127, 127, 127));
		QPainter dragpainter;
		dragpainter.begin(&dragpixmap);
		for (int index=0; index<IconList.count(); index++)
        {
			if (IconList.at(index)->highlighted)
			{
#if QT_VERSION < 0x060000
                QPixmap pixmap = *IconList.at(index)->icon->pixmap();
#else
                QPixmap pixmap = IconList.at(index)->icon->pixmap();
#endif
                QPoint drawPoint(IconList.at(index)->icon->x() - xMin, IconList.at(index)->icon->y() - yMin);
                dragpainter.drawPixmap(drawPoint, pixmap);
			}
        }
		dragpainter.fillRect(dragpixmap.rect(), QColor(127, 127, 127, 127));
		dragpainter.end();
		drag->setPixmap(dragpixmap);
		drag->setHotSpot(event->pos() - QPoint(xMin, yMin));
		for (int index=0; index<IconList.count(); index++)
        {
			if (IconList.at(index)->highlighted)
			{
#if QT_VERSION < 0x060000
                QPixmap tempPixmap = *IconList.at(index)->icon->pixmap();
#else
                QPixmap tempPixmap = IconList.at(index)->icon->pixmap();
#endif
				QPainter painter;
				painter.begin(&tempPixmap);
				painter.fillRect(tempPixmap.rect(), QColor(127, 127, 127, 127));
				painter.end();
                IconList.at(index)->icon->setPixmap(tempPixmap);
            }
        }
#if QT_VERSION < 0x060000
        drag->start(Qt::MoveAction);
#else
        drag->exec(Qt::MoveAction);
#endif
        QPoint shift = mapFromGlobal(QCursor::pos() - click);
// cancel if item position is outside area
        for (int index=0; index<IconList.count(); index++)
        {
            if (IconList.at(index)->highlighted)
            {
                QPoint IconPos = shift + IconList.at(index)->icon->pos();
                if (IconPos.x() < 0) return;
                if (IconPos.y() < 0) return;
                if ((IconPos.x() + IconList.at(index)->icon->width()) > parent->workspaceX.value()) return;
                if ((IconPos.y() + IconList.at(index)->icon->height()) > parent->workspaceY.value()) return;
            }
            if (IconList.at(index)->Thighlighted)
            {
                QPoint TextPos = shift + IconList.at(index)->text->pos();
                if (TextPos.x() < 0) return;
                if (TextPos.y() < 0) return;
                if ((TextPos.x() + IconList.at(index)->text->width()) > parent->workspaceX.value()) return;
                if ((TextPos.y() + IconList.at(index)->text->height()) > parent->workspaceY.value()) return;
            }
            if (IconList.at(index)->Vhighlighted)
            {
                QPoint ValuePos = shift + IconList.at(index)->value->pos();
                if (ValuePos.x() < 0) return;
                if (ValuePos.y() < 0) return;
                if ((ValuePos.x() + IconList.at(index)->value->width()) > parent->workspaceX.value()) return;
                if ((ValuePos.y() + IconList.at(index)->value->height()) > parent->workspaceY.value()) return;
            }
        }
        for (int index=0; index<TextList.count(); index++)
        {
            if (TextList[index]->highlighted)
            {
                QPoint ValuePos = shift + TextList[index]->text->pos();
                if (ValuePos.x() < 0) return;
                if (ValuePos.y() < 0) return;
                if ((ValuePos.x() + TextList[index]->text->width()) > parent->workspaceX.value()) return;
                if ((ValuePos.y() + TextList[index]->text->height()) > parent->workspaceY.value()) return;
            }
        }
// process move
        for (int index=0; index<IconList.count(); index++)
        {
            QPoint IconPos = shift + IconList.at(index)->icon->pos();
            if (IconList.at(index)->highlighted) { IconList.at(index)->icon->move(IconPos); IconList.at(index)->setHighlighted(true); }
            QPoint TextPos = shift + IconList.at(index)->text->pos();
            if (IconList.at(index)->Thighlighted) { IconList.at(index)->text->move(TextPos); IconList.at(index)->setTHighlighted(true); }
            QPoint ValuePos = shift + IconList.at(index)->value->pos();
            if (IconList.at(index)->Vhighlighted) { IconList.at(index)->value->move(ValuePos); IconList.at(index)->setVHighlighted(true); }
        }
        for (int index=0; index<TextList.count(); index++)
        {
            QPoint TextPos = shift + TextList[index]->text->pos();
            if (TextList[index]->highlighted) { TextList[index]->text->move(TextPos); TextList[index]->setHighlighted(true); }
        }














    }
	else
	{
		QByteArray itemData;
		QDataStream dataStream(&itemData, QIODevice::WriteOnly);
		dataStream << QPoint(event->pos() - childClicked->pos());

		QMimeData *mimeData = new QMimeData;
		mimeData->setData(MoveIcons, itemData);

		QString txt = childClicked->text();
        //QPixmap tempPixmap = QPixmap::grabWidget(childClicked);
        QPixmap tempPixmap = childClicked->grab();
		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->setPixmap(tempPixmap);
		drag->setHotSpot(event->pos() - childClicked->pos());

		//QPainter painter;
		//painter.begin(&tempPixmap);
		//painter.fillRect(tempPixmap.rect(), QColor(127, 127, 127, 127));
		//painter.end();
		//childClicked->setPixmap(tempPixmap);
#if QT_VERSION < 0x060000
        drag->start(Qt::MoveAction);
#else
        drag->exec(Qt::MoveAction);
#endif
		childClicked->move(mapFromGlobal(QCursor::pos()) - (event->pos() - childClicked->pos()));
		childClicked->setText(txt);
	}
	//captureUndo = true;
	PushUndo();
}



void IconeArea::mouseReleaseEvent(QMouseEvent *event)
{
	resize_Icon = -1;
	if(rubberBandActive)
	{
        endSelectionPoint = event->pos();
		rubberBandActive = false;
		rubberband->hide();
		QRect area = QRect(startSelectionPoint, endSelectionPoint);
        setHighlighted(area);
    }

	childClicked = nullptr;
    if (captureUndo) PushUndo();
	captureUndo = false;
}



void IconeArea::setHighlighted(QRect &area)
{
    for (int index=0; index<IconList.count(); index++)
    {
        bool highlight = false;
        bool Thighlight = false;
        bool Vhighlight = false;
        if (area.contains(IconList.at(index)->icon->pos())) highlight = true;
        if (area.contains(IconList.at(index)->text->pos())) Thighlight = true;
        if (area.contains(IconList.at(index)->value->pos())) Vhighlight = true;
        QPoint UR(IconList.at(index)->icon->pos().x() + IconList.at(index)->icon->width(), IconList.at(index)->icon->pos().y());
        QPoint LL(IconList.at(index)->icon->pos().x(), IconList.at(index)->icon->pos().y() + IconList.at(index)->icon->height());
        QPoint LR(IconList.at(index)->icon->pos().x() + IconList.at(index)->icon->width(), IconList.at(index)->icon->pos().y() + IconList.at(index)->icon->height());
        if (area.contains(UR)) highlight = true;
        if (area.contains(LL)) highlight = true;
        if (area.contains(LR)) highlight = true;
        QPoint TUR(IconList.at(index)->text->pos().x() + IconList.at(index)->text->width(), IconList.at(index)->text->pos().y());
        QPoint TLL(IconList.at(index)->text->pos().x(), IconList.at(index)->text->pos().y() + IconList.at(index)->text->height());
        QPoint TLR(IconList.at(index)->text->pos().x() + IconList.at(index)->text->width(), IconList.at(index)->text->pos().y() + IconList.at(index)->text->height());
        if (area.contains(TUR)) Thighlight = true;
        if (area.contains(TLL)) Thighlight = true;
        if (area.contains(TLR)) Thighlight = true;
        QPoint VUR(IconList.at(index)->value->pos().x() + IconList.at(index)->value->width(), IconList.at(index)->value->pos().y());
        QPoint VLL(IconList.at(index)->value->pos().x(), IconList.at(index)->value->pos().y() + IconList.at(index)->value->height());
        QPoint VLR(IconList.at(index)->value->pos().x() + IconList.at(index)->value->width(), IconList.at(index)->value->pos().y() + IconList.at(index)->value->height());
        if (area.contains(VUR)) Vhighlight = true;
        if (area.contains(VLL)) Vhighlight = true;
        if (area.contains(VLR)) Vhighlight = true;
        unsigned int state = QApplication::keyboardModifiers();
        if (highlight) IconList.at(index)->setHighlighted(true);
        else if (!(state & Qt::ControlModifier)) IconList.at(index)->setHighlighted(false);
        if (Thighlight) IconList.at(index)->setTHighlighted(true);
        else if (!(state & Qt::ControlModifier)) IconList.at(index)->setTHighlighted(false);
        if (Vhighlight) IconList.at(index)->setVHighlighted(true);
        else if (!(state & Qt::ControlModifier)) IconList.at(index)->setVHighlighted(false);
    }
    for (int index=0; index<TextList.count(); index++)
    {
        bool highlight = false;
        if (area.contains(TextList.at(index)->text->pos())) highlight = true;
        QPoint TUR(TextList.at(index)->text->pos().x() + TextList.at(index)->text->width(), TextList.at(index)->text->pos().y());
        QPoint TLL(TextList.at(index)->text->pos().x(), TextList.at(index)->text->pos().y() + TextList.at(index)->text->height());
        QPoint TLR(TextList.at(index)->text->pos().x() + TextList.at(index)->text->width(), TextList.at(index)->text->pos().y() + TextList.at(index)->text->height());
        if (area.contains(TUR)) highlight = true;
        if (area.contains(TLL)) highlight = true;
        if (area.contains(TLR)) highlight = true;
        unsigned int state = QApplication::keyboardModifiers();
        if (highlight) TextList.at(index)->setHighlighted(true);
        else if (!(state & Qt::ControlModifier)) TextList.at(index)->setHighlighted(false);
    }
}


bool IconeArea::hasSelection()
{
    for (int index=0; index<IconList.count(); index++)
    {
        if ((IconList.at(index)->highlighted) or (IconList.at(index)->Thighlighted) or (IconList.at(index)->Vhighlighted))  return true;
    }
    return false;
}



void IconeArea::getIconHtml(QTextStream &out, iconf *item, int id)
{
    QString name = item->actualFileName.text();
    if (name.isEmpty()) name = item->path;
    int r = item->iconRotate.value();
    if (r != 0) name.prepend(QString("R=%1.").arg(r));
    out << QString("<div id=\"wb_Image%1\" style=\"position:absolute;left:%2px;top:%3px;width:%4px;height:%5px;z-index:%6;\"><img src=\"").arg(id).arg(item->icon->pos().x()).arg(item->icon->pos().y()).arg(item->icon->width()).arg(item->icon->height()).arg(id)\
    + name + QString("\" id=\"Image%6\" alt=\"\" style=\"width:%7px;height:%8px;\"></div>\n").arg(id).arg(item->icon->width()).arg(item->icon->height());
    //    out << (QString("<input type=\"image\" name=\"ImageName%1\" id=%2 src=\"").arg(id).arg(id) +\
    //    name + QString("\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:1;\">\n")\
    //    .arg(item->icon->pos().x()).arg(item->icon->pos().y()).arg(item->icon->width()).arg(item->icon->height()));
}



void IconeArea::getIconHtmlLink(QTextStream &out, iconf *item, int id)
{
    QString name = item->actualFileName.text();
    if (name.isEmpty()) name = item->path;
    int r = item->iconRotate.value();
    if (r != 0) name.prepend(QString("R=%1.").arg(r));
    out << QString("<div id=\"wb_Image%1\" style=\"position:absolute;left:%2px;top:%3px;width:%4px;height:%5px;z-index:%6;\"><a href=\"").arg(id).arg(item->icon->pos().x()).arg(item->icon->pos().y()).arg(item->icon->width()).arg(item->icon->height()).arg(id)\
    + item->htmlCommand.text() + "\" " + item->htmlCommand.text() + "=\"Titre\"><img src=\"" + name + QString("\" id=\"Image%1\" alt= \"\" style=\"width:%2px;height:%3px;\"></a></div>\n").arg(id).arg(item->icon->width()).arg(item->icon->height());
}



void IconeArea::getIconHtmlCommand(QTextStream &out, iconf *item, int id)
{
// window.location='command=(Evenement|Selection|night)webid=()'
    QString name = item->actualFileName.text();
    if (name.isEmpty()) name = item->path;
    out << (QString("<input type=\"image\" name=\"ImageName%1\" id=%2 src=\"").arg(id).arg(id) +\
    name + QString("\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;\" onclick=\"window.location='command=(" + item->htmlCommand.text() + ")webid=()'; return false;\">\n")\
    .arg(item->icon->pos().x()).arg(item->icon->pos().y()).arg(item->icon->width()).arg(item->icon->height())).arg(id);
}



void IconeArea::getTextHtml(QTextStream &out, iconf *item, int id)
{
    out <<  (QString("<div id=\"wb_Text%1\" style=\"position:absolute;left:%2px;top:%3px;width:%4px;height:%5px;z-index:%6;text-align:left;\">").arg(id).arg(item->text->pos().x()).arg(item->text->pos().y()).arg(item->text->width() + extrapix).arg(item->text->height()).arg(id)\
    + "<span style=\"color:#" + item->TextStyleHex + QString(";font-family:") + item->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(item->textsize.value()+2) + item->text->text() + "</strong></span></div>");
}



void IconeArea::getValueHtml(QTextStream &out, iconf *item, int id)
{
    out <<  (QString("<div id=\"wb_Text%1\" style=\"position:absolute;left:%2px;top:%3px;width:%4px;height:%5px;z-index:%6;text-align:left;\">").arg(id).arg(item->value->pos().x()).arg(item->value->pos().y()).arg(item->value->width() + extrapix).arg(item->value->height()).arg(id)\
    + "<span style=\"color:#" + item->CurentValueStyleleHex + QString(";font-family:") + item->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(item->valuesize.value()+2) + item->value->text() + "</strong></span></div>");
}



// get html from distant computer
void IconeArea::getHtml(QTextStream &out)
{
    for (int index=0; index<IconList.count(); index++)
    {
        iconf *item = IconList.at(index);
// Icon
        if (!item->NoIcon.isChecked())
        {
            QString name = item->actualFileName.text();
            if (name.isEmpty()) name = item->path;
            int r = item->iconRotate.value();
            if (r != 0) name.prepend(QString("R=%1.").arg(r));
            if (item->htmlCommand.text().isEmpty()) getIconHtml(out, item, index);
            else
            {
                if (item->htmlCommand.text().startsWith("http")) getIconHtmlLink(out, item, index);
                else getIconHtmlCommand(out, item, index);
            }
        }
// Text
        if (!item->NoText.isChecked()) getTextHtml(out, item, index);
// Value
        if (!item->NoValue.isChecked())
        {
            if (item->getDisplayMode() == iconf::DeviceMode)
            {
                if (item->htmlCommand.text().isEmpty() or item->htmlCommand.text().startsWith("http")) getValueHtml(out, item, index);
                else
                {
out << (QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
.arg(item->value->pos().x()).arg(item->value->pos().y()).arg(item->value->width() + extrapix).arg(item->value->height()).arg(index) +
"<span style=\"color:#" + item->CurentValueStyleleHex +\
QString(";font-family:") + item->fontName.currentText() + QString(";font-size:%1px;\"><strong>[[").arg(item->valuesize.value()+2)+\
item->htmlCommand.text() + "]]</strong></span></div>");
                }
            }
            else
            {
                QString str = QDateTime::currentDateTime().toString(item->getDisplayModeStr());
                if (!str.isEmpty())
                {
out << (QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
.arg(item->value->pos().x()).arg(item->value->pos().y()).arg(item->value->width() + extrapix).arg(item->value->height()).arg(index) +
"<span style=\"color:#" + item->CurentValueStyleleHex +\
QString(";font-family:") + item->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(item->valuesize.value()+2)+\
str + "</strong></span></div>");
                }
            }
        }
    }
    for (int index=0; index<TextList.count(); index++)
    {
        out <<  (QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
        .arg(TextList.at(index)->text->pos().x()).arg(TextList.at(index)->text->pos().y()).arg(TextList.at(index)->text->width() + extrapix).arg(TextList.at(index)->text->height()).arg(index) +\
        "<span style=\"color:#" + TextList.at(index)->TextStyleHex +\
        QString(";font-family:") + TextList.at(index)->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(TextList.at(index)->textsize.value()+2)+\
        TextList.at(index)->text->text().replace("\n", "<br>") + "</strong></span></div>");
    }
}

// get html for local file generation
void IconeArea::getHtml(QString &html)
{
    for (int index=0; index<IconList.count(); index++)
    {
// Icon
        if (!IconList.at(index)->NoIcon.isChecked())
        {
            if (IconList.at(index)->htmlCommand.text().isEmpty())
            {
html.append(QString("<input type=\"image\" name=\"ImageName%1\" id=%2 src=\"").arg(index).arg(index) +\
IconList.at(index)->path + QString("\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;\">\n")\
.arg(IconList.at(index)->icon->pos().x()).arg(IconList.at(index)->icon->pos().y()).arg(IconList.at(index)->icon->width()).arg(IconList.at(index)->icon->height())).arg(index);
            }
            else
            {
html.append(QString("<input type=\"image\" name=\"ImageName%1\" id=%2 src=\"").arg(index).arg(index) +\
IconList.at(index)->path + QString("\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;\" onclick=\"" + IconList.at(index)->htmlCommand.text() + "\">\n")\
.arg(IconList.at(index)->icon->pos().x()).arg(IconList.at(index)->icon->pos().y()).arg(IconList.at(index)->icon->width()).arg(IconList.at(index)->icon->height())).arg(index);
            }
        }
// Text
        if (!IconList.at(index)->NoText.isChecked())
        {
html.append(QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
.arg(IconList.at(index)->text->pos().x()).arg(IconList.at(index)->text->pos().y()).arg(IconList.at(index)->text->width() + extrapix).arg(IconList.at(index)->text->height()).arg(index) +\
QString("<span style=\"color:#000000;font-family:") + IconList.at(index)->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(IconList.at(index)->textsize.value())+\
IconList.at(index)->text->text() + "</strong></span></div>");
        }
// Valuel.append(QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left
        if (!IconList.at(index)->NoValue.isChecked())
        {
            if (IconList.at(index)->getDisplayMode() == iconf::DeviceMode)
            {
html.append(QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
.arg(IconList.at(index)->value->pos().x()).arg(IconList.at(index)->value->pos().y()).arg(IconList.at(index)->value->width()).arg(IconList.at(index)->value->height()).arg(index) +
QString("<span style=\"color:#000000;font-family:") + IconList.at(index)->fontName.currentText() + QString(";font-size:%1px;\"><strong>[[").arg(IconList.at(index)->valuesize.value())+\
IconList.at(index)->romid + "]]</strong></span></div>");
            }
            else
            {   // eventually further development, QDateTime usrs conversion to html code
                //QString str = QDateTime::currentDateTime().toString(IconList.at(index)->getDisplayModeStr());
                //if (!str.isEmpty())
                //{
//html.append(QString("<div id=\"wb_Text4\" style=\"position:absolute;left:%1px;top:%2px;width:%3px;height:%4px;z-index:%5;text-align:left;\">")\
//.arg(IconList.at(index)->value->pos().x()).arg(IconList.at(index)->value->pos().y()).arg(IconList.at(index)->value->width()*2).arg(IconList.at(index)->value->height()).arg(index) +
//QString("<span style=\"color:#000000;font-family:") + IconList.at(index)->fontName.currentText() + QString(";font-size:%1px;\"><strong>").arg(IconList.at(index)->valuesize.value())+\
//str + "</strong></span></div>");
                //}
            }
        }
    }
}



void IconeArea::clearSelection()
{
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted) IconList.at(index)->setHighlighted(false);
        if (IconList.at(index)->Thighlighted) IconList.at(index)->setTHighlighted(false);
        if (IconList.at(index)->Vhighlighted) IconList.at(index)->setVHighlighted(false);
    }
    for (int index=0; index<TextList.count(); index++)
        if (TextList[index]->highlighted) TextList[index]->setHighlighted(false);
}


void IconeArea::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
        click = event->pos();
        resize_Icon = -1;
		if (!locked)
		{
			QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
			childClicked = child;
            lastClicked = nullptr;  // lastClicked used for group alignement to the last one clicked
			if (!child)		// set selection rect
			{
                unsigned int state = QApplication::keyboardModifiers();
                if (!(state & Qt::ControlModifier)) clearSelection();
                if (!rubberband) rubberband = new QRubberBand(QRubberBand::Rectangle, this);
				startSelectionPoint = event->pos();
				rubberband->setGeometry(QRect(startSelectionPoint,QSize()));
				rubberband->show();
				rubberBandActive = true;
				return;
			}
            unsigned int state = QApplication::keyboardModifiers();
			for (int index=0; index<IconList.count(); index++)
			{
                if (IconList.at(index)->icon == child) parent->setPalette(&IconList.at(index)->setup);
                else if (IconList.at(index)->text == child) parent->setPalette(&IconList.at(index)->setup);
                else if (IconList.at(index)->value == child) parent->setPalette(&IconList.at(index)->setup);
// click on icon
                if (IconList.at(index)->icon == child)
                {
                    lastClicked = IconList.at(index);
                    if (IconList.at(index)->highlighted)   // Click on icon
					{
                        if (IconList.at(index)->icon_rect().contains(event->pos() - child->pos()))
						{
                            resize_Icon = index; // mouse down inside handle to resize
							return;
						}
                        if (state & Qt::ControlModifier)
                        {
                            if ((IconList.at(index)->highlighted) and (!IconList.at(index)->Thighlighted or !IconList.at(index)->Vhighlighted))
                            {// if icon highlighted on click highlight text and value
                                IconList.at(index)->setTHighlighted(true);
                                IconList.at(index)->setVHighlighted(true);
                            }
                            else if (IconList.at(index)->highlighted and IconList.at(index)->Thighlighted and IconList.at(index)->Vhighlighted)
                            {// if all highlighted on click unselect all
                                IconList.at(index)->setTHighlighted(false);
                                IconList.at(index)->setVHighlighted(false);
                                IconList.at(index)->setHighlighted(false);
                            }// first click select only icon
                            else IconList.at(index)->setHighlighted(false);
                        }
					}
                    else
                    {
						if (!(state & Qt::ControlModifier))
                        {
                            clearSelection();
                            mouseWheelDelta = 0;
                        }
                        IconList.at(index)->setHighlighted(!IconList.at(index)->highlighted);
                    }
                }
// click on icon text
                if (IconList.at(index)->text == child) // Cick on text
                {
                    lastClicked = IconList.at(index);
                    if (!(state & Qt::ControlModifier)) clearSelection();
                    IconList.at(index)->setTHighlighted(!IconList.at(index)->Thighlighted);
                }
// click on icon value
                if (IconList.at(index)->value == child)    // Click on value
                {
                    lastClicked = IconList.at(index);
                    if (!(state & Qt::ControlModifier)) clearSelection();
                    IconList.at(index)->setVHighlighted(!IconList.at(index)->Vhighlighted);
                }
            }
// click on icon text only (icont)
            for (int index=0; index<TextList.count(); index++)
            {
                if (TextList.at(index)->text == child) parent->setPalette(&TextList.at(index)->setup);
                    if (TextList[index]->text == child)
                    {
                        if (!(state & Qt::ControlModifier)) clearSelection();
                        TextList[index]->setHighlighted(true);
                    }
            }
// if Control is not pressed, check if it is a selected icon, if not clear selection
			if (!(state & Qt::ControlModifier))
			{
				bool noSelection = true;
				for (int index=0; index<IconList.count(); index++)
				{
                    if (IconList.at(index)->highlighted)
					{
						noSelection = false;
                        break;
					}
				}
				if (noSelection)
				{
					for (int index=0; index<IconList.count(); index++)
                        if (IconList.at(index)->icon != child) IconList.at(index)->setHighlighted(false);
				}
			}
		}
// locked show palette of device
        else
		{
			QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
			if (!child) return;
				for (int index=0; index<IconList.count(); index++)
				{
                    if (IconList.at(index)->icon == child)
                        if (!IconList.at(index)->htmlCommand.text().isEmpty()) {
                                QString command = IconList.at(index)->htmlCommand.text();
                                QStringList list = command.split(htmlsperarator);
                                if (list.count() == 3) parent->htmlLinkCommand(command); }
					QString RomID;
                    if (IconList.at(index)->icon == child) RomID = IconList.at(index)->romid;
                    else if (IconList.at(index)->text == child) RomID = IconList.at(index)->romid;
                    else if (IconList.at(index)->value == child) RomID = IconList.at(index)->romid;
					onewiredevice *device = parent->configwin->DeviceExist(RomID);
					if (device) parent->setPalette(&device->setup);
				}
		}
	}
	if (event->button() == Qt::RightButton)
	{
		QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
		if (!child) return;
		if (locked)
		{
			int areaindex = -1;
			for (int index=0; index<IconList.count(); index++) 
                    if (IconList.at(index)->icon == child) areaindex = index;
			if (areaindex == -1) return;
			onewiredevice *device = parent->configwin->DeviceExist(IconList[areaindex]->romid);
			if (!device) return;
			QContextMenuEvent Event(QContextMenuEvent::Mouse, QCursor::pos());
			device->contextMenuEvent(&Event);
		}
		else
		{
			int areaindex = -1;
			for (int index=0; index<IconList.count(); index++) 
                    if (IconList.at(index)->icon == child) areaindex = index;
			if (areaindex != -1)
			{
				iconmenu(areaindex, event);
				return;
			}
			areaindex = -1;
			for (int index=0; index<IconList.count(); index++) 
                if (IconList.at(index)->text == child) areaindex = index;
			if (areaindex != -1)
			{
				textmenu(IconList[areaindex]->text, event);
				return;
			}
			areaindex = -1;
			for (int index=0; index<IconList.count(); index++) 
                if (IconList.at(index)->value == child) areaindex = index;
			if (areaindex != -1)
			{
				textmenu(IconList[areaindex]->value, event);
				return;
			}
		}
	}
}



void IconeArea::iconAlignLeft()
{
	int count = 0;
	iconf *last = nullptr;
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->highlighted)
		{
            last = IconList.at(index);
			count++;
		}
	}
	if (count < 2) return;
	if (lastClicked == nullptr) lastClicked = last;
	int xRef = lastClicked->icon->x();
	for (int index=0; index<IconList.count(); index++)
	{
        if ((IconList.at(index)->highlighted) && (IconList.at(index) != lastClicked))
		{
            int labelMove = xRef - IconList.at(index)->icon->x();
            int y = IconList.at(index)->icon->y();
            IconList.at(index)->icon->move(xRef, y);
            int x = IconList.at(index)->text->x() + labelMove;
            y = IconList.at(index)->text->y();
            IconList.at(index)->text->move(x, y);
            x = IconList.at(index)->value->x() + labelMove;
            y = IconList.at(index)->value->y();
            IconList.at(index)->value->move(x, y);
		}
	}
	PushUndo();
}



void IconeArea::iconAlignRight()
{
	int count = 0;
	iconf *last = nullptr;
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->highlighted)
		{
            last = IconList.at(index);
			count++;
		}
	}
	if (count < 2) return;
	if (lastClicked == nullptr) lastClicked = last;
	int xRef = lastClicked->icon->x() + lastClicked->icon->width();
	for (int index=0; index<IconList.count(); index++)
	{
        if ((IconList.at(index)->highlighted) && (IconList.at(index) != lastClicked))
		{
            int labelMove = xRef - IconList.at(index)->icon->width() - IconList.at(index)->icon->x();
            int y = IconList.at(index)->icon->y();
            IconList.at(index)->icon->move(xRef - IconList.at(index)->icon->width(), y);
            int x = IconList.at(index)->text->x() + labelMove;
            y = IconList.at(index)->text->y();
            IconList.at(index)->text->move(x, y);
            x = IconList.at(index)->value->x() + labelMove;
            y = IconList.at(index)->value->y();
            IconList.at(index)->value->move(x, y);
		}
	}
	PushUndo();
}



void IconeArea::iconAlignTop()
{
	int count = 0;
	iconf *last = nullptr;
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->highlighted)
		{
            last = IconList.at(index);
			count++;
		}
	}
	if (count < 2) return;
	if (lastClicked == nullptr) lastClicked = last;
	int yRef = lastClicked->icon->y();
	for (int index=0; index<IconList.count(); index++)
	{
        if ((IconList.at(index)->highlighted) && (IconList.at(index) != lastClicked))
		{
            int labelMove = yRef - IconList.at(index)->icon->y();
            int x = IconList.at(index)->icon->x();
            IconList.at(index)->icon->move(x, yRef);
            int y = IconList.at(index)->text->y() + labelMove;
            x = IconList.at(index)->text->x();
            IconList.at(index)->text->move(x, y);
            y = IconList.at(index)->value->y() + labelMove;
            x = IconList.at(index)->value->x();
            IconList.at(index)->value->move(x, y);
		}
	}
	PushUndo();
}




void IconeArea::iconAlignBottom()
{
	int count = 0;
	iconf *last = nullptr;
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->highlighted)
		{
            last = IconList.at(index);
			count++;
		}
	}
	if (count < 2) return;
	if (lastClicked == nullptr) lastClicked = last;
	int yRef = lastClicked->icon->y() + lastClicked->icon->height();
	for (int index=0; index<IconList.count(); index++)
	{
        if ((IconList.at(index)->highlighted) && (IconList.at(index) != lastClicked))
		{
            int labelMove = yRef - IconList.at(index)->icon->height() - IconList.at(index)->icon->y();
            int x = IconList.at(index)->icon->x();
            IconList.at(index)->icon->move(x, yRef - IconList.at(index)->icon->height());
            int y = IconList.at(index)->text->y() + labelMove;
            x = IconList.at(index)->text->x();
            IconList.at(index)->text->move(x, y);
            y = IconList.at(index)->value->y() + labelMove;
            x = IconList.at(index)->value->x();
            IconList.at(index)->value->move(x, y);
		}
	}
	PushUndo();
}



void IconeArea::iconCopysize()
{
	int count = 0;
	iconf *last = nullptr;
	for (int index=0; index<IconList.count(); index++)
	{
        if (IconList.at(index)->highlighted)
		{
            last = IconList.at(index);
			count++;
		}
	}
	if (count < 2) return;
	if (lastClicked == nullptr) lastClicked = last;
	QSize size = lastClicked->icon->size();
	for (int index=0; index<IconList.count(); index++)
	{
        if ((IconList.at(index)->highlighted) && (IconList.at(index) != lastClicked))
		{
            IconList.at(index)->keepRatio.setCheckState(Qt::Unchecked);
            IconList.at(index)->iconsize_X.setValue(size.width());
            IconList.at(index)->iconsize_Y.setValue(size.height());
            IconList.at(index)->reloadpict();
		}
	}
	PushUndo();
}



void IconeArea::iconCopy(QString &str)
{
	str.clear();
	for (int index=0; index<IconList.count(); index++)
    if (IconList.at(index)->highlighted) IconList.at(index)->getConfigStr(str);
	PushUndo();
}



void IconeArea::iconPaste(const QString &str)
{
    QString configdata;
	configdata.append(str);
	QString ReadRomID, ReadName;
	QString TAG_Begin = Icon_Begin;
	QString TAG_End = Icon_Finished;
	SearchLoopBegin
	ReadName = logisdom::getvalue("IconFileName", strsearch);
	if (ReadName != "")
	{
		iconf *icon = newIcon(this);
		icon->setConfig(strsearch);
        icon->setHighlighted(true);
        icon->setVHighlighted(true);
        icon->setTHighlighted(true);
    }
	SearchLoopEnd
	PushUndo();
}




void IconeArea::iconUndo()
{
	if (undoIndex < 0) return;
	if (undoIndex > 0)
	{
		int count = IconList.count();
		for (int index=0; index<count; index++) delete IconList.at(index);
		IconList.clear();
		undoIndex--;
        QString configdata;
		configdata.append(undo.at(undoIndex));
		QString ReadRomID, ReadName;
		QString TAG_Begin = Icon_Begin;
		QString TAG_End = Icon_Finished;
		SearchLoopBegin
		ReadName = logisdom::getvalue("IconFileName", strsearch);
		if (ReadName != "")
		{
			iconf *icon = newIcon(this);
			icon->setConfig(strsearch);
		}
		SearchLoopEnd
	}
}




void IconeArea::iconRedo()
{
	if (undoIndex == -1) return;
	if (undoIndex < (undo.count()-1))
	{
		int count = IconList.count();
		for (int index=0; index<count; index++) delete IconList.at(index);
		IconList.clear();
		undoIndex++;
        QString configdata;
        configdata.append(undo.at(undoIndex).toLatin1());
		QString ReadRomID, ReadName;
		QString TAG_Begin = Icon_Begin;
		QString TAG_End = Icon_Finished;
		SearchLoopBegin
		ReadName = logisdom::getvalue("IconFileName", strsearch);
		if (ReadName != "")
		{
			iconf *icon = newIcon(this);
			icon->setConfig(strsearch);
		}
		SearchLoopEnd
	}
}





void IconeArea::iconAddOne()
{
	iconf *icon = newIcon(this);
    icon->settxt("TOTO", QPoint(10, 10));
}




void IconeArea::PushUndo()
{
	if (undoIndex != -1) while (undoIndex < (undo.count()-1)) undo.removeLast();
	QString str;
    for (int index=0; index<IconList.count(); index++) IconList.at(index)->getConfigStr(str);
	undo.append(str);
	if (undo.count() > 100) undo.removeFirst();
	undoIndex = undo.count() - 1;
//qDebug() << QString("Push Count = %1 Index = %2").arg(undo.count()).arg(undoIndex);
}



void IconeArea::iconmenu(int areaindex, QMouseEvent *event)
{
    onewiredevice *device = nullptr;
    bool duplicateVirtual = false;
    int count = IconList.count();
    for (int index=0; index<count; index++)
    {
        if (IconList.at(index)->highlighted)
        {
            device = parent->configwin->DeviceExist(IconList.at(index)->romid);
            if (device)
            {
                if (device->isVirtualFamily())
                {
                    duplicateVirtual = true;
                    break;
                }
            }
        }
    }
    QMenu contextualmenu;
	QAction Choose(tr("&Choose Detector"), this);
	QAction Show(tr("&Show Detector"), this);
	QAction Duplicate(tr("&Duplicate"), this);
    QAction DuplicateWithFormula(tr("&Duplicate with virtual device"), this);
    QAction Rename(tr("&Rename"), this);
	QAction Delete(tr("&Delete"), this);
	QAction ChangeIcon(tr("&Change Icon"), this);
    QAction Formula(tr("Show formula items"), this);
    contextualmenu.addAction(&Choose);
	contextualmenu.addAction(&Show);
	contextualmenu.addAction(&Duplicate);
    if (duplicateVirtual) contextualmenu.addAction(&DuplicateWithFormula);
    contextualmenu.addAction(&Rename);
	contextualmenu.addAction(&Delete);
	contextualmenu.addAction(&ChangeIcon);
    contextualmenu.addAction(&Formula);
    QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &Choose) Capteur(areaindex);
	if (selection == &Show) VoirCapteur(areaindex);
	if (selection == &Duplicate) Dupliquer(areaindex);
    if (selection == &DuplicateWithFormula) DupliquerwFormula(areaindex);
    if (selection == &Rename) Renommer(areaindex);
	if (selection == &Delete) Supprimer(areaindex);
	if (selection == &ChangeIcon) changeIcon(areaindex);
    if (selection == &Formula) ShowFormula(areaindex);
}




void IconeArea::textmenu(QLabel *label, QMouseEvent *event)
{
	QMenu contextualmenu;
	QAction FontSize(tr("&Font size"), this);
	contextualmenu.addAction(&FontSize);
	QAction *selection;
	selection = contextualmenu.exec(event->globalPos());
	if (selection == &FontSize) 
	{
		bool ok;
		QFont font = label->font();
        int f = inputDialog::getIntegerPalette(this, tr("Font size"), tr("Font size"), font.pointSize(), 1, 999, 1, &ok, parent);
		if (!ok) return;
		font.setPointSize(f);
		label->setFont(font);
		label->resize(label->sizeHint());
	}
}



void IconeArea::checkFile(bool &state)
{
	for (int index=0; index<IconList.count(); index++)
	{
		IconList.at(index)->checkFile(state);
	}
}




iconf *IconeArea::newIcon(QWidget *wparent)
{
	iconf *icon = new iconf(wparent, this);
	IconList.append(icon);
	connect(icon, SIGNAL(setAsBackground(iconf*)), this, SLOT(setAsBackground(iconf*)));
	connect(icon, SIGNAL(setInFront(iconf*)), this, SLOT(setInFront(iconf*)));
    connect(icon, SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
    return icon;
}



icont *IconeArea::newText(QWidget *wparent)
{
    icont *icon = new icont(wparent, this);
    TextList.append(icon);
    connect(icon, SIGNAL(setInFront(icont*)), this, SLOT(setInFront(icont*)));
    connect(icon, SIGNAL(newFont(QString)), this, SLOT(newFont(QString)));
    return icon;
}


void IconeArea::Capteur(iconf *icon)
{
    onewiredevice *device = nullptr;
    devfinder *devFinder;
    devFinder = new devfinder(parent);
    devFinder->sort();
    devFinder->exec();
    device = devFinder->choosedDevice;
    if (device)
    {
        icon->romid = device->getromid();
        icon->value->setText("");
        device->lecture();
    }
    delete devFinder;
    setValue();
}



void IconeArea::Capteur(int areaindex)
{
   onewiredevice *device = parent->configwin->chooseDevice();
    if (device)
    {
        IconList[areaindex]->romid = device->getromid();
        IconList[areaindex]->value->setText("");
        device->lecture();
    }
}



	
void IconeArea::VoirCapteur(int areaindex)
{
    onewiredevice *device = parent->configwin->DeviceExist(IconList[areaindex]->romid);
    if (device) device->show();
}


void IconeArea::VoirCapteur(QString romID)
{
    onewiredevice *device = parent->configwin->DeviceExist(romID);
    if (device) parent->setPalette(&device->setup);
}


void IconeArea::DupliquerwFormula(int)
{
    int count = IconList.count();
    for (int index=0; index<count; index++)
    {
        if (IconList.at(index)->highlighted)
        {
            IconList.at(index)->setHighlighted(false);
            onewiredevice *device = parent->configwin->DeviceExist(IconList.at(index)->romid);
            QString str;
            IconList.at(index)->getConfigStr(str);
            iconf *icon = new iconf(this, this);
            IconList.append(icon);
            icon->setConfig(str);
            icon->icon->move(IconList.at(index)->icon->pos() + QPoint(20, 20));
            icon->text->move(IconList.at(index)->text->pos() + QPoint(20, 20));
            icon->value->move(IconList.at(index)->value->pos() + QPoint(20, 20));
            connect(icon, SIGNAL(setAsBackground(iconf*)), this, SLOT(setAsBackground(iconf*)));
            connect(icon, SIGNAL(setInFront(iconf*)), this, SLOT(setInFront(iconf*)));
            icon->setHighlighted(true);
            if (device)
            {
                if (device->isVirtualFamily())
                {
                    QString cfg;
                    device->getCfgStr(cfg);
                    cfg = cfg.replace("NAME=()", "");
                    QString name = device->getname();
                    onewiredevice *newdevice = parent->configwin->addVD(name);
                    if (newdevice)
                    {
                        newdevice->setCfgStr(cfg);
                        icon->romid = newdevice->getromid();
                    }
                }
            }
            PushUndo();
        }
    }
}



void IconeArea::mouseWheel(QWheelEvent *event)
{
#if QT_VERSION < 0x060000
    if (event->delta() > 0)
#else
    if (!event->angleDelta().manhattanLength() > 0)
#endif
    {
        mouseWheelDelta ++;
    }
#if QT_VERSION < 0x060000
    if (event->delta() < 0)
#else
    if (event->angleDelta().manhattanLength() < 0)
#endif
    {
        mouseWheelDelta --;
    }
    //qDebug() << QString("Zoom %1").arg(mouseWheelDelta);
    int count = IconList.count();
    for (int index=0; index<count; index++)
    {
#if QT_VERSION < 0x060000
        if (event->delta() > 0) IconList.at(index)->bigger();
        if (event->delta() < 0) IconList.at(index)->smaller();
#else
        if (event->angleDelta().manhattanLength() > 0) IconList.at(index)->bigger();
        if (event->angleDelta().manhattanLength() < 0) IconList.at(index)->smaller();
#endif
    }
    PushUndo();
}



void IconeArea::valuerezise(int fontsize)
{
    int count = IconList.count();
    for (int index=0; index<count; index++)
    {
        if (IconList.at(index)->Vhighlighted or IconList.at(index)->highlighted) IconList.at(index)->valuerezise(fontsize);
    }
    PushUndo();
}



void IconeArea::textrezise(int fontsize)
{
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->Thighlighted or IconList.at(index)->highlighted) IconList.at(index)->textrezise(fontsize);
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList[index]->highlighted) TextList[index]->textrezise(fontsize);
    }
    PushUndo();
}



void IconeArea::Dupliquer(int)
{
	int count = IconList.count();
	for (int index=0; index<count; index++)
	{
        if (IconList.at(index)->highlighted)
		{
            IconList.at(index)->setHighlighted(false);
			QString str;
            IconList.at(index)->getConfigStr(str);
			iconf *icon = new iconf(this, this);
			IconList.append(icon);
			icon->setConfig(str);
            icon->icon->move(IconList.at(index)->icon->pos() + QPoint(20, 20));
            icon->text->move(IconList.at(index)->text->pos() + QPoint(20, 20));
            icon->value->move(IconList.at(index)->value->pos() + QPoint(20, 20));
            connect(icon, SIGNAL(setAsBackground(iconf*)), this, SLOT(setAsBackground(iconf*)));
			connect(icon, SIGNAL(setInFront(iconf*)), this, SLOT(setInFront(iconf*)));
            icon->setHighlighted(true);
		}
	}
	PushUndo();
}



void IconeArea::sendPressEvent(QKeyEvent *event)
{
    unsigned int state = QApplication::keyboardModifiers();
    if (event->key() == Qt::Key_Space)
    {
        if (parent->palette.isHidden()) parent->palette.show(); else parent->palette.hide();
    }
        if (event->key() == Qt::Key_Left) moveLeft();
        if (event->key() == Qt::Key_Right) moveRight();
        if (event->key() == Qt::Key_Up) moveUp();
        if (event->key() == Qt::Key_Down) moveDown();
        if (event->matches(QKeySequence::Paste)) paste(event); // this is new
        if (state & Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_A) selectAll();
    }
}




void IconeArea::paste(QKeyEvent *)
{
    QClipboard *clipboard = QApplication::clipboard();
    QPixmap pixmap = clipboard->pixmap();
    if (!pixmap.isNull())
    {
        //qDebug() << "Ctr-V Picture";
        bool ok;
        QString newFileName;
Retry:
        newFileName = inputDialog::getTextPalette(this, tr("Paste Icone File Name"), tr("Icon File Name"), QLineEdit::Normal, clipboard->text(), &ok, parent);
        if (newFileName.isEmpty()) return;
        if (!ok) return;
        QString iconFileName = QString(repertoireicon) + QDir::separator() + newFileName + ".png";
        QFile iconFile(iconFileName);
        if (iconFile.exists())
        {
            messageBox::warningHide(this, "LogisDom", cstr::toStr(cstr::AlreadyExixts), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            goto Retry;
        }
        if (ok && !newFileName.isEmpty())
        {
            pixmap.save(iconFileName);
            QPoint location;
            location.setX(QCursor::pos().x());
            location.setY(QCursor::pos().y());
            iconf *icon = newIcon(this); // ici
            QPoint IconPos = QPoint(mapFromGlobal(location) - QPoint(pixmap.width()/2, pixmap.height()/2));
            icon->setpict(iconFileName, IconPos);
            QPoint TextPos = QPoint(mapFromGlobal(location) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(pixmap.width(), pixmap.height()));
            icon->settxt(newFileName, TextPos);
            QPoint ValuePos = QPoint(mapFromGlobal(location) - QPoint(pixmap.width()/2, pixmap.height()/2) + QPoint(0, pixmap.height()));
            icon->setvalue("", ValuePos);
            icon->setHighlighted(true);
            icon->setTHighlighted(true);
            icon->setVHighlighted(true);
            PushUndo();
            clipboard->clear();
        }
    }
}


void IconeArea::moveLeft()
{
    int step = 1;
    unsigned int state = QApplication::keyboardModifiers();
    if (state & Qt::ShiftModifier) step = 5;
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
            if (IconList.at(index)->icon->x() <= step) return;
        if (IconList.at(index)->Thighlighted)
            if (IconList.at(index)->text->x() <= step) return;
        if (IconList.at(index)->Vhighlighted)
            if (IconList.at(index)->value->x() <= step) return;
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
            if (TextList.at(index)->text->x() <= step) return;
    }
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
        {
            int x = IconList.at(index)->icon->x() - step;
            if (x < 0) x = 0;
            int y = IconList.at(index)->icon->y();
            if (x > 0) IconList.at(index)->icon->move(x, y);
        }
        if (IconList.at(index)->Thighlighted)
        {
            int x = IconList.at(index)->text->x() - step;
            if (x < 0) x = 0;
            int y = IconList.at(index)->text->y();
            if (x > 0) IconList.at(index)->text->move(x, y);
        }
        if (IconList.at(index)->Vhighlighted)
        {
            int x = IconList.at(index)->value->x() - step;
            if (x < 0) x = 0;
            int y = IconList.at(index)->value->y();
            if (x > 0) IconList.at(index)->value->move(x, y);
        }
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
        {
            int x = TextList.at(index)->text->x() - step;
            if (x < 0) x = 0;
            int y = TextList.at(index)->text->y();
            if (x > 0) TextList.at(index)->text->move(x, y);
        }
    }
}


void IconeArea::moveRight()
{
    int step = 1;
    unsigned int state = QApplication::keyboardModifiers();
    if (state & Qt::ShiftModifier) step = 5;
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
            if ((IconList.at(index)->icon->x() + IconList.at(index)->icon->width()) >= parent->workspaceX.value() - step) return;
        if (IconList.at(index)->Thighlighted)
            if ((IconList.at(index)->text->x() + IconList.at(index)->text->width()) >= parent->workspaceX.value() - step) return;
        if (IconList.at(index)->Vhighlighted)
            if ((IconList.at(index)->value->x() + IconList.at(index)->value->width()) >= parent->workspaceX.value() - step) return;
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
            if ((TextList.at(index)->text->x() + TextList.at(index)->text->width()) >= parent->workspaceX.value() - step) return;
    }
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
        {
            int x = IconList.at(index)->icon->x() + step;
            int y = IconList.at(index)->icon->y();
            IconList.at(index)->icon->move(x, y);
        }
        if (IconList.at(index)->Thighlighted)
        {
            int x = IconList.at(index)->text->x() + step;
            int y = IconList.at(index)->text->y();
            IconList.at(index)->text->move(x, y);
        }
        if (IconList.at(index)->Vhighlighted)
        {
            int x = IconList.at(index)->value->x() + step;
            int y = IconList.at(index)->value->y();
            IconList.at(index)->value->move(x, y);
        }
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
        {
            int x = TextList.at(index)->text->x() + step;
            int y = TextList.at(index)->text->y();
            TextList.at(index)->text->move(x, y);
        }
    }
}





void IconeArea::moveUp()
{
    int step = 1;
    unsigned int state = QApplication::keyboardModifiers();
    if (state & Qt::ShiftModifier) step = 5;
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
            if (IconList.at(index)->icon->y() <= step) return;
        if (IconList.at(index)->Thighlighted)
            if (IconList.at(index)->text->y() <= step) return;
        if (IconList.at(index)->Vhighlighted)
            if (IconList.at(index)->value->y() <= step) return;
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
            if (TextList.at(index)->text->y() <= step) return;
    }
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
        {
            int x = IconList.at(index)->icon->x();
            int y = IconList.at(index)->icon->y() - step;
            if (y < 0) y = 0;
            IconList.at(index)->icon->move(x, y);
        }
        if (IconList.at(index)->Thighlighted)
        {
            int x = IconList.at(index)->text->x();
            int y = IconList.at(index)->text->y() - step;
            if (y < 0) y = 0;
            IconList.at(index)->text->move(x, y);
        }
        if (IconList.at(index)->Vhighlighted)
        {
            int x = IconList.at(index)->value->x();
            int y = IconList.at(index)->value->y() - step;
            if (y < 0) y = 0;
            IconList.at(index)->value->move(x, y);
        }
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
        {
            int x = TextList.at(index)->text->x();
            int y = TextList.at(index)->text->y() - step;
            if (y < 0) y = 0;
            TextList.at(index)->text->move(x, y);
        }
    }
}


void IconeArea::setHtmlEnabled(bool state)
{
    htmlEnabled = state;
}


bool IconeArea::isHtmlEnabled()
{
    return htmlEnabled;
}


void IconeArea::selectAll()
{
    bool state = true;
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted) state = false;
        if (IconList.at(index)->Thighlighted) state = false;
        if (IconList.at(index)->Vhighlighted) state = false;
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted) state = false;
    }
    for (int index=0; index<IconList.count(); index++)
    {
        IconList.at(index)->setHighlighted(state);
        IconList.at(index)->setTHighlighted(state);
        IconList.at(index)->setVHighlighted(state);
    }
    for (int index=0; index<TextList.count(); index++)
    {
        TextList.at(index)->setHighlighted(state);
    }
}


void IconeArea::moveDown()
{
    int step = 1;
    unsigned int state = QApplication::keyboardModifiers();
    if (state & Qt::ShiftModifier) step = 5;
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
            if ((IconList.at(index)->icon->y() + IconList.at(index)->icon->height()) >= parent->workspaceY.value() - step) return;
        if (IconList.at(index)->Thighlighted)
            if ((IconList.at(index)->text->y() +  IconList.at(index)->text->height()) >= parent->workspaceY.value() - step) return;
        if (IconList.at(index)->Vhighlighted)
            if ((IconList.at(index)->value->y() +  IconList.at(index)->value->height()) >= parent->workspaceY.value() - step) return;
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
            if ((TextList.at(index)->text->y() +  TextList.at(index)->text->height()) >= parent->workspaceY.value() - step) return;
    }
    for (int index=0; index<IconList.count(); index++)
    {
        if (IconList.at(index)->highlighted)
        {
            int x = IconList.at(index)->icon->x();
            int y = IconList.at(index)->icon->y() + step ;
            IconList.at(index)->icon->move(x, y);
        }
        if (IconList.at(index)->Thighlighted)
        {
            int x = IconList.at(index)->text->x();
            int y = IconList.at(index)->text->y() + step;
            IconList.at(index)->text->move(x, y);
        }
        if (IconList.at(index)->Vhighlighted)
        {
            int x = IconList.at(index)->value->x();
            int y = IconList.at(index)->value->y() + step;
            IconList.at(index)->value->move(x, y);
        }
    }
    for (int index=0; index<TextList.count(); index++)
    {
        if (TextList.at(index)->highlighted)
        {
            int x = TextList.at(index)->text->x();
            int y = TextList.at(index)->text->y() + step;
            TextList.at(index)->text->move(x, y);
        }
    }
}


void IconeArea::changeIcon(int)
{
}



void IconeArea::ShowFormula(int areaindex)
{
    parent->setPalette(&IconList[areaindex]->stat_setup);
}




void IconeArea::Supprimer(int areaindex)
{
    int selection = 0;
    for (int index=0; index<IconList.count(); index++)
        if (IconList.at(index)->highlighted) selection++;
    for (int index=0; index<TextList.count(); index++)
        if (TextList.at(index)->highlighted) selection++;
    if (selection > 1)
    {
        if (messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to delete selection ?"), parent, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes) return;
        for (int index=0; index<IconList.count(); index++)
        {
            if ((IconList.at(index)->highlighted) or (IconList.at(index)->Thighlighted) or (IconList.at(index)->Vhighlighted))
            {
                parent->removeWidget(&IconList.at(index)->setup);
                delete IconList.takeAt(index--);
            }
        }
        for (int index=0; index<TextList.count(); index++)
        {
            if (TextList.at(index)->highlighted)
            {
                parent->removeWidget(&TextList[index]->setup);
                delete TextList.takeAt(index--);
            }
        }
    }
    else
    {
        if (messageBox::questionHide(this, tr("Remove ?"), tr("Do you really want to remove ") + IconList[areaindex]->text->text(), parent, QMessageBox::No | QMessageBox::Yes) != QMessageBox::Yes) return;
        parent->removeWidget(&IconList[areaindex]->setup);
        delete IconList.takeAt(areaindex);
    }
}





void IconeArea::Renommer(int areaindex)
{
	bool ok;
    QString item = inputDialog::getTextPalette(this, cstr::toStr(cstr::Name), cstr::toStr(cstr::Name2dot), QLineEdit::Normal,  IconList[areaindex]->text->text(), &ok, parent);
	if (ok && !item.isEmpty()) IconList[areaindex]->text->setText(item);
	IconList[areaindex]->text->resize(IconList[areaindex]->text->sizeHint());
}


