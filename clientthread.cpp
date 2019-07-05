#include "clientthread.h"
#include <QByteArray>
#include <QDebug>
#include "klog.h"

ClientThread::ClientThread(int socketDescriptor, QObject *parent) :
    QObject(parent),
    _socketDescriptor(socketDescriptor)
{
    init();
}

void ClientThread::init()
{
    if(!_socket.setSocketDescriptor(_socketDescriptor))
    {
       emit error(_socket.error());
       return;
    }

    connect(&_socket, &QTcpSocket::readyRead, this, &ClientThread::handleReadyRead);
    connect(&_socket, &QTcpSocket::disconnected, this, &ClientThread::handleSocketClosed);
    connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ClientThread::handleSocketError);
    connect(&_thread, &QThread::finished, this, &ClientThread::deleteLater);

    moveToThread(&_thread);
    _socket.moveToThread(&_thread);

    _thread.start();
}

void ClientThread::handleReadyRead()
{
    QByteArray data = _socket.readAll();
    qDebug() << QString().sprintf("ready read %d bytes", data.length());
}

void ClientThread::handleSocketError()
{
    qDebug() << "socket error";
}

void ClientThread::handleSocketClosed()
{
    _thread.quit();
    deleteLater();
}

void ClientThread::scanReady(QByteArray data)
{
    KLog::sysLogText(KLOG_DEBUG, "Scan complete in client");
    _socket.write(data);
}
