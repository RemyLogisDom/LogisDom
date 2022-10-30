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


#ifndef MBUSTHREAD_H
#define MBUSTHREAD_H

#include <QtCore>
#include <QThread>
#include <QTcpSocket>

class mbusthread : public QThread
{
    Q_OBJECT
public:

struct device
{
    int Address;
    QString RomID;
    QString Scratchpad;
    QString Return;
};
private:
struct FIFOStruc
{
    QString RomID;
    QString Request;
    int Request_ID;
    int device_ID;
    int scratchpad_ID;
};
//
// Packet formats:
//
// ACK: size = 1 byte
//
//   byte1: ack   = 0xE5
//
// SHORT: size = 5 byte
//
//   byte1: start   = 0x10
//   byte2: control = ...
//   byte3: address = ...
//   byte4: chksum  = ...
//   byte5: stop    = 0x16
//
// CONTROL: size = 9 byte
//
//   byte1: start1  = 0x68
//   byte2: length1 = ...
//   byte3: length2 = ...
//   byte4: start2  = 0x68
//   byte5: control = ...
//   byte6: address = ...
//   byte7: ctl.info= ...
//   byte8: chksum  = ...
//   byte9: stop    = 0x16
//
// LONG: size = N >= 9 byte
//
//   byte1: start1  = 0x68
//   byte2: length1 = ...
//   byte3: length2 = ...
//   byte4: start2  = 0x68
//   byte5: control = ...
//   byte6: address = ...
//   byte7: ctl.info= ...
//   byte8: chksum  = ...
//   byte9: data    = ...
//          ...     = ...
//   byteN: stop    = 0x16
//
//
//

struct mbus_frame {

    quint8 start1;
    quint8 length1;
    quint8 length2;
    quint8 start2;
    quint8 control;
    quint8 address;
    quint8 control_information;
    // variable data field
    quint8 checksum;
    quint8 stop;

    quint8   data[252];
    size_t data_size;

