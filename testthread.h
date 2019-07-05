#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QObject>
#include <QThread>

class TestThread : public QObject
{
    Q_OBJECT

public:
    explicit TestThread(QObject *parent = nullptr);

    void Start();
    void SendData();
    void Wait();

private:
    QThread _worker;

signals:
    void sendData();

public slots:
    void handleSendData();
};

#endif // TESTTHREAD_H
