#ifndef BLOCKINGSERIALTHREAD_H
#define BLOCKINGSERIALTHREAD_H
#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QString>

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
    bool _quit;

signals:
    void readyRead(QByteArray buffer);

public slots:
    void handleReadyWrite(QByteArray buffer);
};

#endif // BLOCKINGSERIALTHREAD_H
