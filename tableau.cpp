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




#include <QtWidgets/QFileDialog>
#include "globalvar.h"
#include "logisdom.h"
#include "onewire.h"
#include "configwindow.h"
#include "inputdialog.h"
#include "tableau.h"
#include "messagebox.h"
#include "devfinder.h"
#include <qwt_plot_curve.h>





tableau::tableau(QString *Name, logisdom *Parent)
{
	parent = Parent;
	previousSetup = nullptr;
    isRefilling = false;
	name = *Name;
//	setSizeGripEnabled(true);
//	setWindowFlags(Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	setWindowTitle(name);
	QIcon icon;
	icon.addPixmap(QPixmap(QString::fromUtf8(":/images/images/kfm_home.png")), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);
    layout = new QGridLayout(this);
    setLayout(layout);
    layout->addWidget(&table, 0, 0, 1, 9);
// palette setup
	setup.setLayout(&setupLayout);
    int layoutIndex = 1;
    ui.ui.saveNow->setText(tr("Store now"));
    connect(ui.ui.saveNow, SIGNAL(clicked()), this, SLOT(savenow()));
    ui.ui.Browse->setText(tr("Browse"));
    connect(ui.ui.Browse, SIGNAL(clicked()), this, SLOT(browse()));
    connect(ui.ui.pushButtonReprocess, SIGNAL(clicked()), this, SLOT(refill()));
    ui.ui.AddDevice->setText(tr("Add device"));
    connect(ui.ui.AddDevice, SIGNAL(clicked()), this, SLOT(addDevice()));
    ui.ui.RemoveDevice->setText(tr("Remove device"));
    connect(ui.ui.RemoveDevice, SIGNAL(clicked()), this, SLOT(removeDevice()));
    connect(ui.ui.clearTable, SIGNAL(clicked()), this, SLOT(clearTable()));
    ui.ui.MaxRow->setPrefix(tr("Max row to display : "));
    ui.ui.MaxRow->setRange(2, 9999);
    ui.ui.MaxRow->setValue(10);
    ui.ui.PreloadDisplay->setText(tr("Preload"));
    ui.ui.SaveFile->setText(tr("Save to file"));
    ui.saveInterval.setName(tr("Save Interval"));
    ui.saveInterval.enableYear();
    ui.ui.FirstDateTime->setText("dd-MM-yyyy");
    ui.ui.SecondDateTime->setText("HH:mm");
    ui.ui.SeparationChar->setText(";");
    ui.saveInterval.setName(tr("Save interval"));
    rebuild();
    connect(&table, SIGNAL(clicked(QModelIndex)), this, SLOT(clickEvent(QModelIndex)));
    setupLayout.addWidget(&ui, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    ui.ui.comboBoxDeviceList->hide();
    ui.ui.checkBoxIndexOn->hide();
    StartButtonName = ui.ui.pushButtonReprocess->text();
    ui.ui.dateEdit->setDate(QDateTime::currentDateTime().date());

}

//calculInterval.setName(tr("Calculate Interval"));
//setupLayout.addWidget(&calculInterval,  4, 1, 1, 1);



tableau::~tableau()
{
}


void tableau::rebuild()
{
    table.clearContents();
    table.setColumnCount(DateTimeWidth);
    table.setRowCount(1);
    QTableWidgetItem *item = new QTableWidgetItem(tr("Date"));
    table.setItem(0, 0, item);
    item = new QTableWidgetItem(tr("Time"));
    table.setItem(0, 1, item);
    for (int n=0; n<tableauDeviceList.count(); n++)
    {
        int index = table.columnCount();
        table.setColumnCount(index + 1);
        QString romid = tableauDeviceList.at(n);
        onewiredevice *device = parent->configwin->DeviceExist(romid);
        QString dev;
        if (device) dev = device->getname();
            else dev = romid;
        QTableWidgetItem *item = new QTableWidgetItem(dev);
        table.setItem(0, index, item);
    }
}


void tableau::mousePressEvent(QMouseEvent*)
{
	emit(setupClick(&setup));
}


void tableau::clickEvent()
{
	emit(setupClick(&setup));
}


void tableau::clickEvent(QModelIndex)
{
	emit(setupClick(&setup));
}


void tableau::save()
{
    if (isRefilling) return;
    if (ui.saveInterval.isitnow()) savenow();
}




void tableau::browse()
{
	QString directory;
    if (ui.ui.FileName->text().isEmpty())
		directory = QFileDialog::getExistingDirectory(this, tr("Path"), QDir::currentPath());
	else
        directory = QFileDialog::getExistingDirectory(this, tr("Path"), ui.ui.FileName->text());
    if (!directory.isEmpty()) ui.ui.FileName->setText(directory);
}





void tableau::refill()
{
    isRefilling = true;
    if (tableauDeviceList.count() == 0) return;
    if (messageBox::questionHide(this, tr("Refill table ?"), tr("Do you want to refill the table ?") , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
// get device list
    QList <onewiredevice*> deviceList;
    for (int n=0; n<tableauDeviceList.count(); n++)
    {
        onewiredevice *device = parent->configwin->DeviceExist(tableauDeviceList[n]);
        if (device) deviceList.append(device);
        else
        {
            messageBox::warningHide(this, tr("Device is missing"), tr("Device") + tableauDeviceList[n] + tr("cannot be found"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
            return;
        }
    }
    if (deviceList.isEmpty())
    {
        messageBox::warningHide(this, tr("No device"), tr("Please add some devices"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        return;
    }
    ui.ui.pushButtonReprocess->setText(tr("processing ..."));
    ui.ui.pushButtonReprocess->setEnabled(false);
// set begining QDateTime
    QDateTime begin = ui.ui.dateEdit->dateTime();
    QDateTime index = ui.saveInterval.nextOne.dateTime();
    int gap = ui.saveInterval.getSecs();
    if (gap > Minutes2Weeks)
    {
        while (index.secsTo(begin) <= 0) index = index.addMonths(-1);
        if (index.secsTo(begin) != 0) index = index.addMonths(1);
    }
    else
    {
        int backward = index.secsTo(begin) / gap;
        index = index.addSecs(-gap * backward);
        while (index.secsTo(begin) <= 0)
        {
            index = index.addSecs(-gap);
        }
        if (index.secsTo(begin) != 0) index = index.addSecs(gap);
   }
// rebuild table
    rebuild();
    QDateTime now = QDateTime::currentDateTime();
    int row = table.rowCount();
    do
    {
        table.setRowCount(row + 1);
        QString T1 = index.toString(ui.ui.FirstDateTime->text());
        QString T2 = index.toString(ui.ui.SecondDateTime->text());
        QTableWidgetItem *item = new QTableWidgetItem(T1);
        table.setItem(row, 0, item);
        if (!T2.isEmpty())
        {
            item = new QTableWidgetItem(T2);
            table.setItem(row, 1, item);
        }
        double V;
        QString S;
        for (int n=0; n<deviceList.count(); n++)
        {
            bool loading = false;
            onewiredevice *device = deviceList.at(n);
            V = device->getMainValue(index, loading);
            while (loading)
            {
                QTime dieTime = QTime::currentTime().addSecs(1);
                while( QTime::currentTime() < dieTime )
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                loading = false;
                V = device->getMainValue(index, loading);
            }
            if (ui.ui.checkBoxNoText->isChecked()) S = device->ValueToStr(V, true);
            else S = device->ValueToStr(V);
            QTableWidgetItem *item = new QTableWidgetItem(S.remove(device->getunit()));
            table.setItem(row, n + DateTimeWidth, item);
        }
        if (gap > Minutes2Weeks)
        {
            index = index.addMonths(1);
        }
        else
        {
            index = index.addSecs(gap);
        }
        row ++;
    }
    while (index.secsTo(now) >= 0);
    isRefilling = false;
    ui.ui.pushButtonReprocess->setEnabled(true);
    ui.ui.pushButtonReprocess->setText(StartButtonName);
    emit(updateChart());
    if (messageBox::questionHide(this, tr("Save to file ?"), tr("Do you want to save to file ?") , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
    QString filename = getFileName();
    if (filename.isEmpty()) return;
    QFile file(filename);
    saveAll(file);
}




void tableau::getData(const QDateTime Origin, QVector <QwtIntervalSample> &data)
{
    bool ok;
    int row = table.rowCount();
    int col = table.columnCount();
    if (row < 2) return;
    if (col < 3) return;
    for (int n=1; n<row-1; n++)
    {
        QTableWidgetItem *itemD1 = table.item(n, 0);
        QTableWidgetItem *itemH1 = table.item(n, 1);
        QTableWidgetItem *itemD2 = table.item(n+1, 0);
        QTableWidgetItem *itemH2 = table.item(n+1, 1);
        QTableWidgetItem *itemVal = table.item(n+1, col-1);
        if (!itemD1) return;
        if (!itemH1) return;
        if (!itemD2) return;
        if (!itemH2) return;
        if (!itemVal) return;
        QString D1 = itemD1->text();
        QString H1 = itemH1->text();
        QString D2 = itemD2->text();
        QString H2 = itemH2->text();
        double val = itemVal->text().toDouble(&ok);
        QDateTime time1 = QDateTime(QDate::fromString(D1,ui.ui.FirstDateTime->text()), QTime::fromString(H1, ui.ui.SecondDateTime->text()));
        QDateTime time2 = QDateTime(QDate::fromString(D2, ui.ui.FirstDateTime->text()), QTime::fromString(H2, ui.ui.SecondDateTime->text()));
        if (time1.isValid() && time2.isValid() && ok)
        {
            double t1 = Origin.secsTo(time1);
            t1 /= SecsInDays;
            double t2 = Origin.secsTo(time2);
            t2 /= SecsInDays;
            if ((n == 1))
            {
                QTableWidgetItem *itemValfirst = table.item(n, col-1);
                double valfirst = itemValfirst->text().toDouble(&ok);
                if (ok)
                {
                    QwtIntervalSample sample(valfirst, t1 - (t2 - t1), t1);
                    data.append(sample);
                 }
            }
            QwtIntervalSample sample(val, t1, t2);
            data.append(sample);
        }
    }
}



void tableau::savenow()
{
	if (tableauDeviceList.count() == 0) return;
	int row = table.rowCount();
	table.setRowCount(row + 1);
	int count = tableauDeviceList.count();
	QDateTime now = QDateTime::currentDateTime();
    QString T1 = now.toString(ui.ui.FirstDateTime->text());
    QString T2 = now.toString(ui.ui.SecondDateTime->text());
	QTableWidgetItem *item = new QTableWidgetItem(T1);
	table.setItem(row, 0, item);
    QString line = T1 + ui.ui.SeparationChar->text();
	if (!T2.isEmpty())
	{
		item = new QTableWidgetItem(T2);
		table.setItem(row, 1, item);
        line += T2 + ui.ui.SeparationChar->text();
	}
	for (int n=0; n<count; n++)
	{
		onewiredevice *device = parent->configwin->DeviceExist(tableauDeviceList[n]);
		if (device)
		{
			QString S = device->MainValueToStr().remove(device->getunit());
			QTableWidgetItem *item = new QTableWidgetItem(S);
            if (item)
            {
                table.setItem(row, n + DateTimeWidth, item);
                line += S;
                line += ui.ui.SeparationChar->text();
            }
		}
	}
	row = table.rowCount();
    bool tooLarge = false;
    while (row > ui.ui.MaxRow->value())
    {
        table.removeRow(1);  row = table.rowCount();
        tooLarge = true;
    }
    table.scrollToBottom();
    emit(updateChart());
    if (!ui.ui.SaveFile->isChecked()) return;
// set file name
    QString filename = getFileName();
    if (filename.isEmpty()) return;
    QFile file(filename);
// if too large we have to save entire file
    if (tooLarge)
    {
        saveAll(file);
        return;
    }
// Write header if new file
    if (!file.exists()) saveAll(file);
    else {
        // Write line
        if(file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            out << line;
            out << "\n";
            file.close();
        }
        else
        {
            parent->logfile("cannot open file " + filename);
            return;
        }
    }
}





QString tableau::getFileName()
{
// create csv folder if no name specified
    if (!QDir().exists(repertoirecsv) && (ui.ui.FileName->text().isEmpty()))
    {
        parent->logfile("Try to create Dir " repertoirecsv);// repertoiredat);
        if (!QDir().mkdir(repertoirecsv))
        {
            parent->logfile("Cannot create Dir " repertoirecsv);
            return "";
        }
        parent->logfile("Dir " repertoirecsv " created");
    }
    QString filename;
    if (ui.ui.FileName->text().isEmpty()) filename = QString(repertoirecsv) + QDir::separator() + name + ".csv";
    else
    {
        QFileInfo fileInfo(ui.ui.FileName->text());
        if (fileInfo.isDir()) filename = ui.ui.FileName->text() + QDir::separator() + name + ".csv";
        else filename = ui.ui.FileName->text();
    }
    return filename;
}




void tableau::saveAll(QFile &file)
{
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        QString header;
        int c = table.columnCount();
        for (int n=0; n<c; n++)
        header += table.item(0, n)->text() + ui.ui.SeparationChar->text();
        out << header;
        out << "\n";
        file.close();
    // Write all lines
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            int c = table.columnCount();
            int l = table.rowCount();
            for (int L=0; L<l; L++)
            {
                QString str;
                for (int C=0; C<c; C++)
                {
                    QTableWidgetItem *item = table.item(L, C);
                    if (item) str += item->text() + ui.ui.SeparationChar->text();
                }
                out << str;
                out << "\n";
            }
            file.close();
        }
    }
    else
    {
        parent->logfile("cannot open file " + file.fileName());
        return;
    }
}




void tableau::addDevice()
{
    onewiredevice *device = nullptr;
    devfinder *devFinder;
    devFinder = new devfinder(parent);
    for (int n=0; n<parent->configwin->devicePtArray.count(); n++)
        if (!IsRomIDthere(parent->configwin->devicePtArray[n]->getromid()))
        {
            devFinder->devicesList.append(parent->configwin->devicePtArray[n]);
        }
    if (devFinder->devicesList.count() == 0)
    {
        messageBox::warningHide(this, cstr::toStr(cstr::MainName), tr("No more device could be found"), parent, QFlag(QMessageBox::AcceptRole | QMessageBox::NoIcon));
        delete devFinder;
        return;
    }
    devFinder->sort();
    devFinder->exec();
    device = devFinder->choosedDevice;
    delete devFinder;
    if (device)
    {
        int index = table.columnCount();
        table.setColumnCount(index + 1);
        tableauDeviceList.append(device->getromid());
        QTableWidgetItem *item = new QTableWidgetItem(device->getname());
        table.setItem(0, index, item);
    }
}





void tableau::addDevice(QString romid)
{
	int index = table.columnCount();
	table.setColumnCount(index + 1);
	tableauDeviceList.append(romid);
	onewiredevice *device = parent->configwin->DeviceExist(romid);
	QString dev;
	if (device) dev = device->getname();
		else dev = romid;
	QTableWidgetItem *item = new QTableWidgetItem(dev);
	table.setItem(0, index, item);
}




void tableau::clearTable()
{
    if (messageBox::questionHide(this, tr("Clear table ?"), tr("Do you want to clear the table ?") , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
    rebuild();
    if (messageBox::questionHide(this, tr("Clear file ?"), tr("Do you want to clear the file also ?") , parent, QMessageBox::No | QMessageBox::Yes) == QMessageBox::No) return;
    QString filename = getFileName();
    if (filename.isEmpty()) return;
    QFile file(filename);
    saveAll(file);
}



void tableau::removeDevice()
{
	bool ok;
	QStringList deviceList;
	for (int n=0; n<tableauDeviceList.count(); n++)
	{
		onewiredevice *device = parent->configwin->DeviceExist(tableauDeviceList[n]);
		if (device)
		{
			QString name = device->getname();
			if (name.isEmpty()) name = device->getromid();
			deviceList << name;
		}
	}
    QString devicechoise = inputDialog::getItemPalette(this, tr("Remove device"), tr("Select device : "), deviceList, 0, false, &ok, parent);
	if (!ok) return;
	onewiredevice *device = parent->configwin->Devicenameexist(devicechoise);
	if (!device) return;
	int n = tableauDeviceList.indexOf(device->getromid());
	if (n != -1)
	{
		tableauDeviceList.removeAt(n);
        table.removeColumn(n + 2) ;
	}
}




void tableau::updateName(onewiredevice *device)
{
	for (int n=0; n<tableauDeviceList.count(); n++)
	{
		if (device->getromid() == tableauDeviceList[n])
		{
			QString name = device->getname();
			if (name.isEmpty()) name = device->getromid();
			QTableWidgetItem *newitem = new QTableWidgetItem(name);
			table.setItem(0, n + DateTimeWidth, newitem);
		}
	}
}



bool tableau::IsRomIDthere(QString RomID)
{
	for (int n=0; n<tableauDeviceList.count(); n++)
		if (tableauDeviceList[n] == RomID) return true;
	return false;
}





void tableau::setPalette(QWidget *graphsetup)
{
	if (previousSetup) previousSetup->hide();
	previousSetup = graphsetup;
	if (graphsetup) setupLayout.addWidget(graphsetup, 6, 0);
	if (graphsetup) graphsetup->show();
}





QString tableau::getName()
{
	return name;
}




void tableau::checkPreload()
{
    if (!ui.ui.PreloadDisplay->isChecked()) return;
	if (tableauDeviceList.count() <= 0) return;
    QString filename;
    if (ui.ui.FileName->text().isEmpty()) filename = QString(repertoirecsv) + QDir::separator() + name + ".csv";
    else
    {
        QFileInfo fileInfo(ui.ui.FileName->text());
        if (fileInfo.isDir()) filename = ui.ui.FileName->text() + QDir::separator() + name + ".csv";
        else filename = ui.ui.FileName->text();
    }
    QFile file(filename);
	if(file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream in(&file);
		QString data = in.readLine();
		while (!in.atEnd())
		{
			data = in.readLine();
            QStringList split = data.split(ui.ui.SeparationChar->text());
			int row = table.rowCount();
			int col = table.columnCount();
			table.setRowCount(row + 1);
			for (int L=0; L<col; L++)
			{
				if (L < split.count())
				{
					QTableWidgetItem *item = new QTableWidgetItem(split.at(L));
					table.setItem(row, L, item);
				}
			}
		}
		file.close();
	}
    table.scrollToBottom();
}



void tableau::setTableauConfig(QString &str)
{
	int x, y, w, h;
	bool ok_x = 0, ok_y = 0, ok_w = 0, ok_h = 0, ok;
	x = logisdom::getvalue("Window_X", str).toInt(&ok_x, 10);
	y = logisdom::getvalue("Window_Y", str).toInt(&ok_y, 10);
	w = logisdom::getvalue(Window_Width_Mark, str).toInt(&ok_w, 10);
	h = logisdom::getvalue(Window_Height_Mark, str).toInt(&ok_h, 10);
        if (ok_x && ok_y && ok_w && ok_h) setGeometry(x, y, w, h);
	int maxrow = logisdom::getvalue("MaxRow", str).toInt(&ok);
        if (ok) ui.ui.MaxRow->setValue(maxrow);
	int savemode = logisdom::getvalue("SaveMode", str).toInt(&ok);
        if (ok) ui.saveInterval.setMode(savemode);
	QString next = logisdom::getvalue("NextSave", str);
        if (next.isEmpty()) ui.saveInterval.setNext(QDateTime::currentDateTime());
                else ui.saveInterval.setNext(QDateTime::fromString (next, Qt::ISODate));
	int saveen = logisdom::getvalue("SaveEnabled", str).toInt(&ok);
    if (!ok) ui.saveInterval.setSaveMode(true);
        else if (saveen) ui.saveInterval.setSaveMode(true);
            else  ui.saveInterval.setSaveMode(false);
    //int disponly = logisdom::getvalue("MaxDisplayOnly", str).toInt(&ok);
    //if (!ok) ui.ui.MaxForDisplayOnly.setChecked(true);
    //	else if (disponly) MaxForDisplayOnly.setChecked(true);
    //		else  MaxForDisplayOnly.setChecked(false);
	int preload = logisdom::getvalue("PreloadDisplay", str).toInt(&ok);
    if (!ok) ui.ui.PreloadDisplay->setChecked(false);
        else if (preload) ui.ui.PreloadDisplay->setChecked(true);
            else  ui.ui.PreloadDisplay->setChecked(false);
	int index = 0;
	QString NextDevice = logisdom::getvalue(QString("Device_%1").arg(index), str);
	while (!NextDevice.isEmpty())
        {
            addDevice(NextDevice);
            index ++;
	    NextDevice = logisdom::getvalue(QString("Device_%1").arg(index), str);
        }
	int fileen = logisdom::getvalue("FileEnabled", str).toInt(&ok);
    if (!ok) ui.ui.SaveFile->setCheckState(Qt::Checked);
        else if (fileen) ui.ui.SaveFile->setCheckState(Qt::Checked);
            else  ui.ui.SaveFile->setCheckState(Qt::Unchecked);
	QString filename =  logisdom::getvalue("FileName", str);
    if (!filename.isEmpty()) ui.ui.FileName->setText(filename);
	QString fdt =  logisdom::getvalue("FirstDateTime", str);
    if (!fdt.isEmpty()) ui.ui.FirstDateTime->setText(fdt);
	QString sdt =  logisdom::getvalue("SecondDateTime", str);
    if (!sdt.isEmpty()) ui.ui.SecondDateTime->setText(sdt);
	QString sep =  logisdom::getvalue("SeparationChar", str);
    if (!sep.isEmpty()) ui.ui.SeparationChar->setText(sep);
	checkPreload();
    //int hidden = logisdom::getvalue("Window_Hidden", str).toInt(&ok_h);
    //if (ok_h && hidden) done(0); else show();
}






void tableau::getTableauConfig(QString &str)
{
	str += "\nNew_Tableau_Begin\n";
	str += logisdom::saveformat("Name", name);
	int x = geometry().x();
	str += logisdom::saveformat("Window_X", QString("%1").arg(x));
	int y = geometry().y();
	str += logisdom::saveformat("Window_Y", QString("%1").arg(y));
	str += logisdom::saveformat(Window_Width_Mark, QString("%1").arg(width()));
	str += logisdom::saveformat(Window_Height_Mark, QString("%1").arg(height()));
	str += logisdom::saveformat("Window_Hidden", QString("%1").arg(isHidden()));
    str += logisdom::saveformat("SaveMode", ui.saveInterval.getMode());
    str += logisdom::saveformat("NextSave", ui.saveInterval.getNext());
    str += logisdom::saveformat("MaxRow", QString("%1").arg(ui.ui.MaxRow->value()));
    str += logisdom::saveformat("SaveEnabled", ui.saveInterval.getSaveMode());
    //str += logisdom::saveformat("MaxDisplayOnly",  QString("%1").arg(ui.ui.MaxForDisplayOnly.isChecked()));
    str += logisdom::saveformat("PreloadDisplay",  QString("%1").arg(ui.ui.PreloadDisplay->isChecked()));
    str += logisdom::saveformat("FileEnabled", QString("%1").arg(ui.ui.SaveFile->isChecked()));
    str += logisdom::saveformat("FileName", ui.ui.FileName->text());
    str += logisdom::saveformat("SeparationChar", ui.ui.SeparationChar->text());
    str += logisdom::saveformat("FirstDateTime", ui.ui.FirstDateTime->text());
    str += logisdom::saveformat("SecondDateTime", ui.ui.SecondDateTime->text());
	for (int n=0; n<tableauDeviceList.count(); n++)
		str += logisdom::saveformat(QString("Device_%1").arg(n),  tableauDeviceList[n]);
	str += "New_Tableau_Finished\n";
}




void tableau::setName(QString Name)
{
	name = Name;
	setWindowTitle(name);
}

 


