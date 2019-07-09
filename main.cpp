#include <QCoreApplication>
#include <QFile>
#include <QSerialPort>
#include <QStringList>
#include <QTextStream>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QCommandLineParser>
#include <QSettings>

#include "klog.h"
#include "lidar.h"
#include "monitorthread.h"
#include "testthread.h"
#include "lidarserver.h"
#include "pathutil.h"
#include "pigs.h"
#include "addresshelper.h"

int main(int argc, char *argv[])
{
    QString serialPort, pigsHost;
    quint16 listenPort = 0, pigsPort = 0;
    int motorPin = 0;
    int verbosity = 0;

    // default location
    QString confFile = "/etc/lidar/lidar.conf";
    QString logFile = "/tmp/lidar.log";

    // keys for arguments / configuration file
    QString keyConfFile = "conf-file";
    QString keyMotorControlPin = "motor-control-pin";
    QString keyListenPort = "listen-port";
    QString keyPigsAddress = "pigs-address";
    QString keyLogFile = "log-file";
    QString keySerialPort = "serial-port";
    QString keyVerbosity = "verbosity";

    QCoreApplication coreApplication(argc, argv);

    QTextStream standardOutput(stdout);

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription("RP Lidar Daemon");
    parser.addHelpOption();

    parser.addOptions({
                          {{ "p", keyPigsAddress},
                              QCoreApplication::translate("main", "Address and port for pigs gpio control."),
                              QCoreApplication::translate("main", "pigsAddress")},
                          {{ "v", keyVerbosity},
                              QCoreApplication::translate("main", "Logging output verbosity"),
                              QCoreApplication::translate("main", "verbosity")},
                          {{ "c", keyConfFile},
                              QCoreApplication::translate("main", "Configuration File"),
                              QCoreApplication::translate("main", "configurationFile") },
                          {{ "s", keySerialPort},
                              QCoreApplication::translate("main", "Serial port."),
                              QCoreApplication::translate("main", "serialPort") },
                          {{ "l", keyListenPort},
                              QCoreApplication::translate("main", "Listen Port for lidar data server."),
                              QCoreApplication::translate("main", "listenPort") },
                          {{ "o", keyLogFile},
                              QCoreApplication::translate("main", "Log file for output"),
                              QCoreApplication::translate("main", "logFile") },
                          {{ "m", keyMotorControlPin},
                              QCoreApplication::translate("main", "GPIO pin for lidar motor control"),
                              QCoreApplication::translate("main", "motorControl")},
    });
    parser.process(coreApplication);

    QStringList names = parser.optionNames();

    // configuration file location
    if(parser.isSet(keyConfFile))
    {
        confFile = parser.value(keyConfFile);
        if(QFile::exists(confFile) == false)
        {
            standardOutput << "Configuration file at " << confFile << " does not exist";
            exit(-1);
        }
    }

    // load the configuration file if it exists
    QSettings settings(confFile, QSettings::Format::IniFormat);
    if(QFile::exists(confFile))
    {
        motorPin = settings.value("Main/" + keyMotorControlPin).toInt();
        serialPort = settings.value("Main/" + keySerialPort).toString();
        listenPort = settings.value("Main/" + keyListenPort).toInt();
        logFile = settings.value("Main/" + keyLogFile).toString();
        verbosity = settings.value("Main/" + keyVerbosity, 0).toInt();
        QString pigsAddress = settings.value("Main/" + keyPigsAddress).toString();
        AddressHelper::tryParseAddressPort(pigsAddress, pigsHost, pigsPort);
    }

    if(parser.isSet("serial-port"))
        serialPort = parser.value(keySerialPort);
    else if(serialPort.isEmpty())
    {
        standardOutput << "Serial port not set" << parser.helpText();
        exit(-1);
    }

    // get address for pigs gpio server
    if(parser.isSet(keyPigsAddress))
    {
        QStringList parts = parser.value(keyPigsAddress).split(':');
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
    else if(pigsHost.isEmpty() == false)
    {
        Pigs::setAddress(pigsHost, pigsPort);
    }

    // TCP port number for serving lidar data
    if(parser.isSet(keyListenPort))
    {
        bool success;
        listenPort = parser.value(keyListenPort).toInt(&success);
        if(!success)
        {
            qDebug() << "Illegal port number";
            exit(-1);
        }
    }

    // gpio pin for motor enable
    if(parser.isSet(keyMotorControlPin))
    {
        bool success;
        motorPin = parser.value(keyMotorControlPin).toInt(&success);
        if(success == false || motorPin < 1 || motorPin > 28)
        {
            qDebug() << "Illegal pin number";
            exit(-1);
        }
    }

    // Verbosity
    if(parser.isSet(keyVerbosity))
    {
        bool success;
        listenPort = parser.value(keyVerbosity).toInt(&success);
        if(success == false || verbosity < 0 || verbosity > 10)
        {
            qDebug() << "Illegal verbosity level. Must be between 0 and 10";
            exit(-1);
        }
    }

    standardOutput << "Opening lidar on " << serialPort << " and logging to " << logFile << endl;

    KLog::setSystemLogFile(logFile);
    KLog::setSystemVerbosity(verbosity);
    KLog::setSystemOutputFlags((KLog::OutputFlags)(KLog::systemOutputFlags() & ~KLog::OutputFlags::Console));
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, "");
    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));
    KLog::sysLogText(KLOG_INFO, QString("RPLidar TCP daemon started at %1").arg(QDateTime::currentDateTimeUtc().toString()));
    KLog::sysLogText(KLOG_INFO, QString("-----------------------------------------------------------------------------------"));

    MonitorThread monitor(serialPort, .25, Lidar::BlockingSerial, listenPort, (GPIO::Pin)motorPin);

    int result = coreApplication.exec();
    return result;
}
