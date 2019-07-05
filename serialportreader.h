#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H
#include <QSerialPort>
#include <QObject>
#include "datareader.h"

class SerialPortReader : public DataReader
{
    Q_OBJECT
public:
    explicit SerialPortReader(const QString& portName);

protected:
    QString _portName;
    QSerialPort* _serialPort;
    int _byteTotal;


signals:

public slots:
};

#endif // SERIALPORTREADER_H
