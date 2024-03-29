#ifndef BLOCKINGSERIALTHREAD_H
#define BLOCKINGSERIALTHREAD_H
#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QString>
#include <QMutex>
#include <QDateTime>

class BlockingInterface;

class BlockingSerialThread : public QThread
{
    Q_OBJECT

public:
    explicit BlockingSerialThread(const QString& portName, BlockingInterface* parent);

    void run();
    void stop() { _quit = true; }

    void startMotor();
    void stopMotor();

private:
    void initializeSerialPort();

    QSerialPort* _serialPort;
    QString _portName;
    BlockingInterface* _parent;
    QByteArray _sendBuffer;
    QMutex _sendBufferLock;
    bool _quit;
    bool _wasOpen;

signals:
    void readyRead(QDateTime timestamp, QByteArray buffer);
    void portOpened();
    void portClosed();

public slots:
    void handleReadyWrite(QByteArray buffer);
};

#endif // BLOCKINGSERIALTHREAD_H
