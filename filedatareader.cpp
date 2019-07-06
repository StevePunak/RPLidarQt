#include "fileinterface.h"

FileInterface::FileInterface(const QString &fileName) :
    _fileName(fileName)
{

}

void FileInterface::send(QByteArray &data)
{
    Q_UNUSED(data);
}
