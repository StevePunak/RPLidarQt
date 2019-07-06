#include "serialinterface.h"
#include "klog.h"

SerialInterface::SerialInterface(const QString& portName, bool initPort) :
    DeviceInterface(),
    _portName(portName),
    _serialPort(nullptr),
    _byteTotal(0)
{
    if(initPort)
    {
        initializeSerialPort();
    }
}

void SerialInterface::initializeSerialPort()
{
    _serialPort = new QSerialPort("Lidar Read");
    _serialPort->setPortName(_portName);
    _serialPort->setBaudRate(QSerialPort::Baud115200);

    if(_serialPort->open(QIODevice::ReadWrite) == false)
    {
        QSerialPort::SerialPortError error = _serialPort->error();
        KLog::sysLogText(KLOG_ERROR, tr("Serial port open error %1")
                                                .arg(error));
    }

    _serialPort->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    stopMotor();

#ifdef DEBUG_SERIAL2
    _dumpOutputFile.setFileName("/home/pi/tmp/lidar.bin");
    if(!_dumpOutputFile.open(QIODevice::WriteOnly))
    {
        KLog::sysLogText(KLOG_ERROR, "Could not open file");
    }
    _dumpOutputFile.moveToThread(&_thread);
#endif
}

void SerialInterface::startMotor()
{
    _serialPort->setDataTerminalReady(false);
}

void SerialInterface::stopMotor()
{
    _serialPort->setDataTerminalReady(true);
}

