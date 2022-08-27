MBUS_PATH = mbus

FORMS += $${MBUS_PATH}/mbus.ui


HEADERS += $${MBUS_PATH}/mbus.h \
 $${MBUS_PATH}/mbusthread.h \
 $${MBUS_PATH}/devmbus.h

SOURCES += $${MBUS_PATH}/mbus.cpp \
 $${MBUS_PATH}/mbusthread.cpp \
 $${MBUS_PATH}/devmbus.cpp
