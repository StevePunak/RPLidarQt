#include "clientthread.h"
#include <QByteArray>
#include <QDebug>
#include "klog.h"

ClientThread::ClientThread(int socketDescriptor, QObject *parent) :
    QObject(parent),
    _socketDescriptor(socketDescriptor)
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
    init();
}

ClientThread::~ClientThread()
{
    KLog::sysLogText(KLOG_DEBUG, "Delete client thread %p", this);
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
    connect(&_thread, &QThread::finished, this, &ClientThread::deleteLater);
    connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ClientThread::handleSocketError);
    moveToThread(&_thread);
    _socket.moveToThread(&_thread);

    _thread.start();
}

void ClientThread::disconnectSignals()
{
    disconnect(&_socket, &QTcpSocket::readyRead, this, &ClientThread::handleReadyRead);
    disconnect(&_socket, &QTcpSocket::disconnected, this, &ClientThread::handleSocketClosed);
    disconnect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ClientThread::handleSocketError);
    disconnect(&_thread, &QThread::finished, this, &ClientThread::deleteLater);
}

void ClientThread::handleReadyRead()
{
    QByteArray data = _socket.readAll();
    qDebug() << QString().sprintf("ready read %d bytes", data.length());
}

void ClientThread::handleSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "socket error " << socketError;
    disconnectSignals();
}

void ClientThread::handleSocketClosed()
{
    KLog::sysLogText(KLOG_DEBUG, "Cient socket closed");
    disconnectSignals();
    _thread.quit();
}

void ClientThread::handleRangeMap(QByteArray data)
{
    KLog::sysLogText(KLOG_DEBUG, "Scan complete in client");
    _socket.write(data);
}
