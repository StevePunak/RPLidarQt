#include "blockingserialthread.h"
#include "blockinginterface.h"
#include "klog.h"

BlockingSerialThread::BlockingSerialThread(const QString& portName, BlockingInterface* parent) :
    QThread(),
    _portName(portName),
    _parent(parent),
    _quit(false)
{
}

void BlockingSerialThread::run()
{
    initializeSerialPort();
    while(true)
    {
        KLog::sysLogText(KLOG_DEBUG, "wait");
        if(_serialPort->waitForReadyRead(1000))
        {
            QByteArray data = _serialPort->readAll();
            KLog::sysLogText(KLOG_DEBUG, tr("read %1").arg(data.length()));
            if(data.length() > 0)
            {
                emit readyRead(data);
            }
            msleep(500);
        }
    }
}

void BlockingSerialThread::handleReadyWrite(QByteArray buffer)
{
    KLog::sysLogText(KLOG_INFO, "Write data");
    KLog::sysLogHex(buffer);
    _serialPort->write(buffer);
}

void BlockingSerialThread::initializeSerialPort()
{
    KLog::sysLogText(KLOG_INFO, tr("Opening %1").arg(_portName));
    _serialPort = new QSerialPort("Lidar Read");
    _serialPort->setPortName(_portName);
    _serialPort->setBaudRate(QSerialPort::Baud115200);

    if(_serialPort->open(QIODevice::ReadWrite) == false)
    {
        QSerialPort::SerialPortError error = _serialPort->error();
        KLog::sysLogText(KLOG_ERROR, tr("Serial port open error %1")
                                                .arg(error));
    }
}

void BlockingSerialThread::startMotor()
{
    _serialPort->setDataTerminalReady(false);
}

void BlockingSerialThread::stopMotor()
{
    _serialPort->setDataTerminalReady(true);
}

