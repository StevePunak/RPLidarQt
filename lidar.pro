QT -= gui
QT += serialport network

CONFIG += c++11 console static
CONFIG -= app_bundle

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

unix:
contains(CONFIG, cross_compile):{
        message("building for PI")
        LIBS += -L${HOME}/lib/arm
        LIBS += -Wl,-rpath=/usr/local/lib

    }else{
        message("Not building for PI")
        LIBS += -L${HOME}/lib/x86
    }

LIBS += -lKanoopCommon -lKanoopPiQt -lwiringPi

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


