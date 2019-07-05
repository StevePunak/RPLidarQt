#include "testthread.h"
#include <QDebug>

TestThread::TestThread(QObject *parent)
    : QObject(parent)
{

}

void TestThread::Start()
{
    connect(this, &TestThread::sendData, this, &TestThread::handleSendData, Qt::ConnectionType::QueuedConnection);
    this->moveToThread(&_worker);
    _worker.start();
}

void TestThread::SendData()
{
    qDebug() << "Send data on " << QThread::currentThreadId();
    emit sendData();
}

void TestThread::Wait()
{
    _worker.wait();
}

void TestThread::handleSendData()
{
    qDebug() << "handle send data on " << QThread::currentThreadId();
}
