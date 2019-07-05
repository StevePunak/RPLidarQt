#ifndef FILEDATAREADER_H
#define FILEDATAREADER_H
#include <QObject>
#include <QString>
#include "datareader.h"

class FileDataReader : public DataReader
{
    Q_OBJECT

public:
    FileDataReader(const QString& fileName);

    void send(QByteArray& data);

private:
    QString _fileName;
};

#endif // FILEDATAREADER_H
