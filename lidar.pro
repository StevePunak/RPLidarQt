QT -= gui
QT += serialport network

CONFIG += c++11 console static
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH+=$$(HOME)/src/KanoopCommonQt
INCLUDEPATH+=$$(HOME)/src/KanoopPiQt

SOURCES += \
        asynchinterface.cpp \
        blockinginterface.cpp \
        blockingserialthread.cpp \
        deviceinterface.cpp \
        filedatareader.cpp \
        generictest.cpp \
        lidardaemon.cpp \
        main.cpp \
        lidarcommand.cpp \
        lidarprotocol.cpp \
        lidarresponse.cpp \
        lidarsample.cpp \
        lidartypes.cpp \
        cabin.cpp \
        lidar.cpp \
        bearingandrange.cpp \
        monitorthread.cpp \
        rangemap.cpp \
        serialinterface.cpp \
        testthread.cpp \
        lidarserver.cpp \
        clientthread.cpp

LIBS += -L$(PLATFORM_LIBS)
LIBS += -lKanoopCommon -lKanoopPiQt

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/pi/lidar
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    asynchinterface.h \
    blockinginterface.h \
    blockingserialthread.h \
    deviceinterface.h \
    fileinterface.h \
    generictest.h \
    lidarcommand.h \
    lidardaemon.h \
    lidarprotocol.h \
    lidarresponse.h \
    lidarsample.h \
    lidartypes.h \
    lidarvector.h \
    cabin.h \
    lidar.h \
    bearingandrange.h \
    monitorthread.h \
    rangemap.h \
    serialinterface.h \
    testthread.h \
    lidarserver.h \
    clientthread.h


