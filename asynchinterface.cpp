#define DEBUG_SERIAL
#undef SERIAL_TRACE
#include <QDateTime>
#include "asynchinterface.h"
#include "klog.h"

AsynchInterface::AsynchInterface(const QString& portName, GPIO::Pin motorPin) :
    SerialInterface(portName, true, motorPin),
    _timer(nullptr),
    _bufferBytes(4096),
    _bufferMsecs(5000),
    _lastDeliveryMsecs(0)
{
    _timer = new QTimer();

    connect(_serialPort, &QSerialPort::readyRead, this, &AsynchInterface::handleReadyRead);
    connect(_serialPort, &QSerialPort::errorOccurred, this, &AsynchInterface::handleError);
    connect(this, &AsynchInterface::readyWrite, this, &AsynchInterface::handleReadyWrite);
    connect(_timer, &QTimer::timeout, this, &AsynchInterface::handleTimeout);
    connect(&_thread, &QThread::started, this, &AsynchInterface::handleThreadStart);

    moveToThread(&_thread);
    _serialPort->moveToThread(&_thread);
    _timer->moveToThread(&_thread);
    _thread.start();
}

void AsynchInterface::send(QByteArray &data)
{
    emit readyWrite(data);
}

void AsynchInterface::deliverData()
{
    _lastDeliveryMsecs = QDateTime::currentMSecsSinceEpoch();
    emit dataReady(_recvBuffer);
    _recvBuffer.clear();
    _recvBuffer.reserve(65536);
}

void AsynchInterface::handleReadyRead()
{
    QByteArray data = _serialPort->readAll();

    _byteTotal += data.length();

#ifdef SERIAL_TRACE
    _dumpOutputFile.write(data);
    _dumpOutputFile.flush();
#endif

    _recvBuffer.append(data);

    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if(now > _lastDeliveryMsecs + _bufferMsecs || _recvBuffer.length() > _bufferBytes)
    {
        KLog::sysLogText(KLOG_DEBUG, "Delivering %d bytes %lld > %lld + %d || %d > %d",
                         _recvBuffer.length(), QDateTime::currentMSecsSinceEpoch(), _lastDeliveryMsecs, _bufferMsecs,
                         _recvBuffer.length(), _bufferBytes);
        deliverData();
    }
    _timer->start(1000);
}

void AsynchInterface::handleTimeout()
{
    if(_recvBuffer.length() > 0)
    {
        KLog::sysLogText(KLOG_DEBUG, "Timeout with %d bytes", _recvBuffer.length());
        deliverData();
    }
    _timer->start(1000);
}

void AsynchInterface::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError)
    {
        KLog::sysLogText(KLOG_WARNING, tr("Serial port error %1")
                         .at(serialPortError));
    }
}

void AsynchInterface::handleReadyWrite(QByteArray buffer)
{
    KLog::sysLogText(KLOG_INFO, tr("Write %1 bytes").arg(buffer.length()));
    KLog::sysLogHex(buffer);
    _serialPort->write(buffer);
}

void AsynchInterface::handleThreadStart()
{
    _timer->start(1000);
}

