#include "blockingserialreader.h"
#include "klog.h"

BlockingSerialReader::BlockingSerialReader(const QString& portName) :
    SerialPortReader(portName)
{
}

void BlockingSerialReader::send(QByteArray &data)
{
    _serialPort->write(data);
}

void BlockingSerialReader::deliverData()
{
    emit dataReady(_recvBuffer);
    _recvBuffer.clear();
    _recvBuffer.reserve(65536);
}

