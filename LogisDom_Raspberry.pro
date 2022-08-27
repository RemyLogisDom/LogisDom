message(Project path : $$OUT_PWD)
message(Source path : $$PWD)

QWT_PATH = $${OUT_PWD}/qwt-6.2.0
message(QWT path : $$QWT_PATH)
LIBS += -lqwt -L$${QWT_PATH}/lib

QUAZIP_PATH = $${OUT_PWD}/quazip-0.7.3
message(Quazip path : $$QUAZIP_PATH)
LIBS *= -lquazip -L$${QUAZIP_PATH}/lib

QT_NO_DEBUG_OUTPUT

debug::BUILD_PATH = $${OUT_PWD}/debug
release::BUILD_PATH = $${OUT_PWD}/release

target.path = /home/pi
INSTALLS += target

TARGET = LogisDom
DEFINES += _TTY_POSIX_ POSIX

include (teleinfo/teleinfo.pri)
include (x10/x10.pri)
include (plcbus/plcbus.pri)
include (devonewire/devonewire.pri)
include (deveo/deveo.pri)
include (ha7s/ha7s.pri)
include (ha7net/ha7net.pri)
include (enocean/enocean.pri)
include (mbus/mbus.pri)
include (fts800/fts800.pri)
include (modbus/modbus.pri)
include (ecogest/ecogest.pri)
include (mail/mail.pri)
include (logisdom.pri)

