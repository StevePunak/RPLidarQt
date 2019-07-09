#ifndef BLOCKING_INTERFACE_H
#define BLOCKING_INTERFACE_H
#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include "blockingserialthread.h"
#include "serialinterface.h"

class BlockingInterface : public SerialInterface
{
    Q_OBJECT

public:
    BlockingInterface(const QString& portName, GPIO::Pin motorPin);

    void send(QByteArray& data);

    void startMotor();
    void stopMotor();

private:
    BlockingSerialThread* _portThread;
    QThread _thread;

signals:
    void readyWrite(QByteArray buffer);

private slots:
    void handleReadyRead(QByteArray buffer);
    void handlePortOpened();
    void handlePortClosed();
};

#endif // BLOCKING_INTERFACE_H
