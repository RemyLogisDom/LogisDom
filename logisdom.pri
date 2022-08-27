DESTDIR = $${BUILD_PATH}
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
#QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
unix:QMAKE_CXXFLAGS += -Wno-deprecated-copy
TEMPLATE = app
DEPENDPATH += $${QWT_PATH}
INCLUDEPATH += $${PWD} $${QWT_PATH}/src $${QUAZIP_PATH}/quazip $$[QT_INSTALL_HEADERS]/QtZlib
UI_DIR = $$OUT_PWD/build/ui
MOC_DIR = $$OUT_PWD/build/moc
RCC_DIR = $$OUT_PWD/build/rcc
OBJECTS_DIR = $$OUT_PWD/build/obj
TRANSLATIONS += trans/logisdom_fr.ts
QT += network core gui xml widgets serialport
CONFIG += qt qwt quazip zlib thread exceptions
RESOURCES += onewire.qrc
HEADERS += ../plugins/interface.h \
 addDaily.h \
 filesave.h \
 addicons.h \
 addProgram.h \
 alarmwarn.h \
 axb.h \
 backup.h \
 calcthread.h \
 calc.h \
 chauffageunit.h \
 histo.h \
 configmanager.h \
 configwindow.h \
 commonstring.h \
 connection.h \
 curve.h \
 dataloader.h \
 daily.h \
 deadevice.h \
 devchooser.h \
 devfinder.h \
 devrps2.h \
 devresol.h \
 devvirtual.h \
 formula.h \
 energiesolaire.h \
 errlog.h \
 global.h \
 globalvar.h \
 graph.h \
 graphconfig.h \
 highlighter.h \
 htmlbinder.h \
 iconearea.h \
 iconf.h \
 icont.h \
 inputdialog.h \
 interval.h \
 lcdonewire.h \
 ledonewire.h \
 linecoef.h \
 logisdom.h \
 mailsender.h \
 messagebox.h \
 meteo.h \
 net1wire.h \
 onewire.h \
 pieceslist.h \
 pngthread.h \
 plot.h \
 programevent.h \
 remote.h \
 reprocessthread.h \
 rps2.h \
 resol.h \
 server.h \
 sendmailthread.h \
 sendsmsthread.h \
 simplecrypt.h \
 tableau.h \
 tableauconfig.h \
 tcpdata.h \
 textedit.h \
 treehtmlwidget.h \
 vmc.h \
 weathercom.h \
 remotethread.h

FORMS += addProgram.ui \
 alarmwarn.ui \
 configgui.ui \
 devvirtual.ui \
 dailyui.ui \
 devfinder.ui \
 deadevice.ui \
 energiesolaire.ui \
 errlog.ui \
 formchauffage.ui \
 formula.ui \
 guinet1wire.ui \
 lcdonewire.ui \
 ledonewire.ui \
 mainw.ui \
 onewiredevice.ui \
 loadicon.ui \
 stdui.ui \
 switchui.ui \
 tableau.ui

SOURCES += addDaily.cpp \
 filesave.cpp \
 addicons.cpp \
 addProgram.cpp \
 alarmwarn.cpp \
 axb.cpp \
 backup.cpp \
 chauffageunit.cpp \
 histo.cpp \
 configmanager.cpp \
 configwindow.cpp \
 connection.cpp \
 commonstring.cpp \
 curve.cpp \
 daily.cpp \
 dataloader.cpp \
 devfinder.cpp \
 devrps2.cpp \
 devresol.cpp \
 devvirtual.cpp \
 deadevice.cpp \
 devchooser.cpp \
 energiesolaire.cpp \
 errlog.cpp \
 formula.cpp \
 calcthread.cpp \
 calc.cpp \
 graph.cpp \
 graphconfig.cpp \
 highlighter.cpp \
 htmlbinder.cpp \
 iconearea.cpp \
 iconf.cpp \
 icont.cpp \
 inputdialog.cpp \
 interval.cpp \
 lcdonewire.cpp \
 ledonewire.cpp \
 linecoef.cpp \
 mailsender.cpp \
 main.cpp \
 meteo.cpp \
 messagebox.cpp \
 logisdom.cpp \
 net1wire.cpp \
 onewire.cpp \
 pieceslist.cpp \
 pngthread.cpp \
 plot.cpp \
 programevent.cpp \
 remote.cpp \
 reprocessthread.cpp \
 rps2.cpp \
 resol.cpp \
 server.cpp \
 sendmailthread.cpp \
 sendsmsthread.cpp \
 simplecrypt.cpp \
 tableau.cpp \
 tableauconfig.cpp \
 tcpdata.cpp \
 textedit.cpp \
 treehtmlwidget.cpp \
 vmc.cpp \
 weathercom.cpp \
 remotethread.cpp
