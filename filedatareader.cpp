#include "fileinterface.h"

FileInterface::FileInterface(const QString &fileName, GPIO::Pin motorPin) :
    DeviceInterface(motorPin),
    _fileName(fileName)
{

}

void FileInterface::send(QByteArray &data)
{
    Q_UNUSED(data);
}
