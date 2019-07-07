#include <QCoreApplication>
#include <QFile>
#include <QSerialPort>
#include <QStringList>
#include <QTextStream>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QCommandLineParser>
#include <klog.h>

#include "lidar.h"
#include "testthread.h"
#include "lidarserver.h"
#include "pathutil.h"
#include "pigs.h"

int main(int argc, char *argv[])
{
    QString serialPort, pigsHost;
    quint16 listenPort = 0, pigsPort = 0;
    int motorPin = 0;

    QCoreApplication coreApplication(argc, argv);

    QTextStream standardOutput(stdout);

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription("RP Lidar Daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
        {{ "s", "serial-port"},
                      QCoreApplication::translate("main", "Serial port."),
                      QCoreApplication::translate("main", "serialPort") },
        {{ "l", "listen-port"},
                      QCoreApplication::translate("main", "Listen Port for lidar data server."),
                      QCoreApplication::translate("main", "listenPort") },
        {{ "m", "motor-control-pin"},
                      QCoreApplication::translate("main", "GPIO pin for lidar motor control"),
                      QCoreApplication::translate("main", "motorControl")},
        {{ "p", "pigs-address"},
                      QCoreApplication::translate("main", "Address and port for pigs gpio control."),
                      QCoreApplication::translate("main", "pigsAddress")},
                      });
    parser.process(coreApplication);

    if(parser.isSet("serial-port"))
        serialPort = parser.value("serial-port");
    else
    {
        standardOutput << "Serial port not set" << parser.helpText();
        exit(-1);
    }

    if(parser.isSet("pigs-address"))
    {
        QStringList parts = parser.value("pigs-address").split(':');
        if(parts.length() == 2)
        {
            bool ok;
            pigsHost = parts[0];
            pigsPort = parts[1].toUShort(&ok);
            if(ok == false)
            {
                standardOutput << "Illegal pigs address. Must be in address:port format" << endl;
                exit(-1);
            }
            Pigs::setAddress(pigsHost, pigsPort);
        }
        else
        {
            standardOutput << "Illegal pigs address. Must be in address:port format" << endl;
            exit(-1);
        }
    }

    if(parser.isSet("listen-port"))
    {
        bool success;
        listenPort = parser.value("listen-port").toInt(&success);
        if(!success)
        {
            qDebug() << "Illegal port number";
            exit(-1);
        }
    }

    if(parser.isSet("motor-control-pin"))
    {
        bool success;
        motorPin = parser.value("motor-control-pin").toInt(&success);
        if(success == false || motorPin < 1 || motorPin > 28)
        {
            qDebug() << "Illegal pin number";
            exit(-1);
        }
    }

    standardOutput << "Opening lidar on " << serialPort << endl;

    Lidar lidar(serialPort, .25, Lidar::BlockingSerial, listenPort, (GPIO::Pin)motorPin);
    if(lidar.GetDeviceInfo())
    {
        lidar.StartScan();
    }

    int result = coreApplication.exec();
    return result;
}
