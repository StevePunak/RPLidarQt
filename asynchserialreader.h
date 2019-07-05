#ifndef ASYNCHSERIALREADER_H
#define ASYNCHSERIALREADER_H
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QTimer>
#include "serialportreader.h"

class AsynchSerialReader : public SerialPortReader
{
    Q_OBJECT

public:
    AsynchSerialReader(const QString& portName);

    void send(QByteArray& data);

private:
    void deliverData();

    QTimer* _timer;
    int _bufferBytes;
    qint32 _bufferMsecs;
    quint64 _lastDeliveryMsecs;

    QByteArray _recvBuffer;

private slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);
    void readyWrite();
};

#endif // ASYNCHSERIALREADER_H
