MODBUS_PATH = modbus

FORMS += $${MODBUS_PATH}/devmodbus.ui \
    $$PWD/Pdevmodbus.ui \
    $$PWD/modbus.ui


HEADERS += $${MODBUS_PATH}/modbus.h \
 $${MODBUS_PATH}/modbusthread.h \
 $${MODBUS_PATH}/devmodbus.h


SOURCES += $${MODBUS_PATH}/modbus.cpp \
 $${MODBUS_PATH}/modbusthread.cpp \
 $${MODBUS_PATH}/devmodbus.cpp
 
