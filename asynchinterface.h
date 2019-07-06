#ifndef ASYNCH_INTERFACE_H
#define ASYNCH_INTERFACE_H
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QTimer>
#include <QThread>
#include "serialinterface.h"

class AsynchInterface : public SerialInterface
{
    Q_OBJECT

public:
    AsynchInterface(const QString& portName);

    void send(QByteArray& data);

private:
    void deliverData();

    QTimer* _timer;
    int _bufferBytes;
    qint32 _bufferMsecs;
    quint64 _lastDeliveryMsecs;

    QByteArray _recvBuffer;
    QThread _thread;

private slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);
    void handleReadyWrite(QByteArray buffer);
    void handleThreadStart();

signals:
    void readyWrite(QByteArray buffer);
};

#endif // ASYNCH_INTERFACE_H
