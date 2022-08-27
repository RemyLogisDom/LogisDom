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



#ifndef NET1WIRE_H
#define NET1WIRE_H
#include <QtNetwork>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "ui_guinet1wire.h"


class QTcpSocket;
class QAbstractSocket;
class QStringListModel;
class QBuffer;
class onewiredevice;
class errlog;
class htmlBinder;
class logisdom;
class remote;
class eocean;


#define HA7Net_No_Thread

class net1wire : public QWidget
{
#define fifomax 1000
	Q_OBJECT
	friend class ha7net;
	friend class fts800;
	friend class x10;
	friend class plcbus;
	friend class ecogest;
    friend class eocean;
    friend class remote;
	friend class pic;
	friend class abstractOneWire;
	friend class teleinfo;
	friend class rps2;
	friend class resol;
	friend class ha7s;
	friend class mbus;
	friend class modbus;
	enum NetTraffic
	{
		Disabled, Connecting, Waitingforanswer, Disconnected, Connected, Paused, Simulated
	};
private:		// Variable
	Ui::guinet1wire ui;
	QMutex mutex;
	QMutex MutexGenMsg;
	QMutex mutexsettraffic;
	QString moduleipaddress, name, unID;
	QGridLayout *framelayout;
    int type;
    quint16 port;
	QPushButton Bouton1, Bouton2, Bouton3, Bouton4, Bouton5, Bouton6;
	virtual void removeDuplicates();
private:		// functions
	void disconnectall();
	void log(const QString &txt);
	void settraffic(int state);
	void settabtraffic(int state);
	QList <onewiredevice*> localdevice;
    bool tobeDeleted;
    QIcon runIcon, stopIcon, tcpIconUnconnectedState, tcpIconHostLookupState, tcpIconConnectingState, tcpIconConnectedState, tcpIconClosingState;
public:
	net1wire(logisdom *Parent);
	~net1wire();
	logisdom *parent;
    void setTobeDeleted();
    QTcpSocket tcp;
	virtual void convert();
    virtual void closeTCP();
	virtual void startTCP();
    virtual bool receiveData();
	QTimer TimerReqDone;
	void UpdateLocalDeviceList();
	QDateTime lastConnectionRequest;
	void writeTcp(char c);
	void writeTcp(const QByteArray req);
	void setName(QString Name);
	virtual bool isUploading();
	virtual bool isGlobalConvert();
	int gettype();
	int getport();
	bool isDataValid();
	bool logenabled();
	bool logactenabled();
	void fifoListRemoveFirst();
	void fifoListInsertFirst(const QString &order, int position = 1);
	void fifoListAdd(const QString &order);
	QString fifoListNext();
	QString fifoListLast();
	bool fifoListEmpty();
	bool fifoListContains(const QString &str);
    int fifoListCount();
	virtual void setport(int Port);
	QString getipaddress();
	QString getname();
	void setlogenabled(bool state);
	void setlogactenabled(bool state);
	void setname(const QString &Name);
	virtual void addtofifo(const QString &order);
	virtual void addtofifo(int order);
	virtual void addtofifo(int order, const QString &RomID, bool priority = false);
	virtual void addtofifo(int order, const QString &RomID, const QString &Data, bool priority = false);
	void addremotefifo(QString &str);
	virtual void setipaddress(const QString &adr);
	virtual void init();
	QString GenError(int ErrID, const QString Msg);
	void GenMsg(const QString Msg);
	errlog *ErrorLog;
	int errortabindex;
	virtual void getMainValue();
	virtual void saveMainValue();
	void getCfgStr(QString &str);
	void setCfgStr(const QString &strsearch);
	virtual void getConfig(QString &str);
	virtual void setConfig(const QString &strsearch);
	remote *Parent();
	htmlBinder *htmlBind;
	bool OnOff;
	virtual void switchOnOff(bool state);
	void createDevice(QString RomID);
    void newDevice(const QString &RomID);
    QMutex deviceRT, mutexFifoList;
	void setunID(const QString &ID);
	QString getUid();
	virtual QString getScratchPad(const QString&, int scratchpad_ID = 0);
    virtual bool checkDevConfig(const QString&, int config);
    virtual QString writeScratchPad(const QString&, int scratchpad_ID = 0);
    virtual QString writeScratchPad(const QString&, const QString&);
    QString ip2Hex(const QString &ip);
    QString getFifoString(const QString Order, const QString RomID, const QString Data);
    static QString IPtoHex(const QString &ip);
    static QString port2Hex(int port);
    static int getorder(QString &str);
    static void getDeviceList(const QString RomID, QStringList &DevList);
    static QString getOrder(const QString &str);
    static QString getData(const QString &str);
    static QString getRomID(const QString &str);
    virtual bool removeDeviceFromCatalog(onewiredevice *);
    void Lock(bool state);
private slots:
	void ShowDevice();
	void changeport();
	void tcpconnected();
	void searchbutton();
    void connecttcp(bool force = false);
	void voiralldevice();
    virtual void setipaddress();
    virtual void tcpdisconnected();
	virtual void tcpConnectionError(QAbstractSocket::SocketError socketError);
	virtual void reconnecttcp();
	virtual void fifonext();
    virtual void remoteCommand(const QString &);
	void switchOn();
	void switchOff();
	void clearfifo();
	void TcpStateChanged(QAbstractSocket::SocketState);
	void emitReqDone();
    void deviceReturn(const QString, const QString);
signals:
	void requestdone();
	void TcpStateConnected(net1wire*);
	void TcpStateDisonnected(net1wire*);
protected:
	void mousePressEvent(QMouseEvent *event);
	virtual void setrequest(const QString &req);
};

#endif


