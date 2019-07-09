#include "clientthread.h"
#include <QByteArray>
#include <QDebug>
#include "klog.h"
#include "lidarserver.h"

ClientThread::ClientThread(int socketDescriptor, LidarServer *parent) :
    _socketDescriptor(socketDescriptor),
    _parent((LidarServer*)parent)
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
    init();
}

ClientThread::~ClientThread()
{
}

void ClientThread::init()
{
    _socket = new QTcpSocket(this);
    if(!_socket->setSocketDescriptor(_socketDescriptor))
    {
       emit error(_socket->error());
       return;
    }

    KLog::sysLogText(KLOG_INFO, tr("Inbound connection from %1 port %2").
                     arg(_socket->peerAddress().toString()).
                     at(_socket->peerPort()));

    connect(_socket, &QTcpSocket::readyRead, this, &ClientThread::handleReadyRead, Qt::ConnectionType::QueuedConnection);
    connect(_socket, &QTcpSocket::disconnected, this, &ClientThread::handleSocketClosed, Qt::ConnectionType::QueuedConnection);
    _socket->moveToThread(this);
}

void ClientThread::handleReadyRead()
{
    QByteArray data = _socket->readAll();
    qDebug() << QString().sprintf("ready read %d bytes", data.length());
}

void ClientThread::handleSocketClosed()
{
    KLog::sysLogText(KLOG_DEBUG, "Cient socket closed in 0x%x", QThread::currentThreadId());
    quit();
}

void ClientThread::handleRangeMap(QByteArray data)
{
    KLog::sysLogText(KLOG_DEBUG, "Scan complete in client  in 0x%x", QThread::currentThreadId());
    _socket->write(data);
}

