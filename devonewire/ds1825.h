#ifndef ds1825_H
#define ds1825_H
#include "axb.h"
#include "onewire.h"
#include "ui_ds1825.h"

class ds1825 : public onewiredevice
{
	Q_OBJECT
public:
    ds1825(net1wire *Master, logisdom *Parent, QString RomID);
	bool isTempFamily();
	QString MainValueToStrLocal(const QString &str);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	void writescratchpad();
    double calcultemperature(const QString &THex);
	void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
    void SetOrder(const QString &order);
	QPushButton ResolutionButton;
	QString textResolution;
// Palette setup
    //axb AXplusB;
    QLabel ID;
    QCheckBox MAX31850;
private:
    Ui::ds1825ui ui;
	void changealarmebasse();
	void changealarmehaute();
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots :
	 void changresoultion();
     void typeChanged(int);
};


#endif
