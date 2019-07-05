#include <QDateTime>
#include "asynchserialreader.h"
#include "klog.h"

AsynchSerialReader::AsynchSerialReader(const QString& portName) :
    SerialPortReader(portName),
    _timer(nullptr),
    _bufferBytes(4096),
    _bufferMsecs(5000),
    _lastDeliveryMsecs(0)
{
    _timer = new QTimer();

    connect(_serialPort, &QSerialPort::readyRead, this, &AsynchSerialReader::handleReadyRead);
    connect(_serialPort, &QSerialPort::errorOccurred, this, &AsynchSerialReader::handleError);
    connect(_timer, &QTimer::timeout, this, &AsynchSerialReader::handleTimeout);

    _timer->start(1000);
}

void AsynchSerialReader::send(QByteArray &data)
{
    _serialPort->write(data);
}

void AsynchSerialReader::deliverData()
{
    emit dataReady(_recvBuffer);
    _recvBuffer.clear();
    _recvBuffer.reserve(65536);
}

void AsynchSerialReader::handleReadyRead()
{
    QByteArray data = _serialPort->readAll();

    _byteTotal += data.length();

#ifdef DEBUG_SERIAL2
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

void AsynchSerialReader::handleTimeout()
{
    if(_recvBuffer.length() > 0)
    {
        KLog::sysLogText(KLOG_DEBUG, "Timeout with %d bytes", _recvBuffer.length());
        deliverData();
    }
    _timer->start(1000);
}

void AsynchSerialReader::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError)
    {
        KLog::sysLogText(KLOG_WARNING, tr("Serial port error %1")
                         .at(serialPortError));
    }
}

void AsynchSerialReader::readyWrite()
{
//    _serialPort->write()
}

