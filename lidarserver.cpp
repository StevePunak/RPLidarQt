#include "lidarserver.h"
#include "klog.h"

LidarServer::LidarServer(quint16 listenPort) :
    QTcpServer(),
    _listenPort(listenPort),
    _clientCount(0)
{
    KLog::sysLogText(KLOG_INFO, tr("Listening for new connections on port %1").arg(_listenPort));
    listen(QHostAddress::Any, _listenPort);
}

void LidarServer::incomingConnection(qintptr socketDescriptor)
{
    ClientThread* thread = new ClientThread(int(socketDescriptor), this);
    connect(this, &LidarServer::rangeMapReady, thread, &ClientThread::handleRangeMap);
    connect(thread, &ClientThread::started, this, &LidarServer::handleClientStarted);
    connect(thread, &ClientThread::finished, thread, &ClientThread::deleteLater);
    connect(thread, &ClientThread::finished, this, &LidarServer::handleClientFinished);
    thread->start();
}

void LidarServer::handleScanReady(QByteArray rangeData)
{
    emit rangeMapReady(rangeData);
}

void LidarServer::handleClientStarted()
{
    _clientCount++;
}

void LidarServer::handleClientFinished()
{
    _clientCount--;
}

