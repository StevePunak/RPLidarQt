#include "blockinginterface.h"
#include "klog.h"

BlockingInterface::BlockingInterface(const QString& portName) :
    SerialInterface(portName, false),
    _portThread(nullptr)
{
    _portThread = new BlockingSerialThread(portName, this);
    connect(this, &BlockingInterface::readyWrite, _portThread, &BlockingSerialThread::handleReadyWrite);
    connect(_portThread, &BlockingSerialThread::readyRead, this, &BlockingInterface::handleReadyRead);
    KLog::sysLogText(KLOG_DEBUG, "Connected %p to %p", _portThread, this);
    _portThread->start();
    moveToThread(&_thread);
    _thread.start();
}


void BlockingInterface::send(QByteArray &data)
{
    emit readyWrite(data);
}

void BlockingInterface::handleReadyRead(QByteArray buffer)
{
    emit dataReady(buffer);
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


