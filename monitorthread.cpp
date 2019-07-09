#include "monitorthread.h"
#include "klog.h"

MonitorThread::MonitorThread(const QString& sourceName, qreal vectorSize, Lidar::ReaderType readerType, quint16 listenPort, GPIO::Pin motorPin) :
    _lidar(nullptr),
    _sourceName(sourceName),
    _vectorSize(vectorSize),
    _readerType(readerType),
    _listenPort(listenPort),
    _motorPin(motorPin)
{
    _thread.setObjectName("Monitor Thread");
    connect(&_thread, &QThread::started, this, &MonitorThread::handleThreadStarted);
    moveToThread(&_thread);
    _thread.start();
}

void MonitorThread::handleThreadStarted()
{
    _lidar = new Lidar(_sourceName, _vectorSize, Lidar::BlockingSerial, _listenPort, _motorPin);

    _timer = new QTimer();
    connect(_timer, &QTimer::timeout, this, &MonitorThread::handleTimerExpired);
    connect(_lidar, &Lidar::serialPortOpened, this, &MonitorThread::handleSerialPortOpened);

    _lidar->start();

    _timer->start(1000);
}

void MonitorThread::handleThreadStopped()
{

}

void MonitorThread::handleSerialPortOpened()
{
    _timer->start(3*1000);
}

void MonitorThread::handleTimerExpired()
{
    if(_lidar->deviceOpen())
    {
        if(_lidar->GetDeviceInfo())
        {
            _lidar->StartScan();
            _timer->stop();
        }
    }
    else
    {
        _timer->start(10*1000);
    }
}
