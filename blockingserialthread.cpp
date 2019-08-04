#include "blockingserialthread.h"
#include "blockinginterface.h"
#include "klog.h"
#include "pigs.h"

BlockingSerialThread::BlockingSerialThread(const QString& portName, BlockingInterface* parent) :
    QThread(),
    _portName(portName),
    _parent(parent),
    _quit(false),
    _wasOpen(false)
{
    setObjectName("LidarBlockingSerial");
    qRegisterMetaType<QSerialPort::SerialPortError>();
}

void BlockingSerialThread::run()
{
    while(true)
    {
        if(_serialPort == nullptr || _serialPort->isOpen() == false)
        {
            if(_wasOpen)
            {
                emit portClosed();
                _wasOpen = false;
            }
            initializeSerialPort();
            if(_serialPort == nullptr || _serialPort->isOpen() == false)
            {
                sleep(10);
                continue;
            }
            else
            {
                if(_wasOpen == false)
                {
                    emit portOpened();
                    _wasOpen = true;
                }
            }
        }

        if(_serialPort->isOpen() == true && _serialPort->error() != QSerialPort::SerialPortError::NoError && _serialPort->error() != QSerialPort::SerialPortError::TimeoutError)
        {
            _serialPort->close();
            _serialPort = nullptr;
            delete _serialPort;
            continue;
        }

        if(_serialPort->isOpen() == true)
        {
            if(_serialPort->waitForReadyRead(1000))
            {
                QByteArray data = _serialPort->readAll();
                if(data.length() > 0)
                {
                    emit readyRead(QDateTime::currentDateTimeUtc(), data);
                }
                msleep(100);
            }
            _sendBufferLock.lock();
            if(_sendBuffer.length() > 0)
            {
                KLog::sysLogText(KLOG_INFO, tr("Writing %1 bytes of data").arg(_sendBuffer.length()));
                _serialPort->write(_sendBuffer);
                _sendBuffer.clear();
            }
            _sendBufferLock.unlock();
        }
    }
}

void BlockingSerialThread::handleReadyWrite(QByteArray buffer)
{
    _sendBufferLock.lock();
    _sendBuffer = buffer;
    _sendBufferLock.unlock();
}

void BlockingSerialThread::initializeSerialPort()
{
    if(_serialPort != nullptr)
    {
        _serialPort->close();
        delete _serialPort;
        _serialPort = nullptr;
    }
    KLog::sysLogText(KLOG_INFO, tr("Opening %1").arg(_portName));
    _serialPort = new QSerialPort("Lidar Read");
    _serialPort->setPortName(_portName);
    _serialPort->setBaudRate(QSerialPort::Baud115200);

    KLog::sysLogText(KLOG_DEBUG, tr("Serial port read buffer is %1 %2").
                                            arg(_serialPort->readBufferSize())
                     );
    _serialPort->setReadBufferSize(65536);

    if(_serialPort->open(QIODevice::ReadWrite) == false)
    {
        QSerialPort::SerialPortError error = _serialPort->error();
        KLog::sysLogText(KLOG_ERROR, tr("Serial port open error %1 %2").
                                                arg(error).
                                                arg(_serialPort->errorString()));

        delete _serialPort;
        _serialPort = nullptr;

    }
    else
    {
        KLog::sysLogText(KLOG_INFO, tr("Serial port open successful"));
    }
}

void BlockingSerialThread::startMotor()
{
    _serialPort->setDataTerminalReady(false);
    Pigs::SetOutputPin(_parent->motorPin(), true);
}

void BlockingSerialThread::stopMotor()
{
    _serialPort->setDataTerminalReady(true);
    Pigs::SetOutputPin(_parent->motorPin(), false);
}

