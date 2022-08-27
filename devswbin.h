#ifndef devswbin_H
#define devswbin_H
#include "onewire.h"
#include "ui_ds2423.h"

class devswbin : public onewiredevice
{
	Q_OBJECT
public:
	devswbin(net1wire *Master, logisdom *Parent, QString RomID);
	QString MainValueToStrLocal(const QString &str);
	QString ValueToStr(double Value);
	void lecture();
	void lecturerec();
	void SetOrder(QString order);
	QComboBox ParameterList;
	void setconfig(const QString &strsearch);
	void GetConfigStr(QString &str);
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
	//QComboBox counterMode;
	//QSpinBox Offset;
	//QDoubleSpinBox Coef;
	//interval countInit;
	//QCheckBox SaveOnUpdate;
	//long int LastCounter, Delta;
	void setLocalMainValue(double value, bool enregistremode);
private:
	Ui::ds2423ui ui;
	QString VStr;
	void setResult(long int NewValue);
private slots:
	void contextMenuEvent(QContextMenuEvent *event);
	void saveOnUpdateChanged(int state);
};

#endif