    int type;    //mbus_frame_data frame_data;
};


typedef struct _mbus_slave_data {

    int state_fcb;
    int state_acd;

} mbus_slave_data;

#define vifeMax 10
#define difeMax 10

//------------------------------------------------------------------------------
// MBUS FRAME DATA FORMATS
//

// DATA RECORDS
static const char MBUS_DIB_DIF_EXTENSION_BIT = 0x80;
static const char MBUS_DIB_VIF_EXTENSION_BIT = 0x80;

typedef struct _mbus_data_information_block {

    quint8 dif;
    quint8 dife[10];
    qreal value;
    size_t  ndife;

} mbus_data_information_block;

typedef struct _mbus_value_information_block {

    quint8 vif;
    quint8 vife[10];
    qreal coef;
    QString unit;
    size_t  nvife;

} mbus_value_information_block;

typedef struct _mbus_data_record_header {

    mbus_data_information_block  dib;
    mbus_value_information_block vib;

} mbus_data_record_header;

typedef struct _mbus_data_record {
    mbus_data_record_header drh;
    quint8 data[234];
    size_t data_len;
} mbus_data_record;

//
// HEADER FOR VARIABLE LENGTH DATA FORMAT
//
typedef struct _mbus_data_variable_header {

    //Ident.Nr. Manufr. Version Medium Access No. Status  Signature
    //4 Byte    2 Byte  1 Byte  1 Byte   1 Byte   1 Byte  2 Byte

    // ex
    // 88 63 80 09 82 4D 02 04 15 00 00 00

    quint8 id_bcd[4];         // 88 63 80 09
    quint8 manufacturer[2];   // 82 4D
    quint8 version;           // 02
    quint8 medium;            // 04
    quint8 access_no;         // 15
    quint8 status;            // 00
    quint8 signature[2];      // 00 00

} mbus_data_variable_header;

//
// VARIABLE LENGTH DATA FORMAT
//
typedef struct _mbus_data_variable {

    mbus_data_variable_header header;
    QList <mbus_data_record*> records;
    quint8 *data;
    size_t  data_len;
    // are these needed/used?
    quint8  mdh;
    quint8 *mfg_data;
    size_t  mfg_data_len;

} mbus_data_variable;

//
// FIXED LENGTH DATA FORMAT
//
typedef struct _mbus_data_fixed {

    // ex
    // 35 01 00 00 counter 2 = 135 l (historic value)
    //
    // e.g.
    //
    // 78 56 34 12 identification number = 12345678
    // 0A          transmission counter = 0Ah = 10d
    // 00          status 00h: counters coded BCD, actual values, no errors
    // E9 7E       Type&Unit: medium water, unit1 = 1l, unit2 = 1l (same, but historic)
    // 01 00 00 00 counter 1 = 1l (actual value)
    // 35 01 00 00 counter 2 = 135 l (historic value)

    quint8 id_bcd[4];
    quint8 tx_cnt;
    quint8 status;
    quint8 cnt1_type;
    quint8 cnt2_type;
    quint8 cnt1_val[4];
    quint8 cnt2_val[4];

} mbus_data_fixed;

//
// ABSTRACT DATA FORMAT (either fixed or variable length)
//
static const char MBUS_DATA_TYPE_FIXED		= 1;
static const char MBUS_DATA_TYPE_VARIABLE	= 2;
typedef struct _mbus_frame_data {

    mbus_data_variable data_var;
    mbus_data_fixed    data_fix;

    int type;

} mbus_frame_data;


//------------------------------------------------------------------------------
// FRAME types
//
static const char MBUS_FRAME_TYPE_ANY		= 0x00;
static const char MBUS_FRAME_TYPE_ACK		= 0x01;
static const char MBUS_FRAME_TYPE_SHORT		= 0x02;
static const char MBUS_FRAME_TYPE_CONTROL	= 0x03;
static const char MBUS_FRAME_TYPE_LONG		= 0x04;

static const char MBUS_FRAME_ACK_BASE_SIZE       = 1;
static const char MBUS_FRAME_SHORT_BASE_SIZE     = 5;
static const char MBUS_FRAME_CONTROL_BASE_SIZE   = 9;
static const char MBUS_FRAME_LONG_BASE_SIZE      = 9;

static const char MBUS_FRAME_BASE_SIZE_ACK      = 1;
static const char MBUS_FRAME_BASE_SIZE_SHORT    = 5;
static const char MBUS_FRAME_BASE_SIZE_CONTROL  = 9;
static const char MBUS_FRAME_BASE_SIZE_LONG     = 9;

static const char MBUS_FRAME_FIXED_SIZE_ACK     = 1;
static const char MBUS_FRAME_FIXED_SIZE_SHORT   = 5;
static const char MBUS_FRAME_FIXED_SIZE_CONTROL = 6;
static const char MBUS_FRAME_FIXED_SIZE_LONG    = 6;

static const char MBUS_FRAME_ACK_START		= 0xE5;
static const char MBUS_FRAME_SHORT_START	= 0x10;
static const char MBUS_FRAME_CONTROL_START	= 0x68;
static const char MBUS_FRAME_LONG_START		= 0x68;
static const char MBUS_FRAME_STOP		= 0x16;

#define MBUS_MAX_PRIMARY_SLAVES	= 256;
#define MBUS_DIB_DIF_WITHOUT_EXTENSION 0x7F
#define MBUS_DIB_DIF_EXTENSION_BIT 0x80
#define MBUS_DIB_VIF_WITHOUT_EXTENSION 0x7F
#define MBUS_DIB_VIF_EXTENSION_BIT 0x80
#define MBUS_DIB_DIF_MANUFACTURER_SPECIFIC 0x0F
#define MBUS_DIB_DIF_MORE_RECORDS_FOLLOW 0x1F
#define MBUS_DIB_DIF_IDLE_FILLER 0x2F

#define MBUS_DATA_RECORD_DIF_MASK_DATA 0x0F
#define MBUS_DATA_RECORD_DIF_MASK_FUNCTION 0x30
#define MBUS_DATA_RECORD_DIF_MASK_NON_DATA 0xF0

#define MBUS_DATA_RECORD_DIFE_MASK_STORAGE_NO 0x0F
#define MBUS_DATA_RECORD_DIFE_MASK_TARIFF 0x30
#define MBUS_DATA_RECORD_DIFE_MASK_DEVICE 0x40
#define MBUS_DATA_RECORD_DIFE_MASK_EXTENSION 0x80

static const char MBUS_CONTROL_FIELD_DIRECTION	= 0x07;
static const char MBUS_CONTROL_FIELD_FCB	= 0x06;
static const char MBUS_CONTROL_FIELD_ACD	= 0x06;
static const char MBUS_CONTROL_FIELD_FCV	= 0x05;
static const char MBUS_CONTROL_FIELD_DFC	= 0x05;
static const char MBUS_CONTROL_FIELD_F3		= 0x04;
static const char MBUS_CONTROL_FIELD_F2		= 0x03;
static const char MBUS_CONTROL_FIELD_F1		= 0x02;
static const char MBUS_CONTROL_FIELD_F0		= 0x01;

static const char MBUS_CONTROL_MASK_SND_NKE	= 0x40;
static const char MBUS_CONTROL_MASK_SND_UD	= 0x53;
static const char MBUS_CONTROL_MASK_REQ_UD2	= 0x5B;
static const char MBUS_CONTROL_MASK_REQ_UD1	= 0x5A;
static const char MBUS_CONTROL_MASK_RSP_UD	= 0x08;

static const char MBUS_CONTROL_MASK_FCB		= 0x20;
static const char MBUS_CONTROL_MASK_FCV		= 0x10;

static const char MBUS_CONTROL_MASK_ACD		= 0x20;
static const char MBUS_CONTROL_MASK_DFC		= 0x10;

static const char MBUS_CONTROL_MASK_DIR		= 0x40;
static const char MBUS_CONTROL_MASK_DIR_M2S	= 0x40;
static const char MBUS_CONTROL_MASK_DIR_S2M	= 0x00;

//
// Address field
//
static const char MBUS_ADDRESS_BROADCAST_REPLY		= 0xFE;
static const char MBUS_ADDRESS_BROADCAST_NOREPLY	= 0xFF;
static const char MBUS_ADDRESS_NETWORK_LAYER		= 0xFD;

//
// Control Information field
//
//Mode 1 Mode 2                   Application                   Definition in
// 51h    55h                       data send                    EN1434-3
// 52h    56h                  selection of slaves           Usergroup July  Ì93
// 50h                          application reset           Usergroup March  Ì94
// 54h                          synronize action                 suggestion
// B8h                     set baudrate to 300 baud          Usergroup July  Ì93
// B9h                     set baudrate to 600 baud          Usergroup July  Ì93
// BAh                    set baudrate to 1200 baud          Usergroup July  Ì93
// BBh                    set baudrate to 2400 baud          Usergroup July  Ì93
// BCh                    set baudrate to 4800 baud          Usergroup July  Ì93
// BDh                    set baudrate to 9600 baud          Usergroup July  Ì93
// BEh                   set baudrate to 19200 baud              suggestion
// BFh                   set baudrate to 38400 baud              suggestion
// B1h           request readout of complete RAM content     Techem suggestion
// B2h          send user data (not standardized RAM write) Techem suggestion
// B3h                 initialize test calibration mode      Usergroup July  Ì93
// B4h                           EEPROM read                 Techem suggestion
// B6h                         start software test           Techem suggestion
// 90h to 97h              codes used for hashing           longer recommended

static const char MBUS_CONTROL_INFO_DATA_SEND		= 0x51;
static const char MBUS_CONTROL_INFO_DATA_SEND_MSB	= 0x55;
static const char MBUS_CONTROL_INFO_SELECT_SLAVE	= 0x52;
static const char MBUS_CONTROL_INFO_SELECT_SLAVE_MSB	= 0x56;
static const char MBUS_CONTROL_INFO_APPLICATION_RESET	= 0x50;
static const char MBUS_CONTROL_INFO_SYNC_ACTION		= 0x54;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_300	= 0xB8;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_600	= 0xB9;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_1200	= 0xBA;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_2400	= 0xBB;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_4800	= 0xBC;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_9600	= 0xBD;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_19200	= 0xBE;
static const char MBUS_CONTROL_INFO_SET_BAUDRATE_38400	= 0xBF;
static const char MBUS_CONTROL_INFO_REQUEST_RAM_READ	= 0xB1;
static const char MBUS_CONTROL_INFO_SEND_USER_DATA	= 0xB2;
static const char MBUS_CONTROL_INFO_INIT_TEST_CALIB	= 0xB3;
static const char MBUS_CONTROL_INFO_EEPROM_READ		= 0xB4;
static const char MBUS_CONTROL_INFO_SW_TEST_START	= 0xB6;

static const char MBUS_CONTROL_INFO_RESP_FIXED		= 0x73;
static const char MBUS_CONTROL_INFO_RESP_FIXED_MSB	= 0x77;

static const char MBUS_CONTROL_INFO_RESP_VARIABLE	= 0x72;
static const char MBUS_CONTROL_INFO_RESP_VARIABLE_MSB	= 0x73;

static const char MBUS_DATA_FIXED_STATUS_FORMAT_MASK	= 0x80;
static const char MBUS_DATA_FIXED_STATUS_FORMAT_BCD	= 0x00;
static const char MBUS_DATA_FIXED_STATUS_FORMAT_INT	= 0x80;
static const char MBUS_DATA_FIXED_STATUS_DATE_MASK	= 0x40;
static const char MBUS_DATA_FIXED_STATUS_DATE_STORED	= 0x40;
static const char MBUS_DATA_FIXED_STATUS_DATE_CURRENT	= 0x00;

static const char MBUS_DATA_RECORD_DIF_MASK_INST	= 0x00;
static const char MBUS_DATA_RECORD_DIF_MASK_MIN		= 0x10;

static const char MBUS_DATA_RECORD_DIF_MASK_TYPE_INT32	= 0x04;
static const char MBUS_DATA_RECORD_DIF_MASK_STORAGE_NO	= 0x40;
static const char MBUS_DATA_RECORD_DIF_MASK_EXTENTION	= 0x80;

static const char MBUS_VARIABLE_DATA_MEDIUM_OTHER	= 0x00;
static const char MBUS_VARIABLE_DATA_MEDIUM_OIL		= 0x01;
static const char MBUS_VARIABLE_DATA_MEDIUM_ELECTRICITY	= 0x02;
static const char MBUS_VARIABLE_DATA_MEDIUM_GAS		= 0x03;
static const char MBUS_VARIABLE_DATA_MEDIUM_HEAT	= 0x04;
static const char MBUS_VARIABLE_DATA_MEDIUM_STEAM	= 0x05;
static const char MBUS_VARIABLE_DATA_MEDIUM_HOT_WATER	= 0x06;
static const char MBUS_VARIABLE_DATA_MEDIUM_WATER	= 0x07;
static const char MBUS_VARIABLE_DATA_MEDIUM_HEAT_COST	= 0x08;
static const char MBUS_VARIABLE_DATA_MEDIUM_COMPR_AIR	= 0x09;
static const char MBUS_VARIABLE_DATA_MEDIUM_COOL_OUT	= 0x0A;
static const char MBUS_VARIABLE_DATA_MEDIUM_COOL_IN	= 0x0B;
static const char MBUS_VARIABLE_DATA_MEDIUM_BUS		= 0x0E;
static const char MBUS_VARIABLE_DATA_MEDIUM_COLD_WATER	= 0x16;
static const char MBUS_VARIABLE_DATA_MEDIUM_DUAL_WATER	= 0x17;
static const char MBUS_VARIABLE_DATA_MEDIUM_PRESSURE	= 0x18;
static const char MBUS_VARIABLE_DATA_MEDIUM_ADC		= 0x19;

#define MBusReadLoop\
    if (!adrToRead.isEmpty())\
    {\
        while (!adrToRead.isEmpty())\
        {\
            bool ok;\
            QString Adr = adrToRead.takeFirst();\
            int adr = Adr.remove("Adr ").toInt(&ok);\
            if (ok) readMbus(MySocket, adr);\
            else str = tr("Can't Read : ") + Adr + QDateTime::currentDateTime().toString(" hh:mm");\
        }\
        str = tr("Read Device finished ") + QDateTime::currentDateTime().toString(" hh:mm");\
        emit(ReadingDevDone());\
        str += tr(" and Wait till minute %1").arg(minute);\
        emit(whatdoUDo(str));\
    }\
    if (QDateTime::currentDateTime().time().minute() != minuteCheckDev) { minuteCheckDev = QDateTime::currentDateTime().time().minute(); emit(CheckDev()); } sleep(1);

public:
    mbusthread();
    ~mbusthread();
    void run();
    QString moduleipaddress;
    int port;
    QString log;
    bool logEnabled;
    bool endLessLoop;
    QAbstractSocket::SocketState tcpStatus;
    QMutex mutexData;
    int searchMax;
    bool searchDone;
    int readInterval;
    QStringList devices;
    QStringList adrToRead;
private:
    void get(QTcpSocket &Socket, mbus_frame &frame, QByteArray &Data, int timeout = 30000);
    void TCPconnect(QTcpSocket &Socket, QString &Status);
    void readMbus(QTcpSocket &Socket, int adr);
    void searchMbus(QTcpSocket &Socket);
    void mbus_setType(mbus_frame &frame, int frame_type);
    int mbus_frame_pack(mbus_frame &frame, QByteArray &data);
    int mbus_frame_calc_length(mbus_frame &frame);
    quint8 calc_length(const mbus_frame &frame);
    int mbus_frame_calc_checksum(mbus_frame &frame);
    quint8 calc_checksum(mbus_frame &frame);
    int mbus_frame_data_parse(mbus_frame &frame, mbus_frame_data &data, int adr);
    int mbus_data_variable_parse(mbus_frame &frame, mbus_data_variable &data, int adr);
    int mbus_data_fixed_parse(mbus_frame &frame, mbus_data_fixed &data, int adr);
    int mbus_parse(mbus_frame &frame, QByteArray &data);
    int mbus_frame_verify(mbus_frame &frame);
    void mbus_data_variable_print(mbus_data_variable &data, int adr);
    void mbus_data_variable_header_print(mbus_data_variable_header &header, int adr);
    unsigned char mbus_dif_datalength_lookup(unsigned char dif);
    qint32 mbus_data_bcd_decode(quint8 *bcd_data, size_t bcd_data_size);
    QString mbus_data_variable_medium_lookup(quint8 medium);
    QString mbus_data_fixed_medium(mbus_data_fixed &data);
    QString mbus_data_fixed_unit(int medium_unit_byte);
    QString mbus_unit_prefix(int exp);
    QString mbus_vif_unit_lookup(quint8 vif);
    QString mbus_vib_unit_lookup(mbus_value_information_block &vib);
    void mbus_dif_value_lookup(mbus_data_information_block &dib);
    const quint8 *mbus_decode_manufacturer(quint8 byte1, quint8 byte2);
    QString mbus_data_type(mbus_data_record *record);
    void mbus_data_str_type(QString &dst, const quint8 *src);
    QString mbus_data_record_decode(mbus_data_record *record);
    quint32 mbus_data_int_decode(quint8 *int_data, size_t int_data_size);
    qreal mbus_data_float_decode(quint8 *int_data, size_t int_data_size);
    QDateTime mbus_data_F_Format(quint8 *int_data);
    QDate mbus_data_G_Format(quint8 *int_data);
    quint64 mbus_data_long_decode(quint8 *int_data, size_t int_data_size);
    void mbus_data_str_decode(QString &dst, mbus_data_record *record);
    void addTreeItem(int adr, const QString &parameter, const QString &value, const QString &comment);
    void addTreeItem(int adr, int record, const QString &parameter, const QString &value, const QString &comment, bool mainTreeItem = false);
public slots:
    void appendAdr(const QString &str);
signals:
    void newTreeAddress(int);
    void setTreeItem(const QString&);
    void setMainTreeItem(const QString&);
    void tcpStatusUpdate(const QString&);
    void tcpStatusChange();
    void whatdoUDo(const QString&);
    void ReadingDone();
    void ReadingDevDone();
    void CheckDev();
};

#endif // MBUSTHREAD_H
