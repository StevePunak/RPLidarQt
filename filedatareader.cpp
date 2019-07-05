#include "filedatareader.h"

FileDataReader::FileDataReader(const QString &fileName) :
    _fileName(fileName)
{

}

void FileDataReader::send(QByteArray &data)
{
    Q_UNUSED(data);
}
