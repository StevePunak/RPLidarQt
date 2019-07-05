#include "serialportreader.h"
#include "klog.h"

SerialPortReader::SerialPortReader(const QString& portName) :
    DataReader(),
    _portName(portName)
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
#ifdef DEBUG_SERIAL2
    _dumpOutputFile.setFileName("/home/pi/tmp/lidar.bin");
    if(!_dumpOutputFile.open(QIODevice::WriteOnly))
    {
        KLog::sysLogText(KLOG_ERROR, "Could not open file");
    }
    _dumpOutputFile.moveToThread(&_thread);
#endif
}
