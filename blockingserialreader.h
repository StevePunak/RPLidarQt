#ifndef BLOCKINGSERIALREADER_H
#define BLOCKINGSERIALREADER_H
#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>

#include "serialportreader.h"

class BlockingSerialReader : public SerialPortReader
{
    Q_OBJECT

public:
    BlockingSerialReader(const QString& portName);

    void send(QByteArray& data);

private:
    void deliverData();


    QByteArray _sendBuffer;
    QByteArray _recvBuffer;

};

#endif // BLOCKINGSERIALREADER_H
