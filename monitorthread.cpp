#include "monitorthread.h"
#include "klog.h"

MonitorThread::MonitorThread(const QString& sourceName, qreal vectorSize, Lidar::ReaderType readerType, quint16 listenPort, GPIO::Pin motorPin) :
    _lidar(nullptr),
    _server(nullptr),
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
    _server = new LidarServer(_listenPort);

    connect(_lidar, &Lidar::scanComplete, _server, &LidarServer::handleScanReady);

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
    if(_lidar->isDeviceOpen() &&QDateTime::currentDateTimeUtc() > _lidar->lastScanCompletion().addSecs(10) && _server->clientCount() > 0)
    {
        KLog::sysLogText(KLOG_INFO, tr("Attempting to start lidar with %1 clients").arg(_server->clientCount()));
        if(_lidar->getDeviceInfo())
        {
            _lidar->startMotor();
            _lidar->startScan();
        }
    }
    else if(_server->clientCount() < 1)
    {
        _lidar->stopMotor();
    }
}

void MonitorThread::stop()
{
    _thread.quit();
    if(_lidar != nullptr)
        _lidar->stop();
}
