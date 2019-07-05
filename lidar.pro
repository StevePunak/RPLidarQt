QT -= gui
QT += serialport network

CONFIG += c++11 console static
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH+=/home/spunak/pub/src/qt/KanoopCommon

#message($$QMAKESPEC)
#unix {
#    contains(QMAKESPEC, pi3)
#    {
#        LIBS=-L"/home/spunak/lib/arm" -lKanoopCommon
#    }
#    !contains(QMAKESPEC, pi3)
#    {
#        LIBS=-L"/home/spunak/lib/x86" -lKanoopCommon
#    }
#}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        asynchserialreader.cpp \
        blockingserialreader.cpp \
        datareader.cpp \
        filedatareader.cpp \
        main.cpp \
        lidarcommand.cpp \
        lidarprotocol.cpp \
        lidarresponse.cpp \
        lidarsample.cpp \
        lidartypes.cpp \
        lidarvector.cpp \
        cabin.cpp \
        lidar.cpp \
        bearingandrange.cpp \
        testthread.cpp \
        lidarserver.cpp \
        clientthread.cpp

LIBS += -L$(PLATFORM_LIBS)
LIBS += -lKanoopCommon

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /home/pi/lidar
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    asynchserialreader.h \
    blockingserialreader.h \
    datareader.h \
    filedatareader.h \
    lidarcommand.h \
    lidarprotocol.h \
    lidarresponse.h \
    lidarsample.h \
    lidartypes.h \
    lidarvector.h \
    cabin.h \
    lidar.h \
    bearingandrange.h \
    testthread.h \
    lidarserver.h \
    clientthread.h


