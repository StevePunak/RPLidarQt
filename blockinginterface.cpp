#include "blockinginterface.h"
#include "klog.h"

BlockingInterface::BlockingInterface(const QString& portName, GPIO::Pin motorPin) :
    SerialInterface(portName, false, motorPin),
    _portThread(nullptr)
{
    _portThread = new BlockingSerialThread(portName, this);
    connect(this, &BlockingInterface::readyWrite, _portThread, &BlockingSerialThread::handleReadyWrite);
    connect(_portThread, &BlockingSerialThread::readyRead, this, &BlockingInterface::handleReadyRead);
    connect(_portThread, &BlockingSerialThread::portOpened, this, &BlockingInterface::handlePortOpened);
    connect(_portThread, &BlockingSerialThread::portClosed, this, &BlockingInterface::handlePortClosed);
    KLog::sysLogText(KLOG_DEBUG, "Connected %p to %p", _portThread, this);
    _portThread->start();
    moveToThread(&_thread);
    _thread.start();
}


void BlockingInterface::send(QByteArray &data)
{
    emit readyWrite(data);
}

void BlockingInterface::handleReadyRead(QDateTime timestamp, QByteArray buffer)
{
    emit dataReady(timestamp, buffer);
}

void BlockingInterface::handlePortOpened()
{
    KLog::sysLogText(KLOG_INFO, tr("Device %1 became open").arg(_portName));
    _deviceOpen = true;
    emit deviceOpened();
}

void BlockingInterface::handlePortClosed()
{
    KLog::sysLogText(KLOG_INFO, tr("Device %1 became closed").arg(_portName));
    _deviceOpen = false;
}

void BlockingInterface::startMotor()
{
    if(_portThread != nullptr)
        _portThread->startMotor();
}

void BlockingInterface::stopMotor()
{
    if(_portThread != nullptr)
        _portThread->stopMotor();
}


