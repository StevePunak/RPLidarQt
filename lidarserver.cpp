#include "lidarserver.h"
#include "klog.h"

LidarServer::LidarServer(Lidar* lidar, quint16 listenPort) :
    QTcpServer(),
    _lidar(lidar),
    _listenPort(listenPort)
{
    KLog::sysLogText(KLOG_INFO, tr("Listening for new connections on port %1").arg(_listenPort));
    listen(QHostAddress::Any, _listenPort);
}

void LidarServer::incomingConnection(qintptr socketDescriptor)
{
    ClientThread* thread = new ClientThread(int(socketDescriptor), this);
    connect(this, &LidarServer::rangeMapReady, thread, &ClientThread::handleRangeMap);
    connect(thread, &ClientThread::finished, thread, &ClientThread::deleteLater);
    thread->start();
}

void LidarServer::handleScanReady(QByteArray rangeData)
{
    emit rangeMapReady(rangeData);
}

