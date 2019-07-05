#include <QCoreApplication>
#include <QFile>
#include <QSerialPort>
#include <QStringList>
#include <QTextStream>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <klog.h>

#include "lidar.h"
#include "testthread.h"
#include "lidarserver.h"
#include "pathutil.h"

int main(int argc, char *argv[])
{

    QCoreApplication coreApplication(argc, argv);
    const int argumentCount = QCoreApplication::arguments().size();
    const QStringList argumentList = QCoreApplication::arguments();

    KLog::sysLogText(KLOG_DEBUG, "Hey there %s", "you asshole");

    QTextStream standardOutput(stdout);

    if (argumentCount == 1) {
        standardOutput << QObject::tr("Usage: %1 <serialportname> [baudrate]")
                          .arg(argumentList.first()) << endl;
        return 1;
    }

    const QString serialPortName = argumentList.at(1);
    standardOutput << "Opening lidar on " << serialPortName << endl;

    Lidar lidar(serialPortName, .25, Lidar::BlockingSerial);
    if(lidar.GetDeviceInfo())
    {
        lidar.StartScan();
    }

    int result = coreApplication.exec();
    return result;
}
